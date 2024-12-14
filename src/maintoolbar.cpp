// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "maintoolbar.h"

#include "config/globalconf.h"

#include "icons/iconmanager.h"

#include "control/controlutil.h"
#include "control/controlid.h"

#include "session.h"
#include "global.h"

using namespace CORE;


MainToolBar::MainToolBar()
    : SKELETON::ToolBar( nullptr )
    , m_button_go( ICON::GO, ITEM_NAME_GO )

    , m_button_bbslist( ICON::BBSLISTVIEW, ITEM_NAME_BBSLISTVIEW )
    , m_button_favorite( ICON::FAVORITEVIEW, ITEM_NAME_FAVORITEVIEW )
    , m_button_hist( ICON::HISTVIEW, ITEM_NAME_HISTVIEW )
    , m_button_hist_board( ICON::HIST_BOARDVIEW, ITEM_NAME_HIST_BOARDVIEW )
    , m_button_hist_close( ICON::HIST_CLOSEVIEW, ITEM_NAME_HIST_CLOSEVIEW )
    , m_button_hist_closeboard( ICON::HIST_CLOSEBOARDVIEW, ITEM_NAME_HIST_CLOSEBOARDVIEW )
    , m_button_hist_closeimg( ICON::HIST_CLOSEIMGVIEW, ITEM_NAME_HIST_CLOSEIMGVIEW )

    , m_button_board( ICON::BOARDVIEW, ITEM_NAME_BOARDVIEW )

    , m_button_thread( ICON::ARTICLEVIEW, ITEM_NAME_ARTICLEVIEW )

    , m_button_image( ICON::IMAGEVIEW, ITEM_NAME_IMAGEVIEW )
{
    m_entry_url.set_size_request( 0 );
    m_tool_url.add( m_entry_url );
    m_tool_url.set_expand( true );

    set_tooltip( m_button_go, ITEM_NAME_GO );

    set_tooltip( m_button_bbslist, std::string( ITEM_NAME_BBSLISTVIEW )
                 + "\n\nお気に入りに切替え " + CONTROL::get_str_motions( CONTROL::TabRight ) );
    set_tooltip( m_button_favorite, std::string( ITEM_NAME_FAVORITEVIEW )
                 + "\n\n板一覧に切替え " + CONTROL::get_str_motions( CONTROL::TabLeft ) );
    set_tooltip( m_button_hist, std::string( ITEM_NAME_HISTVIEW ) );
    set_tooltip( m_button_hist_board, std::string( ITEM_NAME_HIST_BOARDVIEW ) );
    set_tooltip( m_button_hist_close, std::string( ITEM_NAME_HIST_CLOSEVIEW ) );
    set_tooltip( m_button_hist_closeboard, std::string( ITEM_NAME_HIST_CLOSEBOARDVIEW ) );
    set_tooltip( m_button_hist_closeimg, std::string( ITEM_NAME_HIST_CLOSEIMGVIEW ) );

    set_tooltip( m_button_board, std::string( ITEM_NAME_BOARDVIEW )
                 + "\n\n" + CONTROL::get_label_motions( CONTROL::ToggleArticle ) );

    set_tooltip( m_button_thread, std::string( ITEM_NAME_ARTICLEVIEW )
                 + "\n\n" + CONTROL::get_label_motions( CONTROL::ToggleArticle ) );

    set_tooltip( m_button_image, std::string( ITEM_NAME_IMAGEVIEW )
                 + "\n\nスレビューに切替 "
                 + CONTROL::get_str_motions( CONTROL::ToggleArticle ) + " , " + CONTROL::get_str_motions( CONTROL::Left ) );

    MainToolBar::pack_buttons();
}


MainToolBar::~MainToolBar() noexcept = default;


/**
 * @brief ボタンのアイコンを再読み込み
 */
void MainToolBar::reload_ui_icon()
{
    SKELETON::ToolBar::reload_ui_icon();

    set_button_icon( &m_button_go, ICON::GO );
}


// ボタンのパッキング
// virtual
void MainToolBar::pack_buttons()
{
    int num = 0;
    for(;;){
        int item = SESSION::get_item_main_toolbar( num );
        if( item == ITEM_END ) break;
        switch( item ){

            case ITEM_BBSLISTVIEW:
                get_buttonbar().append( m_button_bbslist );
                break;

            case ITEM_FAVORITEVIEW:
                get_buttonbar().append( m_button_favorite );
                break;

            case ITEM_HISTVIEW:
                get_buttonbar().append( m_button_hist );
                break;

            case ITEM_HIST_BOARDVIEW:
                get_buttonbar().append( m_button_hist_board );
                break;

            case ITEM_HIST_CLOSEVIEW:
                get_buttonbar().append( m_button_hist_close );
                break;

            case ITEM_HIST_CLOSEBOARDVIEW:
                get_buttonbar().append( m_button_hist_closeboard );
                break;

            case ITEM_HIST_CLOSEIMGVIEW:
                get_buttonbar().append( m_button_hist_closeimg );
                break;

            case ITEM_BOARDVIEW:
                get_buttonbar().append( m_button_board );
                break;

            case ITEM_ARTICLEVIEW:
                get_buttonbar().append( m_button_thread );
                break;

            case ITEM_IMAGEVIEW:
                if( CONFIG::get_use_image_view() ) get_buttonbar().append( m_button_image );
                break;

            case ITEM_URL:
                get_buttonbar().append( m_tool_url );
                break;

            case ITEM_GO:
                get_buttonbar().append( m_button_go );
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
