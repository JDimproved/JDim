// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"
#include "gtkmmversion.h"

#include "dragtreeview.h"
#include "view.h"
#include "popupwin.h"

#include "config/globalconf.h"

#include "jdlib/miscutil.h"

#include "control/controlid.h"

#include "command.h"
#include "colorid.h"
#include "dndmanager.h"
#include "session.h"

#ifndef MAX
#define MAX( a, b ) ( a > b ? a : b )
#endif


#ifndef MIN
#define MIN( a, b ) ( a < b ? a : b )
#endif


using namespace SKELETON;


#if GTKMM_CHECK_VERSION(3,0,0)
constexpr const char* DragTreeView::s_css_classname;
#endif

DragTreeView::DragTreeView( const std::string& url, const std::string& dndtarget,
    const bool use_usr_fontcolor, const std::string& fontname, const int colorid_text, const int colorid_bg, const int colorid_bg_even )
    : JDTreeViewBase(),
      m_url( url ),
      m_dndtarget( dndtarget ),
      m_dragging( false ),
      m_use_bg_even( false ),
      m_popup_win( NULL ),
      m_popup_shown( false )
{
#ifdef _DEBUG
    std::cout << "DragTreeView::DragTreeView\n";
#endif

    set_enable_search( false );
    set_rules_hint( CONFIG::get_use_tree_gtkrc() );
    add_events( Gdk::LEAVE_NOTIFY_MASK );

#if GTKMM_CHECK_VERSION(3,0,0)
    auto context = get_style_context();
    context->add_class( s_css_classname );
    context->add_provider( m_provider, GTK_STYLE_PROVIDER_PRIORITY_APPLICATION );
#endif

    if( use_usr_fontcolor ){
        init_color( colorid_text, colorid_bg, colorid_bg_even );
        init_font( fontname );
    }

    get_selection()->set_mode( Gtk::SELECTION_MULTIPLE );
    get_selection()->signal_changed().connect( sigc::mem_fun( *this, &DragTreeView::slot_selection_changed ) );

    // D&D 設定
    std::vector< Gtk::TargetEntry > targets;
    targets.push_back( Gtk::TargetEntry( get_dndtarget(), Gtk::TARGET_SAME_APP, 0 ) );

    // ドラッグ開始ボタン設定
    Gdk::ModifierType type = Gdk::BUTTON1_MASK;  
    GdkEventButton event;
    m_control.get_eventbutton( CONTROL::DragStartButton, event );
    switch( event.button ){
        case 1: type = Gdk::BUTTON1_MASK; break;
        case 2: type = Gdk::BUTTON2_MASK; break;
        case 3: type = Gdk::BUTTON3_MASK; break;
        case 4: type = Gdk::BUTTON4_MASK; break;
        case 5: type = Gdk::BUTTON5_MASK; break;
    }

    drag_source_set( targets, type );
}


DragTreeView::~DragTreeView()
{
    delete_popup();
}


//
// 他のアプリからの text/url-list のドロップを有効にする
// ドロップされるとSIG_DROPPED_URI_LIST を発行する
//
void DragTreeView::set_enable_drop_uri_list()
{
    std::vector< Gtk::TargetEntry > targets_drop;
    targets_drop.push_back( Gtk::TargetEntry( "text/uri-list" ) );

    // ドロップされると on_drag_data_received() が呼び出される
    drag_dest_set( targets_drop, Gtk::DEST_DEFAULT_ALL, Gdk::ACTION_COPY );
}


//
// 再描画
//
void DragTreeView::redraw_view()
{
    // 今のところ特に何もしない

    // ポップアップが表示されていたらポップアップを再描画
    if( is_popup_shown() && m_popup_win->view() ) m_popup_win->view()->redraw_view();
}


//
// 色初期化
//
void DragTreeView::init_color( const int colorid_text, const int colorid_bg, const int colorid_bg_even )
{
    if( CONFIG::get_use_tree_gtkrc() ) {
#if GTKMM_CHECK_VERSION(3,0,0)
        m_use_bg_even = false;
        m_provider->load_from_data( u8"" );
#endif
        return;
    }

    // 文字色
    m_color_text.set( CONFIG::get_color( colorid_text ) );
    // 背景色
    m_color_bg.set( CONFIG::get_color( colorid_bg ) );

    m_use_bg_even = ! ( CONFIG::get_color( colorid_bg ) == CONFIG::get_color( colorid_bg_even ) );
    m_color_bg_even.set( CONFIG::get_color( colorid_bg_even ) );

#if GTKMM_CHECK_VERSION(3,0,0)
    // TreeViewの偶数/奇数行を指定するCSSセレクタは動作しないバージョンが
    // 存在するため偶数行の背景色設定は既存のコードをそのまま使う
    // https://gitlab.gnome.org/GNOME/gtk/issues/581
    try {
        m_provider->load_from_data( Glib::ustring::compose(
            u8".%1.view:not(:selected) { color: %2; background-color: %3; }",
            s_css_classname, m_color_text.to_string(), m_color_bg.to_string() ) );
    }
    catch( Gtk::CssProviderError& err ) {
#ifdef _DEBUG
        std::cout << "ERROR:DragTreeView::init_color load from data failed: "
                  << err.what() << std::endl;
#endif
    }
#else // !GTKMM_CHECK_VERSION(3,0,0)
    modify_text( get_state(), m_color_text );
    modify_base( get_state(), m_color_bg );
#endif
}


//
// フォント初期化
//
void DragTreeView::init_font( const std::string& fontname )
{
    Pango::FontDescription pfd( fontname );
    pfd.set_weight( Pango::WEIGHT_NORMAL );
#if GTKMM_CHECK_VERSION(3,0,0)
    override_font( pfd );
#else
    modify_font( pfd );
#endif

    m_tooltip.modify_font_label( fontname );
}


//
// クロック入力
//
void DragTreeView::clock_in()
{
    // ポップアップ表示中はそちらにクロックを回す
    if( is_popup_shown() && m_popup_win->view() ){
        m_popup_win->view()->clock_in();
        return;
    }

    m_tooltip.clock_in();
}


//
// ツールチップに文字をセット
//
void DragTreeView::set_str_tooltip( const std::string& text )
{
    m_tooltip.set_text( text );
}


//
// ツールチップ最小幅設定
//
void DragTreeView::set_tooltip_min_width( const int& min_width )
{
    m_tooltip.set_min_width( min_width);
}


//
// ツールチップを表示
//
void DragTreeView::show_tooltip()
{
    m_tooltip.show_tooltip();
}


//
// ツールチップ隠す
//
void DragTreeView::hide_tooltip()
{
    m_tooltip.hide_tooltip();
}


//
// ポップアップが表示されていてかつマウスがその上にあるか
//
bool DragTreeView::is_mouse_on_popup()
{
    if( ! is_popup_shown() ) return false;
    if( ! m_popup_win->view() ) return false;

    return m_popup_win->view()->is_mouse_on_view();
}


//
// ポップアップウィンドウ表示
//
void DragTreeView::show_popup( const std::string& url, View* view )
{
    const int mrg_x = 10;
    const int mrg_y = 10;

#ifdef _DEBUG
    std::cout << "DragTreeView::show_popup url = " << url << std::endl;
#endif

    delete_popup();

    m_popup_win = new PopupWin( this, view, mrg_x, mrg_y );
    m_popup_win->signal_leave_notify_event().connect( sigc::mem_fun( *this, &DragTreeView::slot_popup_leave_notify_event ) );
    m_popup_win->sig_hide_popup().connect( sigc::mem_fun( *this, &DragTreeView::hide_popup ) );

    m_pre_popup_url = url;
    m_popup_shown = true;
}


//
// popup windowの外にポインタが出た
//
bool DragTreeView::slot_popup_leave_notify_event( GdkEventCrossing* event )
{
#ifdef _DEBUG
    std::cout << "DragTreeView::slot_popup_leave_notify_event\n";
#endif

    // クリックしたときやホイールを回すと event->mode に　GDK_CROSSING_GRAB
    // か GDK_CROSSING_UNGRAB がセットされてイベントが発生する場合がある
    if( event->mode == GDK_CROSSING_GRAB ) return false;
    if( event->mode == GDK_CROSSING_UNGRAB ) return false;

    // ポインタがポップアップ上にある場合はポップアップは消さない
    // 時々ポインタがポップアップの上にあってもイベントが発生する場合がある
    if( is_mouse_on_popup() ) return false;

    hide_popup();

    return true;
}


//
// ポップアップを隠す
//
void DragTreeView::hide_popup()
{
    if( ! is_popup_shown() ) return;

    // ポップアップメニューが表示されてたらhideしない
    if( SESSION::is_popupmenu_shown() ) return;

#ifdef _DEBUG
        std::cout << "DragTreeView::hide_popup\n";
#endif

    m_popup_win->hide();
    if( m_popup_win->view() ) m_popup_win->view()->stop();

    if( get_path_under_mouse().empty() ) m_pre_popup_url = std::string();
    m_popup_shown = false;
}


//
// ポップアップウィンドウ削除
//
void DragTreeView::delete_popup()
{
    if( m_popup_win ){

#ifdef _DEBUG
        std::cout << "DragTreeView::delete_popup\n";
#endif

        delete m_popup_win;
        m_popup_win = NULL;

        m_pre_popup_url = std::string();
        m_popup_shown = false;
    }
}


//
// マウスボタンを押した
//
bool DragTreeView::on_button_press_event( GdkEventButton* event )
{
    Gtk::TreeModel::Path path = get_path_under_xy( (int)event->x, (int)event->y );
    m_dragging = false;
    m_selection_canceled = false;
    sig_button_press().emit( event );

    // ドラッグして範囲選択
    // m_path_dragstart が empty でない時に範囲選択を行う
    // on_motion_notify_event()も参照せよ
    if( m_control.button_alloted( event, CONTROL::TreeRowSelectionButton ) )
        m_path_dragstart = m_path_dragpre = path;
    else m_path_dragstart = m_path_dragpre = Gtk::TreeModel::Path();

    // 複数行選択時の動作
    if( get_selection()->get_selected_rows().size() >= 2 ){

        // D&Dのため、ctrlやshiftなしで普通にクリックしたときも選択解除しない
        if( !( event->state & GDK_CONTROL_MASK )
            && !( event->state & GDK_SHIFT_MASK ) ){

            // ただし範囲選択外をクリックしたとき、または範囲選択開始ボタンを押したときは選択解除
            if( ! get_selection()->is_selected( path )
                || ! m_path_dragstart.empty()
                ){
                get_selection()->unselect_all();
                if( get_row( path ) ) set_cursor( path );
                m_selection_canceled = true; // ボタンを離したときにシグナルをemitしないようにする
            }
            else return true;
        }
    }

    return Gtk::TreeView::on_button_press_event( event );
}



//
// マウスボタンを離した
//
bool DragTreeView::on_button_release_event( GdkEventButton* event )
{
    bool emit_sig = false; // true なら m_sig_button_release をemitする

    Gtk::TreeModel::Path path = get_path_under_xy( (int)event->x, (int)event->y );

    if( ! m_dragging // ドラッグ中ではない
        && !m_selection_canceled // on_button_press_event()で選択状態を解除していない
        && !( event->state & GDK_CONTROL_MASK )
        && !( event->state & GDK_SHIFT_MASK ) // ctrl/shift + クリックで複数行選択操作をしてない場合

        && !( !m_path_dragstart.empty() && ! path.empty() && path != m_path_dragstart ) // 範囲選択操作をしていない場合
        ){ 

        // 以上の場合はシグナルをemitする
        emit_sig = true;

        // 範囲選択状態でポップアップメニュー以外のボタンを離したら選択解除
        if( get_selection()->get_selected_rows().size() >= 2
            && ! m_control.button_alloted( event, CONTROL::PopupmenuButton ) ){
            get_selection()->unselect_all();
            if( get_row( path ) ) set_cursor( path );
            emit_sig = false;
        }
    }

    // ポップアップメニューボタンを押したら必ずシグナルをemit
    if( m_control.button_alloted( event, CONTROL::PopupmenuButton ) ) emit_sig = true;

    m_path_dragstart = m_path_dragpre = Gtk::TreeModel::Path();

    const bool expanded = row_expanded( path );
    const bool ret = Gtk::TreeView::on_button_release_event( event );

    // 左の△ボタンを押してディレクトリを開け閉めした場合は信号のemitをキャンセル
    if( expanded != row_expanded( path ) ) emit_sig = false;

    if( emit_sig ) sig_button_release().emit( event );

    return ret;
}



//
// D&D開始
//
// このtreeがソースで無い時は呼ばれないのに注意
//
void DragTreeView::on_drag_begin( const Glib::RefPtr< Gdk::DragContext >& context )
{
#ifdef _DEBUG
    Gtk::TreeModel::Path path = get_path_under_mouse();
    std::cout << "DragTreeView::on_drag_begin path = " << path.to_string() << std::endl;
#endif

    m_dragging = true;
    CORE::DND_Begin();

    Gtk::TreeView::on_drag_begin( context );
}


//
// D&D 終了
//
// このtreeがソースでない時は呼び出されない
//
void DragTreeView::on_drag_end( const Glib::RefPtr< Gdk::DragContext >& context )
{
#ifdef _DEBUG
    std::cout << "DragTreeView::on_drag_end\n";
#endif

    m_dragging = false;
    CORE::DND_End();

    Gtk::TreeView::on_drag_end( context );
}


//
// ドロップされた
void DragTreeView::on_drag_data_received( const Glib::RefPtr< Gdk::DragContext >& context, int x, int y, 
                                          const Gtk::SelectionData& selection, guint info, guint time )
{
#ifdef _DEBUG
    std::cout << "DragTreeView::on_drag_data_received type = " << selection.get_data_type() << std::endl;
#endif

    if( selection.get_data_type() == "text/uri-list" ){

#ifdef _DEBUG
        std::cout << selection.get_data_as_string() << std::endl;
#endif

        std::list< std::string > uri_list = MISC::get_lines( MISC::url_decode( selection.get_data_as_string() ) );
        m_sig_dropped_url_list.emit( uri_list );
    }

    context->drag_finish( true, false, time );
}


//
// キーボードのキーを押した
//
bool DragTreeView::on_key_press_event( GdkEventKey* event )
{
#ifdef _DEBUG
    std::cout << "DragTreeView::on_key_press_event\n";
#endif

    // ポップアップ表示中はそちらにクロックを回す
    if( is_popup_shown() && m_popup_win->view() ) return m_popup_win->view()->slot_key_press( event );

    return JDTreeViewBase::on_key_press_event( event );
}


//
// マウスを動かした
//
bool DragTreeView::on_motion_notify_event( GdkEventMotion* event )
{
#ifdef _DEBUG
//    std::cout << "DragTreeView::on_motion_notify_event x = " << event->x << " y = " << event->y << std::endl;
#endif

    // drag_source_set() でセットしたボタン以外でドラッグして範囲選択
    // m_path_dragstart が empty で無いときに実行
    // DragTreeView::on_button_press_event() も参照せよ
    Gtk::TreeModel::Path path = get_path_under_xy( (int)event->x, (int)event->y );
    if( ! m_path_dragstart.empty() && !path.empty() && path != m_path_dragpre ){
        get_selection()->unselect_all();
        get_selection()->select( path, m_path_dragstart );
        m_path_dragpre = path;
    }

    sig_motion_notify().emit( event );

    return Gtk::TreeView::on_motion_notify_event( event );
}



// マウスのwheelを回した
bool DragTreeView::on_scroll_event( GdkEventScroll* event )
{
    sig_scroll_event().emit( event );

    return true;
}



//
// マウスホイールの処理
//
void DragTreeView::wheelscroll( GdkEventScroll* event )
{
    auto adj = get_vadjustment();
    double val = adj->get_value();

    int scr_inc = get_row_height() * CONFIG::get_tree_scroll_size();

    if( event->direction == GDK_SCROLL_UP ) val = MAX( 0, val - scr_inc );
    else if( event->direction == GDK_SCROLL_DOWN ) val = MIN( adj->get_upper() - adj->get_page_size(), val + scr_inc );
    adj->set_value( val );

#ifdef _DEBUG
    std::cout << "DragTreeView::on_scroll_event\n";

    std::cout << "scr_inc = " << scr_inc << std::endl;
    std::cout << "lower = " << adj->get_lower() << std::endl;
    std::cout << "upper = " << adj->get_upper() << std::endl;
    std::cout << "value = " << val << std::endl;
    std::cout << "step = " << adj->get_step_increment() << std::endl;
    std::cout << "page = " << adj->get_page_increment() << std::endl;
    std::cout << "page_size = " << adj->get_page_size() << std::endl;
#endif
}


bool DragTreeView::on_leave_notify_event( GdkEventCrossing* event )
{
    m_tooltip.hide_tooltip();
    if( ! is_mouse_on_popup() ){
        hide_popup();
        m_pre_popup_url = std::string();
    }
    return Gtk::TreeView::on_leave_notify_event( event );
}


//
// 範囲選択更新
//
void DragTreeView::slot_selection_changed()
{
    int size = get_selection()->get_selected_rows().size();

    std::string str;
    if( size >= 2 ) str = "選択数 " + MISC::itostr( size );
    CORE::core_set_command( "set_info" ,"", str );
}



//
// 実際の描画の際に cellrenderer のプロパティをセットするスロット関数
//
void DragTreeView::slot_cell_data( Gtk::CellRenderer* cell, const Gtk::TreeModel::iterator& it )
{
    if( ! m_use_bg_even ){
        cell->property_cell_background_set() = false;
        return;
    }

    Gtk::TreeModel::Row row = *it;
    Gtk::TreePath path = get_model()->get_path( row );
    std::string path_str = path.to_string();

#ifdef _DEBUG
    std::cout << "DragTreeView::slot_cell_data path = " << path_str << std::endl;
#endif

    bool even = false;

    int rownum = atoi( path_str.c_str() );
    if( rownum %2 ) even = true;

    size_t pos = path_str.find( ":" );
    while( pos != std::string::npos ){
        path_str = path_str.substr( pos+1 );
        rownum = atoi( path_str.c_str() );

        if( ( even && ( rownum %2 ) ) || ( ! even && !( rownum %2 ) ) ) even = true;
        else even = false;

        pos = path_str.find( ":" );
    }

    // 偶数行に色を塗る
    if( even ){
#if GTKMM_CHECK_VERSION(3,0,0)
        cell->property_cell_background_rgba() = m_color_bg_even;
#else
        cell->property_cell_background_gdk() = m_color_bg_even;
#endif
        cell->property_cell_background_set() = true;
    }

    else cell->property_cell_background_set() = false;
}

