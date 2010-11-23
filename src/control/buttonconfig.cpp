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


ButtonConfig::~ButtonConfig()
{}


//
// 設定ファイル読み込み
//
void ButtonConfig::load_conf()
{
    std::string str_motions;
    JDLIB::ConfLoader cf( CACHE::path_buttonconf(), std::string() );

    // デフォルト動作
    SETMOTION( "ClickButton", BUTTONCONF_ClickButton );
    SETMOTION( "DblClickButton", BUTTONCONF_DblClickButton );
    SETMOTION( "TrpClickButton", BUTTONCONF_TrpClickButton );
    SETMOTION( "CloseTabButton", BUTTONCONF_CloseTabButton );
    SETMOTION( "ReloadTabButton", BUTTONCONF_ReloadTabButton );
    SETMOTION( "AutoScrollButton", BUTTONCONF_AutoScrollButton );
    SETMOTION( "GestureButton", BUTTONCONF_GestureButton );
    SETMOTION( "PopupmenuButton", BUTTONCONF_PopupmenuButton );
    SETMOTION( "DragStartButton", BUTTONCONF_DragStartButton );
    SETMOTION( "TreeRowSelectionButton", BUTTONCONF_TreeRowSelectionButton );
    SETMOTION( "Reload", BUTTONCONF_Reload );
    SETMOTION( "ToggleArticle", BUTTONCONF_ToggleArticle );

    SETMOTION( "Right", BUTTONCONF_Right );
    SETMOTION( "Left", BUTTONCONF_Left );

    // BBSLIST用ボタン設定
    SETMOTION( "OpenBoardButton", BUTTONCONF_OpenBoardButton );
    SETMOTION( "OpenBoardTabButton", BUTTONCONF_OpenBoardTabButton );

    // BOARD用ボタン設定
    SETMOTION( "OpenArticleButton", BUTTONCONF_OpenArticleButton );
    SETMOTION( "OpenArticleTabButton", BUTTONCONF_OpenArticleTabButton );

    SETMOTION( "ScrollRightBoard", BUTTONCONF_ScrollRightBoard );
    SETMOTION( "ScrollLeftBoard", BUTTONCONF_ScrollLeftBoard );

    // ARTICLE用ボタン設定
    SETMOTION( "PopupWarpButton", BUTTONCONF_PopupWarpButton );

    SETMOTION( "ReferResButton", BUTTONCONF_ReferResButton );
    SETMOTION( "BmResButton", BUTTONCONF_BmResButton );
    SETMOTION( "PopupmenuResButton", BUTTONCONF_PopupmenuResButton );

    SETMOTION( "DrawoutAncButton", BUTTONCONF_DrawoutAncButton );
    SETMOTION( "PopupmenuAncButton", BUTTONCONF_PopupmenuAncButton );
    SETMOTION( "JumpAncButton", BUTTONCONF_JumpAncButton );

    SETMOTION( "PopupIDButton", BUTTONCONF_PopupIDButton );
    SETMOTION( "DrawoutIDButton", BUTTONCONF_DrawoutIDButton );
    SETMOTION( "PopupmenuIDButton", BUTTONCONF_PopupmenuIDButton );

    SETMOTION( "OpenImageButton", BUTTONCONF_OpenImageButton );
    SETMOTION( "OpenBackImageButton", BUTTONCONF_OpenBackImageButton );
    SETMOTION( "PopupmenuImageButton", BUTTONCONF_PopupmenuImageButton );

    SETMOTION( "OpenBeButton", BUTTONCONF_OpenBeButton );
    SETMOTION( "PopupmenuBeButton", BUTTONCONF_PopupmenuBeButton );

    // IMAGE ICON用ボタン設定
    SETMOTION( "CloseImageTabButton", BUTTONCONF_CloseImageTabButton );

    // IMAGE用ボタン設定
    SETMOTION( "CloseImageButton", BUTTONCONF_CloseImageButton );
    SETMOTION( "ScrollImageButton", BUTTONCONF_ScrollImageButton );
    SETMOTION( "CancelMosaicButton", BUTTONCONF_CancelMosaicButton );
    SETMOTION( "SaveImageButton", BUTTONCONF_SaveImageButton );
    SETMOTION( "ResizeImageButton", BUTTONCONF_ResizeImageButton );
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
const bool ButtonConfig::is_toggled_tab_button()
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
const bool ButtonConfig::is_popup_warpmode()
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
