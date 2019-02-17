// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "dragnote.h"
#include "tablabel.h"
#include "toolbar.h"
#include "view.h"
#include "iconpopup.h"

#include "icons/iconmanager.h"

#include "control/controlid.h"

#include "dndmanager.h"
#include "session.h"

#include <cstring>

using namespace SKELETON;

DragableNoteBook::DragableNoteBook()
    : Gtk::VBox()
    , m_notebook_tab( this )
    , m_notebook_toolbar( this )
    , m_notebook_view( this )
    , m_bt_tabswitch( this )
    , m_page( -1 )
    , m_dragging_tab( false )
    , m_dragable( false )
    , m_down_arrow( NULL )
{
    set_spacing( 0 );

    m_notebook_tab.signal_switch_page().connect( sigc::mem_fun( *this, &DragableNoteBook::slot_switch_page_tab ) );
    m_notebook_tab.sig_button_press().connect( sigc::mem_fun( *this, &DragableNoteBook::slot_button_press_event ) );
    m_notebook_tab.sig_button_release().connect( sigc::mem_fun( *this, &DragableNoteBook::slot_button_release_event ) );

    m_notebook_tab.sig_tab_motion_event().connect( sigc::mem_fun(*this, &DragableNoteBook::slot_motion_event ) );
    m_notebook_tab.sig_tab_leave_event().connect( sigc::mem_fun(*this, &DragableNoteBook::slot_leave_event ) );

    m_notebook_tab.sig_tab_drag_motion().connect( sigc::mem_fun(*this, &DragableNoteBook::slot_drag_motion ) );

    m_notebook_tab.sig_scroll_event().connect( sigc::mem_fun(*this, &DragableNoteBook::slot_scroll_event ) );

    m_hbox_tab.pack_start( m_notebook_tab );
    m_hbox_tab.pack_start( m_bt_tabswitch, Gtk::PACK_SHRINK );
    constexpr const char* bt_tabswitch_tip = "タブの一覧表示";
#if GTKMM_CHECK_VERSION(2,12,0)
    m_bt_tabswitch.set_tooltip_text( bt_tabswitch_tip );
#else
    m_tooltip_tabswitch.set_tip( m_bt_tabswitch, bt_tabswitch_tip );
#endif

    pack_start( m_hbox_tab, Gtk::PACK_SHRINK );
    pack_start( m_notebook_toolbar, Gtk::PACK_SHRINK );
    pack_start( m_notebook_view );
    m_show_tabs = true;
    m_show_toolbar = true;

    memset( &m_alloc_old, 0, sizeof( Alloc_NoteBook ) );

    show_all_children();
}



DragableNoteBook::~DragableNoteBook()
{
    if( m_down_arrow ) delete m_down_arrow;
}



//
// クロック入力
//
void DragableNoteBook::clock_in()
{
    m_notebook_tab.clock_in();
    m_tooltip.clock_in();
}


//
// フォーカスアウト
//
void DragableNoteBook::focus_out()
{
    m_tooltip.hide_tooltip();
}


//
// Auroraなどテーマによっては m_notebook_toolbar が m_notebook_view に上書きされて
// 消えてしまうのでもう一度 m_notebook_toolbar を描画する
//
#if !GTKMM_CHECK_VERSION(3,0,0)
bool DragableNoteBook::on_expose_event( GdkEventExpose* event )
{
    const bool ret =  Gtk::VBox::on_expose_event( event );

    // ツールバー再描画
   if( m_notebook_toolbar.get_parent() == this ){

        const int min_toolbar_height = 4;
        const Alloc_NoteBook alloc = get_alloc_notebook();
        const int y_start = get_show_tabs() ? alloc.y_tabbar + alloc.height_tabbar : alloc.y_toolbar;
        const int y_end = y_start + alloc.height_toolbar;

        if( alloc.height_toolbar > min_toolbar_height  // ツールバーの中身が表示されている
            && y_start <= event->area.y + event->area.height && y_end >= event->area.y ){

            GdkEventExpose event_toolbar = *event;
            if( event_toolbar.area.y < y_start ){
                event_toolbar.area.height -= ( y_start - event_toolbar.area.y );
                event_toolbar.area.y = y_start;
            }
            if( event_toolbar.area.y + event_toolbar.area.height > y_end ) event_toolbar.area.height = y_end - event_toolbar.area.y;

#ifdef _DEBUG
            std::cout << "DragableNoteBook::on_expose_event"
                      << " y_s = " << y_start
                      << " y_e = " << y_end
                      << " x = " << event->area.x
                      << " y = " << event->area.y
                      << " w = " << event->area.width
                      << " h = " << event->area.height
                      << "\n-> x_t = " << event_toolbar.area.x
                      << " y_t = " << event_toolbar.area.y
                      << " w_t = " << event_toolbar.area.width
                      << " h_t = " << event_toolbar.area.height
                      << std::endl;
#endif
            if( event_toolbar.area.height ){
 
                propagate_expose( m_notebook_toolbar, &event_toolbar );

                // (注意) Auroraなどテーマによってはクリップ領域( event->area )を無視するものがあり
                // ビューのスクロールバーが消えてしまう時があるので明示的に再描画する
                m_notebook_view.redraw_scrollbar();
            }
        }
    }
    return ret;
}
#endif // !GTKMM_CHECK_VERSION(3,0,0)


//
// DragableNoteBook を構成している各Notebookの高さ
// 及びタブの高さと位置を取得 ( 枠の描画用 )
//
#if !GTKMM_CHECK_VERSION(3,0,0)
Alloc_NoteBook DragableNoteBook::get_alloc_notebook()
{
    Alloc_NoteBook alloc;

    m_notebook_tab.get_alloc_tab( alloc );

    alloc.y_tabbar = m_notebook_tab.get_allocation().get_y();
    alloc.height_tabbar = m_notebook_tab.get_allocation().get_height();

    alloc.y_toolbar = m_notebook_toolbar.get_allocation().get_y();
    alloc.height_toolbar = m_notebook_toolbar.get_allocation().get_height();

    const int offset_tabbar = ( get_show_tabs() ? alloc.height_tabbar - ( alloc.y_tab + alloc.height_tab ) : 0 );
    alloc.x_box = m_notebook_view.get_allocation().get_x();
    alloc.y_box = m_notebook_view.get_allocation().get_y() - alloc.height_toolbar - offset_tabbar;
    alloc.width_box =m_notebook_view.get_allocation().get_width();
    alloc.height_box = offset_tabbar + alloc.height_toolbar + m_notebook_view.get_allocation().get_height();

#ifdef _DEBUG
    std::cout << "DragableNoteBook::get_alloc_notebook"
              << " x_tab = " << alloc.x_tab
              << " w_tab = " << alloc.width_tab
              << " h_tab = " << alloc.height_tab
              << " y_tabbar = " << alloc.y_tabbar
              << " h_tabbar = " << alloc.height_tabbar
              << " y_toolbar = " << alloc.y_toolbar
              << " h_toolbar = " << alloc.height_toolbar
              << " x_box = " << alloc.x_box
              << " y_box = " << alloc.y_box
              << " w_box = " << alloc.width_box
              << " h_box = " << alloc.height_box
              << std::endl;
#endif

    return alloc;
}
#endif // !GTKMM_CHECK_VERSION(3,0,0)


//
// 枠描画
//
// gtknotebook.c( Revision 19593, Sat Feb 16 04:09:15 2008 UTC ) からのハック。環境やバージョンによっては問題が出るかもしれないので注意
//
#if !GTKMM_CHECK_VERSION(3,0,0)
void DragableNoteBook::draw_box( Gtk::Widget* widget, GdkEventExpose* event )
{
    const Glib::RefPtr<Gdk::Window> win = widget->get_window();
    const Gdk::Rectangle rect( &(event->area) );
    Alloc_NoteBook alloc = get_alloc_notebook();

    if( alloc.height_box > 0 ){

        if( get_show_tabs() ){

            // gtk2.18以降？では新しいタブを開くときに一瞬タブが消える場合がある
            // その場合は保存しておいた座標と高さを利用してboxを描画する
            if( ! alloc.height_tab ){

                if( ! m_alloc_old.height_tab ) return;

                alloc.y_box = m_alloc_old.y_box;
                alloc.height_box = m_alloc_old.height_box;
            }
            else m_alloc_old = alloc;

            widget->get_style()->paint_box_gap( win,
                                                Gtk::STATE_NORMAL,
                                                Gtk::SHADOW_OUT,
                                                rect,
                                                *widget,
                                                "notebook",
                                                alloc.x_box,
                                                alloc.y_box,
                                                alloc.width_box,
                                                alloc.height_box,
                                                Gtk::POS_TOP,
                                                alloc.x_tab,
                                                alloc.width_tab
                );
        }
        else{

            widget->get_style()->paint_box( win,
                                            Gtk::STATE_NORMAL,
                                            Gtk::SHADOW_OUT,
                                            rect,
                                            *widget,
                                            "notebook",
                                            alloc.x_box,
                                            alloc.y_box,
                                            alloc.width_box,
                                            alloc.height_box
                );
        }
    }
}
#endif // !GTKMM_CHECK_VERSION(3,0,0)


void DragableNoteBook::set_show_tabs( bool show_tabs )
{
#ifdef _DEBUG
    std::cout << "DragableNoteBook::set_show_tabs show_tabs = " << show_tabs << std::endl;
#endif

    if( m_show_tabs ){

        remove( m_hbox_tab );

        m_show_tabs = false;
    }
    else{

        remove( m_notebook_toolbar );
        remove( m_notebook_view );

        pack_start( m_hbox_tab, Gtk::PACK_SHRINK );
        pack_start( m_notebook_toolbar, Gtk::PACK_SHRINK );
        pack_start( m_notebook_view );

        m_show_tabs = true;
    }
}


void DragableNoteBook::set_scrollable( bool scrollable )
{
    m_notebook_tab.set_scrollable( scrollable );
}


int DragableNoteBook::get_n_pages()
{
    return m_notebook_view.get_n_pages();
}


Gtk::Widget* DragableNoteBook::get_nth_page( int page_num )
{
    return m_notebook_view.get_nth_page( page_num );
}


int DragableNoteBook::page_num( const Gtk::Widget& child )
{
    return m_notebook_view.page_num( child );
}


int DragableNoteBook::get_current_page()
{
    return m_notebook_view.get_current_page();
}


void DragableNoteBook::set_current_page( int page_num )
{
    if( get_current_page() == page_num ) return;
    if( page_num >= get_n_pages() ) page_num = get_n_pages()-1;

    m_notebook_tab.set_current_page( page_num );
    m_notebook_view.set_current_page( page_num );

    SKELETON::View* view = dynamic_cast< View* >( get_nth_page( page_num ) );
    if( view ) set_current_toolbar( view->get_id_toolbar(), view );
}


//
// ページのappendとinsert
//
int DragableNoteBook::append_page( const std::string& url, Gtk::Widget& child )
{
    m_notebook_view.append_page( child );

    m_bt_tabswitch.show_button();

    SKELETON::TabLabel* tablabel = create_tablabel( url );
    return m_notebook_tab.append_tab( *tablabel );
}


int DragableNoteBook::insert_page( const std::string& url, Gtk::Widget& child, int page )
{
    m_notebook_view.insert_page( child, page );

    m_bt_tabswitch.show_button();

    SKELETON::TabLabel* tablabel = create_tablabel( url );
    return m_notebook_tab.insert_tab( *tablabel, page );
}


//
// タブの文字列取得
//
const std::string& DragableNoteBook::get_tab_fulltext( const int page )
{
    return m_notebook_tab.get_tab_fulltext( page );
}



//
// タブに文字列をセットとタブ幅調整
//
void DragableNoteBook::set_tab_fulltext( const std::string& str, const int page )
{
    m_notebook_tab.set_tab_fulltext( str, page );
}



//
// ページ取り除き
//
void DragableNoteBook::remove_page( const int page, const bool adjust_tab )
{
    SKELETON::TabLabel* tablabel = m_notebook_tab.get_tablabel( page );

    m_notebook_tab.remove_tab( page, adjust_tab );
    m_notebook_view.remove_page( page );

    if( tablabel ) delete tablabel;

    m_tooltip.hide_tooltip();

    if( ! get_n_pages() ) m_bt_tabswitch.hide_button();
}


//
// ツールバー取得
//
SKELETON::ToolBar* DragableNoteBook::get_toolbar( int page )
{
    return dynamic_cast< SKELETON::ToolBar* >( m_notebook_toolbar.get_nth_page( page ) );
}


//
// ツールバー全体を表示
//
// タブにビューが表示されたら admin から呼び出される
//
void DragableNoteBook::show_toolbar()
{
#ifdef _DEBUG
    std::cout << "DragableNoteBook::show_toolbar\n";
#endif

    if( ! m_show_toolbar && m_notebook_toolbar.get_n_pages() ){

        remove( m_notebook_view );

        pack_start( m_notebook_toolbar, Gtk::PACK_SHRINK );
        pack_start( m_notebook_view );

        m_show_toolbar = true;
    }
}


//
// ツールバー全体を非表示
//
// タブに全てのビューが無くなったら admin から呼び出される
//
void DragableNoteBook::hide_toolbar()
{
#ifdef _DEBUG
    std::cout << "DragableNoteBook::hide_toolbar\n";
#endif

    if( m_show_toolbar && m_notebook_toolbar.get_n_pages() ){

        remove( m_notebook_toolbar );

        m_show_toolbar = false;
    }
}


//
// ツールバーセット
//
// 各Adminクラスの virtual void show_toolbar() でセットされる
//
void DragableNoteBook::append_toolbar( Gtk::Widget& toolbar )
{
#ifdef _DEBUG
    std::cout << "DragableNoteBook::append_toolbar\n";
#endif

    m_notebook_toolbar.append_page( toolbar );
}


//
// ツールバー切り替え
//
void DragableNoteBook::set_current_toolbar( const int id_toolbar, SKELETON::View* view )
{
    // タブ操作中
    if( SESSION::is_tab_operating( view->get_url_admin() ) ) return;

#ifdef _DEBUG
    std::cout << "DragableNoteBook::set_current_toolbar id = " << id_toolbar
              << " / " << m_notebook_toolbar.get_n_pages() << std::endl
              << "url = " << view->get_url() << std::endl;
#endif

    if( m_notebook_toolbar.get_n_pages() <= id_toolbar ) return;

    m_notebook_toolbar.set_current_page( id_toolbar );

    // ツールバーのラベルなどの情報を更新
    SKELETON::ToolBar* toolbar = get_toolbar( id_toolbar );
    if( toolbar ){
        toolbar->set_view( view );
        toolbar->show_toolbar();
    }
}


int DragableNoteBook::get_current_toolbar()
{
    return m_notebook_toolbar.get_current_page();
}


//
// ツールバー内の検索ボックスにフォーカスを移す
//
void DragableNoteBook::focus_toolbar_search()
{
    SKELETON::ToolBar* toolbar = get_toolbar( m_notebook_toolbar.get_current_page() );
    if( toolbar ) toolbar->focus_entry_search();
}


//
// ツールバーURL更新
//
void DragableNoteBook::update_toolbar_url( std::string& url_old, std::string& url_new )
{
    for( int i = 0; i < m_notebook_toolbar.get_n_pages(); ++i ){
        SKELETON::ToolBar* toolbar = get_toolbar( i );
        if( toolbar && toolbar->get_url() == url_old ) toolbar->set_url( url_new );
    }
}


//
// ツールバーボタン表示更新
//
void DragableNoteBook::update_toolbar_button()
{
    for( int i = 0; i < m_notebook_toolbar.get_n_pages(); ++i ){
        SKELETON::ToolBar* toolbar = get_toolbar( i );
        if( toolbar ) toolbar->update_button();
    } 
}


//
// タブのアイコンを取得する
//
int DragableNoteBook::get_tabicon( const int page )
{
    SKELETON::TabLabel* tablabel = m_notebook_tab.get_tablabel( page );
    if( tablabel ) return tablabel->get_id_icon();

    return ICON::NONE;
}


//
// タブにアイコンをセットする
//
void DragableNoteBook::set_tabicon( const std::string& iconname, const int page, const int id )
{
    SKELETON::TabLabel* tablabel = m_notebook_tab.get_tablabel( page );
    if( tablabel && id != ICON::NONE ) tablabel->set_id_icon( id );
}


//
// タブ作成
//
SKELETON::TabLabel* DragableNoteBook::create_tablabel( const std::string& url )
{
    SKELETON::TabLabel *tablabel = new SKELETON::TabLabel( url );

    // ドラッグ設定
    GdkEventButton event;
    m_control.get_eventbutton( CONTROL::DragStartButton, event );
    tablabel->set_dragable( m_dragable, event.button );

    tablabel->sig_tab_motion_event().connect( sigc::mem_fun(*this, &DragableNoteBook::slot_motion_event ) );
    tablabel->sig_tab_leave_event().connect( sigc::mem_fun(*this, &DragableNoteBook::slot_leave_event ) );

    tablabel->sig_tab_drag_begin().connect( sigc::mem_fun(*this, &DragableNoteBook::slot_drag_begin ) );
    tablabel->sig_tab_drag_data_get().connect( sigc::mem_fun(*this, &DragableNoteBook::slot_drag_data_get ) );
    tablabel->sig_tab_drag_end().connect( sigc::mem_fun(*this, &DragableNoteBook::slot_drag_end ) );
    
    return tablabel;
}


//
// タブの幅を固定するか
//
void DragableNoteBook::set_fixtab( bool fix )
{
    m_notebook_tab.set_fixtab( fix );
}


//
// タブ幅調整
//
bool DragableNoteBook::adjust_tabwidth()
{
    return m_notebook_tab.adjust_tabwidth();
}


//
// notebook_tabのタブが切り替わったときに呼ばれるslot
//
void DragableNoteBook::slot_switch_page_tab( GtkNotebookPage* bookpage, guint page )
{
    // view も切り替える
    m_notebook_view.set_current_page( page );

    // ツールバー(枠) 再描画
    m_notebook_toolbar.queue_draw();

    m_sig_switch_page.emit( bookpage, page );
}


//
// タブの上でボタンを押した
//
bool DragableNoteBook::slot_button_press_event( GdkEventButton* event )
{
    // ボタンを押した時点でのページ番号を記録しておく
    m_page = m_notebook_tab.get_page_under_mouse();
    m_dragging_tab = false;

    // ダブルクリック
    // button_release_eventでは event->type に必ず GDK_BUTTON_RELEASE が入る
    m_dblclick = false;
    if( event->type == GDK_2BUTTON_PRESS ) m_dblclick = true; 

#ifdef _DEBUG
    std::cout << "DragableNoteBook::on_button_press_event page = " << m_page  << std::endl;
    std::cout << "x = " << (int)event->x_root << " y = " << (int)event->y_root
              << " dblclick = " << m_dblclick << std::endl;
#endif

    if( m_page >= 0 && m_page < get_n_pages() ){

        // ページ切替え
        if( m_control.button_alloted( event, CONTROL::ClickButton ) ){

            set_current_page( m_page );
            m_sig_tab_clicked.emit( m_page );
        }

        return true;
    }
    else m_page = -1;

    return false;
}


//
// タブの上でボタンを離した
//
bool DragableNoteBook::slot_button_release_event( GdkEventButton* event )
{
    const int page = m_notebook_tab.get_page_under_mouse();
    const int x = (int)event->x_root;
    const int y = (int)event->y_root;

#ifdef _DEBUG
    std::cout << "DragableNoteBook::on_button_release_event\n";
    std::cout << "x = " << (int)event->x_root << " y = " << (int)event->y_root << std::endl;
#endif

    if( ! m_dragging_tab && m_page >= 0 && m_page < get_n_pages() ){

        // ダブルクリックの処理のため一時的にtypeを切替える
        GdkEventType type_copy = event->type;
        if( m_dblclick ) event->type = GDK_2BUTTON_PRESS;

        // タブを閉じる
        if( page == m_page && m_control.button_alloted( event, CONTROL::CloseTabButton ) ){
            m_sig_tab_close.emit( m_page );

            // タブにページが残ってなかったらtrueをreturnしないと落ちる
            // TabNotebook::on_button_release_event() も参照せよ
            //
            // (注意) なぜか m_notebook_tab.get_n_pages() < m_notebook_view.get_n_pages() の時があって
            // 以前の様に
            // if( get_n_pages() == 0 )
            // という条件では m_notebook_tab.get_n_pages() = 0 でも get_n_pages() != 0 になって落ちることがある
            if( m_notebook_tab.get_n_pages() == 0 ){
                m_page = -1;
                return true;
            }
        }

        // タブを再読み込み
        else if( m_control.button_alloted( event, CONTROL::ReloadTabButton ) ) m_sig_tab_reload.emit( m_page );

        // ポップアップメニュー
        else if( m_control.button_alloted( event, CONTROL::PopupmenuButton ) ) m_sig_tab_menu.emit( m_page, x, y );

        m_page = -1;
        event->type = type_copy;
    }

    return false;
}



//
// タブの中でマウスを動かした
//
void DragableNoteBook::slot_motion_event()
{
    const int page = m_notebook_tab.get_page_under_mouse();

#ifdef _DEBUG
    std::cout << "DragableNoteBook::slot_motion_event page = " << page << std::endl;
#endif

    // ツールチップにテキストをセット
    if( page >= 0 && page < m_notebook_tab.get_n_pages() ){
        SKELETON::TabLabel* tab = m_notebook_tab.get_tablabel( page );
        if( tab ) m_tooltip.set_text( tab->get_fulltext() );
    }
    else m_tooltip.hide_tooltip();
}


//
// タブからマウスが出た
//
void DragableNoteBook::slot_leave_event()
{
#ifdef _DEBUG
    std::cout << "DragableNoteBook::slot_leave_event page\n";
#endif

    m_tooltip.hide_tooltip();
}


// notebook_tab の上でホイールを回した
bool DragableNoteBook::slot_scroll_event( GdkEventScroll* event )
{
#ifdef _DEBUG
    std::cout << "DragableNoteBook::slot_scroll_event direction = " << event->direction << " page = " << get_current_page() << std::endl;
#endif

    bool ret = false;

    // gtk3ではタブの循環に加え隣のタブへ切り替える処理も実装する必要がある
    // REVIEW: GTK3版ではホイールによるタブの切り替えが動作しない環境がある
    if( event->direction == GDK_SCROLL_UP ) {
        const int current_page = get_current_page();
        if( current_page == 0 ) {
            set_current_page( get_n_pages() - 1 );
            ret = true;
        }
#if GTKMM_CHECK_VERSION(3,0,0)
        else {
            set_current_page( current_page - 1 );
            ret = true;
        }
#endif
    }
    else if( event->direction == GDK_SCROLL_DOWN ) {
        const int current_page = get_current_page();
        if( current_page == get_n_pages() - 1 ) {
            set_current_page( 0 );
            ret = true;
        }
#if GTKMM_CHECK_VERSION(3,0,0)
        else {
            set_current_page( current_page + 1 );
            ret = true;
        }
#endif
    }

    m_sig_tab_scrolled.emit( event );

    return ret;
}



//
// タブのドラッグを開始
//
void DragableNoteBook::slot_drag_begin()
{
#ifdef _DEBUG
    std::cout << "DragableNoteBook::slot_drag_begin page = " << m_page << std::endl;
#endif

    CORE::DND_Begin();

    m_dragging_tab = true;
}


//
// タブをドラッグ中
//
// 矢印アイコンをタブの上に表示する
//
void DragableNoteBook::slot_drag_motion( const int page, const int tab_x, const int tab_y, const int tab_width )
{
#ifdef _DEBUG
    std::cout << "DragableNoteBook::slot_drag_motion page = " << page
              << " tab_x = " << tab_x << " tab_y = " << tab_y << " tab_w " << tab_width << std::endl;
#endif

    if( page < 0 || page == m_page ){
        if( m_down_arrow ) m_down_arrow->hide();
    }
    else if( m_dragging_tab ){

        if( ! m_down_arrow ) m_down_arrow = new SKELETON::IconPopup( ICON::DOWN );
        m_down_arrow->show();

        const int space = 4;
        int x, y;
        m_notebook_tab.get_window()->get_origin( x, y );

        x += tab_x - m_down_arrow->get_img_width()/2;
        y += tab_y - m_down_arrow->get_img_height() - space;
        if( page > m_page ) x += tab_width;

        m_down_arrow->move( x , y );
    }
}


//
// D&Dで受信側がデータ送信を要求してきた
//
void DragableNoteBook::slot_drag_data_get( Gtk::SelectionData& selection_data )
{
#ifdef _DEBUG
    std::cout << "DragableNoteBook::slot_drag_data_get target = " << selection_data.get_target()
              << " page = " << m_page << std::endl;
#endif

    // お気に入りがデータ送信を要求してきた
    if( selection_data.get_target() == DNDTARGET_FAVORITE ) m_sig_drag_data_get.emit( selection_data, m_page );

    // タブの入れ替え処理
    else if( selection_data.get_target() == DNDTARGET_TAB ){

        int page = m_notebook_tab.get_page_under_mouse();
        if( page >= m_notebook_tab.get_n_pages() ) page = m_notebook_tab.get_n_pages()-1;

        // ドラッグ前とページが変わっていたら入れ替え
        if( m_page != -1 && page != -1 && m_page != page ){

#ifdef _DEBUG
            std::cout << "reorder_chiled " << m_page << " -> " << page << std::endl;
#endif
            m_notebook_tab.reorder_child( m_page, page );
            m_notebook_tab.queue_draw();
            m_notebook_view.reorder_child( *m_notebook_view.get_nth_page( m_page ), page );
            m_page = -1;
        }

        if( m_down_arrow ) m_down_arrow->hide();
    }
}


//
// タブのドラッグを終了
//
void DragableNoteBook::slot_drag_end()
{
#ifdef _DEBUG
    std::cout << "DragableNoteBook::slot_drag_end\n";
#endif

    m_dragging_tab = false;

    if( m_down_arrow ) m_down_arrow->hide();

    CORE::DND_End();
}
