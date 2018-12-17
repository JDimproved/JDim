// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "mouseconfig.h"
#include "defaultconf.h"

#include "jdlib/miscutil.h"
#include "jdlib/confloader.h"

#include "cache.h"

#ifdef _DEBUG
#include "controlutil.h"
#endif

using namespace CONTROL;


MouseConfig::MouseConfig()
    : MouseKeyConf()
{}



MouseConfig::~MouseConfig() noexcept
{}


//
// 設定ファイル読み込み
//
void MouseConfig::load_conf()
{
    JDLIB::ConfLoader cf( CACHE::path_mouseconf(), std::string() );

    // 共通
    load_motions( cf, "Right", MOUSECONF_Right );
    load_motions( cf, "Left", MOUSECONF_Left );
    load_motions( cf, "TabRight", MOUSECONF_TabRight );
    load_motions( cf, "TabLeft", MOUSECONF_TabLeft );
    load_motions( cf, "TabRightUpdated", MOUSECONF_TabRightUpdated );
    load_motions( cf, "TabLeftUpdated", MOUSECONF_TabLeftUpdated );
    load_motions( cf, "CloseAllTabs", MOUSECONF_CloseAllTabs );
    load_motions( cf, "CloseOtherTabs", MOUSECONF_CloseOtherTabs );
    load_motions( cf, "RestoreLastTab", MOUSECONF_RestoreLastTab );
    load_motions( cf, "CheckUpdateTabs", MOUSECONF_CheckUpdateTabs );
    load_motions( cf, "ToggleArticle", MOUSECONF_ToggleArticle );
    load_motions( cf, "ShowSideBar", MOUSECONF_ShowSideBar );
    load_motions( cf, "ShowMenuBar", MOUSECONF_ShowMenuBar );
    load_motions( cf, "ShowToolBarMain", MOUSECONF_ShowToolBarMain );
    load_motions( cf, "Home", MOUSECONF_Home );
    load_motions( cf, "End", MOUSECONF_End );
    load_motions( cf, "Quit", MOUSECONF_Quit );
    load_motions( cf, "Reload", MOUSECONF_Reload );
    load_motions( cf, "Delete", MOUSECONF_Delete );
    load_motions( cf, "StopLoading", MOUSECONF_StopLoading );
    load_motions( cf, "AppendFavorite", MOUSECONF_AppendFavorite );

    load_motions( cf, "NewArticle", MOUSECONF_NewArticle );
    load_motions( cf, "WriteMessage", MOUSECONF_WriteMessage );

    load_motions( cf, "SearchTitle", MOUSECONF_SearchTitle );

    load_motions( cf, "CheckUpdateRoot", MOUSECONF_CheckUpdateRoot );
    load_motions( cf, "CheckUpdateOpenRoot", MOUSECONF_CheckUpdateOpenRoot );

    load_motions( cf, "QuitJD", MOUSECONF_QuitJD );
    load_motions( cf, "MaximizeMainWin", MOUSECONF_MaximizeMainWin );
    load_motions( cf, "IconifyMainWin", MOUSECONF_IconifyMainWin );

    // ARTICLE
    load_motions( cf, "GotoNew", MOUSECONF_GotoNew );
    load_motions( cf, "SearchNextArticle", MOUSECONF_SearchNextArticle );
    load_motions( cf, "SearchWeb", MOUSECONF_SearchWeb );
    load_motions( cf, "LiveStartStop", MOUSECONF_LiveStartStop );

    // IMAGE
    load_motions( cf, "CancelMosaicButton", MOUSECONF_CancelMosaicButton );
}



// ひとつの操作をデータベースに登録
void MouseConfig::set_one_motion_impl( const int id, const int mode, const std::string& name, const std::string& str_motion )
{
    if( name.empty() || str_motion.empty() ) return;

#ifdef _DEBUG
    std::cout << "MouseConfig::set_one_motion_impl " << name << std::endl;
    std::cout << "motion = " << str_motion << std::endl;
    std::cout << CONTROL::get_label( id  ) << std::endl;
#endif

    const bool ctrl = false;
    const bool shift = false;
    const bool alt = false;
    const guint motion = atoi( str_motion.c_str() );
    const bool dblclick = false;
    const bool trpclick = false;

    vec_items().push_back( MouseKeyItem( id, mode, name, str_motion, motion, ctrl, shift, alt, dblclick, trpclick ) );
}


// 操作文字列取得
std::string MouseConfig::get_str_motions( const int id_ )
{
    int id = id_;

    // (注) この行が無いと画像ビューのコンテキストメニューにマウスジェスチャが表示されない
    if( id == CONTROL::CancelMosaic ) id = CONTROL::CancelMosaicButton;

    return MouseKeyConf::get_str_motions( id );
}


// IDからデフォルトの操作文字列取得
std::string MouseConfig::get_default_motions( const int id_ )
{
    int id = id_;
    if( id == CONTROL::CancelMosaic ) id = CONTROL::CancelMosaicButton;

    return MouseKeyConf::get_default_motions( id );
}
