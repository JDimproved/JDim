// ライセンス: GPL2

//#define _DEBUG
//#define _DEBUG_RESIZE_TAB
#include "jddebug.h"

#include "dragnote.h"
#include "tabnote.h"
#include "tablabel.h"

#include "config/globalconf.h"

#include "session.h"

#include <gtk/gtk.h>


//////////////////////////////////////////

// gtknotebook.c( Revision 19311, 2008-01-06 ) より引用

struct _GtkNotebookPage
{
  GtkWidget *child;
  GtkWidget *tab_label;
  GtkWidget *menu_label;
  GtkWidget *last_focus_child;	/* Last descendant of the page that had focus */

  guint default_menu : 1;	/* If true, we create the menu label ourself */
  guint default_tab  : 1;	/* If true, we create the tab label ourself */
  guint expand       : 1;
  guint fill         : 1;
  guint pack         : 1;
  guint reorderable  : 1;
  guint detachable   : 1;

  GtkRequisition requisition;
  GtkAllocation allocation;

  gulong mnemonic_activate_signal;
  gulong notify_visible_handler;
};

//////////////////////////////////////////

using namespace SKELETON;


class DummyWidget : public Gtk::Widget
{
public:
    DummyWidget() : Gtk::Widget(){ set_flags(Gtk::NO_WINDOW); }
    virtual ~DummyWidget(){}
};


//////////////////////////////////////////////


TabNotebook::TabNotebook( DragableNoteBook* parent )
    : Gtk::Notebook(),
      m_parent( parent ),
      m_fixtab( false )
{
    m_layout_tab = create_pango_layout( "" );

    set_border_width( 0 );
    set_size_request( 1, -1 ); // これが無いと最大化を解除したときにウィンドウが勝手にリサイズする

    add_events( Gdk::POINTER_MOTION_MASK );
    add_events( Gdk::LEAVE_NOTIFY_MASK );

    // DnD設定
    // ドロップ側に設定する
    drag_source_unset();
    drag_dest_unset();
    std::list< Gtk::TargetEntry > targets;
    targets.push_back( Gtk::TargetEntry( "text/plain", Gtk::TARGET_SAME_APP, 0 ) );
    drag_dest_set( targets, Gtk::DEST_DEFAULT_MOTION | Gtk::DEST_DEFAULT_DROP );

    Glib::RefPtr< Gtk::RcStyle > rcst = get_modifier_style();
    Glib::RefPtr< Gtk::Style > st = get_style();
    int ythickness = rcst->get_ythickness();
    if( ythickness <= 0 ) ythickness = st->get_ythickness();

#ifdef _DEBUG
    std::cout << "TabNotebook::TabNotebook ythick = " << ythickness << std::endl;
#endif

    if( ythickness > 0 ){
        rcst->set_ythickness( 0 );
        modify_style( rcst );
        property_tab_vborder() = property_tab_vborder() + ythickness * 2;
    }

    m_tab_mrg = rcst->get_xthickness();
    if( m_tab_mrg <= 0 ) m_tab_mrg = st->get_xthickness();

    m_pre_width = get_width();
}

//
// クロック入力
//
void TabNotebook::clock_in()
{
    // Gtk::NoteBook は configure_event()をキャッチ出来ないので
    // 応急処置としてタイマーの中でサイズが変更したか調べて
    // 変わっていたらタブ幅を調整する
    if( ! m_fixtab && m_pre_width != get_width() ) adjust_tabwidth();
}


int TabNotebook::append_tab( Widget& tab )
{
#ifdef _DEBUG
    std::cout << "TabNotebook::append_tab\n";
#endif

    // ダミーWidgetを作成してtabにappend (表示はされない )
    // remve_tab()でdeleteする
    DummyWidget* dummypage = new DummyWidget();

    return append_page( *dummypage , tab );
}


int TabNotebook::insert_tab( Widget& tab, int page )
{
#ifdef _DEBUG
    std::cout << "TabNotebook::insert_tab position = "  << page << std::endl;
#endif

    // ダミーWidgetを作成してtabにappend (表示はされない )
    // remve_page()でdeleteする
    DummyWidget* dummypage = new DummyWidget();

    return insert_page( *dummypage, tab, page );
}


void TabNotebook::remove_tab( int page )
{
#ifdef _DEBUG
    std::cout << "TabNotebook::remove_tab page = " << page << std::endl;
#endif

    // ダミーWidgetをdelete
    Gtk::Widget* dummypage = get_nth_page( page );
    remove_page( page );

    if( dummypage ) delete dummypage;

    adjust_tabwidth();
}


void TabNotebook::reorder_child( int page1, int page2 )
{
    Gtk::Notebook::reorder_child( *get_nth_page( page1 ), page2 );
}


// タブ取得
SKELETON::TabLabel* TabNotebook::get_tablabel( int page )
{
    return dynamic_cast< SKELETON::TabLabel* >( get_tab_label( *get_nth_page( page ) ) );
}


//
// マウスの下にあるタブの番号を取得
//
// タブ上では無いときは-1を返す
// マウスがタブの右側にある場合はページ数( get_n_pages() )を返す
//
const int TabNotebook::get_page_under_mouse()
{
    int x, y;
    Gdk::Rectangle rect = get_allocation();
    get_pointer( x, y );
    x += rect.get_x();
    y += rect.get_y();

#ifdef _DEBUG
    std::cout << "TabNotebook::get_page_under_mouse x = " << x << " y = " << y << std::endl;
#endif

    if( y < rect.get_y() || y > rect.get_y() + rect.get_height() ) return -1;

    calc_tabsize();

    const int pages = get_n_pages();
    int ret = pages;
    for( int i = 0; i < pages; ++i ){

        SKELETON::TabLabel* tab = get_tablabel( i );
        if( tab ){

            int tab_x = tab->get_tab_x();
            int tab_w = tab->get_tab_width();

            if( tab_x < 0 ) continue;

#ifdef _DEBUG
            std::cout << "page = " << i << " x = " << tab_x << " w = " << tab_w << std::endl;
#endif

            if( x < tab_x ){
                ret = -1;
                break;
            }
            if( x >= tab_x && x <= tab_x + tab_w ){
                ret = i;
                break;
            }
        }
    }

#ifdef _DEBUG
    std::cout << "ret = " << ret << std::endl;
#endif

    return ret;
}


//
// タブの文字列取得
//
const std::string TabNotebook::get_tab_fulltext( int page )
{
    SKELETON::TabLabel* tablabel = get_tablabel( page );
    if( ! tablabel ) return std::string();

    return tablabel->get_fulltext();
}



//
// タブに文字列をセットとタブ幅調整
//
void TabNotebook::set_tab_fulltext( const std::string& str, int page )
{
#ifdef _DEBUG
    std::cout << "TabNotebook::set_tab_fulltext page = " << page << " " << str << std::endl;
#endif

    SKELETON::TabLabel* tablabel = get_tablabel( page );
    if( tablabel ){
        tablabel->set_fulltext( str );
        if( m_fixtab ) tablabel->resize_tab( str.length() );
        else adjust_tabwidth();
    }
}


//
// 各タブのサイズと座標を取得
//
void TabNotebook::calc_tabsize()
{
#ifdef _DEBUG
    std::cout << "TabNotebook::calc_tabsize\n";
#endif

    Gdk::Rectangle rect = get_allocation();
    int pre_x = rect.get_x();

    bool first_tab = true;
    m_tab_mrg = 0;
    const int pages = get_n_pages();
    for( int i = 0; i < pages; ++i ){

        SKELETON::TabLabel* tab = get_tablabel( i );
        if( tab ){

            int tab_x = -1;
            int tab_y = -1;
            int tab_w = -1;
            int tab_h = -1;

            if( tab->is_mapped() ){

                rect = tab->get_allocation();
                tab_x = rect.get_x();
                tab_y = rect.get_y();
                tab_w = rect.get_width();
                tab_h = rect.get_height();

                if( ! first_tab ) m_tab_mrg = tab_x - pre_x;
                else m_tab_mrg = 0; // 最初のタブはマージン計算をしない
                first_tab = false;

                pre_x = tab_x + tab_w;
                tab_x -= m_tab_mrg;
                tab_w += m_tab_mrg * 2;
            }

#ifdef _DEBUG
            std::cout << "page = " << i << " x = " << tab_x << " w = " << tab_w << " mrg = " << m_tab_mrg << std::endl;
#endif
            tab->set_tab_x( tab_x );
            tab->set_tab_y( tab_y );
            tab->set_tab_width( tab_w );
            tab->set_tab_height( tab_h );
        }
    }
}


//
// タブ幅調整
//
bool TabNotebook::adjust_tabwidth()
{
    // 起動中とシャットダウン中は処理しない
    if( SESSION::is_booting() ) return false;
    if( SESSION::is_quitting() ) return false;

    const int mrg_notebook = 30;

    const int pages = get_n_pages();
    if( ! pages ) return false;

    // layoutにラベルのフォントをセットする
    SKELETON::TabLabel* tab = get_tablabel( 0 );
    if( ! tab ) return false;
    m_layout_tab->set_font_description( tab->get_label_font_description() );

    std::vector< int > vec_width; // 変更後のタブの文字数
    vec_width.resize( pages );

    m_pre_width = get_width();
    const int width_notebook = m_pre_width - mrg_notebook;

    const int avg_width_tab = (int)( (double)width_notebook / MAX( 3, pages+0.5 ) );  // タブ幅の平均値

#ifdef _DEBUG_RESIZE_TAB
    std::cout << "TabNotebook::adjust_tabwidth\n"
              << "width_notebook = " << width_notebook << " page = " << pages << std::endl
              << "avg_width_tab = " << avg_width_tab
              << " tab_mrg = " << m_tab_mrg
              << std::endl;
#endif

    // 一端、全てのタブの幅を平均値以下に縮める
    for( int i = 0; i < pages; ++i ){

        tab = get_tablabel( i );
        if( tab ){

            Glib::ustring ulabel( tab->get_fulltext() );
            vec_width[ i ] = ulabel.length();

            while( vec_width[ i ] > CONFIG::get_tab_min_str() ){

                m_layout_tab->set_text( ulabel.substr( 0,  vec_width[ i ] ) );
                int width = m_layout_tab->get_pixel_ink_extents().get_width() + tab->get_image_width() + m_tab_mrg *2;

#ifdef _DEBUG_RESIZE_TAB
                std::cout << "s " << i << " " << width << " / " << avg_width_tab
                          << " lng = " << vec_width[ i ] << " : " << ulabel.substr( 0, vec_width[ i ] ) << std::endl;
#endif

                if( width < avg_width_tab ) break;
                --vec_width[ i ];
                if( vec_width[ i ] < 0 ) vec_width[ i ] = 0;
            }
        }
    }

    // 横をはみださないようにタブ幅を延ばしていく
    int width_total = 0;
    for( int i = 0; i < pages; ++i ){

        SKELETON::TabLabel* tab = get_tablabel( i );
        if( tab ){

            Glib::ustring ulabel( tab->get_fulltext() );
            int lng_max = ulabel.length();
            if( ! lng_max ) continue;

            for(;;){

                if( vec_width[ i ] >= lng_max ) break;

                ++vec_width[ i ];

                m_layout_tab->set_text( ulabel.substr( 0,  vec_width[ i ] ) );
                int width = m_layout_tab->get_pixel_ink_extents().get_width() + tab->get_image_width() + m_tab_mrg *2;

#ifdef _DEBUG_RESIZE_TAB
                std::cout << "w " << i << " " << width << " / " << avg_width_tab
                          << " total= " << width_total + width << " / " << avg_width_tab * ( i + 1 )
                          << " lng = " << vec_width[ i ] << " : " << ulabel.substr( 0, vec_width[ i ] ) << std::endl;
#endif
                // 最大値を越えたらひとつ戻してbreak;
                if( width_total + width > avg_width_tab * ( i + 1 ) ){
                    --vec_width[ i ];
                    break;
                }
            }

            m_layout_tab->set_text( ulabel.substr( 0,  vec_width[ i ] ) );
            width_total += ( m_layout_tab->get_pixel_ink_extents().get_width() + tab->get_image_width() + m_tab_mrg *2 );

            tab->resize_tab( vec_width[ i ] );
        }
    }

    return true;
}


// タブのgap位置を取得 ( 描画用 )
void TabNotebook::get_gap( int& x, int& width )
{
    x = 0;
    width = 0;

    GtkNotebook *notebook = gobj();
    if( notebook && notebook->cur_page ){

        const int bw = get_border_width();
        const int xx = get_allocation().get_x() + bw;

        x = notebook->cur_page->allocation.x - xx;
        width = notebook->cur_page->allocation.width;
    }
}



//
// 描画イベント
//
// gtknotebook.c( Revision 19593, Sat Feb 16 04:09:15 2008 UTC ) からのハック。環境やバージョンによっては問題が出るかもしれないので注意
//
bool TabNotebook::on_expose_event( GdkEventExpose* event )
{
    GtkNotebook *notebook = gobj();
    GtkWidget *widget = GTK_WIDGET( notebook );
    const Heights_NoteBook heights = m_parent->get_heights_notebook();
    if( ! notebook || ! notebook->cur_page ) return Notebook::on_expose_event( event );

#ifdef _DEBUG
    std::cout << "TabNotebook::on_expose_event\n";
#endif

    // 一時的にページ高さをごまかして描画する
    const int height_page = notebook->cur_page->allocation.height;
    const int height_widget = widget->allocation.height;
    widget->allocation.height = heights.height_tab + heights.height_toolbar + heights.height_view;
    notebook->cur_page->allocation.height = heights.height_toolbar + heights.height_view;
    bool ret = Notebook::on_expose_event( event );
    widget->allocation.height = height_widget;
    notebook->cur_page->allocation.height = height_page;

    return ret;
}



// signal_button_press_event と signal_button_release_event は emit されない
// ときがあるので自前でemitする
bool TabNotebook::on_button_press_event( GdkEventButton* event )
{
#ifdef _DEBUG
    std::cout << "TabNotebook::on_button_press_event\n";
#endif

    if( m_sig_button_press.emit( event ) ) return true;

#ifdef _DEBUG
    std::cout << "Gtk::Notebook::on_button_press_event\n";
#endif

    return Gtk::Notebook::on_button_press_event( event );
}


bool TabNotebook::on_button_release_event( GdkEventButton* event )
{
#ifdef _DEBUG
    std::cout << "TabNotebook::on_button_release_event\n";
#endif

    if( m_sig_button_release.emit( event ) ) return true;

#ifdef _DEBUG
    std::cout << "Gtk::Notebook::on_button_release_event\n";
#endif

    return Gtk::Notebook::on_button_release_event( event );
}


//
// マウスが動いた
//
bool TabNotebook::on_motion_notify_event( GdkEventMotion* event )
{
#ifdef _DEBUG
    std::cout << "TabNotebook::on_motion_notify_event\n";
#endif

    m_sig_tab_motion_event.emit();

    return Gtk::Notebook::on_motion_notify_event( event );
}


//
// マウスが出た
//
bool TabNotebook::on_leave_notify_event( GdkEventCrossing* event )
{
#ifdef _DEBUG
    std::cout << "TabNotebook::leave\n";
#endif

    m_sig_tab_leave_event.emit();

    return Gtk::Notebook::on_leave_notify_event( event );
}


//
// ドラッグ中にマウスを動かした
//
bool TabNotebook::on_drag_motion( const Glib::RefPtr<Gdk::DragContext>& context, int x, int y, guint time)
{
#ifdef _DEBUG
    std::cout << "Gtk::Notebook::on_drag_motion x = " << x << " y = " << y << std::endl;
#endif

    int tab_x = -1;
    int tab_y = -1;
    int tab_w = -1;

    int page = get_page_under_mouse();
    if( page >= 0 ){

        if( page >= get_n_pages() ) page = get_n_pages() -1;

        SKELETON::TabLabel* tab = get_tablabel( page );
        if( tab ){

            tab_x = tab->get_tab_x();
            tab_y = tab->get_tab_y();
            tab_w = tab->get_tab_width();

#ifdef _DEBUG
            std::cout << "page = " << page
                      << " tab_x = " << tab_x << " tab_y = " << tab_y << " tab_w " << tab_w << std::endl;
#endif
        }
    }

    m_sig_tab_drag_motion( page, tab_x, tab_y, tab_w );

    // on_drag_motion をキャンセルしないとDnD中にタブが勝手に切り替わる( gtknotebook.c をハック )
    return true;
}


/////////////////////////////////////////////////////////////////


ToolBarNotebook::ToolBarNotebook( DragableNoteBook* parent )
    : Gtk::Notebook(),
      m_parent( parent ),
      m_show_tab_notebook( true )
{
    set_show_border( true );
    set_show_tabs( false );
    set_border_width( 0 );
}


// タブを表示しない場合は枠の上側も描画する
void ToolBarNotebook::set_show_tab_notebook( const bool show )
{
    m_show_tab_notebook = show;
}


//
// 描画イベント
//
// 自前で枠を描画する
//
// gtknotebook.c( Revision 19593, Sat Feb 16 04:09:15 2008 UTC ) からのハック。環境やバージョンによっては問題が出るかもしれないので注意
//
bool ToolBarNotebook::on_expose_event( GdkEventExpose* event )
{
    const Glib::RefPtr<Gdk::Window> win = get_window();
    const Gdk::Rectangle rect( &(event->area) );
    const Heights_NoteBook heights = m_parent->get_heights_notebook();

    const int x = get_allocation().get_x();
    const int y = get_allocation().get_y();
    const int w = get_allocation().get_width();
    const int h = heights.height_toolbar + heights.height_view;
                                              
#ifdef _DEBUG
    std::cout << "ToolBarNotebook::on_expose_event\n"
              << "x = " << x
              << " y = " << y
              << " w = " << w
              << " h = " << h
              << std::endl;
#endif

    if( h > 0 ){

        get_style()->paint_box_gap( win,
                                    Gtk::STATE_NORMAL,
                                    Gtk::SHADOW_OUT,
                                    rect,
                                    *this,
                                    "notebook",
                                    x,
                                    y,
                                    w,
                                    h,
                                    Gtk::POS_TOP,
                                    heights.gap_x,
                                    heights.gap_width
            );

    }

    // 枠は自前で書いたので gtk_notebook_expose では枠を描画させない
    GtkNotebook *notebook = gobj();
    notebook->show_border = false;
    bool ret = Notebook::on_expose_event( event );
    notebook->show_border = true;

    return ret;
}


/////////////////////////////////////////////////////////////////


ViewNotebook::ViewNotebook( DragableNoteBook* parent )
    : Gtk::Notebook(),
      m_parent( parent )
{
    set_show_border( true );
    set_show_tabs( false );
    set_border_width( 0 );
}


//
// 描画イベント
//
// 自前で枠を描画する
//
// gtknotebook.c( Revision 19593, Sat Feb 16 04:09:15 2008 UTC ) からのハック。環境やバージョンによっては問題が出るかもしれないので注意
//
bool ViewNotebook::on_expose_event( GdkEventExpose* event )
{
    const Glib::RefPtr<Gdk::Window> win = get_window();
    const Gdk::Rectangle rect( &(event->area) );
    const Heights_NoteBook heights = m_parent->get_heights_notebook();

    const int x = get_allocation().get_x();
    const int y = get_allocation().get_y() - heights.height_toolbar;
    const int w = get_allocation().get_width();
    const int h = heights.height_toolbar + heights.height_view;
                                              
#ifdef _DEBUG
    std::cout << "ToolBarNotebook::on_expose_event\n"
              << "x = " << x
              << " y = " << y
              << " w = " << w
              << " h = " << h
              << std::endl;
#endif

    if( h > 0 ){

        get_style()->paint_box_gap( win,
                                    Gtk::STATE_NORMAL,
                                    Gtk::SHADOW_OUT,
                                    rect,
                                    *this,
                                    "notebook",
                                    x,
                                    y,
                                    w,
                                    h,
                                    Gtk::POS_TOP,
                                    heights.gap_x,
                                    heights.gap_width
            );

    }

    // 枠は自前で書いたので gtk_notebook_expose では枠を描画させない
    GtkNotebook *notebook = gobj();
    notebook->show_border = false;
    bool ret = Notebook::on_expose_event( event );
    notebook->show_border = true;

    return ret;
}
