// ライセンス: GPL2

//#define _DEBUG
#include "gtkmmversion.h"
#include "jddebug.h"

#include "editview.h"
#include "aamenu.h"

#include "control/controlid.h"
#include "control/controlutil.h"

#include "aamanager.h"
#include "environment.h"
#include "session.h"

#include "jdlib/misccharcode.h"
#include "jdlib/miscutil.h"
#include "config/globalconf.h"


using namespace SKELETON;

enum
{
    MIN_AAMENU_LINES = 12
};

// extend-selectionのハンドラーはC APIを利用するためヘッダーには公開しない
static gboolean EditTextView_slot_extend_selection( GtkTextView*,
                                                    GtkTextExtendSelection granularity,
                                                    GtkTextIter* location,
                                                    GtkTextIter* start,
                                                    GtkTextIter* end,
                                                    gpointer );


// 区切り文字の判定
// FIXME: src/article/drawareabase.cpp にある同名の関数と統合する
static inline bool is_separate_char( char32_t c )
{
    return g_unichar_isspace( c ) || g_unichar_ispunct( c );
}


EditTextView::EditTextView()
    : Gtk::TextView()
    , m_pre_offset{ -1 }
    , m_pre_line{ -1 }
    , m_line_offset{ -1 }
{
    // コントロールモード設定
    m_control.add_mode( CONTROL::MODE_EDIT );
    m_control.add_mode( CONTROL::MODE_MESSAGE );

    get_buffer()->signal_changed().connect( sigc::mem_fun( *this, &EditTextView::slot_buffer_changed ) );

    // NOTE: gtkmmでextend-selectionシグナルが公開されていなかった
    g_signal_connect( G_OBJECT( gobj() ), "extend-selection",
                      G_CALLBACK( &EditTextView_slot_extend_selection ), nullptr );
}


// メンバーに不完全型のスマートポインターがあるためデストラクタはinlineにできない
EditTextView::~EditTextView() noexcept = default;


//
// カーソルの位置に挿入
//
// use_br == true なら改行を入れる
//
void EditTextView::insert_str( const std::string& str, bool use_br )
{
    std::string br;

    if( use_br ){
        Gtk::TextIter it = get_buffer()->get_insert()->get_iter();
        if( it.get_chars_in_line() > 1 ) br = "\n\n";
        else if( it.backward_char() && it.get_chars_in_line() > 1 ) br = "\n";
    }

    get_buffer()->insert_at_cursor( br + str );
    scroll_to( get_buffer()->get_insert(), 0.0 );
}


void EditTextView::cursor_up_down( bool up )
{
    Gtk::TextIter it = get_buffer()->get_insert()->get_iter();
    int line = it.get_line();
    int offset = it.get_line_offset();
    it.set_line_offset( 0 );

    // 上
    if( up ){

        if( ! it.backward_line() ) return;

    }
    // 下
    else{

        if( ! it.forward_line() ){

            // 最終行に文字が無い場合
            if( get_buffer()->get_line_count() -1 == it.get_line() ) it.forward_to_end();

            else return;
        }
    }

    if( m_line_offset >= 0 && offset == m_pre_offset && line == m_pre_line ) offset = m_line_offset;

    if( offset < it.get_chars_in_line() ){
        it.set_line_offset( offset );
        m_line_offset = -1;
    }

    // 文字数オーバー
    else{
        m_line_offset = offset;
        if( it.get_chars_in_line() > 1 && ! it.forward_to_line_end() ) it.forward_to_end();
    }

    get_buffer()->place_cursor( it );
    scroll_to( get_buffer()->get_insert(), 0.0 );

    m_pre_line = get_buffer()->get_insert()->get_iter().get_line();
    m_pre_offset = get_buffer()->get_insert()->get_iter().get_line_offset();
}

void EditTextView::cursor_up()
{
    cursor_up_down( true );
}


void EditTextView::cursor_down()
{
    cursor_up_down( false );
}


void EditTextView::cursor_left()
{
    Gtk::TextIter it = get_buffer()->get_insert()->get_iter();

    if( it.backward_char() ) get_buffer()->place_cursor( it );
}


void EditTextView::cursor_right()
{
    Gtk::TextIter it = get_buffer()->get_insert()->get_iter();

    if( ! it.forward_char() ) it.forward_to_end();
    get_buffer()->place_cursor( it );
}


void EditTextView::cursor_home()
{
    Gtk::TextIter it = get_buffer()->get_insert()->get_iter();

    it.set_line_offset( 0 );
    get_buffer()->place_cursor( it );
}


void EditTextView::cursor_end()
{
    Gtk::TextIter it = get_buffer()->get_insert()->get_iter();

    if( ! it.forward_to_line_end() ) it.forward_to_end();
    get_buffer()->place_cursor( it );
}



void EditTextView::delete_char()
{
    // 範囲選択消去
    if( get_buffer()->erase_selection() ) return;

    Gtk::TextIter it = get_buffer()->get_insert()->get_iter();
    Gtk::TextIter it2 = it;
    if( ! it2.forward_char() ) it2.forward_to_end();

    m_delete_pushed = true;
    get_buffer()->erase( it, it2 );
}


void EditTextView::backsp_char()
{
    // 範囲選択消去
    if( get_buffer()->erase_selection() ) return;

    Gtk::TextIter it = get_buffer()->get_insert()->get_iter();
    Gtk::TextIter it2 = it;
    if( it2.backward_char() ) get_buffer()->erase( it2, it );
}


//
// バッファの文字列が変化したときに UNDO バッファを変更する
//
void EditTextView::slot_buffer_changed()
{
    if( m_cancel_change ) return;

#ifdef _DEBUG
    std::cout << "EditTextView::slot_buffer_changed\n";
#endif

    Glib::ustring text = get_buffer()->get_text();
    unsigned int lng = text.length();
    unsigned int pre_lng = m_pre_text.length();
    unsigned int size = lng > pre_lng ? lng - pre_lng : pre_lng - lng;

    if( size ){

        UndoDatum udata;
        udata.pos = 0;
        udata.append = false;

        // diff を取る
        for( ; udata.pos < MIN( lng, pre_lng ) && text.at( udata.pos ) == m_pre_text.at( udata.pos ); ++udata.pos );

        // 追加
        if( lng > pre_lng ){
            udata.append = true;
            udata.str_diff = text.substr( udata.pos, size );
        }

        // 削除
        else udata.str_diff = m_pre_text.substr( udata.pos, size );

        // カーソルの位置を取得
        udata.pos_cursor = get_buffer()->get_insert()->get_iter().get_offset();
        if( udata.append ) udata.pos_cursor -= size;
        else {

            if( size > 1  // 範囲選択で消した場合
                || ! m_delete_pushed // backspaceで消した場合
                ) udata.pos_cursor += size;
        }

#ifdef _DEBUG
        std::cout << "size = " << size << " offset = " << udata.pos << " cursor = " << udata.pos_cursor;
        if( udata.append ) std::cout << " + "; else std::cout << " - ";
        std::cout << "pre = " << m_pre_text << " text = " << text << " diff = " << udata.str_diff << std::endl;
#endif

        m_undo_tree.push_back( udata );
        m_undo_pos = m_undo_tree.size() -1;
    }

    m_pre_text = text;
}


//
// undo
//
void EditTextView::undo()
{
    if( ! m_undo_tree.size() ) return;

    if( m_undo_pos < 0 ){ // 根元まで来たらツリーの先頭に戻る
        m_undo_pos = m_undo_tree.size() -1;
        return;
    }

    UndoDatum udata = m_undo_tree[ m_undo_pos-- ];

#ifdef _DEBUG
    std::cout << "EditTextView::undo pos = " << m_undo_pos << " size = " << m_undo_tree.size() << std::endl;
    std::cout << "offset = " << udata.pos << " cursor = " << udata.pos_cursor;
    if( udata.append ) std::cout << " - ";
    else std::cout << " + ";
    std::cout << udata.str_diff << std::endl;
#endif

    m_cancel_change = true; // slot_buffer_changed() の呼出をキャンセル

    // 追加と削除を逆転
    udata.append = ! udata.append;

    // 追加
    if( udata.append ) get_buffer()->insert( get_buffer()->get_iter_at_offset( udata.pos ), udata.str_diff );

    // 削除
    else get_buffer()->erase( get_buffer()->get_iter_at_offset( udata.pos ), get_buffer()->get_iter_at_offset( udata.pos + udata.str_diff.length() ) );

    // カーソル移動
    Gtk::TextIter it = get_buffer()->get_iter_at_offset( udata.pos_cursor );
    get_buffer()->place_cursor( it );

    m_cancel_change = false;

    m_pre_text = get_buffer()->get_text();

    // 逆方向にツリーを延ばす
    if( udata.append ) udata.pos_cursor += udata.str_diff.length();
    m_undo_tree.push_back( udata );
}


void EditTextView::clear_undo()
{
    m_undo_tree.clear();
}


//
// マウスボタン入力のフック
//
bool EditTextView::on_button_press_event( GdkEventButton* event )
{
    m_sig_button_press.emit( event );

    return Gtk::TextView::on_button_press_event( event );
}



//
// キー入力のフック
//
bool EditTextView::on_key_press_event( GdkEventKey* event )
{
#ifdef _DEBUG    
    std::cout << "EditTextView::on_key_press_event key = " << event->keyval << std::endl;
#endif

    bool cancel_event = false;
    m_delete_pushed = false;
    if( event->keyval == GDK_KEY_Delete ) m_delete_pushed = true;

    const int controlid = m_control.key_press( event );

    switch( controlid ){

        case CONTROL::ExecWrite:
        case CONTROL::CancelWrite:
        case CONTROL::FocusWrite:
        case CONTROL::TabLeft:
        case CONTROL::TabRight:
        case CONTROL::TabLeftUpdated:
        case CONTROL::TabRightUpdated:
        case CONTROL::ToggleSage:
        case CONTROL::HomeEdit:
        case CONTROL::EndEdit:
        case CONTROL::UpEdit:
        case CONTROL::DownEdit:
        case CONTROL::RightEdit:
        case CONTROL::LeftEdit:
        case CONTROL::DeleteEdit:
        case CONTROL::BackspEdit:
        case CONTROL::UndoEdit:
        case CONTROL::InputAA:
        {
            if( im_context_filter_keypress( event ) ) {
#ifdef _DEBUG
                std::cout << "gtk_im_context_filter_keypress\n";
#endif
                reset_im_context();
                return true;
            }
        }
    }

    switch( controlid ){

        // MessageViewでショートカットで書き込むと文字が挿入されてしまうので
        // キャンセルする
        case CONTROL::ExecWrite:
        case CONTROL::CancelWrite:
        case CONTROL::FocusWrite:
        case CONTROL::TabLeft:
        case CONTROL::TabRight:
        case CONTROL::ToggleSage:
            cancel_event = true;
            break;

        case CONTROL::HomeEdit: cursor_home(); return true; 
        case CONTROL::EndEdit: cursor_end(); return true;

        case CONTROL::UpEdit: cursor_up(); return true;
        case CONTROL::DownEdit: cursor_down(); return true;
        case CONTROL::RightEdit: cursor_right(); return true;
        case CONTROL::LeftEdit: cursor_left(); return true;

        case CONTROL::DeleteEdit: delete_char(); return true;
        case CONTROL::BackspEdit: backsp_char(); return true;
        case CONTROL::UndoEdit: undo(); return true;

        case CONTROL::EnterEdit:
            event->keyval = GDK_KEY_Return;
            event->state &= ~GDK_CONTROL_MASK;
            event->state &= ~GDK_SHIFT_MASK;
            event->state &= ~GDK_MOD1_MASK;
            break;

        case CONTROL::InputAA: show_aalist_popup(); return true;
    }

    m_sig_key_press.emit( event );
    if( cancel_event ) return true;

    return Gtk::TextView::on_key_press_event( event );
}


bool EditTextView::on_key_release_event( GdkEventKey* event )
{
#ifdef _DEBUG    
    std::cout << "EditTextView::on_key_release_event key = " << event->keyval << std::endl;
#endif

    bool cancel_event = false;

    switch( m_control.key_press( event ) ){

        // MessageViewでショートカットで書き込むと文字が挿入されてしまうので
        // キャンセルする
        case CONTROL::ExecWrite:
        case CONTROL::CancelWrite:
        case CONTROL::FocusWrite:
        case CONTROL::TabLeft:
        case CONTROL::TabRight:
        case CONTROL::ToggleSage:
            cancel_event = true;
            break;

        case CONTROL::HomeEdit:
        case CONTROL::EndEdit:

        case CONTROL::UpEdit:
        case CONTROL::DownEdit:
        case CONTROL::RightEdit:
        case CONTROL::LeftEdit:

        case CONTROL::DeleteEdit:
        case CONTROL::BackspEdit:
        case CONTROL::UndoEdit:

        case CONTROL::InputAA:
            return true;
    }

    m_sig_key_release.emit( event );
    if( cancel_event ) return true;

    return Gtk::TextView::on_key_release_event( event );
}


//
// コンテキストメニュー表示
//
void EditTextView::on_populate_popup( Gtk::Menu* menu )
{
#ifdef _DEBUG
    std::cout << "EditTextView::on_populate_popup\n";
#endif

    m_context_menu = menu;
    menu->signal_map().connect( sigc::mem_fun( *this, &EditTextView::slot_map_popupmenu ) ); 
    menu->signal_hide().connect( sigc::mem_fun( *this, &EditTextView::slot_hide_popupmenu ) );

    // セパレータ
    Gtk::MenuItem* menuitem = Gtk::manage( new Gtk::SeparatorMenuItem() );
    menu->prepend( *menuitem );

    // JDimの動作環境を記入
    menuitem = Gtk::manage( new Gtk::MenuItem( "JDimの動作環境を記入" ) );
    menuitem->signal_activate().connect( sigc::mem_fun( *this, &EditTextView::slot_write_jdinfo ) );
    menu->prepend( *menuitem );

    // 変換(スペース⇔&#160;)
    menuitem = Gtk::manage( new Gtk::MenuItem( "変換(スペース⇔&#160;)" ) );
    menuitem->signal_activate().connect( sigc::mem_fun( *this, &EditTextView::slot_convert_space ) );
    menu->prepend( *menuitem );

    // クリップボードから引用
    menuitem = Gtk::manage( new Gtk::MenuItem( "クリップボードから引用" ) );
    menuitem->signal_activate().connect( sigc::mem_fun( *this, &EditTextView::slot_quote_clipboard ) );

    Glib::RefPtr< Gtk::Clipboard > clip = Gtk::Clipboard::get();
    if( clip->wait_is_text_available() ) menuitem->set_sensitive( true );
    else menuitem->set_sensitive( false );
    menu->prepend( *menuitem );

    // AA入力メニュー追加
    if( CORE::get_aamanager()->get_size() ){

        menuitem = Gtk::manage( new Gtk::MenuItem( CONTROL::get_label_motions( CONTROL::InputAA ) ) );
        menuitem->signal_activate().connect( sigc::mem_fun( *this, &EditTextView::slot_select_aamenu ) );
        menu->prepend( *menuitem );
    }

    menu->show_all_children();

    Gtk::TextView::on_populate_popup( menu );
}


//
// AA追加メニュー
//
void EditTextView::slot_select_aamenu()
{
    if( m_context_menu ) m_context_menu->hide();
    show_aalist_popup();
}


//
// クリップボードから引用して貼り付け
//
void EditTextView::slot_quote_clipboard()
{
    if( m_context_menu ) m_context_menu->hide();

    Glib::RefPtr< Gtk::Clipboard > clip = Gtk::Clipboard::get();
    std::string text = clip->wait_for_text();

    std::string str_res = CONFIG::get_ref_prefix();

    text = MISC::replace_str( text, "\n", "\n" + str_res );
    insert_str( str_res + text, false );
}


//
// 変換(スペース⇔&#160;)
//
void EditTextView::slot_convert_space()
{
    if( m_context_menu ) m_context_menu->hide();

    Glib::RefPtr< Gtk::TextBuffer > buffer = get_buffer();
    std::string text = buffer->get_text();
    std::string converted;

    // &#160;が含まれていたらスペースに変換する
    if( text.find( "&#160;" ) != std::string::npos )
    {
        converted = MISC::replace_str( text, "&#160;", " " );
    }
    else
    {
        std::list<std::string> lines = MISC::get_lines( text );
        for( std::string& line : lines ) {

            if( ! line.empty() ) {

                // 行末のスペースを取り除く
                line.erase( line.find_last_not_of( ' ' ) + 1 );

                // 行頭のスペースを&#160;に変換する
                if( line.front() == ' ' ) {
                    line.replace( 0, 1, "&#160;" );
                }

                // 連続スペースを&#160;に変換する
                converted.append( MISC::replace_str( line, "  ", " &#160;" ) );
            }

            converted.push_back( '\n' );
        }

        // 最後の改行を取り除く
        converted.pop_back();
    }

    buffer->set_text( converted );
}


//
// JDの動作環境を記入
//
void EditTextView::slot_write_jdinfo()
{
    if( m_context_menu ) m_context_menu->hide();

    std::string jdinfo = ENVIRONMENT::get_jdinfo();

    insert_str( jdinfo, false );
}


//
// ポップアップメニューがmapしたときに呼ばれるslot
//
void EditTextView::slot_map_popupmenu()
{
#ifdef _DEBUG
    std::cout << "EditTextView::slot_map_popupmenu\n";
#endif

    SESSION::set_popupmenu_shown( true );
}


//
// コンテキストメニューが閉じた
//
void EditTextView::slot_hide_popupmenu()
{
#ifdef _DEBUG
    std::cout << "EditTextView::slot_hide_popupmenu\n";
#endif

    m_context_menu = nullptr;
    SESSION::set_popupmenu_shown( false );
}


//
// カーソルの画面上の座標
//
Gdk::Rectangle EditTextView::get_cursor_root_origin()
{
    Gdk::Rectangle rect;
    int wx, wy;
    int x, y;

    get_iter_location( get_buffer()->get_insert()->get_iter(), rect );
    buffer_to_window_coords( Gtk::TEXT_WINDOW_TEXT,  rect.get_x(), rect.get_y(), wx, wy );
    get_window( Gtk::TEXT_WINDOW_TEXT )->get_origin( x, y );

    rect.set_x( x + wx );
    rect.set_y( y + wy );

    return rect;
}


//
// AA ポップアップメニュー表示
//
void EditTextView::show_aalist_popup()
{
    if( CORE::get_aamanager()->get_size() )
    {
        m_aapopupmenu = std::make_unique<AAMenu>( *dynamic_cast<Gtk::Window*>( get_toplevel() ) );

        m_aapopupmenu->sig_selected().connect( sigc::mem_fun( *this, &EditTextView::slot_aamenu_selected ) );
        m_aapopupmenu->signal_map().connect( sigc::mem_fun( *this, &EditTextView::slot_map_aamenu ) ); 
        m_aapopupmenu->signal_hide().connect( sigc::mem_fun( *this, &EditTextView::slot_hide_aamenu ) );

        m_aapopupmenu->popup( Gtk::Menu::SlotPositionCalc(
                            sigc::mem_fun( *this, &EditTextView::slot_popup_aamenu_pos ) ),
                            0, gtk_get_current_event_time() );
    }
}


//
// AA ポップアップの表示位置を決定
//
void EditTextView::slot_popup_aamenu_pos( int& x, int& y, bool& push_in )
{
    push_in = false;

    const Gdk::Rectangle rect = get_cursor_root_origin();
    const int line_height = rect.get_height();
    const int sh = get_screen()->get_height();
    const int min_height = MIN( CORE::get_aamanager()->get_size(), MIN_AAMENU_LINES ) * line_height;

    x = rect.get_x();
    y = rect.get_y() + line_height;

    // 最低でも MIN_AAMENU_LINES 行よりも表示領域が高くなるようにする
    if( y + min_height > sh ) y = sh - min_height;
}


//
// AAポップアップで選択された
//
void EditTextView::slot_aamenu_selected( const std::string& aa )
{
    insert_str( aa, false );
}


//
// AAポップアップメニューがmapしたときに呼ばれるslot
//
void EditTextView::slot_map_aamenu()
{
#ifdef _DEBUG
    std::cout << "EditTextView::slot_map_aamenu\n";
#endif

    SESSION::set_popupmenu_shown( true );
}


//
// AAポップアップが閉じた
void EditTextView::slot_hide_aamenu()
{
#ifdef _DEBUG
    std::cout << "EditTextView::slot_hide_aamenu\n";
#endif

    SESSION::set_popupmenu_shown( false );
}


//
// 範囲選択を実行する
//
static gboolean EditTextView_slot_extend_selection( GtkTextView*,
                                                    GtkTextExtendSelection granularity,
                                                    GtkTextIter* location,
                                                    GtkTextIter* start,
                                                    GtkTextIter* end,
                                                    gpointer )
{
    const char32_t loc_char = gtk_text_iter_get_char( location );
    if( loc_char == 0 ) return GDK_EVENT_PROPAGATE;

    gtk_text_iter_assign( start, location );
    gtk_text_iter_assign( end, location );
    Gtk::TextIter& start_iter = Glib::wrap( start );
    Gtk::TextIter& end_iter = Glib::wrap( end );

    if( granularity == GTK_TEXT_EXTEND_SELECTION_WORD ) {
        const auto block = MISC::get_unicodeblock( loc_char );
        const bool sep = is_separate_char( loc_char );
        const auto find_char = [block, sep]( char32_t c ) {
            return block != MISC::get_unicodeblock( c ) || sep != is_separate_char( c );
        };
        if( start_iter.backward_find_char( find_char ) ) {
            start_iter.forward_char();
        }
        end_iter.forward_find_char( find_char );
        return GDK_EVENT_STOP;
    }
    else if( granularity == GTK_TEXT_EXTEND_SELECTION_LINE ) {
        while( start_iter.backward_char() ) {
            if( start_iter.ends_line() ) {
                start_iter.forward_char();
                break;
            }
        }
        if( !end_iter.ends_line() ) {
            end_iter.forward_to_line_end();
        }
        return GDK_EVENT_STOP;
    }
    return GDK_EVENT_PROPAGATE;
}


//////////////////////////////////////////////

EditView::EditView()
    : Gtk::ScrolledWindow()
{
    set_policy( Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC );
    add( m_textview );
    auto context = m_textview.get_style_context();
    context->add_class( s_css_classname );
    context->add_provider( m_provider, GTK_STYLE_PROVIDER_PRIORITY_APPLICATION );
    show_all_children();
}


EditView::~EditView() noexcept = default;


constexpr const char* EditView::s_css_classname;

//
// EditTextViewのスタイルを更新する
//
void EditView::update_style( const Glib::ustring& custom_css )
{
#ifdef _DEBUG
        std::cout << "EditView::update_style custom css: " << custom_css << std::endl;
#endif
    try {
        m_provider->load_from_data( custom_css );
    }
    catch( Gtk::CssProviderError& err ) {
#ifdef _DEBUG
        std::cout << "ERROR:EditView::update_style fail " << err.what() << std::endl;
#endif
    }
}
