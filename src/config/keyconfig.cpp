// ライセンス: 最新のGPL

//#define _DEBUG
//#define _DEBUG_XML
#include "jddebug.h"

#include "keyconfig.h"
#include "mousekeyitem.h"

#include "cache.h"
#include "controlutil.h"

#include "jdlib/miscutil.h"
#include "jdlib/miscmsg.h"
#include "jdlib/jdregex.h"
#include "jdlib/confloader.h"


CONFIG::KeyConfig* instance_keyconfig = NULL;

CONFIG::KeyConfig* CONFIG::get_keyconfig()
{
    if( ! instance_keyconfig ) instance_keyconfig = new CONFIG::KeyConfig();

    return instance_keyconfig;
}


void CONFIG::delete_keyconfig()
{
    if( instance_keyconfig ) delete instance_keyconfig;
    instance_keyconfig = NULL;
}

//////////////////////////////////////////////////////////

using namespace CONFIG;


KeyConfig::KeyConfig()
{
    load_conf();
}


KeyConfig::~KeyConfig()
{
    MouseKeyConf::save_conf( CACHE::path_keyconf() );
}



//
// 設定ファイル読み込み
//
void KeyConfig::load_conf()
{
    std::string str_motion;
    JDLIB::ConfLoader cf( CACHE::path_keyconf(), std::string() );

    // 共通設定
    SETMOTION( "Up", "k Up" );
    SETMOTION( "UpMid", "u" );
    SETMOTION( "UpFast", "b Page_Up" );

    SETMOTION( "Down", "j Down" );
    SETMOTION( "DownMid", "d" );
    SETMOTION( "DownFast", "Page_Down Space" );

    SETMOTION( "Right", "l Right" );
    SETMOTION( "Left", "h Left" );

    SETMOTION( "TabRight", "Ctrl+Page_Down Ctrl+Tab Ctrl+Left_Tab Ctrl+l Ctrl+Right" );
    SETMOTION( "TabLeft", "Ctrl+Page_Up Ctrl+Shift+Tab Ctrl+Shift+Left_Tab Ctrl+h Ctrl+Left" );

    SETMOTION( "PreBookMark", "Ctrl+F2" );
    SETMOTION( "NextBookMark", "F2" );
    SETMOTION( "ToggleArticle", "Alt+x" );

    SETMOTION( "Home", "Home g <" );
    SETMOTION( "End", "End G >" );

    SETMOTION( "Quit", "Ctrl+w q" );
    SETMOTION( "Save", "Ctrl+s" );
    SETMOTION( "Delete", "Delete" );
    SETMOTION( "Reload", "F5 s" );
    SETMOTION( "StopLoading", "Escape" ); // = CONTROL::Cancel
    SETMOTION( "Copy", "Ctrl+c" );

    SETMOTION( "Search", "Ctrl+f /" );
    SETMOTION( "SearchInvert", "?" );
    SETMOTION( "SearchNext", "Enter F3 Ctrl+g n" );
    SETMOTION( "SearchPrev", "Shift+Enter Ctrl+F3 Ctrl+G N" );
    SETMOTION( "DrawOutAnd", "Ctrl+Enter" );

    // BBSLIST
    SETMOTION( "OpenBoard", "Space" );

    // BOARD
    SETMOTION( "OpenArticle", "Space" );
    SETMOTION( "NewArticle", "w" );

    // ARTICLE
    SETMOTION( "GotoNew", "F4" );
    SETMOTION( "WriteMessage", "w Alt+w" );

    // IMAGE
    SETMOTION( "CancelMosaic", "c" );
    SETMOTION( "ZoomFitImage", "x" );
    SETMOTION( "ZoomInImage", "Plus" );
    SETMOTION( "ZoomOutImage", "-" );
    SETMOTION( "OrgSizeImage", "z" );

    // MESSAGE
    SETMOTION( "CancelWrite", "Alt+q" );
    SETMOTION( "ExecWrite", "Alt+w" ); 
}



// ひとつの操作をデータベースに登録
void KeyConfig::set_one_motion( const std::string& name, const std::string& str_motion )
{
    if( name.empty() || str_motion.empty() ) return;

    int id = CONTROL::get_id( name );
    if( id == CONTROL::None ) return;

    int mode = MouseKeyConf::get_mode( id );
    if( mode == CONTROL::MODE_ERROR ) return;

    JDLIB::Regex regex;
    if( regex.exec( "(Ctrl)?(\\+?Shift)?(\\+?Alt)?\\+?(.*)", str_motion, 0, true ) ){

        guint motion;
        bool ctrl = false;
        bool shift = false;
        bool alt = false;

        if( ! regex.str( 1 ).empty() ) ctrl = true;
        if( ! regex.str( 2 ).empty() ) shift = true;
        if( ! regex.str( 3 ).empty() ) alt = true;

        std::string str_key = regex.str( 4 );
        if( str_key.empty() ) return;

        if( str_key == "Space" ) motion = ' ';
        else if( str_key == "Escape" ) motion = GDK_Escape;
        else if( str_key == "Delete" ) motion = GDK_Delete;
        else if( str_key == "Enter" ) motion = GDK_Return;

        else if( str_key == "Up" ) motion = GDK_Up;
        else if( str_key == "Down" ) motion = GDK_Down;
        else if( str_key == "Left" ) motion = GDK_Left;
        else if( str_key == "Right" ) motion = GDK_Right;
        else if( str_key == "Page_Up" ) motion = GDK_Page_Up;
        else if( str_key == "Page_Down" ) motion = GDK_Page_Down;
        else if( str_key == "Tab" ) motion = GDK_Tab;
        else if( str_key == "Left_Tab" ) motion = GDK_ISO_Left_Tab;
        else if( str_key == "Home" ) motion = GDK_Home;
        else if( str_key == "End" ) motion = GDK_End;

        else if( str_key == "F1" ) motion = GDK_F1;
        else if( str_key == "F2" ) motion = GDK_F2;
        else if( str_key == "F3" ) motion = GDK_F3;
        else if( str_key == "F4" ) motion = GDK_F4;
        else if( str_key == "F5" ) motion = GDK_F5;
        else if( str_key == "F6" ) motion = GDK_F6;
        else if( str_key == "F7" ) motion = GDK_F7;
        else if( str_key == "F8" ) motion = GDK_F8;
        else if( str_key == "F9" ) motion = GDK_F9;
        else if( str_key == "F10" ) motion = GDK_F10;
        else if( str_key == "F11" ) motion = GDK_F11;
        else if( str_key == "F12" ) motion = GDK_F12;

        else if( str_key == "Plus" ) motion = '+';
        else motion = str_key[ 0 ];

        // 大文字やshiftが必要な文字の時はshiftも有効にする
        if( motion >= 'A' && motion <= 'Z' ) shift = true;
        if( motion == '?' || motion == '<' || motion == '>' || motion == '+' ) shift = true;

        int id_check = MouseKeyConf::check_conflict( mode, motion, ctrl, shift, alt, false );
        if( id_check != CONTROL::None ){
            MISC::ERRMSG( "key config : " + str_motion + " is already used." );
            return;
        }

        MouseKeyItem* item = new MouseKeyItem( id, mode, name, str_motion, motion, ctrl, shift, alt, false );
        MouseKeyConf::vec_items().push_back( item );
    }
}



// 操作文字列取得
const std::string KeyConfig::get_str_motion( int id )
{
    std::string str_motion = MouseKeyConf::get_str_motion( id );
    str_motion = MISC::replace_str( str_motion, "Plus", "+" );

    return str_motion;
}
