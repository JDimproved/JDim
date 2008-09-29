// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "buttonconfig.h"
#include "mousekeyitem.h"
#include "controlutil.h"

#include "jdlib/confloader.h"
#include "jdlib/jdregex.h"

#include "cache.h"

CONTROL::ButtonConfig* instance_buttonconfig = NULL;


CONTROL::ButtonConfig* CONTROL::get_buttonconfig()
{
    if( ! instance_buttonconfig ) instance_buttonconfig = new CONTROL::ButtonConfig();

    return instance_buttonconfig;
}


void CONTROL::delete_buttonconfig()
{
    if( instance_buttonconfig ) delete instance_buttonconfig;
    instance_buttonconfig = NULL;
}

//////////////////////////////////////////////////////////

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
    SETMOTION( "ClickButton", "Left" );
    SETMOTION( "DblClickButton", "DblLeft" );
    SETMOTION( "TrpClickButton", "TrpLeft" );
    SETMOTION( "CloseTabButton", "Mid" );
    SETMOTION( "ReloadTabButton", "DblLeft" );
    SETMOTION( "AutoScrollButton", "Mid" );
    SETMOTION( "GestureButton", "Right" );
    SETMOTION( "PopupmenuButton", "Right" );
    SETMOTION( "DragStartButton", "Left" );
    SETMOTION( "TreeRowSelectionButton", "Mid" );
    SETMOTION( "Reload", "Button4" );
    SETMOTION( "ToggleArticle", "Button5" );

    SETMOTION( "Right", "" );
    SETMOTION( "Left", "" );

    // BBSLIST用ボタン設定
    SETMOTION( "OpenBoardButton", "Left" );
    SETMOTION( "OpenBoardTabButton", "Mid" );

    // BOARD用ボタン設定
    SETMOTION( "OpenArticleButton", "Left" );
    SETMOTION( "OpenArticleTabButton", "Mid" );

    SETMOTION( "ScrollRightBoard", "Tilt_Right" );
    SETMOTION( "ScrollLeftBoard", "Tilt_Left" );

    // ARTICLE用ボタン設定
    SETMOTION( "PopupWarpButton", "" );

    SETMOTION( "ReferResButton", "Right" );
    SETMOTION( "BmResButton", "Mid" );
    SETMOTION( "PopupmenuResButton", "Left" );

    SETMOTION( "DrawoutAncButton", "Mid" );
    SETMOTION( "PopupmenuAncButton", "Left Right" );

    SETMOTION( "PopupIDButton", "Right" );
    SETMOTION( "DrawoutIDButton", "Mid" );
    SETMOTION( "PopupmenuIDButton", "Left" );

    SETMOTION( "OpenImageButton", "Left" );
    SETMOTION( "OpenBackImageButton", "Mid Ctrl+Left" );
    SETMOTION( "PopupmenuImageButton", "Right" );

    SETMOTION( "OpenBeButton", "Left Mid" );
    SETMOTION( "PopupmenuBeButton", "Right" );

    // IMAGE ICON用ボタン設定
    SETMOTION( "CloseImageTabButton", "Mid" );

    // IMAGE用ボタン設定
    SETMOTION( "CloseImageButton", "Mid" );
    SETMOTION( "ScrollImageButton", "Left" );
    SETMOTION( "CancelMosaicButton", "" );
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
    if( regex.exec( "(Ctrl)?(\\+?Shift)?(\\+?Alt)?\\+?(.*)", str_motion, 0, true ) ){

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

    // ひとつのボタンに複数の機能が割り当てられているので重複チェックはしない

    // データベース登録
    vec_items().push_back( MouseKeyItem( id, mode, name, str_motion, motion, ctrl, shift, alt, dblclick, trpclick ) );
}



// タブで開くボタンを入れ替えているか
const bool ButtonConfig::is_toggled_tab_button()
{
    return ( get_str_motions( CONTROL::OpenArticleTabButton ).find( "Left" ) != std::string::npos );
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
