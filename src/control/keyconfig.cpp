// ライセンス: GPL2

//#define _DEBUG
//#define _DEBUG_XML
#include "jddebug.h"

#include "keyconfig.h"
#include "controlutil.h"
#include "defaultconf.h"

#include "jdlib/miscutil.h"
#include "jdlib/jdregex.h"
#include "jdlib/confloader.h"

#include "cache.h"


using namespace CONTROL;


KeyConfig::KeyConfig()
    : MouseKeyConf()
{}


KeyConfig::~KeyConfig()
{}



//
// 設定ファイル読み込み
//
// 設定を追加したら CONFIG::KeyPref::append_row() にも追加すること
//
void KeyConfig::load_conf()
{
    std::string str_motions;
    JDLIB::ConfLoader cf( CACHE::path_keyconf(), std::string() );

    // 共通設定
    SETKEYMOTION( "Up", KEYCONF_Up );
    SETKEYMOTION( "Down", KEYCONF_Down );

    SETKEYMOTION( "Right", KEYCONF_Right );
    SETKEYMOTION( "Left", KEYCONF_Left );

    SETKEYMOTION( "TabRight", KEYCONF_TabRight );
    SETKEYMOTION( "TabLeft", KEYCONF_TabLeft );
    SETKEYMOTION( "TabRightUpdated", KEYCONF_TabRightUpdated );
    SETKEYMOTION( "TabLeftUpdated", KEYCONF_TabLeftUpdated );

    SETKEYMOTION( "TabNum1", KEYCONF_TabNum1 );
    SETKEYMOTION( "TabNum2", KEYCONF_TabNum2 );
    SETKEYMOTION( "TabNum3", KEYCONF_TabNum3 );
    SETKEYMOTION( "TabNum4", KEYCONF_TabNum4 );
    SETKEYMOTION( "TabNum5", KEYCONF_TabNum5 );
    SETKEYMOTION( "TabNum6", KEYCONF_TabNum6 );
    SETKEYMOTION( "TabNum7", KEYCONF_TabNum7 );
    SETKEYMOTION( "TabNum8", KEYCONF_TabNum8 );
    SETKEYMOTION( "TabNum9", KEYCONF_TabNum9 );

    SETKEYMOTION( "RestoreLastTab", KEYCONF_RestoreLastTab );

    SETKEYMOTION( "PreBookMark", KEYCONF_PreBookMark );
    SETKEYMOTION( "NextBookMark", KEYCONF_NextBookMark );

    SETKEYMOTION( "PrevView", KEYCONF_PrevView );
    SETKEYMOTION( "NextView", KEYCONF_NextView );

    SETKEYMOTION( "ToggleArticle", KEYCONF_ToggleArticle );

    SETKEYMOTION( "ShowPopupMenu", KEYCONF_ShowPopupMenu );

    SETKEYMOTION( "ShowMenuBar", KEYCONF_ShowMenuBar );
    SETKEYMOTION( "ShowToolBarMain", KEYCONF_ShowToolBarMain );
    SETKEYMOTION( "ShowSideBar", KEYCONF_ShowSideBar );

    SETKEYMOTION( "PageUp", KEYCONF_PageUp );
    SETKEYMOTION( "PageDown", KEYCONF_PageDown );

    SETKEYMOTION( "PrevDir", KEYCONF_PrevDir );
    SETKEYMOTION( "NextDir", KEYCONF_NextDir );

    SETKEYMOTION( "Home", KEYCONF_Home );
    SETKEYMOTION( "End", KEYCONF_End );

    SETKEYMOTION( "Back", KEYCONF_Back );

    SETKEYMOTION( "Undo", KEYCONF_Undo );
    SETKEYMOTION( "Redo", KEYCONF_Redo );

    SETKEYMOTION( "Quit", KEYCONF_Quit );
    SETKEYMOTION( "Save", KEYCONF_Save );
    SETKEYMOTION( "Delete", KEYCONF_Delete );
    SETKEYMOTION( "Reload", KEYCONF_Reload );
    SETKEYMOTION( "ReloadArticle", KEYCONF_ReloadArticle );
    SETKEYMOTION( "StopLoading",  KEYCONF_StopLoading ); // = CONTROL::Cancel
    SETKEYMOTION( "OpenURL", KEYCONF_OpenURL );
    SETKEYMOTION( "Copy", KEYCONF_Copy );
    SETKEYMOTION( "SelectAll", KEYCONF_SelectAll );
    SETKEYMOTION( "AppendFavorite", KEYCONF_AppendFavorite );

    SETKEYMOTION( "Search", KEYCONF_Search );
    SETKEYMOTION( "SearchInvert", KEYCONF_SearchInvert );
    SETKEYMOTION( "SearchNext", KEYCONF_SearchNext );
    SETKEYMOTION( "SearchPrev", KEYCONF_SearchPrev );
    SETKEYMOTION( "DrawOutAnd", KEYCONF_DrawOutAnd );

    SETMOTION( "CheckUpdateRoot", KEYCONF_CheckUpdateRoot );
    SETMOTION( "CheckUpdateOpenRoot", KEYCONF_CheckUpdateOpenRoot );

    // BBSLIST
    SETKEYMOTION( "OpenBoard", KEYCONF_OpenBoard );
    SETKEYMOTION( "OpenBoardTab", KEYCONF_OpenBoardTab );

    // BOARD
    SETKEYMOTION( "OpenArticle", KEYCONF_OpenArticle );
    SETKEYMOTION( "OpenArticleTab", KEYCONF_OpenArticleTab );
    SETKEYMOTION( "NewArticle", KEYCONF_NewArticle );
    SETKEYMOTION( "SearchCache", KEYCONF_SearchCache );

    SETKEYMOTION( "ScrollRightBoard", KEYCONF_ScrollRightBoard );
    SETKEYMOTION( "ScrollLeftBoard", KEYCONF_ScrollLeftBoard );

    // ARTICLE
    SETKEYMOTION( "UpMid", KEYCONF_UpMid );
    SETKEYMOTION( "UpFast", KEYCONF_UpFast );

    SETKEYMOTION( "DownMid", KEYCONF_DownMid );
    SETKEYMOTION( "DownFast", KEYCONF_DownFast );

    SETKEYMOTION( "PrevRes", KEYCONF_PrevRes );
    SETKEYMOTION( "NextRes", KEYCONF_NextRes );

    SETKEYMOTION( "PrePost", KEYCONF_PrePost );
    SETKEYMOTION( "NextPost", KEYCONF_NextPost );

    SETKEYMOTION( "GotoNew", KEYCONF_GotoNew );
    SETKEYMOTION( "WriteMessage", KEYCONF_WriteMessage );

    SETKEYMOTION( "LiveStartStop", KEYCONF_LiveStartStop );

    SETKEYMOTION( "SearchNextArticle", KEYCONF_SearchNextArticle );
    SETKEYMOTION( "SearchWeb", KEYCONF_SearchWeb );
    SETKEYMOTION( "SearchTitle", KEYCONF_SearchTitle );
    SETKEYMOTION( "SearchCacheLocal", KEYCONF_SearchCacheLocal );
    SETKEYMOTION( "SearchCacheAll", KEYCONF_SearchCacheAll );

    // IMAGE
    SETKEYMOTION( "CancelMosaic", KEYCONF_CancelMosaic );
    SETKEYMOTION( "ZoomFitImage", KEYCONF_ZoomFitImage );
    SETKEYMOTION( "ZoomInImage", KEYCONF_ZoomInImage );
    SETKEYMOTION( "ZoomOutImage", KEYCONF_ZoomOutImage );
    SETKEYMOTION( "OrgSizeImage", KEYCONF_OrgSizeImage );

    SETKEYMOTION( "ScrollUpImage", KEYCONF_ScrollUpImage );
    SETKEYMOTION( "ScrollDownImage", KEYCONF_ScrollDownImage );
    SETKEYMOTION( "ScrollLeftImage", KEYCONF_ScrollLeftImage );
    SETKEYMOTION( "ScrollRightImage", KEYCONF_ScrollRightImage );

    // MESSAGE
    SETKEYMOTION( "CancelWrite", KEYCONF_CancelWrite );
    SETKEYMOTION( "ExecWrite", KEYCONF_ExecWrite );

    SETKEYMOTION( "FocusWrite", KEYCONF_FocusWrite );
    SETKEYMOTION( "ToggleSage", KEYCONF_ToggleSage );

    // EDIT
    SETKEYMOTION( "HomeEdit", KEYCONF_HomeEdit );
    SETKEYMOTION( "EndEdit", KEYCONF_EndEdit );

    SETKEYMOTION( "UpEdit", KEYCONF_UpEdit );
    SETKEYMOTION( "DownEdit", KEYCONF_DownEdit );
    SETKEYMOTION( "RightEdit", KEYCONF_RightEdit );
    SETKEYMOTION( "LeftEdit", KEYCONF_LeftEdit );

    SETKEYMOTION( "DeleteEdit", KEYCONF_DeleteEdit );
    SETKEYMOTION( "BackspEdit", KEYCONF_BackspEdit );
    SETKEYMOTION( "UndoEdit", KEYCONF_UndoEdit );
    SETKEYMOTION( "EnterEdit", KEYCONF_EnterEdit );

    SETKEYMOTION( "InputAA", KEYCONF_InputAA );
}



// ひとつの操作をデータベースに登録
void KeyConfig::set_one_motion_impl( const int id, const int mode, const std::string& name, const std::string& str_motion )
{
    if( name.empty() ) return;

#ifdef _DEBUG
    std::cout << "KeyConfig::set_one_motion_impl " << name << std::endl;
    std::cout << "motion = " << str_motion << std::endl;
#endif

#ifdef _DEBUG
    std::cout << CONTROL::get_label( id  ) << std::endl;
#endif

    guint key = 0;
    bool ctrl = false;
    bool shift = false;
    bool alt = false;
    const bool dblclick = false;
    const bool trpclick = false;

    JDLIB::Regex regex;
    const size_t offset = 0;
    const bool icase = true; // 大文字小文字区別しない
    const bool newline = true;
    const bool usemigemo = false;
    const bool wchar = false;

    if( regex.exec( "(Ctrl\\+)?(Shift\\+)?(Alt\\+)?(.*)", str_motion, offset, icase, newline, usemigemo, wchar ) ){

        if( ! regex.str( 1 ).empty() ) ctrl = true;
        if( ! regex.str( 2 ).empty() ) shift = true;
        if( ! regex.str( 3 ).empty() ) alt = true;

        const std::string str_key = regex.str( 4 );
        if( str_key.empty() ) return;

        key = CONTROL::get_keysym( str_key );

        // keyがアスキー文字の場合は shift を無視する (大文字除く)
        // Control::key_press() も参照せよ
        if( CONTROL::is_ascii( key ) ){
            if( str_motion.length() == 1 && key >= 'A' && key <= 'Z' ) shift = true;
            else shift = false;
        }
    }
    else return;

#ifdef _DEBUG
    std::cout << "str_motion = " << str_motion << " key = " << key;
    if( ctrl ) std::cout << " ctrl";
    if( shift ) std::cout << " shift";
    if( alt ) std::cout << " alt";
    std::cout << "\n";
#endif

    vec_items().push_back( MouseKeyItem( id, mode, name, str_motion, key, ctrl, shift, alt, dblclick, trpclick ) );
}


// editviewの操作をemacs風か
const bool KeyConfig::is_emacs_mode()
{
    return ( get_str_motions( CONTROL::UpEdit ).find( "Ctrl+p" ) != std::string::npos );
}


// editviewの操作をemacs風にする
void KeyConfig::toggle_emacs_mode()
{
    bool mode = is_emacs_mode();

    remove_motions( CONTROL::HomeEdit );
    remove_motions( CONTROL::HomeEdit );
    remove_motions( CONTROL::EndEdit );

    remove_motions( CONTROL::UpEdit );
    remove_motions( CONTROL::DownEdit );
    remove_motions( CONTROL::RightEdit );
    remove_motions( CONTROL::LeftEdit );

    remove_motions( CONTROL::DeleteEdit );
    remove_motions( CONTROL::EnterEdit );

    if( mode ){

        set_one_motion( "HomeEdit", KEYCONF_HomeEdit );
        set_one_motion( "EndEdit", KEYCONF_EndEdit );

        set_one_motion( "UpEdit", KEYCONF_UpEdit );
        set_one_motion( "DownEdit", KEYCONF_DownEdit );
        set_one_motion( "RightEdit", KEYCONF_RightEdit );
        set_one_motion( "LeftEdit", KEYCONF_LeftEdit );

        set_one_motion( "DeleteEdit", KEYCONF_DeleteEdit );
        set_one_motion( "EnterEdit", KEYCONF_EnterEdit );
    }
    else{

        set_one_motion( "HomeEdit", "Ctrl+a" );
        set_one_motion( "EndEdit", "Ctrl+e" );

        set_one_motion( "UpEdit", "Ctrl+p" );
        set_one_motion( "DownEdit", "Ctrl+n" );
        set_one_motion( "RightEdit", "Ctrl+f" );
        set_one_motion( "LeftEdit", "Ctrl+b" );

        set_one_motion( "DeleteEdit", "Ctrl+d" );
        set_one_motion( "EnterEdit", "Ctrl+m" );
    }
}


// タブで開くキーを入れ替えているか
const bool KeyConfig::is_toggled_tab_key()
{
    const bool ret = ( get_str_motions( CONTROL::OpenBoard ).find( "Ctrl+Space" ) != std::string::npos
                       && get_str_motions( CONTROL::OpenBoardTab ).find( "Space" ) != std::string::npos

                       && get_str_motions( CONTROL::OpenArticle ).find( "Ctrl+Space" ) != std::string::npos
                       && get_str_motions( CONTROL::OpenArticleTab ).find( "Space" ) != std::string::npos
        );

#ifdef _DEBUG
    std::cout << "KeyConfig::is_toggled_tab_key ret = " << ret << std::endl;
#endif

    return ret;
}


// タブで開くキーを入れ替える
// toggle == true ならスペースをタブで開くボタンにする
void KeyConfig::toggle_tab_key( const bool toggle )
{
    remove_motions( CONTROL::OpenBoard );
    remove_motions( CONTROL::OpenBoardTab );
    remove_motions( CONTROL::OpenArticle );
    remove_motions( CONTROL::OpenArticleTab );

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
