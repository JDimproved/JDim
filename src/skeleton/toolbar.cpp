// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "toolbar.h"
#include "admin.h"
#include "view.h"
#include "imgtoolbutton.h"
#include "imgtoggletoolbutton.h"
#include "menubutton.h"
#include "toolmenubutton.h"
#include "backforwardbutton.h"
#include "compentry.h"

#include "dbtree/interface.h"

#include "jdlib/miscutil.h"

#include "icons/iconmanager.h"

#include "config/globalconf.h"

#include "command.h"
#include "controlutil.h"
#include "controlid.h"

#include <gtk/gtk.h>  // gtk_separator_tool_item_set_draw
#include <gtk/gtkbutton.h>
#include <list>

using namespace SKELETON;


ToolBar::ToolBar( Admin* admin )
    : m_admin( admin ),
      m_enable_slot( true ),
      m_toolbar_shown( false ),

      m_tool_label( NULL ),
      m_ebox_label( NULL ),
      m_label( NULL ),

      m_searchbar( NULL ),
      m_searchbar_shown( false ),
      m_button_open_searchbar( NULL ),
      m_button_close_searchbar( NULL ),
      m_button_up_search( NULL ),
      m_button_down_search( NULL ),

      m_tool_search( NULL ),
      m_entry_search( NULL ),

      m_label_board( NULL ),
      m_button_board( NULL ),

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
    m_buttonbar.set_border_width( 0 );
#if GTKMMVER >= 2120
    m_buttonbar.set_icon_size( Gtk::ICON_SIZE_MENU );
#endif
    m_buttonbar.set_toolbar_style( Gtk::TOOLBAR_ICONS );
}


void ToolBar::set_url( const std::string& url )
{
    m_url = url;

    if( m_button_back ) m_button_back->get_backforward_button()->set_url( m_url );
    if( m_button_forward ) m_button_forward->get_backforward_button()->set_url( m_url );
}


// タブが切り替わった時にDragableNoteBook::set_current_toolbar()から呼び出される( Viewの情報を取得する )
// virtual
void ToolBar::set_view( SKELETON::View* view )
{
    if( ! view ) return;

    // slot関数を実行しない
    m_enable_slot = false;

    set_url( view->get_url() );

    // ラベル表示更新
    set_label( view->get_label() );
    if( view->is_broken() ) set_broken();
    if( view->is_old() ) set_old();

    // 閉じるボタンの表示更新
    if( m_button_close ){

        if( view->is_locked() ) m_button_close->set_sensitive( false );
        else m_button_close->set_sensitive( true );

        if( m_button_lock ) m_button_lock->set_active( view->is_locked() );
    }

    if( m_entry_search ) m_entry_search->set_text( view->get_search_query() );

    if( m_label_board ) m_label_board->set_text( DBTREE::board_name( get_url() ) );

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

        pack_start( m_buttonbar, Gtk::PACK_SHRINK );
        if( m_searchbar && m_searchbar_shown ) pack_start( *m_searchbar, Gtk::PACK_SHRINK );

        show_all_children();
        set_relief();
        m_toolbar_shown = true;
    }
}


// ツールバーを隠す
void ToolBar::hide_toolbar()
{
    if( m_toolbar_shown ){

        remove( m_buttonbar );
        show_all_children();
        m_toolbar_shown = false;
    }
}


// ボタン表示更新
void ToolBar::update_button()
{
    unpack_buttons();
    pack_buttons();

    if( m_buttonbar.get_children().size() ) show_toolbar();
    else hide_toolbar();

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
        if( dynamic_cast< Gtk::SeparatorToolItem* >( *it ) ) delete *it;
    }
}


// ボタンのrelief指定
void ToolBar::set_relief()
{
    std::list< Gtk::Widget* > lists_toolbar = get_children();
    std::list< Gtk::Widget* >::iterator it_toolbar = lists_toolbar.begin();
    for( ; it_toolbar != lists_toolbar.end(); ++it_toolbar ){

        Gtk::Toolbar* toolbar = dynamic_cast< Gtk::Toolbar* >( *it_toolbar );
        if( ! toolbar ) continue;

        std::list< Gtk::Widget* > lists = toolbar->get_children();
        std::list< Gtk::Widget* >::iterator it = lists.begin();
        for( ; it != lists.end(); ++it ){

            Gtk::Button* button = NULL;
            Gtk::ToolButton* toolbutton = dynamic_cast< Gtk::ToolButton* >( *it );
            if( toolbutton ) button = dynamic_cast< Gtk::Button* >( toolbutton->get_child() );
            if( ! button ){
                Gtk::ToolItem* toolitem = dynamic_cast< Gtk::ToolItem* >( *it );
                if( toolitem ) button = dynamic_cast< Gtk::Button* >( toolitem->get_child() );
            }
            if( button ){
                if( CONFIG::get_flat_button() ) button->set_relief( Gtk:: RELIEF_NONE );
                else button->set_relief( Gtk:: RELIEF_NORMAL );
            }
        }
    }
}


// 区切り追加
void ToolBar::pack_separator()
{
    Gtk::SeparatorToolItem *sep = Gtk::manage( new Gtk::SeparatorToolItem() ); // delete は unpack_buttons() で行う
    m_buttonbar.append( *sep );
}


// 透明区切り追加
void ToolBar::pack_transparent_separator()
{
    Gtk::SeparatorToolItem *sep = Gtk::manage( new Gtk::SeparatorToolItem() ); // delete は unpack_buttons() で行う
    gtk_separator_tool_item_set_draw( sep->gobj(), false );
    m_buttonbar.append( *sep );
}


//
// ツールチップ
//
void ToolBar::set_tooltip( Gtk::ToolItem& toolitem, const std::string& tip )
{
#if GTKMMVER < 2120
    toolitem.set_tooltip( m_tooltip, tip );
#else
    toolitem.set_tooltip_text( tip );
#endif
}


//
// ラベル
//
Gtk::ToolItem* ToolBar::get_label()
{
    if( ! m_tool_label ){

        m_tool_label = Gtk::manage( new Gtk::ToolItem );

        m_ebox_label = Gtk::manage( new Gtk::EventBox );
        m_label = Gtk::manage( new Gtk::Label );

        m_label->set_size_request( 0, 0 );
        m_label->set_alignment( Gtk::ALIGN_LEFT );
        m_label->set_selectable( true );

        m_ebox_label->add( *m_label );
        m_ebox_label->set_visible_window( false );

        m_tool_label->add( *m_ebox_label );
        m_tool_label->set_expand( true );
    }

    return m_tool_label;
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
    if( m_tool_label ) set_tooltip( *m_tool_label, label );
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
Gtk::Toolbar* ToolBar::get_searchbar()
{
    if( ! m_searchbar ){
        m_searchbar = Gtk::manage( new SKELETON::JDToolbar() );
#if GTKMMVER >= 2120
        m_searchbar->set_icon_size( Gtk::ICON_SIZE_MENU );
#endif
        m_searchbar->set_toolbar_style( Gtk::TOOLBAR_ICONS );
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
        set_relief();
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
Gtk::ToolItem* ToolBar::get_button_open_searchbar()
{
    if( ! m_button_open_searchbar ){
        m_button_open_searchbar = Gtk::manage( new SKELETON::ImgToolButton( Gtk::Stock::FIND ) );

        std::string tooltip = "検索バーを開く  " + CONTROL::get_motion( CONTROL::Search );
        set_tooltip( *m_button_open_searchbar, tooltip );
        m_button_open_searchbar->signal_clicked().connect( sigc::mem_fun(*this, &ToolBar::slot_toggle_searchbar ) );
    }

    return m_button_open_searchbar;
}

Gtk::ToolItem* ToolBar::get_button_close_searchbar()
{
    if( ! m_button_close_searchbar ){
        m_button_close_searchbar = Gtk::manage( new SKELETON::ImgToolButton( Gtk::Stock::UNDO ) );
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
Gtk::ToolItem* ToolBar::get_entry_search( const int mode )
{
    if( ! m_tool_search ){

        m_tool_search = Gtk::manage( new Gtk::ToolItem );
        m_entry_search = Gtk::manage( new CompletionEntry( mode ) );

        m_entry_search->signal_changed().connect( sigc::mem_fun( *this, &ToolBar::slot_changed_search ) );
        m_entry_search->signal_activate().connect( sigc::mem_fun( *this, &ToolBar::slot_active_search ) );
        m_entry_search->signal_operate().connect( sigc::mem_fun( *this, &ToolBar::slot_operate_search ) );

        m_tool_search->add( *m_entry_search );
        m_tool_search->set_expand( true );
    }

    return m_tool_search;
}


void ToolBar::add_search_mode( const int mode )
{
    if( m_entry_search ) m_entry_search->add_mode( mode );
}


const std::string ToolBar::get_search_text()
{
    if( ! m_entry_search ) return std::string();

    return m_entry_search->get_text();
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
Gtk::ToolItem* ToolBar::get_button_up_search()
{
    if( ! m_button_up_search ){
        m_button_up_search = Gtk::manage( new ImgToolButton( Gtk::Stock::GO_UP ) );
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
Gtk::ToolItem* ToolBar::get_button_down_search()
{
    if( ! m_button_down_search ){
        m_button_down_search = Gtk::manage( new ImgToolButton( Gtk::Stock::GO_DOWN ) );
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


// 板を開くボタン
Gtk::ToolItem* ToolBar::get_button_board()
{
    if( ! m_button_board ){

        m_label_board = Gtk::manage( new Gtk::Label );
        m_label_board->set_alignment( Gtk::ALIGN_LEFT );

        m_button_board = Gtk::manage( new SKELETON::ToolMenuButton( CONTROL::get_label( CONTROL::OpenParentBoard ), false, true, *m_label_board ) );

        std::vector< std::string > menu;
        menu.push_back( "開く" );
        menu.push_back( "再読み込みして開く" );
        m_button_board->get_button()->append_menu( menu );
        m_button_board->get_button()->signal_selected().connect( sigc::mem_fun(*this, &ToolBar::slot_menu_board ) );
        m_button_board->get_button()->signal_button_clicked().connect( sigc::mem_fun(*this, &ToolBar::slot_open_board ) );

        set_tooltip( *m_button_board, CONTROL::get_label_motion( CONTROL::OpenParentBoard ) );
    }

    return m_button_board;
}


void ToolBar::slot_open_board()
{
    if( ! m_enable_slot ) return;    

    CORE::core_set_command( "open_board", DBTREE::url_subject( get_url() ), "true",
                            "auto" // オートモードで開く
        );
}


void ToolBar::slot_menu_board( int i )
{
    if( ! m_enable_slot ) return;

    if( i == 0 ) slot_open_board();
    else if( i == 1 ) CORE::core_set_command( "open_board", DBTREE::url_subject( get_url() ), "true" );
}


//
// 書き込みボタン
//
Gtk::ToolItem* ToolBar::get_button_write()
{
    if( ! m_button_write ){
        m_button_write = Gtk::manage( new ImgToolButton( ICON::WRITE ) );
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
Gtk::ToolItem* ToolBar::get_button_reload()
{
    if( ! m_button_reload ){
        m_button_reload = Gtk::manage( new ImgToolButton( Gtk::Stock::REFRESH ) );
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
Gtk::ToolItem* ToolBar::get_button_stop()
{
    if( ! m_button_stop ){
        m_button_stop = Gtk::manage( new ImgToolButton( Gtk::Stock::STOP ) );
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
Gtk::ToolItem* ToolBar::get_button_close()
{
    if( ! m_button_close ){
        m_button_close = Gtk::manage( new ImgToolButton( Gtk::Stock::CLOSE ) );
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

    // relief が Gtk:: RELIEF_NONE のときにタブの最後のビューを閉じると、
    // ボタンに leave_notify イベントが送られないため、次にビューを開いたときに
    // 枠が残ったままになる
    //
    // gtk+-2.12.9/gtk/gtkbutton.c の gtk_button_leave_notify() をハックして
    // gtkbutton->in_button = false にすると枠が消えることが分かった
    if( m_admin->get_tab_nums() == 1 ){
        Gtk::Button* button = dynamic_cast< Gtk::Button* >( m_button_close->get_child() );
        GtkButton* gtkbutton = button->gobj();
        gtkbutton->in_button = false;
    }

    m_admin->set_command( "toolbar_close_view", m_url );
}


//
// 削除ボタン
//
Gtk::ToolItem* ToolBar::get_button_delete()
{
    if( ! m_button_delete ){
        m_button_delete = Gtk::manage( new ImgToolButton( Gtk::Stock::DELETE ) );
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
Gtk::ToolItem* ToolBar::get_button_favorite()
{
    if( ! m_button_favorite ){
        m_button_favorite = Gtk::manage( new ImgToolButton( Gtk::Stock::COPY ) );
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
Gtk::ToolItem* ToolBar::get_button_back()
{
    if( ! m_button_back ){

        m_button_back = Gtk::manage( new SKELETON::ToolBackForwardButton( "back", false, m_url, true ) );
        m_button_back->get_button()->signal_button_clicked().connect( sigc::mem_fun(*this, &ToolBar::slot_clicked_back ) );
        m_button_back->get_button()->signal_selected().connect( sigc::mem_fun(*this, &ToolBar::slot_selected_back ) );

        set_tooltip( *m_button_back, CONTROL::get_label_motion( CONTROL::PrevView ) );
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
Gtk::ToolItem* ToolBar::get_button_forward()
{
    if( ! m_button_forward ){

        m_button_forward = Gtk::manage( new SKELETON::ToolBackForwardButton( "forward", false, m_url, false ) );
        m_button_forward->get_button()->signal_button_clicked().connect( sigc::mem_fun(*this, &ToolBar::slot_clicked_forward ) );
        m_button_forward->get_button()->signal_selected().connect( sigc::mem_fun(*this, &ToolBar::slot_selected_forward ) );
        set_tooltip( *m_button_forward, CONTROL::get_label_motion( CONTROL::NextView ) );
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
Gtk::ToolItem* ToolBar::get_button_lock()
{
    if( ! m_button_lock ){
        m_button_lock = Gtk::manage( new SKELETON::ImgToggleToolButton( Gtk::Stock::NO ) );
        set_tooltip( *m_button_lock, CONTROL::get_label_motion( CONTROL::Lock ) );
        m_button_lock->set_label( CONTROL::get_label( CONTROL::Lock ) );
        m_button_lock->signal_clicked().connect( sigc::mem_fun( *this, &ToolBar::slot_lock_clicked ) );
    }

    return m_button_lock;
}


void ToolBar::slot_lock_clicked()
{
    if( ! m_enable_slot ) return;
    m_admin->set_command( "toolbar_lock_view", get_url() );
}
