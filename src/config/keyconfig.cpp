// ライセンス: GPL2

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
    save_conf( CACHE::path_keyconf() );
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
    SETMOTION( "Down", "j Down" );

    SETMOTION( "PageUp", "Page_Up" );
    SETMOTION( "PageDown", "Page_Down" );    

    SETMOTION( "Right", "l Right" );
    SETMOTION( "Left", "h Left" );

    SETMOTION( "TabRight", "Ctrl+Page_Down Ctrl+Tab Ctrl+Left_Tab Ctrl+l Ctrl+Right" );
    SETMOTION( "TabLeft", "Ctrl+Page_Up Ctrl+Shift+Tab Ctrl+Shift+Left_Tab Ctrl+h Ctrl+Left" );

    SETMOTION( "PreBookMark", "Ctrl+F2" );
    SETMOTION( "NextBookMark", "F2" );

    SETMOTION( "PrevView", "Alt+Left" );
    SETMOTION( "NextView", "Alt+Right" );

    SETMOTION( "ToggleArticle", "Alt+x" );

    SETMOTION( "ShowPopupMenu", "Shift+F10 Ctrl+m Menu" );

    SETMOTION( "ShowMenuBar", "F8" );
    SETMOTION( "ShowSideBar", "F9" );

    SETMOTION( "Home", "Home g <" );
    SETMOTION( "End", "End G >" );

    SETMOTION( "Back", "BackSpace" );

    SETMOTION( "Quit", "Ctrl+w q" );
    SETMOTION( "Save", "Ctrl+s" );
    SETMOTION( "Delete", "Delete" );
    SETMOTION( "Reload", "F5 s" );
    SETMOTION( "StopLoading", "Escape" ); // = CONTROL::Cancel
    SETMOTION( "Copy", "Ctrl+c" );
    SETMOTION( "SelectAll", "Ctrl+a" );

    SETMOTION( "Search", "Ctrl+f /" );
    SETMOTION( "SearchInvert", "?" );
    SETMOTION( "SearchNext", "Enter F3 Ctrl+g n" );
    SETMOTION( "SearchPrev", "Shift+Enter Ctrl+F3 Ctrl+G N" );
    SETMOTION( "DrawOutAnd", "Ctrl+Enter" );

    // BBSLIST
    SETMOTION( "OpenBoard", "Space" );
    SETMOTION( "OpenBoardTab", "Ctrl+Space" );

    // BOARD
    SETMOTION( "OpenArticle", "Space" );
    SETMOTION( "OpenArticleTab", "Ctrl+Space" );
    SETMOTION( "NewArticle", "w" );
    SETMOTION( "SearchCache", "Ctrl+Enter" );

    SETMOTION( "ScrollRightBoard", "L Shift+Right" );
    SETMOTION( "ScrollLeftBoard", "H Shift+Left" );

    // ARTICLE
    SETMOTION( "UpMid", "u" );
    SETMOTION( "UpFast", "b Page_Up" );

    SETMOTION( "DownMid", "d" );
    SETMOTION( "DownFast", "Page_Down Space" );

    SETMOTION( "PrevRes", "p" );
    SETMOTION( "NextRes", "n" );

    SETMOTION( "GotoNew", "F4" );
    SETMOTION( "WriteMessage", "w Alt+w" );

    // IMAGE
    SETMOTION( "CancelMosaic", "c" );
    SETMOTION( "ZoomFitImage", "x" );
    SETMOTION( "ZoomInImage", "Plus" );
    SETMOTION( "ZoomOutImage", "-" );
    SETMOTION( "OrgSizeImage", "z" );

    SETMOTION( "ScrollUpImage", "K k Shift+Up Up" );
    SETMOTION( "ScrollDownImage", "J j Shift+Down Down" );

    SETMOTION( "ScrollRightImage", "L Shift+Right" );
    SETMOTION( "ScrollLeftImage", "H Shift+Left" );

    // MESSAGE
    SETMOTION( "CancelWrite", "Alt+q" );
    SETMOTION( "ExecWrite", "Alt+w" ); 

    SETMOTION( "FocusWrite", "Tab" ); 

    // EDIT
    SETMOTION( "HomeEdit", "" );
    SETMOTION( "EndEdit", "" );

    SETMOTION( "UpEdit", "" );
    SETMOTION( "DownEdit", "" );
    SETMOTION( "RightEdit", "" );
    SETMOTION( "LeftEdit", "" );

    SETMOTION( "DeleteEdit", "" );
    SETMOTION( "BackspEdit", "" );

    SETMOTION( "UndoEdit", "Ctrl+/ Ctrl+z" );

    SETMOTION( "InputAA", "Alt+a" );
}



// ひとつの操作をデータベースに登録
void KeyConfig::set_one_motion( const std::string& name, const std::string& str_motion )
{
    if( name.empty() ) return;

#ifdef _DEBUG
    std::cout << "KeyConfig::set_motion " << name << std::endl;
    std::cout << "motion = " << str_motion << std::endl;
#endif

    int id = CONTROL::get_id( name );
    if( id == CONTROL::None ) return;

#ifdef _DEBUG
    std::cout << "id = " << id << std::endl;
#endif

    int mode = get_mode( id );
    if( mode == CONTROL::MODE_ERROR ) return;

    guint motion;
    bool ctrl = false;
    bool shift = false;
    bool alt = false;

    JDLIB::Regex regex;
    if( regex.exec( "(Ctrl)?(\\+?Shift)?(\\+?Alt)?\\+?(.*)", str_motion, 0, true ) ){

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
        else if( str_key == "BackSpace" ) motion = GDK_BackSpace;

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

        else if( str_key == "Menu" ) motion = GDK_Menu;

        else if( str_key == "Plus" ) motion = '+';

        else if( str_key == "Menu" ) motion = GDK_Menu;
        else motion = str_key[ 0 ];

        // 大文字やshiftが必要な文字の時はshiftも有効にする
        if( motion >= 'A' && motion <= 'Z' ) shift = true;
        if( motion == '?' || motion == '<' || motion == '>' || motion == '+' ) shift = true;

        int id_check = check_conflict( mode, motion, ctrl, shift, alt, false );
        if( id_check != CONTROL::None ){
            MISC::ERRMSG( "key config : " + str_motion + " is already used." );
            return;
        }
    }

#ifdef _DEBUG
    std::cout << "motion = " << motion << std::endl;
#endif

    bool dblclick = false;
    bool save = true;

    MouseKeyItem* item = new MouseKeyItem( id, mode, name, str_motion, motion, ctrl, shift, alt, dblclick, save );
    vec_items().push_back( item );
}



// 操作文字列取得
const std::string KeyConfig::get_str_motion( int id )
{
    std::string str_motion = MouseKeyConf::get_str_motion( id );
    str_motion = MISC::replace_str( str_motion, "Plus", "+" );

    return str_motion;
}


// editviewの操作をemacs風か
const bool KeyConfig::is_emacs_mode()
{
    return ( get_str_motion( CONTROL::UpEdit ).find( "Ctrl+p" ) != std::string::npos );
}


// editviewの操作をemacs風にする
void KeyConfig::toggle_emacs_mode()
{
    bool mode = is_emacs_mode();

    remove_items( CONTROL::HomeEdit );
    remove_items( CONTROL::HomeEdit );
    remove_items( CONTROL::EndEdit );

    remove_items( CONTROL::UpEdit );
    remove_items( CONTROL::DownEdit );
    remove_items( CONTROL::RightEdit );
    remove_items( CONTROL::LeftEdit );

    remove_items( CONTROL::DeleteEdit );

    if( mode ){

        set_one_motion( "HomeEdit", "" );
        set_one_motion( "EndEdit", "" );

        set_one_motion( "UpEdit", "" );
        set_one_motion( "DownEdit", "" );
        set_one_motion( "RightEdit", "" );
        set_one_motion( "LeftEdit", "" );

        set_one_motion( "DeleteEdit", "" );
    }
    else{

        set_one_motion( "HomeEdit", "Ctrl+a" );
        set_one_motion( "EndEdit", "Ctrl+e" );

        set_one_motion( "UpEdit", "Ctrl+p" );
        set_one_motion( "DownEdit", "Ctrl+n" );
        set_one_motion( "RightEdit", "Ctrl+f" );
        set_one_motion( "LeftEdit", "Ctrl+b" );

        set_one_motion( "DeleteEdit", "Ctrl+d" );
    }
}


// タブで開くキーを入れ替えているか
const bool KeyConfig::is_toggled_tab_key()
{
    return ( get_str_motion( CONTROL::OpenArticleTab ).find( "Space" ) != std::string::npos
             && get_str_motion( CONTROL::OpenArticleTab ).find( "Ctrl+Space" ) == std::string::npos );
}


// タブで開くキーを入れ替える
// toggle == true ならスペースをタブで開くボタンにする
void KeyConfig::toggle_tab_key( const bool toggle )
{
    remove_items( CONTROL::OpenBoard );
    remove_items( CONTROL::OpenBoardTab );
    remove_items( CONTROL::OpenArticle );
    remove_items( CONTROL::OpenArticleTab );

    if( toggle ){

        set_one_motion( "OpenBoard", "Ctrl+Space" );
        set_one_motion( "OpenBoardTab", "Space" );
        set_one_motion( "OpenArticle", "Ctrl+Space" );
        set_one_motion( "OpenArticleTab", "Space" );
    }
    else{

        set_one_motion( "OpenBoard", "Space" );
        set_one_motion( "OpenBoardTab", "Ctrl+Space" );
        set_one_motion( "OpenArticle", "Space" );
        set_one_motion( "OpenArticleTab", "Ctrl+Space" );
    }
}
