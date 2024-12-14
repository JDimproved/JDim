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

#include "command.h"
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
    , m_show_tabs{ true }
    , m_show_toolbar{ true }
    , m_page( -1 )
{
    set_spacing( 0 );

    m_notebook_tab.signal_switch_page().connect( sigc::mem_fun( *this, &DragableNoteBook::slot_switch_page_tab ) );
    m_notebook_tab.sig_button_press().connect( sigc::mem_fun( *this, &DragableNoteBook::slot_button_press_event ) );
    m_notebook_tab.sig_button_release().connect( sigc::mem_fun( *this, &DragableNoteBook::slot_button_release_event ) );

    m_notebook_tab.sig_tab_drag_motion().connect( sigc::mem_fun(*this, &DragableNoteBook::slot_drag_motion ) );

    m_notebook_tab.sig_scroll_event().connect( sigc::mem_fun(*this, &DragableNoteBook::slot_scroll_event ) );

    m_hbox_tab.pack_start( m_notebook_tab );
    m_hbox_tab.pack_start( m_bt_tabswitch, Gtk::PACK_SHRINK );
    m_bt_tabswitch.set_tooltip_text( "タブの一覧表示" );

    pack_start( m_hbox_tab, Gtk::PACK_SHRINK );
    pack_start( m_notebook_toolbar, Gtk::PACK_SHRINK );
    pack_start( m_notebook_view );

    show_all_children();
}


// メンバーに不完全型のスマートポインターがあるためデストラクタはinlineにできない
DragableNoteBook::~DragableNoteBook() noexcept = default;


//
// クロック入力
//
void DragableNoteBook::clock_in()
{
    m_notebook_tab.clock_in();
}


void DragableNoteBook::set_show_tabs( bool show_tabs )
{
#ifdef _DEBUG
    std::cout << "DragableNoteBook::set_show_tabs show_tabs = " << show_tabs << std::endl;
#endif

    if( ! show_tabs && m_show_tabs ){

        remove( m_hbox_tab );

        m_show_tabs = false;
    }
    else if( show_tabs && ! m_show_tabs ){

        if( m_show_toolbar && m_notebook_toolbar.get_n_pages() )
            remove( m_notebook_toolbar );
        remove( m_notebook_view );

        pack_start( m_hbox_tab, Gtk::PACK_SHRINK );
        if( m_show_toolbar && m_notebook_toolbar.get_n_pages() )
            pack_start( m_notebook_toolbar, Gtk::PACK_SHRINK );
        pack_start( m_notebook_view );

        m_show_tabs = true;
    }
}


void DragableNoteBook::set_scrollable( bool scrollable )
{
    m_notebook_tab.set_scrollable( scrollable );
}


int DragableNoteBook::get_n_pages() const
{
    return m_notebook_view.get_n_pages();
}


Gtk::Widget* DragableNoteBook::get_nth_page( int page_num )
{
    return m_notebook_view.get_nth_page( page_num );
}


int DragableNoteBook::page_num( const Gtk::Widget& child ) const
{
    return m_notebook_view.page_num( child );
}


int DragableNoteBook::get_current_page() const
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

    SKELETON::TabLabel* tablabel = Gtk::manage( create_tablabel( url ) );
    return m_notebook_tab.append_tab( *tablabel );
}


int DragableNoteBook::insert_page( const std::string& url, Gtk::Widget& child, int page )
{
    m_notebook_view.insert_page( child, page );

    m_bt_tabswitch.show_button();

    SKELETON::TabLabel* tablabel = Gtk::manage( create_tablabel( url ) );
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
    // タブはGtk::manage()の効果でdeleteされる
    m_notebook_tab.remove_tab( page, adjust_tab );
    m_notebook_view.remove_page( page );

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


int DragableNoteBook::get_current_toolbar() const
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
void DragableNoteBook::update_toolbar_url( const std::string& url_old, const std::string& url_new )
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


/**
 * @brief ツールバーボタン表示更新
 */
void DragableNoteBook::reload_ui_icon()
{
    for( int i = 0; i < m_notebook_toolbar.get_n_pages(); ++i ){
        SKELETON::ToolBar* toolbar = get_toolbar( i );
        if( toolbar ) toolbar->reload_ui_icon();
    }
}


//
// タブのアイコンを取得する
//
int DragableNoteBook::get_tabicon( const int page )
{
    const SKELETON::TabLabel* tablabel = m_notebook_tab.get_tablabel( page );
    if( tablabel ) return tablabel->get_id_icon();

    return ICON::NONE;
}


//
// タブにアイコンをセットする
//
void DragableNoteBook::set_tabicon( const std::string& iconname, const int page, const int icon_id )
{
    SKELETON::TabLabel* tablabel = m_notebook_tab.get_tablabel( page );
    if( tablabel && icon_id != ICON::NONE ) tablabel->set_id_icon( icon_id );
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
void DragableNoteBook::slot_switch_page_tab( Gtk::Widget* bookpage, guint page )
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
    std::cout << "x = " << static_cast<int>(event->x_root) << " y = " << static_cast<int>(event->y_root)
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
    const int x = static_cast<int>(event->x_root);
    const int y = static_cast<int>(event->y_root);

#ifdef _DEBUG
    std::cout << "DragableNoteBook::on_button_release_event\n";
    std::cout << "x = " << static_cast<int>(event->x_root) << " y = " << static_cast<int>(event->y_root) << std::endl;
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



// notebook_tab の上でホイールを回した
bool DragableNoteBook::slot_scroll_event( GdkEventScroll* event )
{
#ifdef _DEBUG
    std::cout << "DragableNoteBook::slot_scroll_event direction = " << event->direction << " page = " << get_current_page() << std::endl;
#endif

    bool ret = false;
    int next_page = 0;

    if( event->direction == GDK_SCROLL_UP ) {
        next_page = get_current_page() - 1; // If negative, the last page will be used.
        ret = true;
    }
    else if( event->direction == GDK_SCROLL_DOWN ) {
        next_page = ( get_current_page() + 1 ) % get_n_pages();
        ret = true;
    }
    else if( event->direction == GDK_SCROLL_SMOOTH ) {
        constexpr double smooth_scroll_factor{ 4.0 };
        m_smooth_dy += smooth_scroll_factor * event->delta_y;
        if( m_smooth_dy < -1.0 ) {
            next_page = get_current_page() - 1; // If negative, the last page will be used.
            ret = true;
            m_smooth_dy = 0.0;
        }
        else if( m_smooth_dy > 1.0 ) {
            next_page = ( get_current_page() + 1 ) % get_n_pages();
            ret = true;
            m_smooth_dy = 0.0;
        }
    }

    if( ret ) {
        set_current_page( next_page );
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

        if( ! m_down_arrow ) m_down_arrow = std::make_unique<SKELETON::IconPopup>( ICON::DOWN );
        // HACK: サブウインドウにタブ機能がないためメインウインドウに決め打ちしている
        m_down_arrow->set_transient_for( *CORE::get_mainwindow() );
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
