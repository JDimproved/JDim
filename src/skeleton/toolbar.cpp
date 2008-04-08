// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "toolbar.h"
#include "admin.h"
#include "view.h"
#include "imgbutton.h"
#include "imgtogglebutton.h"
#include "backforwardbutton.h"
#include "compentry.h"

#include "jdlib/miscutil.h"

#include "controlutil.h"
#include "controlid.h"

#include <list>

using namespace SKELETON;


ToolBar::ToolBar( Admin* admin )
    : m_admin( admin ),
      m_enable_slot( true ),
      m_toolbar_shown( false ),

      m_ebox_label( NULL ),
      m_label( NULL ),

      m_searchbar( NULL ),
      m_searchbar_shown( false ),
      m_button_open_searchbar( NULL ),
      m_button_close_searchbar( NULL ),

      m_entry_search( NULL ),
      m_button_up_search( NULL ),
      m_button_down_search( NULL ),

      m_button_write( NULL ),
      m_button_reload( NULL ),
      m_button_stop( NULL ),
      m_button_close( NULL ),
      m_button_delete( NULL ),
      m_button_favorite( NULL ),
      m_button_lock( NULL ),

      m_button_back( NULL ),
      m_button_forward( NULL )
{
    m_scrwin.add( m_buttonbar );
    m_scrwin.set_policy( Gtk::POLICY_NEVER, Gtk::POLICY_NEVER );
    m_scrwin.set_border_width( 0 );
    set_border_width( 0 );

    // ツールバーの枠を消す
    Gtk::Viewport* vport = dynamic_cast< Gtk::Viewport* >( m_scrwin.get_child() );
    if( vport ) vport->set_shadow_type( Gtk::SHADOW_NONE);

/*
    m_buttonbar.set_border_width( 0 );
    m_buttonbar.set_size_request( 1, -1 );
*/

    set_size_request( 8 );
}


void ToolBar::set_url( const std::string& url )
{
    m_url = url;

    if( m_button_back ) m_button_back->set_url( m_url );
    if( m_button_forward ) m_button_forward->set_url( m_url );
}


// タブが切り替わった時にDragableNoteBook::set_current_toolbar()から呼び出される( Viewの情報を取得する )
void ToolBar::set_view( SKELETON::View* view )
{
    if( ! view ) return;

    // slot関数を実行しない
    m_enable_slot = false;

    set_url( view->get_url() );

    update_label( view );
    update_close_button( view );
    if( m_entry_search ) m_entry_search->set_text( view->get_search_query() );

    m_enable_slot = true;
}


bool ToolBar::is_empty()
{
    return ( ! m_buttonbar.get_children().size() );
}


// ツールバーを表示
void ToolBar::show_toolbar()
{
    if( ! m_toolbar_shown ){

        if( m_searchbar && m_searchbar_shown ) remove( *m_searchbar );

        pack_start( m_scrwin, Gtk::PACK_SHRINK );
/*        pack_start( m_buttonbar, Gtk::PACK_SHRINK ); */
        if( m_searchbar && m_searchbar_shown ) pack_start( *m_searchbar, Gtk::PACK_SHRINK );

        show_all_children();
        m_toolbar_shown = true;
    }
}


// ツールバーを隠す
void ToolBar::hide_toolbar()
{
    if( m_toolbar_shown ){
        remove( m_scrwin ); 
/*        remove( m_buttonbar ); */
        show_all_children();
        m_toolbar_shown = false;
    }
}


// ラベル表示更新
void ToolBar::update_label( SKELETON::View* view )
{
    if( m_ebox_label && view && m_url == view->get_url() ){
        set_label( view->get_label() );
        if( view->is_broken() ) set_broken();
        if( view->is_old() ) set_old();
    }
}


// 閉じるボタンの表示更新
void ToolBar::update_close_button( SKELETON::View* view )
{
    if( m_button_close && view && m_url == view->get_url() ){

        // slot関数を実行しない
        m_enable_slot = false;

        if( view->is_locked() ) m_button_close->set_sensitive( false );
        else m_button_close->set_sensitive( true );

        if( m_button_lock ) m_button_lock->set_active( view->is_locked() );

        m_enable_slot = true;
    }
}


// ボタン表示更新
void ToolBar::update_button()
{
    bool empty = is_empty();

    unpack_buttons();
    pack_buttons();

    // ツールバーの中身が空の場合は
    // もう一度unpackとpackを繰り返さないと表示されないようだ
    if( empty ){
        unpack_buttons();
        pack_buttons();
    }

    // 進む、戻るボタンのsensitive状態を更新する
    set_url( m_url );
}


// ボタンのアンパック
void ToolBar::unpack_buttons()
{
    std::list< Gtk::Widget* > lists = m_buttonbar.get_children();
    std::list< Gtk::Widget* >::iterator it = lists.begin();
    for( ; it != lists.end(); ++it ){
        m_buttonbar.remove( *(*it) );
        if( dynamic_cast< Gtk::VSeparator* >( *it ) ) delete *it;
    }
}


// 区切り追加
void ToolBar::pack_separator()
{
    Gtk::VSeparator *sep = Gtk::manage( new Gtk::VSeparator() ); // delete は unpack_buttons() で行う
    m_buttonbar.pack_start( *sep, Gtk::PACK_SHRINK );
    sep->show();
}


//
// スレラベル
//
Gtk::EventBox* ToolBar::get_label()
{
    if( ! m_ebox_label ){

        m_ebox_label = Gtk::manage( new Gtk::EventBox );
        m_label = Gtk::manage( new Gtk::Label );

        m_label->set_size_request( 0, 0 );
        m_label->set_alignment( Gtk::ALIGN_LEFT );
        m_label->set_selectable( true );
        m_ebox_label->add( *m_label );
        m_ebox_label->set_visible_window( false );
    }

    return m_ebox_label;
}


void ToolBar::set_label( const std::string& label )
{
    if( ! m_ebox_label ) return;

    // ラベルの文字色と背景色を戻す
    if( m_ebox_label->get_visible_window() ){
        m_ebox_label->set_visible_window( false );
        m_label->unset_fg( Gtk::STATE_NORMAL );
    }

    m_label->set_text( label );
    set_tooltip( *m_ebox_label, label );
}


// viewが壊れている
void ToolBar::set_broken()
{
    if( ! m_ebox_label ) return;

#ifdef _DEBUG
    std::cout << "ToolBar::set_broken\n";
#endif

    m_ebox_label->set_visible_window( true );
    m_label->modify_fg( Gtk::STATE_NORMAL, Gdk::Color( "white" ) );
    m_ebox_label->modify_bg( Gtk::STATE_NORMAL, Gdk::Color( "red" ) );
    m_ebox_label->modify_bg( Gtk::STATE_ACTIVE, Gdk::Color( "red" ) );
}


// viewが古い
void ToolBar::set_old()
{
    if( ! m_ebox_label ) return;

#ifdef _DEBUG
    std::cout << "ToolBar::set_old\n";
#endif

    m_ebox_label->set_visible_window( true );
    m_label->modify_fg( Gtk::STATE_NORMAL, Gdk::Color( "white" ) );
    m_ebox_label->modify_bg( Gtk::STATE_NORMAL, Gdk::Color( "blue" ) );
    m_ebox_label->modify_bg( Gtk::STATE_ACTIVE, Gdk::Color( "blue" ) );
}



// 検索バー
Gtk::HBox* ToolBar::get_searchbar()
{
    if( ! m_searchbar ){
        m_searchbar = Gtk::manage( new Gtk::HBox() );
    }

    return m_searchbar;
}


// 検索バー表示
void ToolBar::open_searchbar()
{
    if( ! m_searchbar ) return;

#ifdef _DEBUG
    std::cout << "ToolBar::open_searchbar\n";
#endif

    if( ! m_searchbar_shown ){
        pack_start( *m_searchbar, Gtk::PACK_SHRINK );
        show_all_children();
        m_searchbar_shown = true;
    }
}


// 検索バー非表示
void ToolBar::close_searchbar()
{
    if( ! m_searchbar ) return;

#ifdef _DEBUG
    std::cout << "ToolBar::close_searchbar\n";
#endif

    if( m_searchbar_shown ){
        remove( *m_searchbar );
        show_all_children();
        m_searchbar_shown = false;
        m_admin->set_command( "focus_current_view" );
    }
}



//
// 検索バーを開く/閉じるボタン
//
SKELETON::ImgButton* ToolBar::get_button_open_searchbar()
{
    if( ! m_button_open_searchbar ){
        m_button_open_searchbar = Gtk::manage( new SKELETON::ImgButton( Gtk::Stock::FIND ) );

        std::string tooltip = "検索バーを開く  " + CONTROL::get_motion( CONTROL::Search );
        set_tooltip( *m_button_open_searchbar, tooltip );
        m_button_open_searchbar->signal_clicked().connect( sigc::mem_fun(*this, &ToolBar::slot_toggle_searchbar ) );
    }

    return m_button_open_searchbar;
}

SKELETON::ImgButton* ToolBar::get_button_close_searchbar()
{
    if( ! m_button_close_searchbar ){
        m_button_close_searchbar = Gtk::manage( new SKELETON::ImgButton( Gtk::Stock::UNDO ) );
        set_tooltip( *m_button_close_searchbar, CONTROL::get_label_motion( CONTROL::CloseSearchBar ) );
        m_button_close_searchbar->signal_clicked().connect( sigc::mem_fun(*this, &ToolBar::slot_toggle_searchbar ) );
    }

    return m_button_close_searchbar;
}


// 検索バー表示/非表示切り替え
void ToolBar::slot_toggle_searchbar()
{
    if( ! m_enable_slot ) return;
    if( ! m_searchbar ) return;

#ifdef _DEBUG
    std::cout << "ToolBar::slot_toggle_searchbar shown = " << m_searchbar_shown << std::endl;
#endif

    if( ! m_searchbar_shown ) m_admin->set_command( "open_searchbar", m_url );
    else m_admin->set_command( "close_searchbar", m_url );
}



//
// 検索 entry
//
SKELETON::SearchEntry* ToolBar::get_entry_search()
{
    if( ! m_entry_search ){
        m_entry_search = Gtk::manage( new SearchEntry() );

        m_entry_search->signal_changed().connect( sigc::mem_fun( *this, &ToolBar::slot_changed_search ) );
        m_entry_search->signal_activate().connect( sigc::mem_fun( *this, &ToolBar::slot_active_search ) );
        m_entry_search->signal_operate().connect( sigc::mem_fun( *this, &ToolBar::slot_operate_search ) );
    }

    return m_entry_search;
}


void ToolBar::slot_changed_search()
{
    if( ! m_enable_slot ) return;
    if( m_url.empty() || ! m_admin ) return;

    std::string query = m_entry_search->get_text();

#ifdef _DEBUG
    std::cout << "ToolBar::slot_changed_search query = " << query << std::endl;
#endif

    m_admin->set_command( "toolbar_set_search_query", m_url, query );
}


void ToolBar::slot_active_search()
{
    if( ! m_enable_slot ) return;
    if( m_url.empty() || ! m_admin ) return;

#ifdef _DEBUG
    std::cout << "ToolBar::slot_active_search\n";
#endif

    m_admin->set_command( "toolbar_exec_search", m_url );
}


void ToolBar::slot_operate_search( int controlid )
{
    if( ! m_enable_slot ) return;
    if( m_url.empty() || ! m_admin ) return;

#ifdef _DEBUG
    std::cout << "ToolBar::slot_operate_search id = " << controlid << std::endl;
#endif

    m_admin->set_command( "toolbar_operate_search", m_url, MISC::itostr( controlid ) );
}


// 検索 entry をフォーカス
void ToolBar::focus_entry_search()
{
    if( m_entry_search ) m_entry_search->grab_focus();
}



//
// 上検索
//
Gtk::Button* ToolBar::get_button_up_search()
{
    if( ! m_button_up_search ){
        m_button_up_search = Gtk::manage( new ImgButton( Gtk::Stock::GO_UP ) );
        set_tooltip( *m_button_up_search, CONTROL::get_label_motion( CONTROL::SearchPrev ) );

        m_button_up_search->signal_clicked().connect( sigc::mem_fun(*this, &ToolBar::slot_clicked_up_search ) );
    }

    return m_button_up_search;
}


void ToolBar::slot_clicked_up_search()
{
    if( ! m_enable_slot ) return;
    if( m_url.empty() || ! m_admin ) return;

#ifdef _DEBUG
    std::cout << "ToolBar::slot_clicked_up_search\n";
#endif

    m_admin->set_command( "toolbar_up_search", m_url );
}


//
// 下検索
//
Gtk::Button* ToolBar::get_button_down_search()
{
    if( ! m_button_down_search ){
        m_button_down_search = Gtk::manage( new ImgButton( Gtk::Stock::GO_DOWN ) );
        set_tooltip( *m_button_down_search, CONTROL::get_label_motion( CONTROL::SearchNext ) );

        m_button_down_search->signal_clicked().connect( sigc::mem_fun(*this, &ToolBar::slot_clicked_down_search ) );
    }

    return m_button_down_search;
}


void ToolBar::slot_clicked_down_search()
{
    if( ! m_enable_slot ) return;
    if( m_url.empty() || ! m_admin ) return;

#ifdef _DEBUG
    std::cout << "ToolBar::slot_clicked_down_search\n";
#endif

    m_admin->set_command( "toolbar_down_search", m_url );
}


//
// 書き込みボタン
//
Gtk::Button* ToolBar::get_button_write()
{
    if( ! m_button_write ){
        m_button_write = Gtk::manage( new ImgButton( ICON::WRITE ) );
        set_tooltip( *m_button_write, CONTROL::get_label_motion( CONTROL::WriteMessage ) );

        m_button_write->signal_clicked().connect( sigc::mem_fun(*this, &ToolBar::slot_clicked_write ) );
    }

    return m_button_write;
}


void ToolBar::slot_clicked_write()
{
    if( ! m_enable_slot ) return;
    if( m_url.empty() || ! m_admin ) return;

#ifdef _DEBUG
    std::cout << "ToolBar::slot_clicked_write\n";
#endif

    m_admin->set_command( "toolbar_write", m_url );
}


//
// 再読み込みボタン
//
Gtk::Button* ToolBar::get_button_reload()
{
    if( ! m_button_reload ){
        m_button_reload = Gtk::manage( new ImgButton( Gtk::Stock::REFRESH ) );
        set_tooltip( *m_button_reload, CONTROL::get_label_motion( CONTROL::Reload ) );

        m_button_reload->signal_clicked().connect( sigc::mem_fun(*this, &ToolBar::slot_clicked_reload ) );
    }

    return m_button_reload;
}


void ToolBar::slot_clicked_reload()
{
    if( ! m_enable_slot ) return;
    if( m_url.empty() || ! m_admin ) return;

#ifdef _DEBUG
    std::cout << "ToolBar::slot_clicked_reload\n";
#endif

    m_admin->set_command( "toolbar_reload", m_url );
}


//
// 読み込み停止ボタン
//
Gtk::Button* ToolBar::get_button_stop()
{
    if( ! m_button_stop ){
        m_button_stop = Gtk::manage( new ImgButton( Gtk::Stock::STOP ) );
        set_tooltip( *m_button_stop, CONTROL::get_label_motion( CONTROL::StopLoading ) );

        m_button_stop->signal_clicked().connect( sigc::mem_fun(*this, &ToolBar::slot_clicked_stop ) );
    }

    return m_button_stop;
}


void ToolBar::slot_clicked_stop()
{
    if( ! m_enable_slot ) return;
    if( m_url.empty() || ! m_admin ) return;

#ifdef _DEBUG
    std::cout << "ToolBar::slot_clicked_stop\n";
#endif

    m_admin->set_command( "toolbar_stop", m_url );
}


//
// 閉じるボタン
//
Gtk::Button* ToolBar::get_button_close()
{
    if( ! m_button_close ){
        m_button_close = Gtk::manage( new ImgButton( Gtk::Stock::CLOSE ) );
        set_tooltip( *m_button_close, CONTROL::get_label_motion( CONTROL::Quit ) );

        m_button_close->signal_clicked().connect( sigc::mem_fun(*this, &ToolBar::slot_clicked_close ) );
    }

    return m_button_close;
}


void ToolBar::slot_clicked_close()
{
    if( ! m_enable_slot ) return;
    if( m_url.empty() || ! m_admin ) return;

#ifdef _DEBUG
    std::cout << "ToolBar::slot_clicked_close\n";
#endif

    m_admin->set_command( "toolbar_close_view", m_url );
}


//
// 削除ボタン
//
Gtk::Button* ToolBar::get_button_delete()
{
    if( ! m_button_delete ){
        m_button_delete = Gtk::manage( new ImgButton( Gtk::Stock::DELETE ) );
        set_tooltip( *m_button_delete, CONTROL::get_label_motion( CONTROL::Delete ) );

        m_button_delete->signal_clicked().connect( sigc::mem_fun(*this, &ToolBar::slot_clicked_delete ) );
    }

    return m_button_delete;
}


void ToolBar::slot_clicked_delete()
{
    if( ! m_enable_slot ) return;
    if( m_url.empty() || ! m_admin ) return;

#ifdef _DEBUG
    std::cout << "ToolBar::slot_clicked_delete\n";
#endif

    m_admin->set_command( "toolbar_delete_view", m_url );
}


//
// お気に入りボタン
//
Gtk::Button* ToolBar::get_button_favorite()
{
    if( ! m_button_favorite ){
        m_button_favorite = Gtk::manage( new ImgButton( Gtk::Stock::COPY ) );
        set_tooltip( *m_button_favorite, CONTROL::get_label_motion( CONTROL::AppendFavorite ) );

        m_button_favorite->signal_clicked().connect( sigc::mem_fun(*this, &ToolBar::slot_clicked_favorite ) );
    }

    return m_button_favorite;
}


void ToolBar::slot_clicked_favorite()
{
    if( ! m_enable_slot ) return;
    if( m_url.empty() || ! m_admin ) return;

#ifdef _DEBUG
    std::cout << "ToolBar::slot_clicked_favorite\n";
#endif

    m_admin->set_command( "toolbar_set_favorite", m_url );
}


//
// 戻るボタン
//
Gtk::Button* ToolBar::get_button_back()
{
    if( ! m_button_back ){
        m_button_back = Gtk::manage( new SKELETON::BackForwardButton( m_url, true ) );
        set_tooltip( *m_button_back, CONTROL::get_label_motion( CONTROL::PrevView ) );

        m_button_back->signal_button_clicked().connect( sigc::mem_fun(*this, &ToolBar::slot_clicked_back ) );
        m_button_back->signal_selected().connect( sigc::mem_fun(*this, &ToolBar::slot_selected_back ) );
    }

    return m_button_back;
}


void ToolBar::slot_clicked_back()
{
    if( ! m_enable_slot ) return;
    if( m_url.empty() || ! m_admin ) return;

#ifdef _DEBUG
    std::cout << "ToolBar::slot_clicked_back : " << m_url << std::endl;
#endif

    m_admin->set_command( "back_viewhistory", m_url, "1" );
}


void ToolBar::slot_selected_back( const int i )
{
    if( ! m_enable_slot ) return;
    if( m_url.empty() || ! m_admin ) return;

#ifdef _DEBUG
    std::cout << "ToolBar::slot_selected_back : " << i << " url = " << m_url << std::endl;
#endif

    m_admin->set_command( "back_viewhistory", m_url, MISC::itostr( i+1 ) );
}


//
// 進むボタン
//
Gtk::Button* ToolBar::get_button_forward()
{
    if( ! m_button_forward ){
        m_button_forward = Gtk::manage( new SKELETON::BackForwardButton( m_url, false ) );
        set_tooltip( *m_button_forward, CONTROL::get_label_motion( CONTROL::NextView ) );

        m_button_forward->signal_button_clicked().connect( sigc::mem_fun(*this, &ToolBar::slot_clicked_forward ) );
        m_button_forward->signal_selected().connect( sigc::mem_fun(*this, &ToolBar::slot_selected_forward ) );
    }

    return m_button_forward;
}


void ToolBar::slot_clicked_forward()
{
    if( ! m_enable_slot ) return;
    if( m_url.empty() || ! m_admin ) return;

#ifdef _DEBUG
    std::cout << "ToolBar::slot_clicked_forward  : " << m_url << std::endl;
#endif

    m_admin->set_command( "forward_viewhistory", m_url, "1" );
}


void ToolBar::slot_selected_forward( const int i )
{
    if( ! m_enable_slot ) return;
    if( m_url.empty() || ! m_admin ) return;

#ifdef _DEBUG
    std::cout << "ToolBar::slot_selected_forward : " << i << " url = " << m_url << std::endl;
#endif

    m_admin->set_command( "forward_viewhistory", m_url, MISC::itostr( i+1 ) );
}


//
// ロックボタン
//
Gtk::ToggleButton* ToolBar::get_button_lock()
{
    if( ! m_button_lock ){
        m_button_lock = Gtk::manage( new SKELETON::ImgToggleButton( Gtk::Stock::NO ) );
        set_tooltip( *m_button_lock, CONTROL::get_label_motion( CONTROL::Lock ) );
        m_button_lock->signal_clicked().connect( sigc::mem_fun( *this, &ToolBar::slot_lock_clicked ) );
    }

    return m_button_lock;
}


void ToolBar::slot_lock_clicked()
{
    if( ! m_enable_slot ) return;
    m_admin->set_command( "toolbar_lock_view", get_url() );
}
