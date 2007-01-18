// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "buttonconfig.h"
#include "mousekeyitem.h"

#include "cache.h"
#include "controlutil.h"

#include "jdlib/confloader.h"
#include "jdlib/jdregex.h"

CONFIG::ButtonConfig* instance_buttonconfig = NULL;


CONFIG::ButtonConfig* CONFIG::get_buttonconfig()
{
    if( ! instance_buttonconfig ) instance_buttonconfig = new CONFIG::ButtonConfig();

    return instance_buttonconfig;
}


void CONFIG::delete_buttonconfig()
{
    if( instance_buttonconfig ) delete instance_buttonconfig;
    instance_buttonconfig = NULL;
}

//////////////////////////////////////////////////////////

using namespace CONFIG;


ButtonConfig::ButtonConfig()
    : MouseKeyConf()
{
    load_conf();
}


ButtonConfig::~ButtonConfig()
{
    save_conf( CACHE::path_buttonconf() );
}


//
// 設定ファイル読み込み
//
void ButtonConfig::load_conf()
{
    std::string str_motion;
    JDLIB::ConfLoader cf( CACHE::path_buttonconf(), std::string() );

    // デフォルト動作
    SETMOTION( "ClickButton", "Left" );
    SETMOTION( "DblClickButton", "DblLeft" );
    SETMOTION( "CloseTabButton", "Mid" );
    SETMOTION( "ReloadTabButton", "DblLeft" );
    SETMOTION( "AutoScrollButton", "Mid" );
    SETMOTION( "GestureButton", "Right" );
    SETMOTION( "PopupmenuButton", "Right" );
    SETMOTION( "DragStartButton", "Left" );
    SETMOTION( "TreeRowSelectionButton", "Mid" );
    SETMOTION( "Reload", "Button4" );
    SETMOTION( "ToggleArticle", "Button5" );

    SETMOTION( "ScrollRight", "Tilt_Right" );
    SETMOTION( "ScrollLeft", "Tilt_Left" );
    SETMOTION( "Right", "" );
    SETMOTION( "Left", "" );

    // BBSLIST用ボタン設定
    SETMOTION( "OpenBoardButton", "Left" );
    SETMOTION( "OpenBoardTabButton", "Mid" );

    // BOARD用ボタン設定
    SETMOTION( "OpenArticleButton", "Left" );
    SETMOTION( "OpenArticleTabButton", "Mid" );

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
}


// ひとつの操作をデータベースに登録
void ButtonConfig::set_one_motion( const std::string& name, const std::string& str_motion )
{
    if( name.empty() ) return;

#ifdef _DEBUG
    std::cout << "ButtonConfig::set_motion " << name << std::endl;
    std::cout << "motion = " << str_motion << std::endl;
#endif

    int id = CONTROL::get_id( name );
    if( id == CONTROL::None ) return;

#ifdef _DEBUG
    std::cout << "id = " << id << std::endl;
#endif

    int mode = get_mode( id );
    if( mode == CONTROL::MODE_ERROR ) return;

    bool ctrl = false;
    bool shift = false;
    bool alt = false;
    bool dblclick = false;
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
    }

#ifdef _DEBUG
    std::cout << "motion = " << motion << " dblclick = " << dblclick << std::endl;
#endif

    // ひとつのボタンに複数の機能が割り当てられているので重複チェックはしない

    // データベース登録
    MouseKeyItem* item = new MouseKeyItem( id, mode, name, str_motion, motion, ctrl, shift, alt, dblclick );
    vec_items().push_back( item );
}



// 中ボタンでタブで開くか
bool ButtonConfig::tab_midbutton()
{
    return ( get_str_motion( CONTROL::OpenArticleTabButton ).find( "Mid" ) != std::string::npos );
}


// タブで開くボタンを入れ替える
void ButtonConfig::toggle_tab_button()
{
    remove_items( CONTROL::OpenBoardButton );
    remove_items( CONTROL::OpenBoardTabButton );
    remove_items( CONTROL::OpenArticleButton );
    remove_items( CONTROL::OpenArticleTabButton );

    if( tab_midbutton() ){

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
    return ( get_str_motion( CONTROL::PopupWarpButton).find( "Left" ) != std::string::npos );
}


// ポップアップ表示の時にクリックでワープする
void ButtonConfig::toggle_popup_warpmode()
{
    remove_items( CONTROL::PopupWarpButton );

    if( is_popup_warpmode() ) set_one_motion( "PopupWarpButton", "" );
    else set_one_motion( "PopupWarpButton", "Left" );
}
