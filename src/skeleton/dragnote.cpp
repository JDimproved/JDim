// ライセンス: 最新のGPL

//#define _DEBUG
#include "jddebug.h"

#include "dragnote.h"
#include "tablabel.h"

#include "controlid.h"

using namespace SKELETON;


DragableNoteBook::DragableNoteBook()
    : Gtk::Notebook()
    , m_page( -1 )
    , m_drag( 0 )
{
    add_events( Gdk::POINTER_MOTION_MASK );
    add_events( Gdk::LEAVE_NOTIFY_MASK );
}


//
// クロック入力
//
void DragableNoteBook::clock_in()
{
    m_tooltip.clock_in();
}


//
// フォーカスアウト
//
void DragableNoteBook::focus_out()
{
    m_tooltip.hide_tooltip();
}



// タブ取得
SKELETON::TabLabel* DragableNoteBook::get_tablabel( int page )
{
    return dynamic_cast< SKELETON::TabLabel* >( get_tab_label( *get_nth_page( page ) ) );
}


SKELETON::TabLabel* DragableNoteBook::get_tablabel( const std::string& url )
{
    for( int i = 0; i < get_n_pages() ; ++i ){
        SKELETON::TabLabel* tablabel = get_tablabel( i );
        if( tablabel && tablabel->get_url() == url ) return tablabel;
    }

    return NULL;
}


// マウスを動かした
bool DragableNoteBook::on_motion_notify_event( GdkEventMotion* event )
{
    SKELETON::TabLabel* tab;
    int page = get_page_under_mouse();

#ifdef _DEBUG
    std::cout << "DragableNoteBook::on_motion_notify_event page = " << page << std::endl;
#endif

    if( page >= 0 && page < get_n_pages() ){
        tab = get_tablabel( page );
        if( tab ) m_tooltip.set_text( tab->get_fulltext() );
    }
    else m_tooltip.hide_tooltip();

    return Gtk::Notebook::on_motion_notify_event( event );
}


// マウスが出た
bool DragableNoteBook::on_leave_notify_event( GdkEventCrossing* event )
{
    int page = get_page_under_mouse();

#ifdef _DEBUG
    std::cout << "DragableNoteBook::on_leave_notify_event page = " << page << std::endl;
#endif

    if( page < 0 || page >= get_n_pages() ) m_tooltip.hide_tooltip();

    return Gtk::Notebook::on_leave_notify_event( event );
}



//
// タブ幅調整
//
void DragableNoteBook::adjust_tabwidth()
{
#ifdef _DEBUG
    std::cout << "DragableNoteBook::adjust_tabwidth\n";
#endif

    const int mrg = 30;

    int width_notebook = get_width() - mrg;
    int pages = get_n_pages();

#ifdef _DEBUG
    std::cout << "width_notebook = " << width_notebook << " page = " << pages << std::endl;
#endif

    if( pages ){

        // タブ幅の平均値
        int avg_width_tab = width_notebook / MAX( 3, pages );

        // タブ幅が平均値をオーバーしているなら縮める
        for( int i = 0; i < pages; ++i ){

            SKELETON::TabLabel* tab = get_tablabel( i );
            if( tab ){

                for(;;){

                    int width = tab->get_tabwidth();
#ifdef _DEBUG
                    std::cout << "s " << i << " " << width << " / " << avg_width_tab << " " << tab->get_text() << std::endl;
#endif
                    if( width < avg_width_tab ) break;
                    if( ! tab->inc() ) break;
                }
            }
        }

        // タブ幅が平均値より短ければ伸ばす
        int width_total = 0;
        for( int i = 0; i < pages; ++i ){

            SKELETON::TabLabel* tab = get_tablabel( i );
            if( tab ){

                for(;;){

                    if( ! tab->dec() ) break;

                    int width = tab->get_tabwidth();
#ifdef _DEBUG
                    std::cout << "w " << i << " " << width << " / " << avg_width_tab << " " << tab->get_text() << std::endl;
#endif
                    // 最大値を越えたらひとつ戻してbreak;
                    if( width_total + width > avg_width_tab * ( i + 1 ) ){
                        tab->inc();
                        break;
                    }
                }

                width_total += tab->get_tabwidth();
            }
        }
    }
}



//
// D&D設定
//
void DragableNoteBook::set_dragable( bool dragable )
{
    if( dragable ){

        // クリックボタンの割り当て取得
        GdkEventButton event;
        if( ! m_control.get_eventbutton( CONTROL::ClickButton, event ) ) return;
        
        std::list< Gtk::TargetEntry > targets;
        targets.push_back( Gtk::TargetEntry( "text/plain", Gtk::TARGET_SAME_WIDGET, 0 ) );

        switch( event.button ){

            case 1: drag_source_set( targets, Gdk::BUTTON1_MASK ); break;
            case 2: drag_source_set( targets, Gdk::BUTTON2_MASK ); break;
            case 3: drag_source_set( targets, Gdk::BUTTON3_MASK ); break;

            default: return;
        }

        drag_dest_set( targets );
    }
    else{
        drag_source_unset();
        drag_dest_unset();
    }
}



bool DragableNoteBook::on_button_press_event( GdkEventButton* event )
{
    // ボタンを押した時点でのページ番号を記録しておく
    m_page = get_page_under_mouse();
    if( m_page >= get_n_pages() ) m_page = get_n_pages() -1;
    m_drag = false;

    // ダブルクリック
    m_dblclick = false;
    if( event->type == GDK_2BUTTON_PRESS ) m_dblclick = true; 

#ifdef _DEBUG
    std::cout << "DragableNoteBook::on_button_press_event from " << m_page  << std::endl;
    std::cout << "x = " << (int)event->x_root << " y = " << (int)event->y_root
              << " dblclick = " << m_dblclick << std::endl;
#endif
    
    return true;
}




bool DragableNoteBook::on_button_release_event( GdkEventButton* event )
{
    int x = (int)event->x_root;
    int y = (int)event->y_root;

#ifdef _DEBUG
    std::cout << "DragableNoteBook::on_button_release_event\n";
    std::cout << "x = " << (int)event->x_root << " y = " << (int)event->y_root << std::endl;
#endif

    // ページ切り替え

    if( !m_drag // D&D中は切替えない

        // なぜかviewをクリックしても呼ばれるので m_page の値で on_button_press_event が呼ばれたかチェック
        && m_page != -1 
        ){

        // ダブルクリックの処理のため一時的にtypeを切替える
        GdkEventType type_copy = event->type;
        if( m_dblclick ) event->type = GDK_2BUTTON_PRESS;

        // ページ切替え
        if( m_control.button_alloted( event, CONTROL::ClickButton ) ) set_current_page( m_page );

        // タブを閉じる
        else if( m_control.button_alloted( event, CONTROL::CloseTabButton ) ){
            m_sig_tab_close.emit( m_page );

            // タブにページが残ってなかったらそのままreturnしないと落ちる
            if( get_n_pages() == 0 ){
                m_page = -1;
                return false;
            }
        }

        // タブを再読み込み
        else if( m_control.button_alloted( event, CONTROL::ReloadTabButton ) ) m_sig_tab_reload.emit( m_page );

        // ポップアップメニュー
        else if( m_control.button_alloted( event, CONTROL::PopupmenuButton ) ) m_sig_tab_menu.emit( m_page, x, y );

        m_page = -1;
        event->type = type_copy;
    }

    return Gtk::Notebook::on_button_release_event( event );
}



void DragableNoteBook::on_drag_begin( const Glib::RefPtr< Gdk::DragContext >& context )
{
#ifdef _DEBUG
    std::cout << "DragableNoteBook::on_drag_begin \n";
#endif
    m_drag = true;
    m_sig_drag_begin.emit( m_page );
}


bool DragableNoteBook::on_drag_motion( const Glib::RefPtr<Gdk::DragContext>& context, int x, int y, guint time )
{
#ifdef _DEBUG
    std::cout << "DragableNoteBook::on_drag_motion\n";
#endif

    m_sig_drag_motion.emit();
    return Gtk::Notebook::on_drag_motion( context, x, y, time );
}



bool DragableNoteBook::on_drag_drop( const Glib::RefPtr<Gdk::DragContext>& context, int x, int y, guint time )
{
    int page = get_page_under_mouse();
    if( page >= get_n_pages() ) page = get_n_pages() -1;

#ifdef _DEBUG
    std::cout << "DragableNoteBook::on_drag_drop page = " << page << std::endl;
#endif

    if( m_drag ){

        // ドラッグ前とページが変わっていたら入れ替え
        if( m_page != -1 && page != -1 && m_page != page ){
            reorder_child( *get_nth_page( m_page ), page );
            m_page = -1;
        }
    }

    m_sig_drag_drop.emit( page );
    return Gtk::Notebook::on_drag_drop( context, x, y, time );
}



void DragableNoteBook::on_drag_end( const Glib::RefPtr< Gdk::DragContext >& context )
{
#ifdef _DEBUG
    std::cout << "DragableNoteBook::on_drag_end\n";
#endif

    m_sig_drag_end.emit();
    m_drag = false;
}


//
// マウスの下にあるタブの番号を取得
//
// タブ上で無いときは-1を返す
// タブの右側の場合は 最大タブ番号 + 1 を返す
//
int DragableNoteBook::get_page_under_mouse()
{
    int x, y;
    int width = 0, height = 0, i;

    for( i = 0; i < get_n_pages() ; ++i ){

        SKELETON::TabLabel* tab = get_tablabel( i );
        if( !tab ){
#ifdef _DEBUG
            std::cout << "DragableNoteBook::get_page_under_mouse: tab = NULL\n";
#endif
            return -1;
        }

        // HBoxは(x,y)座標が取得できないので、幅と高さとマウスの位置からタブの中に
        // マウスがあるかどうか判定する
        width = tab->get_allocation().get_width();
        height = tab->get_allocation().get_height();
        tab->get_pointer( x, y );

#ifdef _DEBUG
//        std::cout << "DragableNoteBook::get_page_under_mouse: x = " << x << " y = " << y
//                  << " xx = " << xx << " yy = " << yy << " w = " << width << " h = " << height << std::endl;
#endif

        if( x >= 0 && x <= width && y >= 0 && y <= height ) return i;
    }

    // 右の空白部分をクリック
    if( x > 0 ) return i;

    return -1;
}

