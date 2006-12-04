// ライセンス: GPL2

//#define _DEBUG
//#difine _DEBUG_RESIZE_TAB
#include "jddebug.h"

#include "dragnote.h"
#include "tablabel.h"

#include "icons/iconmanager.h"

#include "config/globalconf.h"

#include "controlid.h"

using namespace SKELETON;


DragableNoteBook::DragableNoteBook()
    : Gtk::Notebook()
    , m_page( -1 )
    , m_drag( 0 )
    , m_dragable( false )
    , m_fixtab( false )
{
    m_layout_tab = create_pango_layout( "" );

    m_pre_width = get_width();
}


//
// クロック入力
//
void DragableNoteBook::clock_in()
{
    m_tooltip.clock_in();
 
    // Gtk::NoteBook は configure_event()をキャッチ出来ないので
    // 応急処置としてタイマーの中でサイズが変更したか調べて
    // 変わっていたらタブ幅を調整する
    if( ! m_fixtab && m_pre_width != get_width() ) adjust_tabwidth();
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
        if( m_fixtab ) tablabel->resize_tab( str.length() );
        else adjust_tabwidth();
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
    adjust_tabwidth();
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
bool DragableNoteBook::adjust_tabwidth()
{
    const int mrg_notebook = 30;

    if( m_fixtab ) return false;

    int pages = get_n_pages();
    if( ! pages ) return false;

    // layoutにラベルのフォントをセットする
    SKELETON::TabLabel* tab = get_tablabel( 0 );
    if( ! tab ) return false;
    m_layout_tab->set_font_description( tab->get_label_font_description() );
    int mrg_tab = tab->get_margin();

    std::vector< int > vec_width; // 変更後のタブ幅
    vec_width.resize( pages );

    int width_notebook = get_width();
    m_pre_width = width_notebook;
    width_notebook -= mrg_notebook;

    int avg_width_tab = width_notebook / MAX( 3, pages );  // タブ幅の平均値

#ifdef _DEBUG_RESIZE_TAB
    std::cout << "DragableNoteBook::adjust_tabwidth\n"
              << "width_notebook = " << width_notebook
              << " page = " << pages << std::endl
              << "avg_width_tab = " << avg_width_tab << std::endl;
#endif

    // 一端、全てのタブの幅を平均値以下に縮める
    for( int i = 0; i < pages; ++i ){

        tab = get_tablabel( i );
        if( tab ){

            Glib::ustring ulabel( tab->get_fulltext() );
            vec_width[ i ] = ulabel.length();

            while( vec_width[ i ] > CONFIG::get_tab_min_str() ){

                m_layout_tab->set_text( ulabel.substr( 0,  vec_width[ i ] ) );
                int width = m_layout_tab->get_pixel_ink_extents().get_width() + tab->get_image_width() + mrg_tab;

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
                int width = m_layout_tab->get_pixel_ink_extents().get_width() + tab->get_image_width() + mrg_tab;

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
            width_total += ( m_layout_tab->get_pixel_ink_extents().get_width() + tab->get_image_width() + mrg_tab );

            tab->resize_tab( vec_width[ i ] );
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
