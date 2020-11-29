// ライセンス: GPL2

//#define _DEBUG
//#define _DEBUG_XML
#include "jddebug.h"

#include "keyconfig.h"
#include "controlutil.h"
#include "defaultconf.h"

#include "config/globalconf.h"

#include "jdlib/miscutil.h"
#include "jdlib/jdregex.h"
#include "jdlib/confloader.h"

#include "cache.h"


using namespace CONTROL;


KeyConfig::~KeyConfig() noexcept = default;



//
// 設定ファイル読み込み
//
// 設定を追加したら CONFIG::KeyPref::append_row() にも追加すること
//
void KeyConfig::load_conf()
{
    JDLIB::ConfLoader cf( CACHE::path_keyconf(), std::string() );

    // 共通設定
    load_keymotions( cf, "Up", KEYCONF_Up );
    load_keymotions( cf, "Down", KEYCONF_Down );

    load_keymotions( cf, "Right", KEYCONF_Right );
    load_keymotions( cf, "Left", KEYCONF_Left );

    load_keymotions( cf, "TabRight", KEYCONF_TabRight );
    load_keymotions( cf, "TabLeft", KEYCONF_TabLeft );
    load_keymotions( cf, "TabRightUpdated", KEYCONF_TabRightUpdated );
    load_keymotions( cf, "TabLeftUpdated", KEYCONF_TabLeftUpdated );

    load_keymotions( cf, "TabNum1", KEYCONF_TabNum1 );
    load_keymotions( cf, "TabNum2", KEYCONF_TabNum2 );
    load_keymotions( cf, "TabNum3", KEYCONF_TabNum3 );
    load_keymotions( cf, "TabNum4", KEYCONF_TabNum4 );
    load_keymotions( cf, "TabNum5", KEYCONF_TabNum5 );
    load_keymotions( cf, "TabNum6", KEYCONF_TabNum6 );
    load_keymotions( cf, "TabNum7", KEYCONF_TabNum7 );
    load_keymotions( cf, "TabNum8", KEYCONF_TabNum8 );
    load_keymotions( cf, "TabNum9", KEYCONF_TabNum9 );

    load_keymotions( cf, "RestoreLastTab", KEYCONF_RestoreLastTab );

    load_keymotions( cf, "PreBookMark", KEYCONF_PreBookMark );
    load_keymotions( cf, "NextBookMark", KEYCONF_NextBookMark );

    load_keymotions( cf, "PrevView", KEYCONF_PrevView );
    load_keymotions( cf, "NextView", KEYCONF_NextView );

    load_keymotions( cf, "ToggleArticle", KEYCONF_ToggleArticle );

    load_keymotions( cf, "ShowPopupMenu", KEYCONF_ShowPopupMenu );

    load_keymotions( cf, "ShowMenuBar", KEYCONF_ShowMenuBar );
    load_keymotions( cf, "ShowToolBarMain", KEYCONF_ShowToolBarMain );
    load_keymotions( cf, "ShowSideBar", KEYCONF_ShowSideBar );

    load_keymotions( cf, "PageUp", KEYCONF_PageUp );
    load_keymotions( cf, "PageDown", KEYCONF_PageDown );

    load_keymotions( cf, "PrevDir", KEYCONF_PrevDir );
    load_keymotions( cf, "NextDir", KEYCONF_NextDir );

    load_keymotions( cf, "Home", KEYCONF_Home );
    load_keymotions( cf, "End", KEYCONF_End );

    load_keymotions( cf, "Back", KEYCONF_Back );

    load_keymotions( cf, "Undo", KEYCONF_Undo );
    load_keymotions( cf, "Redo", KEYCONF_Redo );

    load_keymotions( cf, "Quit", KEYCONF_Quit );
    load_keymotions( cf, "Save", KEYCONF_Save );
    load_keymotions( cf, "Delete", KEYCONF_Delete );
    load_keymotions( cf, "Reload", KEYCONF_Reload );
    load_keymotions( cf, "ReloadArticle", KEYCONF_ReloadArticle );
    load_keymotions( cf, "StopLoading",  KEYCONF_StopLoading ); // = CONTROL::Cancel
    load_keymotions( cf, "OpenURL", KEYCONF_OpenURL );
    load_keymotions( cf, "Copy", KEYCONF_Copy );
    load_keymotions( cf, "SelectAll", KEYCONF_SelectAll );
    load_keymotions( cf, "AppendFavorite", KEYCONF_AppendFavorite );
    load_motions( cf, "PreferenceView", KEYCONF_PreferenceView );

    load_keymotions( cf, "Search", KEYCONF_Search );
    load_keymotions( cf, "SearchInvert", KEYCONF_SearchInvert );
    load_keymotions( cf, "SearchNext", KEYCONF_SearchNext );
    load_keymotions( cf, "SearchPrev", KEYCONF_SearchPrev );
    load_keymotions( cf, "SearchTitle", KEYCONF_SearchTitle );
    load_keymotions( cf, "DrawOutAnd", KEYCONF_DrawOutAnd );

    load_motions( cf, "CheckUpdateRoot", KEYCONF_CheckUpdateRoot );
    load_motions( cf, "CheckUpdateOpenRoot", KEYCONF_CheckUpdateOpenRoot );

    load_motions( cf, "FullScreen", KEYCONF_FullScreen );

    // BBSLIST
    load_keymotions( cf, "OpenBoard", KEYCONF_OpenBoard );
    load_keymotions( cf, "OpenBoardTab", KEYCONF_OpenBoardTab );

    // BOARD
    load_keymotions( cf, "OpenArticle", KEYCONF_OpenArticle );
    load_keymotions( cf, "OpenArticleTab", KEYCONF_OpenArticleTab );
    load_keymotions( cf, "NewArticle", KEYCONF_NewArticle );
    load_keymotions( cf, "SearchCache", KEYCONF_SearchCache );

    load_keymotions( cf, "ScrollRightBoard", KEYCONF_ScrollRightBoard );
    load_keymotions( cf, "ScrollLeftBoard", KEYCONF_ScrollLeftBoard );

    // ARTICLE
    load_keymotions( cf, "UpMid", KEYCONF_UpMid );
    load_keymotions( cf, "UpFast", KEYCONF_UpFast );

    load_keymotions( cf, "DownMid", KEYCONF_DownMid );
    load_keymotions( cf, "DownFast", KEYCONF_DownFast );

    load_keymotions( cf, "PrevRes", KEYCONF_PrevRes );
    load_keymotions( cf, "NextRes", KEYCONF_NextRes );

    load_keymotions( cf, "PrePost", KEYCONF_PrePost );
    load_keymotions( cf, "NextPost", KEYCONF_NextPost );

    load_keymotions( cf, "GotoNew", KEYCONF_GotoNew );
    load_keymotions( cf, "WriteMessage", KEYCONF_WriteMessage );

    load_keymotions( cf, "LiveStartStop", KEYCONF_LiveStartStop );

    load_keymotions( cf, "SearchNextArticle", KEYCONF_SearchNextArticle );
    load_keymotions( cf, "SearchWeb", KEYCONF_SearchWeb );
    load_keymotions( cf, "SearchCacheLocal", KEYCONF_SearchCacheLocal );
    load_keymotions( cf, "SearchCacheAll", KEYCONF_SearchCacheAll );

    load_keymotions( cf, "ShowSelectImage", KEYCONF_ShowSelectImage );
    load_keymotions( cf, "DeleteSelectImage", KEYCONF_DeleteSelectImage );
    load_keymotions( cf, "AboneSelectImage", KEYCONF_AboneSelectImage );
    load_keymotions( cf, "AboneSelectionRes", KEYCONF_AboneSelectionRes );

    // IMAGE
    load_keymotions( cf, "CancelMosaic", KEYCONF_CancelMosaic );
    load_keymotions( cf, "ZoomFitImage", KEYCONF_ZoomFitImage );
    load_keymotions( cf, "ZoomInImage", KEYCONF_ZoomInImage );
    load_keymotions( cf, "ZoomOutImage", KEYCONF_ZoomOutImage );
    load_keymotions( cf, "OrgSizeImage", KEYCONF_OrgSizeImage );

    load_keymotions( cf, "ScrollUpImage", KEYCONF_ScrollUpImage );
    load_keymotions( cf, "ScrollDownImage", KEYCONF_ScrollDownImage );
    load_keymotions( cf, "ScrollLeftImage", KEYCONF_ScrollLeftImage );
    load_keymotions( cf, "ScrollRightImage", KEYCONF_ScrollRightImage );

    // MESSAGE
    load_keymotions( cf, "CancelWrite", KEYCONF_CancelWrite );
    load_keymotions( cf, "ExecWrite", KEYCONF_ExecWrite );

    load_keymotions( cf, "FocusWrite", KEYCONF_FocusWrite );
    load_keymotions( cf, "ToggleSage", KEYCONF_ToggleSage );

    // EDIT
    load_keymotions( cf, "HomeEdit", KEYCONF_HomeEdit );
    load_keymotions( cf, "EndEdit", KEYCONF_EndEdit );

    load_keymotions( cf, "UpEdit", KEYCONF_UpEdit );
    load_keymotions( cf, "DownEdit", KEYCONF_DownEdit );
    load_keymotions( cf, "RightEdit", KEYCONF_RightEdit );
    load_keymotions( cf, "LeftEdit", KEYCONF_LeftEdit );

    load_keymotions( cf, "DeleteEdit", KEYCONF_DeleteEdit );
    load_keymotions( cf, "BackspEdit", KEYCONF_BackspEdit );
    load_keymotions( cf, "UndoEdit", KEYCONF_UndoEdit );
    load_keymotions( cf, "EnterEdit", KEYCONF_EnterEdit );

    load_keymotions( cf, "InputAA", KEYCONF_InputAA );

    // JD globals
    load_motions( cf, "JDExit", KEYCONF_JDExit );
    if( CONFIG::get_disable_close() ) remove_motions( CONTROL::JDExit );

    load_motions( cf, "JDHelp", KEYCONF_JDHelp );
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

        if( regex.length( 1 ) ) ctrl = true;
        if( regex.length( 2 ) ) shift = true;
        if( regex.length( 3 ) ) alt = true;

        const std::string str_key = regex.str( 4 );
        if( str_key.empty() ) return;

        key = CONTROL::get_keysym( str_key );

        // keyがアスキー文字の場合は shift を無視する (大文字除く)
        // Control::key_press() も参照せよ
        if( CONTROL::is_ascii( key ) ){
            if( regex.length( 4 ) == 1 && key >= 'A' && key <= 'Z' ) shift = true;
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
bool KeyConfig::is_emacs_mode()
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
bool KeyConfig::is_toggled_tab_key()
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


//
// Gtk アクセラレーションキーを取得
//
Gtk::AccelKey KeyConfig::get_accelkey( const int id )
{
    guint motion;
    bool ctrl, shift, alt, dblclick, trpclick;

    bool found = get_motion( id, motion, ctrl, shift, alt, dblclick, trpclick );
    if( ! found ){
#ifdef _DEBUG
        std::cout << "KeyConfig::get_accelkey id = " << id
                  << " (" << CONTROL::get_name( id ) << ") notfound " << std::endl;
#endif
        return Gtk::AccelKey();
    }

    Gdk::ModifierType type =  static_cast<Gdk::ModifierType>(0);
    if( ctrl ) type |= Gdk::CONTROL_MASK;
    if( shift ) type |= Gdk::SHIFT_MASK;
    if( alt) type |= Gdk::MOD1_MASK;

#ifdef _DEBUG
    std::cout << "KeyConfig::get_accelkey id = " << id
              << " (" << CONTROL::get_name( id ) << ") motion = " << motion
              << " ctrl = " << ctrl << " shift = " << shift << " alt = " << alt << std::endl;
#endif
    return Gtk::AccelKey( motion, type );
}
