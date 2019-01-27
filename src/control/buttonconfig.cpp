// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "buttonconfig.h"
#include "defaultconf.h"

#include "jdlib/confloader.h"
#include "jdlib/jdregex.h"

#include "cache.h"

#ifdef _DEBUG
#include "controlutil.h"
#endif

using namespace CONTROL;


ButtonConfig::ButtonConfig()
    : MouseKeyConf()
{}


ButtonConfig::~ButtonConfig() noexcept
{}


//
// 設定ファイル読み込み
//
void ButtonConfig::load_conf()
{
    JDLIB::ConfLoader cf( CACHE::path_buttonconf(), std::string() );

    // デフォルト動作
    load_motions( cf, "ClickButton", BUTTONCONF_ClickButton );
    load_motions( cf, "DblClickButton", BUTTONCONF_DblClickButton );
    load_motions( cf, "TrpClickButton", BUTTONCONF_TrpClickButton );
    load_motions( cf, "CloseTabButton", BUTTONCONF_CloseTabButton );
    load_motions( cf, "ReloadTabButton", BUTTONCONF_ReloadTabButton );
    load_motions( cf, "AutoScrollButton", BUTTONCONF_AutoScrollButton );
    load_motions( cf, "GestureButton", BUTTONCONF_GestureButton );
    load_motions( cf, "PopupmenuButton", BUTTONCONF_PopupmenuButton );
    load_motions( cf, "DragStartButton", BUTTONCONF_DragStartButton );
    load_motions( cf, "TreeRowSelectionButton", BUTTONCONF_TreeRowSelectionButton );
    load_motions( cf, "Reload", BUTTONCONF_Reload );
    load_motions( cf, "ToggleArticle", BUTTONCONF_ToggleArticle );

    load_motions( cf, "Right", BUTTONCONF_Right );
    load_motions( cf, "Left", BUTTONCONF_Left );

    // BBSLIST用ボタン設定
    load_motions( cf, "OpenBoardButton", BUTTONCONF_OpenBoardButton );
    load_motions( cf, "OpenBoardTabButton", BUTTONCONF_OpenBoardTabButton );

    // BOARD用ボタン設定
    load_motions( cf, "OpenArticleButton", BUTTONCONF_OpenArticleButton );
    load_motions( cf, "OpenArticleTabButton", BUTTONCONF_OpenArticleTabButton );

    load_motions( cf, "ScrollRightBoard", BUTTONCONF_ScrollRightBoard );
    load_motions( cf, "ScrollLeftBoard", BUTTONCONF_ScrollLeftBoard );

    // ARTICLE用ボタン設定
    load_motions( cf, "PopupWarpButton", BUTTONCONF_PopupWarpButton );

    load_motions( cf, "ReferResButton", BUTTONCONF_ReferResButton );
    load_motions( cf, "BmResButton", BUTTONCONF_BmResButton );
    load_motions( cf, "PopupmenuResButton", BUTTONCONF_PopupmenuResButton );

    load_motions( cf, "DrawoutAncButton", BUTTONCONF_DrawoutAncButton );
    load_motions( cf, "PopupmenuAncButton", BUTTONCONF_PopupmenuAncButton );
    load_motions( cf, "JumpAncButton", BUTTONCONF_JumpAncButton );

    load_motions( cf, "PopupIDButton", BUTTONCONF_PopupIDButton );
    load_motions( cf, "DrawoutIDButton", BUTTONCONF_DrawoutIDButton );
    load_motions( cf, "PopupmenuIDButton", BUTTONCONF_PopupmenuIDButton );

    load_motions( cf, "OpenImageButton", BUTTONCONF_OpenImageButton );
    load_motions( cf, "OpenBackImageButton", BUTTONCONF_OpenBackImageButton );
    load_motions( cf, "PopupmenuImageButton", BUTTONCONF_PopupmenuImageButton );

    load_motions( cf, "OpenBeButton", BUTTONCONF_OpenBeButton );
    load_motions( cf, "PopupmenuBeButton", BUTTONCONF_PopupmenuBeButton );

    // IMAGE ICON用ボタン設定
    load_motions( cf, "CloseImageTabButton", BUTTONCONF_CloseImageTabButton );

    // IMAGE用ボタン設定
    load_motions( cf, "CloseImageButton", BUTTONCONF_CloseImageButton );
    load_motions( cf, "ScrollImageButton", BUTTONCONF_ScrollImageButton );
    load_motions( cf, "CancelMosaicButton", BUTTONCONF_CancelMosaicButton );
    load_motions( cf, "SaveImageButton", BUTTONCONF_SaveImageButton );
    load_motions( cf, "ResizeImageButton", BUTTONCONF_ResizeImageButton );
}


// ひとつの操作をデータベースに登録
void ButtonConfig::set_one_motion_impl( const int id, const int mode, const std::string& name, const std::string& str_motion )
{
    if( name.empty() ) return;

#ifdef _DEBUG
    std::cout << "ButtonConfig::set_one_motion_impl " << name << std::endl;
    std::cout << "motion = " << str_motion << std::endl;
#endif

#ifdef _DEBUG
    std::cout << CONTROL::get_label( id  ) << std::endl;
#endif

    bool ctrl = false;
    bool shift = false;
    bool alt = false;
    bool dblclick = false;
    bool trpclick = false;
    guint motion = 0;

    JDLIB::Regex regex;
    const size_t offset = 0;
    const bool icase = true; // 大文字小文字区別しない
    const bool newline = true;
    const bool usemigemo = false;
    const bool wchar = false;

    if( regex.exec( "(Ctrl)?(\\+?Shift)?(\\+?Alt)?\\+?(.*)", str_motion, offset, icase, newline, usemigemo, wchar ) ){

        if( ! regex.str( 1 ).empty() ) ctrl = true;
        if( ! regex.str( 2 ).empty() ) shift = true;
        if( ! regex.str( 3 ).empty() ) alt = true;

        std::string str_button = regex.str( 4 );

        if( str_button == "Left" ) motion = 1;
        if( str_button == "Mid" ) motion = 2;
        if( str_button == "Right" ) motion = 3;
        if( str_button == "Tilt_Left" ) motion = 6;
        if( str_button == "Tilt_Right" ) motion = 7;
        if( str_button == "Button4" ) motion = 8;
        if( str_button == "Button5" ) motion = 9;
        if( str_button == "DblLeft" ){ motion = 1; dblclick = true; }
        if( str_button == "DblMid" ) { motion = 2; dblclick = true; }
        if( str_button == "DblRight" ) { motion = 3; dblclick = true; }
        if( str_button == "TrpLeft" ){ motion = 1; trpclick = true; }
        if( str_button == "TrpMid" ) { motion = 2; trpclick = true; }
        if( str_button == "TrpRight" ) { motion = 3; trpclick = true; }
    }
    else return;

#ifdef _DEBUG
    std::cout << "motion = " << motion << " dblclick = " << dblclick
              << " trpclick = " << trpclick << std::endl << std::endl;
#endif

    // データベース登録
    vec_items().push_back( MouseKeyItem( id, mode, name, str_motion, motion, ctrl, shift, alt, dblclick, trpclick ) );
}



// タブで開くボタンを入れ替えているか
bool ButtonConfig::is_toggled_tab_button()
{
    const bool ret =  ( get_str_motions( CONTROL::OpenBoardButton ).find( "Mid" ) != std::string::npos
                        && get_str_motions( CONTROL::OpenBoardTabButton ).find( "Left" ) != std::string::npos

                        && get_str_motions( CONTROL::OpenArticleButton ).find( "Mid" ) != std::string::npos
                        && get_str_motions( CONTROL::OpenArticleTabButton ).find( "Left" ) != std::string::npos
        );

#ifdef _DEBUG
    std::cout << "KeyConfig::is_toggled_tab_button ret = " << ret << std::endl;
#endif

    return ret;
}


// タブで開くボタンを入れ替える
// toggle == true なら左ボタンをタブで開くボタンにする
void ButtonConfig::toggle_tab_button( const bool toggle )
{
    remove_motions( CONTROL::OpenBoardButton );
    remove_motions( CONTROL::OpenBoardTabButton );
    remove_motions( CONTROL::OpenArticleButton );
    remove_motions( CONTROL::OpenArticleTabButton );

    if( toggle ){

        set_one_motion( "OpenBoardButton", "Mid" );
        set_one_motion( "OpenBoardTabButton", "Left" );

        set_one_motion( "OpenArticleButton", "Mid" );
        set_one_motion( "OpenArticleTabButton", "Left" );
    }
    else{

        set_one_motion( "OpenBoardButton", "Left" );
        set_one_motion( "OpenBoardTabButton", "Mid" );

        set_one_motion( "OpenArticleButton", "Left" );
        set_one_motion( "OpenArticleTabButton", "Mid" );
    }
}



// ポップアップ表示の時にクリックでワープするか
bool ButtonConfig::is_popup_warpmode()
{
    return ( get_str_motions( CONTROL::PopupWarpButton).find( "Left" ) != std::string::npos );
}


// ポップアップ表示の時にクリックでワープする
void ButtonConfig::toggle_popup_warpmode()
{
    bool warp = is_popup_warpmode();

    remove_motions( CONTROL::PopupWarpButton );

    if( warp ) set_one_motion( "PopupWarpButton", "" );
    else set_one_motion( "PopupWarpButton", "Left" );
}
