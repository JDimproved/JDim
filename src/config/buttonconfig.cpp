// ライセンス: 最新のGPL

//#define _DEBUG
#include "jddebug.h"

#include "buttonconfig.h"
#include "mousekeyitem.h"

#include "cache.h"
#include "controlutil.h"

#include "jdlib/confloader.h"

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
    MouseKeyConf::save_conf( CACHE::path_buttonconf() );
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

    // BBSLIST用ボタン設定
    SETMOTION( "OpenBoardButton", "Left" );
    SETMOTION( "OpenBoardTabButton", "Mid" );

    // BOARD用ボタン設定
    SETMOTION( "OpenArticleButton", "Left" );
    SETMOTION( "OpenArticleTabButton", "Mid" );

    // ARTICLE用ボタン設定
    SETMOTION( "ReferResButton", "Right" );
    SETMOTION( "BmResButton", "Mid" );
    SETMOTION( "PopupmenuResButton", "Left" );

    SETMOTION( "DrawoutAncButton", "Mid" );
    SETMOTION( "PopupmenuAncButton", "Left Right" );

    SETMOTION( "PopupIDButton", "Right" );
    SETMOTION( "DrawoutIDButton", "Mid" );
    SETMOTION( "PopupmenuIDButton", "Left" );

    SETMOTION( "OpenImageButton", "Left" );
    SETMOTION( "OpenBackImageButton", "Mid" );
    SETMOTION( "PopupmenuImageButton", "Right" );

    SETMOTION( "OpenBeButton", "Left Mid" );
    SETMOTION( "PopupmenuBeButton", "Right" );
}


// ひとつの操作をデータベースに登録
void ButtonConfig::set_one_motion( const std::string& name, const std::string& str_motion )
{
    if( name.empty() || str_motion.empty() ) return;

#ifdef _DEBUG
    std::cout << "ButtonConfig::set_motion " << name << std::endl;
#endif

    int id = CONTROL::get_id( name );
    if( id == CONTROL::None ) return;

#ifdef _DEBUG
    std::cout << "id = " << id << std::endl;
#endif

    int mode = MouseKeyConf::get_mode( id );
    if( mode == CONTROL::MODE_ERROR ) return;

    bool ctrl = false;
    bool shift = false;
    bool alt = false;
    bool dblclick = false;
    guint motion = 0;
    if( str_motion == "Left" ) motion = 1;
    if( str_motion == "Mid" ) motion = 2;
    if( str_motion == "Right" ) motion = 3;
    if( str_motion == "DblLeft" ){ motion = 1; dblclick = true; }
    if( str_motion == "DblMid" ) { motion = 2; dblclick = true; }
    if( str_motion == "DblRight" ) { motion = 3; dblclick = true; }

#ifdef _DEBUG
    std::cout << "motion = " << motion << " dblclick = " << dblclick << std::endl;
#endif

    if( !motion ) return;

    // ひとつのボタンに複数の機能が割り当てられているので重複チェックはしない

    // データベース登録
    MouseKeyItem* item = new MouseKeyItem( id, mode, name, str_motion, motion, ctrl, shift, alt, dblclick );
    MouseKeyConf::vec_items().push_back( item );
}



// 中ボタンでタブで開くか
bool ButtonConfig::tab_midbutton()
{
    return ( MouseKeyConf::get_str_motion( CONTROL::OpenArticleTabButton ) == "Mid" );
}


// タブで開くボタンを入れ替える
void ButtonConfig::toggle_tab_button()
{
    if( tab_midbutton() ){

        MouseKeyConf::remove_items( CONTROL::OpenBoardButton );
        MouseKeyConf::remove_items( CONTROL::OpenBoardTabButton );
        MouseKeyConf::remove_items( CONTROL::OpenArticleButton );
        MouseKeyConf::remove_items( CONTROL::OpenArticleTabButton );

        set_one_motion( "OpenBoardButton", "Mid" );
        set_one_motion( "OpenBoardTabButton", "Left" );
        set_one_motion( "OpenArticleButton", "Mid" );
        set_one_motion( "OpenArticleTabButton", "Left" );
    }
    else{

        MouseKeyConf::remove_items( CONTROL::OpenBoardButton );
        MouseKeyConf::remove_items( CONTROL::OpenBoardTabButton );
        MouseKeyConf::remove_items( CONTROL::OpenArticleButton );
        MouseKeyConf::remove_items( CONTROL::OpenArticleTabButton );

        set_one_motion( "OpenBoardButton", "Left" );
        set_one_motion( "OpenBoardTabButton", "Mid" );
        set_one_motion( "OpenArticleButton", "Left" );
        set_one_motion( "OpenArticleTabButton", "Mid" );
    }
}
