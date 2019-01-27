// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"
#include "gtkmmversion.h"

#include "toolbar.h"
#include "bbslistadmin.h"

#include "skeleton/view.h"
#include "skeleton/menubutton.h"
#include "skeleton/imgtoolbutton.h"

#include "control/controlutil.h"
#include "control/controlid.h"

#include "icons/iconmanager.h"

#include "config/globalconf.h"

#include "command.h"
#include "session.h"
#include "compmanager.h"
#include "global.h"

using namespace BBSLIST;


BBSListToolBar::BBSListToolBar() :
    SKELETON::ToolBar( BBSLIST::get_admin() ),
    m_button_toggle( "ページ切り替え", true, true, m_label ),
    m_button_check_update_root( NULL ),
    m_button_check_update_open_root( NULL ),
    m_button_stop_check_update( NULL )
{
    m_button_toggle.get_button()->set_tooltip_arrow( "ページ切り替え\n\nマウスホイール回転でも切り替え可能" );

    m_label.set_alignment( Gtk::ALIGN_START );
#if GTKMM_CHECK_VERSION(2,6,0)
    m_label.set_ellipsize( Pango::ELLIPSIZE_END );
#endif
    std::vector< std::string > menu;
    menu.push_back( ITEM_NAME_BBSLISTVIEW );
    menu.push_back( ITEM_NAME_FAVORITEVIEW );
    menu.push_back( ITEM_NAME_HISTVIEW );
    menu.push_back( ITEM_NAME_HIST_BOARDVIEW );
    menu.push_back( ITEM_NAME_HIST_CLOSEVIEW );
    menu.push_back( ITEM_NAME_HIST_CLOSEBOARDVIEW );
    menu.push_back( ITEM_NAME_HIST_CLOSEIMGVIEW );
    m_button_toggle.get_button()->append_menu( menu );
    m_button_toggle.get_button()->signal_selected().connect( sigc::mem_fun(*this, &BBSListToolBar::slot_toggle ) );
    m_button_toggle.get_button()->signal_scroll_event().connect(  sigc::mem_fun( *this, &BBSListToolBar::slot_scroll_event ));
    m_button_toggle.get_button()->set_enable_sig_clicked( false );

#if GTKMM_CHECK_VERSION(2,12,0)
    m_tool_label.set_icon_size( Gtk::ICON_SIZE_MENU );
#endif
    m_tool_label.set_toolbar_style( Gtk::TOOLBAR_ICONS );
    m_tool_label.append( m_button_toggle );
    m_tool_label.append( *get_button_close() );
    pack_start( m_tool_label, Gtk::PACK_SHRINK );

    pack_buttons();
    add_search_control_mode( CONTROL::MODE_BBSLIST );
}


//
// ボタンのパッキング
//
// virtual
void BBSListToolBar::pack_buttons()
{
    int num = 0;
    for(;;){
        int item = SESSION::get_item_sidebar_toolbar( num );
        if( item == ITEM_END ) break;
        switch( item ){

            case ITEM_SEARCHBOX:
                get_buttonbar().append( *get_tool_search( CORE::COMP_SEARCH_BBSLIST ) );
                break;

            case ITEM_CHECK_UPDATE_ROOT:
                if( ! m_button_check_update_root ){
                    m_button_check_update_root = Gtk::manage( new SKELETON::ImgToolButton( ICON::CHECK_UPDATE_ROOT ) );
                    m_button_check_update_root->signal_clicked().connect( sigc::mem_fun(*this, &BBSListToolBar::slot_check_update_root ) );
                    set_tooltip( *m_button_check_update_root, CONTROL::get_label_motions( CONTROL::CheckUpdateRoot ) );
                }
                get_buttonbar().append( *m_button_check_update_root );
                break;

            case ITEM_CHECK_UPDATE_OPEN_ROOT:
                if( ! m_button_check_update_open_root ){
                    m_button_check_update_open_root = Gtk::manage( new SKELETON::ImgToolButton( ICON::CHECK_UPDATE_OPEN_ROOT ) );
                    m_button_check_update_open_root->signal_clicked().connect( sigc::mem_fun(*this, &BBSListToolBar::slot_check_update_open_root ) );
                    set_tooltip( *m_button_check_update_open_root, CONTROL::get_label_motions( CONTROL::CheckUpdateOpenRoot ) );
                }
                get_buttonbar().append( *m_button_check_update_open_root );
                break;

            case ITEM_STOPLOADING:
                if( ! m_button_stop_check_update ) m_button_stop_check_update = get_button_stop();
                get_buttonbar().append( *m_button_stop_check_update );
                break;

            case ITEM_SEARCH_NEXT:
                get_buttonbar().append( *get_button_down_search() );
                break;

            case ITEM_SEARCH_PREV:
                get_buttonbar().append( *get_button_up_search() );
                break;

            case ITEM_SEPARATOR:
                pack_separator();
                break;
        }
        ++num;
    }

    set_relief();
    show_all_children();
}



// タブが切り替わった時にDragableNoteBook::set_current_toolbar()から呼び出される( Viewの情報を取得する )
// virtual
void BBSListToolBar::set_view( SKELETON::View* view )
{
    ToolBar::set_view( view );

    if( view ){
        m_label.set_text( view->get_label() );

        if( view->get_url() == URL_BBSLISTVIEW
            || ( ! CONFIG::get_check_update_board() && view->get_url() == URL_HISTBOARDVIEW ) )
        {
            if( m_button_check_update_root ) m_button_check_update_root->set_sensitive( false );
            if( m_button_check_update_open_root ) m_button_check_update_open_root->set_sensitive( false );
            if( m_button_stop_check_update ) m_button_stop_check_update->set_sensitive( false );
        }
        else{
            if( m_button_check_update_root ) m_button_check_update_root->set_sensitive( true );
            if( m_button_check_update_open_root ) m_button_check_update_open_root->set_sensitive( true );
            if( m_button_stop_check_update ) m_button_stop_check_update->set_sensitive( true );
        }
    }
}


void BBSListToolBar::slot_toggle( const int i )
{
#ifdef _DEBUG
     std::cout << "BBSListToolBar::slot_toggle = " << get_url() << " i = " << i << std::endl;
#endif

     switch( i ){

         case 0:
             if( get_url() != URL_BBSLISTVIEW ) CORE::core_set_command( "switch_sidebar", URL_BBSLISTVIEW );
             break;

         case 1:
             if( get_url() != URL_FAVORITEVIEW ) CORE::core_set_command( "switch_sidebar", URL_FAVORITEVIEW );
             break;

         case 2:
             if( get_url() != URL_HISTTHREADVIEW ) CORE::core_set_command( "switch_sidebar", URL_HISTTHREADVIEW );
             break;

         case 3:
             if( get_url() != URL_HISTBOARDVIEW ) CORE::core_set_command( "switch_sidebar", URL_HISTBOARDVIEW );
             break;

         case 4:
             if( get_url() != URL_HISTCLOSEVIEW ) CORE::core_set_command( "switch_sidebar", URL_HISTCLOSEVIEW );
             break;

         case 5:
             if( get_url() != URL_HISTCLOSEBOARDVIEW ) CORE::core_set_command( "switch_sidebar", URL_HISTCLOSEBOARDVIEW );
             break;

         case 6:
             if( get_url() != URL_HISTCLOSEIMGVIEW ) CORE::core_set_command( "switch_sidebar", URL_HISTCLOSEIMGVIEW );
             break;
     }
}


bool BBSListToolBar::slot_scroll_event( GdkEventScroll* event )
{
    guint direction = event->direction;

#ifdef _DEBUG
    std::cout << "BBSListToolBar::slot_scroll_event dir = " << direction << std::endl;
#endif

    int page = get_admin()->get_current_page();
    const int tab_nums = get_admin()->get_tab_nums();

    if( direction == GDK_SCROLL_UP ) --page;
    if( direction == GDK_SCROLL_DOWN ) ++page;

    if( page < 0 ) page = tab_nums-1;
    else if( page >= tab_nums ) page = 0;

    slot_toggle( page );

    return true;
}


void BBSListToolBar::slot_check_update_root()
{
    CORE::core_set_command( "check_update_root", "" );
}


void BBSListToolBar::slot_check_update_open_root()
{
    CORE::core_set_command( "check_update_open_root", "" );
}



////////////////////////////////////////


EditListToolBar::EditListToolBar() :
    SKELETON::ToolBar( NULL )
{
    pack_buttons();
}


//
// ボタンのパッキング
//
// virtual
void EditListToolBar::pack_buttons()
{
    get_buttonbar().append( *get_tool_search( CORE::COMP_SEARCH_BBSLIST ) );
    get_buttonbar().append( *get_button_down_search() );
    get_buttonbar().append( *get_button_up_search() );
    add_search_control_mode( CONTROL::MODE_BBSLIST );

    get_buttonbar().append( *get_button_undo() );
    get_buttonbar().append( *get_button_redo() );
    get_buttonbar().append( *get_button_close() );

    set_relief();
    show_all_children();
}
