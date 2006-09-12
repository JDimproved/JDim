// ライセンス: 最新のGPL

//#define _DEBUG
//#difine _DEBUG_RESIZE_TAB
#include "jddebug.h"

#include "dragnote.h"
#include "tablabel.h"

#include "icons/iconmanager.h"

#include "controlid.h"

using namespace SKELETON;


DragableNoteBook::DragableNoteBook()
    : Gtk::Notebook()
    , m_page( -1 )
    , m_drag( 0 )
    , m_dragable( false )
    , m_fixtab( false )
    , m_adjust_reserve( false )
    , m_pre_width( -1 )
{}


//
// クロック入力
//
void DragableNoteBook::clock_in()
{
    m_tooltip.clock_in();
    
    if( m_adjust_reserve ){
        m_adjust_reserve = false;
        m_adjust_reserve = ! adjust_tabwidth( false );
    }

    // Gtk::NoteBook は configure_event()をキャッチ出来ないので
    // 応急処置としてタイマーの中でサイズが変更したか調べる
    else if( m_pre_width > 0 ) m_pre_width = get_width();
}


//
// フォーカスアウト
//
void DragableNoteBook::focus_out()
{
    m_tooltip.hide_tooltip();
}


//
// ページのappendとinsert
//
// ついでにタブも作成する
//
int DragableNoteBook::append_page( const std::string& url, Gtk::Widget& child )
{
    SKELETON::TabLabel* tablabel = create_tablabel( url );
    return Gtk::Notebook::append_page( child , *tablabel );
}

int DragableNoteBook::insert_page( const std::string& url, Gtk::Widget& child, int page )
{
    SKELETON::TabLabel* tablabel = create_tablabel( url );
    return Gtk::Notebook::insert_page( child , *tablabel, page );
}



//
// タブの文字列取得
//
const std::string DragableNoteBook::get_tab_fulltext( int page )
{
    SKELETON::TabLabel* tablabel = get_tablabel( page );
    if( ! tablabel ) return std::string();

    return tablabel->get_fulltext();
}



//
// タブに文字列をセットとタブ幅調整
//
void DragableNoteBook::set_tab_fulltext( const std::string& str, int page )
{
    SKELETON::TabLabel* tablabel = get_tablabel( page );
    if( tablabel ){
        tablabel->set_fulltext( str );
        adjust_tabwidth( true );
    }
}



//
// ページ取り除き
//
// ついでにタブも削除する
//
void DragableNoteBook::remove_page( int page )
{
    SKELETON::TabLabel* tablabel = get_tablabel( page );
    Gtk::Notebook::remove_page( page );
    if( tablabel ) delete tablabel;
    m_tooltip.hide_tooltip();
    adjust_tabwidth( true );
}


//
// タブにアイコンをセットする
//
void DragableNoteBook::set_tabicon( const std::string& iconname, int page,
                                    int id_default, int id_update )
{
    SKELETON::TabLabel* tablabel = get_tablabel( page );
    if( tablabel ){

        int id = id_default;

        // タブが切り替わったときにAdmin::slot_switch_page から呼ばれる
        if( iconname == "switch_page" ){

            // update 状態以外の時はアイコンを変更しない
            if( tablabel->get_id_icon() != id_update ) return;
        }

        if( iconname == "loading" ) id = ICON::LOADING;
        if( iconname == "loading_stop" ) id = ICON::LOADING_STOP;
        if( iconname == "update" ){

            // タブがアクティブの時は通常アイコンを表示
            if( page != get_current_page() ) id = id_update;
        }

        tablabel->set_id_icon( id );
    }
}



// タブ作成
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
    tablabel->sig_tab_drag_drop().connect( sigc::mem_fun(*this, &DragableNoteBook::slot_drag_drop ) );
    tablabel->sig_tab_drag_end().connect( sigc::mem_fun(*this, &DragableNoteBook::slot_drag_end ) );
    
    return tablabel;
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




//
// マウスの下にあるタブの番号を取得
//
// タブ上で無いときは-1を返す
//
int DragableNoteBook::get_page_under_mouse()
{
    int page = 0;
    for( ; page < get_n_pages(); ++page ){

        SKELETON::TabLabel* tab = get_tablabel( page );
        if( !tab ) return -1;
        if( tab->is_under_mouse() ) break;
    }

    if( page >= get_n_pages() ) page = -1;

#ifdef _DEBUG
//    std::cout << "DragableNoteBook::get_page_under_mouse: page = " << page << std::endl;
#endif

    return page;
}




//
// タブ幅調整
//
bool DragableNoteBook::adjust_tabwidth( bool force )
{
    if( m_fixtab ) return false;

    const int mrg = 30;

    // 調整待ち
    if( m_adjust_reserve ) return false;

    int width_notebook = get_width();

    // 前回の呼び出し時とnotebookの幅が同じ時はまだraalize/resizeしていないということなので
    // 一旦returnしてクロック入力時に改めて adjust_tabwidth() を呼び出す
    if( ! force && width_notebook == m_pre_width ){
        m_adjust_reserve = true;
        return false;
    }

    m_pre_width = width_notebook;
    m_adjust_reserve = false;
    width_notebook -= mrg;
    int pages = get_n_pages();

    if( pages ){

        // タブ幅の平均値
        int avg_width_tab = width_notebook / MAX( 3, pages );

#ifdef _DEBUG_RESIZE_TAB
        std::cout << "DragableNoteBook::adjust_tabwidth\n";
        std::cout << "width_notebook = " << width_notebook << " page = " << pages << std::endl;
        std::cout << "avg_width_tab = " << avg_width_tab << std::endl;
#endif

        // タブ幅が平均値をオーバーしているなら縮める
        for( int i = 0; i < pages; ++i ){

            SKELETON::TabLabel* tab = get_tablabel( i );
            if( tab ){

                for(;;){

                    int width = tab->get_tabwidth();
#ifdef _DEBUG_RESIZE_TAB
                    std::cout << "s " << i << " " << width << " / " << avg_width_tab << " " << tab->get_text() << std::endl;
#endif
                    if( width < avg_width_tab ) break;
                    if( ! tab->dec() ) break;
                }
            }
        }

        // タブ幅が平均値より短ければ伸ばす
        int width_total = 0;
        for( int i = 0; i < pages; ++i ){

            SKELETON::TabLabel* tab = get_tablabel( i );
            if( tab ){

                for(;;){

                    if( ! tab->inc() ) break;

                    int width = tab->get_tabwidth();
#ifdef _DEBUG_RESIZE_TAB
                    std::cout << "w " << i << " " << width << " / " << avg_width_tab << " " << tab->get_text() << std::endl;
#endif
                    // 最大値を越えたらひとつ戻してbreak;
                    if( width_total + width > avg_width_tab * ( i + 1 ) ){
                        tab->dec();
                        break;
                    }
                }

                width_total += tab->get_tabwidth();
            }
        }
    }

    return true;
}



//
// ボタン押した
//
bool DragableNoteBook::on_button_press_event( GdkEventButton* event )
{
    // ボタンを押した時点でのページ番号を記録しておく
    m_page = get_page_under_mouse();
    m_drag = false;

    // ダブルクリック
    m_dblclick = false;
    if( event->type == GDK_2BUTTON_PRESS ) m_dblclick = true; 

#ifdef _DEBUG
    std::cout << "DragableNoteBook::on_button_press_event page = " << m_page  << std::endl;
    std::cout << "x = " << (int)event->x_root << " y = " << (int)event->y_root
              << " dblclick = " << m_dblclick << std::endl;
#endif

    // ページはon_button_release_eventの中で自前で切り替える
    if( m_page >= 0 && m_page < get_n_pages() ) return true;

    return Gtk::Notebook::on_button_press_event( event );
}


//
// ボタン離した
//
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
        && m_page >= 0 && m_page < get_n_pages()

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



//
// タブの中でマウスを動かした
//
void DragableNoteBook::slot_motion_event()
{
    SKELETON::TabLabel* tab;
    int page = get_page_under_mouse();

#ifdef _DEBUG
//    std::cout << "DragableNoteBook::slot_motion_event page = " << page << std::endl;
#endif

    // ツールチップにテキストをセット
    if( page >= 0 ){
        tab = get_tablabel( page );
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
// タブにドロップされた
//
void DragableNoteBook::slot_drag_drop()
{
    int page = get_page_under_mouse();

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
}


//
// タブのドラッグを終了
//
void DragableNoteBook::slot_drag_end()
{
#ifdef _DEBUG
    std::cout << "DragableNoteBook::on_drag_end\n";
#endif

    m_sig_drag_end.emit();

    m_drag = false;
}
