// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "editview.h"
#include "aamenu.h"

#include "controlid.h"
#include "aamanager.h"

using namespace SKELETON;


EditTextView::EditTextView() :
    Gtk::TextView(),
    m_cancel_change( false ),
    m_line_offset( -1 ),
    m_popupmenu( NULL )
{
    // コントロールモード設定
    m_control.add_mode( CONTROL::MODE_EDIT );
    m_control.add_mode( CONTROL::MODE_MESSAGE );

    get_buffer()->signal_changed().connect( sigc::mem_fun( *this, &EditTextView::slot_buffer_changed ) );
}


EditTextView::~EditTextView()
{
    if( m_popupmenu ) delete m_popupmenu;
    m_popupmenu = NULL;
}


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
    if( get_buffer()->get_has_selection() ){
        get_buffer()->erase_selection();
        return;
    }

    Gtk::TextIter it = get_buffer()->get_insert()->get_iter();
    Gtk::TextIter it2 = it;
    if( ! it2.forward_char() ) it2.forward_to_end();

    m_delete_pushed = true;
    get_buffer()->erase( it, it2 );
}


void EditTextView::backsp_char()
{
    // 範囲選択消去
    if( get_buffer()->get_has_selection() ){
        get_buffer()->erase_selection();
        return;
    }

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

        UNDO_DATA udata;
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

    UNDO_DATA udata = m_undo_tree[ m_undo_pos-- ];
    
#ifdef _DEBUG
    std::cout << "EditTextView::undo pos = " << m_undo_pos << " size = " << m_undo_tree.size() << std::endl;
    std::cout << "offset = " << udata.pos << " cursor = " << udata.pos_cursor;
    if( udata.append ) std::cout << " - ";
    else std::cout << " + ";
    std::cout << udata.str_diff << std::endl;
#endif

    Glib::ustring text;
    Glib::ustring text_head = get_buffer()->get_text().substr( 0, udata.pos );

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
    bool cancel_event = false;
    m_delete_pushed = false;
    if( event->keyval == GDK_Delete ) m_delete_pushed = true;

    switch( m_control.key_press( event ) ){

        // MessageViewでショートカットで書き込むと文字が挿入されてしまうので
        // キャンセルする
        case CONTROL::ExecWrite:
        case CONTROL::CancelWrite:
        case CONTROL::FocusWrite:
        case CONTROL::TabLeft:
        case CONTROL::TabRight:
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

        case CONTROL::InputAA: show_aalist_popup(); return true;
    }

    m_sig_key_press.emit( event );
    if( cancel_event ) return true;

    return Gtk::TextView::on_key_press_event( event );
}


bool EditTextView::on_key_release_event( GdkEventKey* event )
{
    bool cancel_event = false;

    switch( m_control.key_press( event ) ){

        // MessageViewでショートカットで書き込むと文字が挿入されてしまうので
        // キャンセルする
        case CONTROL::ExecWrite:
        case CONTROL::CancelWrite:
        case CONTROL::FocusWrite:
        case CONTROL::TabLeft:
        case CONTROL::TabRight:
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
        if( m_popupmenu ) delete m_popupmenu;
        m_popupmenu = Gtk::manage( new AAMenu( *dynamic_cast< Gtk::Window* >( get_toplevel() ) ) );
        m_popupmenu->popup( Gtk::Menu::SlotPositionCalc(
                            sigc::mem_fun( *this, &EditTextView::slot_popup_aamenu_pos ) ),
                            0, gtk_get_current_event_time() );

        m_popupmenu->sig_selected().connect( sigc::mem_fun( *this, &EditTextView::slot_aamenu_selected ) );
    }
}


//
// AA ポップアップの表示位置を決定
//
void EditTextView::slot_popup_aamenu_pos( int& x, int& y, bool& push_in )
{
    Gdk::Rectangle rect = get_cursor_root_origin();

    x = rect.get_x();
    y = rect.get_y() + rect.get_height();
    push_in = false;
}


//
// AAポップアップで選択された
//
void EditTextView::slot_aamenu_selected( const std::string& aa )
{
    insert_str( aa, false );
}
