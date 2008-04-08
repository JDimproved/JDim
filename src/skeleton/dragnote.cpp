// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "dragnote.h"
#include "tablabel.h"
#include "toolbar.h"
#include "view.h"
#include "iconpopup.h"

#include "icons/iconmanager.h"

#include "controlid.h"

using namespace SKELETON;

DragableNoteBook::DragableNoteBook()
    : Gtk::VBox()
    , m_notebook_tab( this )
    , m_notebook_toolbar( this )
    , m_notebook_view( this )
    , m_page( -1 )
    , m_drag( 0 )
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

    pack_start( m_notebook_tab, Gtk::PACK_SHRINK );
    pack_start( m_notebook_toolbar, Gtk::PACK_SHRINK );
    pack_start( m_notebook_view );
    m_show_tabs = true;

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
// DragableNoteBook を構成している各Notebookの高さ
// 及びタブの高さと位置を取得 ( 枠の描画用 )
//
const Alloc_NoteBook DragableNoteBook::get_alloc_notebook()
{
    Alloc_NoteBook alloc;

    alloc.height_tabbar = m_notebook_tab.get_allocation().get_height();
    alloc.height_toolbar = m_notebook_toolbar.get_allocation().get_height();
    alloc.height_view = m_notebook_view.get_allocation().get_height();

    m_notebook_tab.get_alloc_tab( alloc.x_tab, alloc.width_tab, alloc.height_tab );

#ifdef _DEBUG
    std::cout << "DragableNoteBook::get_alloc_notebook "
              << "tab = " << alloc.height_tab
              << " tabbar = " << alloc.height_tabbar
              << " toolbar = " << alloc.height_toolbar
              << " view = " << alloc.height_view
              << " x_tab = " << alloc.x_tab
              << " width_tab = " << alloc.width_tab
              << " height_tab = " << alloc.height_tab
              << std::endl;
#endif

    return alloc;
}


void DragableNoteBook::set_show_tabs( bool show_tabs )
{
#ifdef _DEBUG
    std::cout << "DragableNoteBook::set_show_tabs show_tabs = " << show_tabs << std::endl;
#endif

    if( m_show_tabs ){

        remove( m_notebook_tab );

        m_show_tabs = false;
    }
    else{

        remove( m_notebook_toolbar );
        remove( m_notebook_view );

        pack_start( m_notebook_tab, Gtk::PACK_SHRINK );
        pack_start( m_notebook_toolbar, Gtk::PACK_SHRINK );
        pack_start( m_notebook_view );

        m_show_tabs = true;
    }
}


void DragableNoteBook::set_scrollable( bool scrollable )
{
    m_notebook_tab.set_scrollable( scrollable );
}


const int DragableNoteBook::get_n_pages()
{
    return m_notebook_view.get_n_pages();
}


Gtk::Widget* DragableNoteBook::get_nth_page( int page_num )
{
    return m_notebook_view.get_nth_page( page_num );
}


const int DragableNoteBook::page_num( const Gtk::Widget& child )
{
    return m_notebook_view.page_num( child );
}


const int DragableNoteBook::get_current_page()
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

    SKELETON::TabLabel* tablabel = create_tablabel( url );
    return m_notebook_tab.append_tab( *tablabel );
}


int DragableNoteBook::insert_page( const std::string& url, Gtk::Widget& child, int page )
{
    m_notebook_view.insert_page( child, page );

    SKELETON::TabLabel* tablabel = create_tablabel( url );
    return m_notebook_tab.insert_tab( *tablabel, page );
}


//
// タブの文字列取得
//
const std::string DragableNoteBook::get_tab_fulltext( int page )
{
    return m_notebook_tab.get_tab_fulltext( page );
}



//
// タブに文字列をセットとタブ幅調整
//
void DragableNoteBook::set_tab_fulltext( const std::string& str, int page )
{
    m_notebook_tab.set_tab_fulltext( str, page );
}



//
// ページ取り除き
//
void DragableNoteBook::remove_page( int page )
{
    SKELETON::TabLabel* tablabel = m_notebook_tab.get_tablabel( page );

    m_notebook_tab.remove_tab( page );
    m_notebook_view.remove_page( page );

    if( tablabel ) delete tablabel;

    m_tooltip.hide_tooltip();
}


//
// ツールバー取得
//
SKELETON::ToolBar* DragableNoteBook::get_toolbar( int page )
{
    return dynamic_cast< SKELETON::ToolBar* >( m_notebook_toolbar.get_nth_page( page ) );
}


//
// ツールバー表示
//
void DragableNoteBook::show_toolbar()
{
    if( m_notebook_toolbar.get_n_pages() ) m_notebook_toolbar.show();
}


//
// ツールバー非表示
//
void DragableNoteBook::hide_toolbar()
{
    if( m_notebook_toolbar.get_n_pages() ) m_notebook_toolbar.hide();
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
void DragableNoteBook::set_current_toolbar( int page_num, SKELETON::View* view )
{
#ifdef _DEBUG
    std::cout << "DragableNoteBook::set_current_toolbar page = " << page_num << " / " << m_notebook_toolbar.get_n_pages() << std::endl
              << "url = " << view->get_url() << std::endl;
#endif

    if( m_notebook_toolbar.get_n_pages() <= page_num ) return;

    m_notebook_toolbar.set_current_page( page_num );

    // ツールバーのラベルなどの情報を更新
    SKELETON::ToolBar* toolbar = get_toolbar( page_num );
    if( toolbar ) toolbar->set_view( view );
}


const int DragableNoteBook::get_current_toolbar()
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
// ツールバーラベル表示更新
//
void DragableNoteBook::update_toolbar_label( SKELETON::View* view )
{
    for( int i = 0; i < m_notebook_toolbar.get_n_pages(); ++i ){
        SKELETON::ToolBar* toolbar = get_toolbar( i );
        if( toolbar ) toolbar->update_label( view );
    }
}


//
// ツールバー閉じるボタン表示更新
//
void DragableNoteBook::update_toolbar_close_button( SKELETON::View* view )
{
    for( int i = 0; i < m_notebook_toolbar.get_n_pages(); ++i ){
        SKELETON::ToolBar* toolbar = get_toolbar( i );
        if( toolbar ) toolbar->update_close_button( view );
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
    m_drag = false;

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
        if( m_control.button_alloted( event, CONTROL::ClickButton ) ) m_sig_tab_click.emit( m_page );

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

    if( ! m_drag && m_page >= 0 && m_page < get_n_pages() ){

        // ダブルクリックの処理のため一時的にtypeを切替える
        GdkEventType type_copy = event->type;
        if( m_dblclick ) event->type = GDK_2BUTTON_PRESS;

        // タブを閉じる
        if( page == m_page && m_control.button_alloted( event, CONTROL::CloseTabButton ) ){
            m_sig_tab_close.emit( m_page );

            // タブにページが残ってなかったらtrueをreturnしないと落ちる
            // TabNotebook::on_button_release_event() も参照せよ
            if( get_n_pages() == 0 ){
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


//
// タブのドラッグを開始
//
void DragableNoteBook::slot_drag_begin()
{

#ifdef _DEBUG
    std::cout << "DragableNoteBook::slot_drag_begin page = " << m_page << std::endl;
#endif

    m_drag = true;

    m_sig_drag_begin.emit( m_page );
}


//
// タブをドラッグ中
//
void DragableNoteBook::slot_drag_motion( const int page, const int tab_x, const int tab_y, const int tab_width )
{
    if( !m_drag ) return;

#ifdef _DEBUG
    std::cout << "DragableNoteBook::slot_drag_motion page = " << page
              << " tab_x = " << tab_x << " tab_y = " << tab_y << " tab_w " << tab_width << std::endl;
#endif

    if( page < 0 || page == m_page ){
        if( m_down_arrow ) m_down_arrow->hide();
    }
    else{

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
// タブのドラッグを終了
//
void DragableNoteBook::slot_drag_end()
{
#ifdef _DEBUG
    std::cout << "DragableNoteBook::slot_drag_end\n";
#endif

    if( m_drag ){

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

    m_sig_drag_end.emit();

    m_drag = false;
}
