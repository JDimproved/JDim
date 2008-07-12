// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "session.h"
#include "cache.h"
#include "global.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "jdlib/confloader.h"
#include "jdlib/miscutil.h"

#include "bbslist/bbslistadmin.h"
#include "article/articleadmin.h"

#include <sstream>
#include <cstring>
#include <vector>

#ifdef HAVE_SYS_UTSNAME_H
#include <sys/utsname.h> // uname()
#endif

bool booting = true;
bool quitting = false;

int mode_pane;
bool mode_online;
bool mode_login2ch;
bool mode_loginbe;

std::string distribution_name;

int win_manager;

int win_hpane_main_pos;
int win_vpane_main_pos;
int win_hpane_main_r_pos;
int win_vpane_main_mes_pos;

int win_notebook_main_page;

int win_bbslist_page;
int win_board_page;
int win_article_page;
int win_image_page;

std::list< std::string > board_urls;
std::list< bool > board_locked;

std::list< std::string > article_urls;
std::list< bool > article_locked;

std::list< std::string > image_urls;
std::list< bool > image_locked;

std::string items_sidebar_str;
std::vector< int > items_sidebar;

std::string items_main_toolbar_str;
std::vector< int > items_main_toolbar;

std::string items_article_toolbar_str;
std::vector< int > items_article_toolbar;

std::string items_board_toolbar_str;
std::vector< int > items_board_toolbar;

std::string items_board_str;
std::vector< int > items_board;

std::string items_msg_toolbar_str;
std::vector< int > items_msg_toolbar;

int board_col_mark;
int board_col_id;
int board_col_subject;
int board_col_number;
int board_col_load;
int board_col_new;
int board_col_since;
int board_col_write;
int board_col_speed;

bool win_show_sidebar;

bool win_show_menubar;

int win_toolbar_pos;

bool show_bbslist_toolbar;
bool show_board_toolbar;
bool show_article_toolbar;

bool show_board_tab;
bool show_article_tab;

int win_focused_admin;
int win_focused_admin_sidebar;

int x_win_main;
int y_win_main;
int x_win_img;
int y_win_img;
int x_win_mes;
int y_win_mes;


int width_win_main;
int height_win_main;
int width_win_img;
int height_win_img;
int width_win_mes;
int height_win_mes;


bool focus_win_main = false;
bool focus_win_img = false;
bool focus_win_mes = false;

bool maximized_win_main = false;
bool maximized_win_img = false;
bool maximized_win_mes = false;

bool iconified_win_main = false;
bool iconified_win_img = false;
bool iconified_win_mes = false;

bool shown_win_main = false;
bool shown_win_img = false;
bool shown_win_mes = false;

bool dialog_shown = false;

bool embedded_img;

bool embedded_mes;
bool close_mes;

std::string img_dir_dat_save;
std::string img_dir_img_save;

std::string dir_draft;

bool popupmenu_shown;

std::vector< std::string > delete_list;

std::vector< std::string > live_urls;


/////////////////////////////////////


// 項目名 -> 番号変換
std::vector< int > parse_items( const std::string& items_str )
{
    std::vector< int > items;
    std::list< std::string > list_order = MISC::split_line( items_str );
    std::list< std::string >::iterator it = list_order.begin();
    for( ; it != list_order.end(); ++it ){

        if( *it == ITEM_NAME_BBSLISTVIEW ) items.push_back(ITEM_BBSLISTVIEW );
        if( *it == ITEM_NAME_FAVORITEVIEW ) items.push_back(ITEM_FAVORITEVIEW );
        if( *it == ITEM_NAME_BOARDVIEW ) items.push_back(ITEM_BOARDVIEW );
        if( *it == ITEM_NAME_ARTICLEVIEW ) items.push_back(ITEM_ARTICLEVIEW );
        if( *it == ITEM_NAME_IMAGEVIEW ) items.push_back(ITEM_IMAGEVIEW );
        if( *it == ITEM_NAME_URL ) items.push_back(ITEM_URL );
        if( *it == ITEM_NAME_GO ) items.push_back(ITEM_GO );
        if( *it == ITEM_NAME_SEPARATOR ) items.push_back(ITEM_SEPARATOR );

        if( *it == ITEM_NAME_MARK ) items.push_back(ITEM_MARK );
        if( *it == ITEM_NAME_ID ) items.push_back(ITEM_ID );
        if( *it == ITEM_NAME_NAME ) items.push_back(ITEM_NAME );
        if( *it == ITEM_NAME_RES ) items.push_back(ITEM_RES );
        if( *it == ITEM_NAME_LOAD ) items.push_back(ITEM_LOAD );
        if( *it == ITEM_NAME_NEW ) items.push_back(ITEM_NEW );
        if( *it == ITEM_NAME_SINCE ) items.push_back(ITEM_SINCE );
        if( *it == ITEM_NAME_LASTWRITE ) items.push_back(ITEM_LASTWRITE );
        if( *it == ITEM_NAME_SPEED ) items.push_back(ITEM_SPEED );

        if( *it == ITEM_NAME_WRITEMSG ) items.push_back( ITEM_WRITEMSG );
        if( *it == ITEM_NAME_OPENBOARD ) items.push_back(ITEM_OPENBOARD );
        if( *it == ITEM_NAME_SEARCH ) items.push_back(ITEM_SEARCH );
        if( *it == ITEM_NAME_RELOAD ) items.push_back( ITEM_RELOAD );
        if( *it == ITEM_NAME_STOPLOADING ) items.push_back( ITEM_STOPLOADING );
        if( *it == ITEM_NAME_FAVORITE ) items.push_back( ITEM_FAVORITE );
        if( *it == ITEM_NAME_DELETE ) items.push_back( ITEM_DELETE );
        if( *it == ITEM_NAME_QUIT ) items.push_back( ITEM_QUIT );
        if( *it == ITEM_NAME_PREVVIEW ) items.push_back( ITEM_PREVVIEW );
        if( *it == ITEM_NAME_NEXTVIEW ) items.push_back( ITEM_NEXTVIEW );
        if( *it == ITEM_NAME_LOCK ) items.push_back( ITEM_LOCK );
        if( *it == ITEM_NAME_LIVE ) items.push_back( ITEM_LIVE );

        if( *it == ITEM_NAME_NEWARTICLE ) items.push_back( ITEM_NEWARTICLE );
        if( *it == ITEM_NAME_SEARCHBOX ) items.push_back( ITEM_SEARCHBOX );
        if( *it == ITEM_NAME_SEARCH_NEXT ) items.push_back( ITEM_SEARCH_NEXT );
        if( *it == ITEM_NAME_SEARCH_PREV ) items.push_back( ITEM_SEARCH_PREV );

        if( *it == ITEM_NAME_INSERTTEXT ) items.push_back( ITEM_INSERTTEXT );
        if( *it == ITEM_NAME_LOCK_MESSAGE ) items.push_back( ITEM_LOCK_MESSAGE );
        if( *it == ITEM_NAME_PREVIEW ) items.push_back( ITEM_PREVIEW );
        if( *it == ITEM_NAME_UNDO ) items.push_back( ITEM_UNDO );
    }
    items.push_back( ITEM_END );

    return items;
}


void read_list_urls( JDLIB::ConfLoader& cf, const std::string& id_urls, const std::string& id_locked,
                     std::list< std::string >& list_urls , std::list< bool >& list_locked )
{
    std::string str_tmp;
    std::list< std::string > list_tmp;
    std::list< std::string >::iterator it_tmp;

    str_tmp = cf.get_option( id_urls, "");
    if( ! str_tmp.empty() ){
        list_tmp = MISC::split_line( str_tmp );
        it_tmp = list_tmp.begin();
        for( ; it_tmp != list_tmp.end(); ++it_tmp ) if( !(*it_tmp).empty() ) list_urls.push_back( (*it_tmp));
    }

    str_tmp = cf.get_option( id_locked, "");
    if( ! str_tmp.empty() ){
        list_tmp = MISC::split_line( str_tmp );
        it_tmp = list_tmp.begin();
        for( ; it_tmp != list_tmp.end(); ++it_tmp ){
            if( ( *it_tmp ) == "1" ) list_locked.push_back( true );
            else list_locked.push_back( false );
        }
    }
}


// ファイル等からディストリ名を取得
std::string get_distribution_name_from_environment()
{
#ifdef _DEBUG
    std::cout << "SESSION::get_distribution_name_from_environment\n";
#endif

    std::string tmp;
    std::string text_data;

    // LSB系 ( Ubuntu ..etc )
    if( CACHE::load_rawdata( "/etc/lsb-release", text_data ) )
    {
        std::list< std::string > lines = MISC::get_lines( text_data );
        std::list< std::string >::reverse_iterator it = lines.rbegin();
        while( it != lines.rend() )
        {
            std::string lsb_name, lsb_data;

            size_t e;
            if( ( e = (*it).find( "=" ) ) != std::string::npos )
            {
                lsb_name = MISC::remove_spaces( (*it).substr( 0, e ) );
                lsb_data = MISC::remove_spaces( (*it).substr( e + 1 ) );
            }

            // 「DISTRIB_DESCRIPTION="Ubuntu 7.10"」などから「Ubuntu 7.10」を取得
            if( lsb_name == "DISTRIB_DESCRIPTION" && ! lsb_data.empty() )
            {
                tmp = MISC::cut_str( lsb_data, "\"", "\"" );
                break;
            }

            ++it;
        }
    }
    // KNOPPIX (LSB？)
    else if( CACHE::load_rawdata( "/etc/knoppix-version", text_data ) )
    {
        tmp = "KNOPPIX ";
        tmp.append( text_data );
    }
    // Debian
    else if( CACHE::load_rawdata( "/etc/debian_version", text_data ) )
    {
        tmp = "Debian GNU/Linux ";
        tmp.append( text_data );
    }
    // Solaris系
    else if( CACHE::load_rawdata( "/etc/release", text_data ) )
    {
        std::list< std::string > lines = MISC::get_lines( text_data );
        std::list< std::string >::iterator it = lines.begin();
        while( it != lines.end() )
        {
            // 名前が含まれている行を取得
            if( (*it).find( "BeleniX" ) != std::string::npos
                || (*it).find( "Nexenta" ) != std::string::npos
                || (*it).find( "SchilliX" ) != std::string::npos
                || (*it).find( "Solaris" ) != std::string::npos )
            {
                tmp = *it;
                break;
            }

            ++it;
        }
    }
    // ファイルの中身がそのままディストリ名として扱える物
    else
    {
        // ディストリ名が書かれているファイル
        std::string dist_files[] =
        {
            "/etc/arch-release",
            "/etc/fedora-release",
            "/etc/gentoo-release",
            "/etc/lfs-release",
            "/etc/mandriva-release",
            "/etc/momonga-release",
            "/usr/lib/setup/plamo-version",
            "/etc/puppyversion",
            "/etc/redhat-release", // Redhat, CentOS, WhiteBox, PCLinuxOS
            "/etc/sabayon-release",
            "/etc/slackware-version",
            "/etc/SuSE-release",
            "/etc/turbolinux-release",
            "/etc/vine-release",
            "/etc/zenwalk-version"
        };

        unsigned int i;
        for( i = 0; i < sizeof( dist_files ) / sizeof( std::string ); ++i )
        {
            if( CACHE::load_rawdata( dist_files[i], text_data ) )
            {
                tmp = text_data;
                break;
            }
        }
    }

    // 文字列両端のスペースなどを削除する
    std::string dist_name = MISC::remove_spaces( tmp );

    // 取得した文字が異常に長い場合は空にする
    if( dist_name.length() > 50 ) dist_name.clear();

#ifdef HAVE_SYS_UTSNAME_H

    char *sysname = 0, *release = 0, *machine = 0;

    // システムコール uname() 準拠：SVr4, POSIX.1-2001.
    struct utsname* uts;
    uts = (struct utsname*)malloc( sizeof( struct utsname ) );
    if( uname( uts ) != -1 )
    {
        sysname = uts->sysname;
        release = uts->release;
        machine = uts->machine;
    }

    // FreeBSD等やディストリ名が取得できなかった場合は"$ uname -rs"と同じ様式
    if( dist_name.empty() && sysname && release )
    {
        dist_name = std::string( sysname ) + " " + std::string( release );
    }

    // アーキテクチャがx86でない場合
    if( machine &&
        ( strlen( machine ) != 4
          || ! ( machine[0] == 'i'
               && machine[1] >= '3' && machine[1] <= '6'
               && machine[2] == '8' && machine[3] == '6' ) ) )
    {
        dist_name.append( " (" + std::string( machine ) + ")" );
    }

    free( uts );
    uts = NULL;

#endif

    return dist_name;
}


// セッション情報読み込み
void SESSION::init_session()
{
#ifdef _DEBUG
    std::cout << "SESSION::init_session\n";
#endif

    JDLIB::ConfLoader cf( CACHE::path_session(), std::string() );

    // オンライン
    mode_online = cf.get_option( "mode_online", true );

    // 2chログイン
    mode_login2ch = cf.get_option( "mode_login2ch", false );

    // beログイン
    mode_loginbe = cf.get_option( "mode_loginbe", false );

    // paneのモード
    mode_pane = cf.get_option( "mode_pane", 0 );

    x_win_main = cf.get_option( "x", 0 );
    y_win_main = cf.get_option( "y", 0 );
    width_win_main = cf.get_option( "width", 800 );
    height_win_main = cf.get_option( "height", 600 );
    maximized_win_main = cf.get_option( "maximized", false );

    win_show_sidebar = cf.get_option( "show_sidebar", true );

    win_show_menubar = cf.get_option( "show_menubar", true );

    win_toolbar_pos = cf.get_option( "toolbar_pos", 0 );

    show_bbslist_toolbar = cf.get_option( "show_bbslist_toolbar", true );
    show_board_toolbar = cf.get_option( "show_board_toolbar", true );
    show_article_toolbar = cf.get_option( "show_article_toolbar", true );

    show_board_tab = cf.get_option( "show_board_tab", true );
    show_article_tab = cf.get_option( "show_article_tab", true );

    win_focused_admin = cf.get_option( "focused_admin", FOCUS_NO );
    win_focused_admin_sidebar = cf.get_option( "focused_admin_sidebar", FOCUS_NO );

    win_hpane_main_pos = cf.get_option( "hpane_main_pos", 190 );
    win_vpane_main_pos = cf.get_option( "vpane_main_pos", 200 );
    win_hpane_main_r_pos = cf.get_option( "hpane_main_r_pos", 300 );
    win_vpane_main_mes_pos = cf.get_option( "vpane_main_mes_pos", 400 );

    win_notebook_main_page = cf.get_option( "notebook_main_page", 0 );

    win_bbslist_page = cf.get_option( "bbslist_page", 0 );
    win_board_page = cf.get_option( "board_page", 0 );
    win_article_page = cf.get_option( "article_page", 0 );
    win_image_page = cf.get_option( "image_page", 0 );

    read_list_urls( cf, "board_urls", "board_locked", board_urls, board_locked );
    read_list_urls( cf, "article_urls", "article_locked", article_urls, article_locked );
    read_list_urls( cf, "image_urls", "image_locked", image_urls, image_locked );

    items_sidebar_str = cf.get_option( "items_sidebar",
                                 ITEM_NAME_SEARCHBOX + std::string ( " " ) +
                                 ITEM_NAME_SEARCH_NEXT + std::string ( " " ) +
                                 ITEM_NAME_SEARCH_PREV + std::string ( " " ) );

    items_sidebar =  parse_items( items_sidebar_str );

    items_main_toolbar_str = cf.get_option( "items_main_toolbar",
                                 ITEM_NAME_BBSLISTVIEW + std::string ( " " ) +
                                 ITEM_NAME_FAVORITEVIEW + std::string ( " " ) +
                                 ITEM_NAME_BOARDVIEW + std::string ( " " ) +
                                 ITEM_NAME_ARTICLEVIEW + std::string ( " " ) +
                                 ITEM_NAME_IMAGEVIEW + std::string ( " " ) +
                                 ITEM_NAME_SEPARATOR + std::string ( " " ) +
                                 ITEM_NAME_URL + std::string ( " " ) +
                                 ITEM_NAME_GO + std::string ( " " ) );

    items_main_toolbar =  parse_items( items_main_toolbar_str );

    items_article_toolbar_str = cf.get_option( "items_article_toolbar",
                                 ITEM_NAME_WRITEMSG + std::string ( " " ) +
                                 ITEM_NAME_OPENBOARD + std::string ( " " ) +
                                 ITEM_NAME_NAME + std::string ( " " ) +
                                 ITEM_NAME_SEARCH + std::string ( " " ) +
                                 ITEM_NAME_RELOAD + std::string ( " " ) +
                                 ITEM_NAME_STOPLOADING + std::string ( " " ) +
                                 ITEM_NAME_FAVORITE + std::string ( " " ) +
                                 ITEM_NAME_DELETE + std::string ( " " ) +
                                 ITEM_NAME_QUIT + std::string ( " " ) );

    items_article_toolbar =  parse_items( items_article_toolbar_str );

    items_board_toolbar_str = cf.get_option( "items_board_toolbar",
                                 ITEM_NAME_NEWARTICLE + std::string ( " " ) +
                                 ITEM_NAME_SEARCHBOX + std::string ( " " ) +
                                 ITEM_NAME_SEARCH_NEXT + std::string ( " " ) +
                                 ITEM_NAME_SEARCH_PREV + std::string ( " " ) +
                                 ITEM_NAME_RELOAD + std::string ( " " ) +
                                 ITEM_NAME_STOPLOADING + std::string ( " " ) +
                                 ITEM_NAME_FAVORITE + std::string ( " " ) +
                                 ITEM_NAME_DELETE + std::string ( " " ) +
                                 ITEM_NAME_QUIT + std::string ( " " ) );

    items_board_toolbar =  parse_items( items_board_toolbar_str );

    items_board_str = cf.get_option( "items_board",
                                 ITEM_NAME_MARK + std::string ( " " ) +
                                 ITEM_NAME_ID + std::string ( " " ) +
                                 ITEM_NAME_NAME + std::string ( " " ) +
                                 ITEM_NAME_RES + std::string ( " " ) +
                                 ITEM_NAME_LOAD + std::string ( " " ) +
                                 ITEM_NAME_NEW + std::string ( " " ) +
                                 ITEM_NAME_SINCE + std::string ( " " ) +
                                 ITEM_NAME_LASTWRITE + std::string ( " " ) +
                                 ITEM_NAME_SPEED + std::string ( " " ) );

    items_board =  parse_items( items_board_str );

    items_msg_toolbar_str = cf.get_option( "items_msg_toolbar",
                                 ITEM_NAME_PREVIEW + std::string ( " " ) +
                                 ITEM_NAME_WRITEMSG+ std::string ( " " ) +
                                 ITEM_NAME_NAME + std::string ( " " ) +
                                 ITEM_NAME_UNDO + std::string ( " " ) +
                                 ITEM_NAME_INSERTTEXT + std::string ( " " ) +
                                 ITEM_NAME_LOCK_MESSAGE + std::string ( " " ) +
                                 ITEM_NAME_QUIT + std::string ( " " ) );

    items_msg_toolbar =  parse_items( items_msg_toolbar_str );

    board_col_mark = cf.get_option( "col_mark", 30 );
    board_col_id = cf.get_option( "col_id", 45 );
    board_col_subject = cf.get_option(  "col_subject", 190 );
    board_col_number = cf.get_option( "col_number", 45 );
    board_col_load = cf.get_option( "col_load", 45 );
    board_col_new = cf.get_option( "col_new", 45 );
    board_col_since = cf.get_option( "col_since", 70 );
    board_col_write = cf.get_option( "col_write", 70 );
    board_col_speed = cf.get_option( "col_speed", 45 );

    embedded_img = cf.get_option( "embedded_img", true );

    x_win_img = cf.get_option( "x_win_img", 0 );
    y_win_img = cf.get_option( "y_win_img", 0 );
    width_win_img = cf.get_option( "width_win_img", 600 );
    height_win_img = cf.get_option( "height_win_img", 400 );

    embedded_mes = cf.get_option( "embedded_mes", false );
    close_mes = cf.get_option( "close_mes", true );

    x_win_mes = cf.get_option( "x_win_mes", 0 );
    y_win_mes = cf.get_option( "y_win_mes", 0 );
    width_win_mes = cf.get_option( "width_win_mes", 600 );
    height_win_mes = cf.get_option( "height_win_mes", 400 );

    img_dir_dat_save = cf.get_option( "img_dir_dat_save", "" );
    img_dir_img_save = cf.get_option( "img_dir_img_save", "" );

    dir_draft = cf.get_option( "dir_draft", "" );

    popupmenu_shown = false;

    // WM 判定
    // TODO: 環境変数で判定できない場合の判定方法を考える
    win_manager = WM_UNKNON;
    const std::string str_wm = MISC::getenv_limited( "DESKTOP_SESSION", 5 );

    if( str_wm.find( "xfce" ) != std::string::npos ) win_manager = WM_XFCE;
    else if( str_wm.find( "gnome" ) != std::string::npos ) win_manager = WM_GNOME;
    else if( str_wm.find( "kde" ) != std::string::npos ) win_manager = WM_KDE;

    if( win_manager == WM_UNKNON ){
        if( ! MISC::getenv_limited( "GNOME_DESKTOP_SESSION_ID" ).empty() ) win_manager = WM_GNOME;
        else{

            const std::string str_wm = MISC::getenv_limited( "KDE_FULL_SESSION", 4 );
            if( str_wm == "true" ) win_manager = WM_KDE;
        }
    }

#ifdef _DEBUG
    std::cout << "x=" << x_win_main << std::endl
              << "y=" << y_win_main << std::endl
              << "w=" << width_win_main << std::endl
              << "h=" << height_win_main << std::endl
              << "max=" << maximized_win_main << std::endl
              << "toolbar_pos=" << win_toolbar_pos << std::endl
              << "sidebar=" << win_show_sidebar << std::endl
              << "menubar=" << win_show_menubar << std::endl
              << "focused_admin=" << win_focused_admin << std::endl
              << "focused_admin_sidebar=" << win_focused_admin_sidebar << std::endl

              << "hpane=" << win_hpane_main_pos << std::endl
              << "vpane=" << win_vpane_main_pos << std::endl
              << "hpane_r=" << win_hpane_main_r_pos << std::endl
              << "vpane_mes=" << win_vpane_main_mes_pos << std::endl

              << "notebook_main_page=" << win_notebook_main_page << std::endl

              << "bbslist_page=" << win_bbslist_page << std::endl
              << "board_page=" << win_board_page << std::endl
              << "article_page=" << win_article_page << std::endl
              << "image_page=" << win_image_page << std::endl;

    std::cout << "board_urls\n";
    it_tmp = board_urls.begin();
    for( ; it_tmp != board_urls.end(); ++it_tmp ) if( !(*it_tmp).empty() ) std::cout << (*it_tmp);

    std::cout << "article_urls\n";
    it_tmp = article_urls.begin();
    for( ; it_tmp != article_urls.end(); ++it_tmp ) if( !(*it_tmp).empty() ) std::cout << (*it_tmp);

    std::cout << "image_urls\n";
    it_tmp = image_urls.begin();
    for( ; it_tmp != image_urls.end(); ++it_tmp ) if( !(*it_tmp).empty() ) std::cout << (*it_tmp);

    std::cout << "columns\n"
              << board_col_mark << std::endl
              << board_col_id << std::endl
              << board_col_subject << std::endl
              << board_col_number << std::endl
              << board_col_load << std::endl
              << board_col_new << std::endl
              << board_col_since << std::endl
              << board_col_write << std::endl
              << board_col_speed << std::endl

              << "embedded_mes = " << embedded_mes << std::endl
              << "close_mes = " << close_mes << std::endl
              << "wx=" << x_win_mes << std::endl
              << "wy=" << y_win_mes << std::endl
              << "ww=" << width_win_mes << std::endl
              << "wh=" << height_win_mes << std::endl;
#endif    
}



// セッション情報保存
void SESSION::save_session()
{
    // 開いているタブのURL
    std::string str_board_urls;
    std::string str_article_urls;
    std::string str_image_urls;

    std::list< std::string >::iterator it_url = board_urls.begin();
    for( ; it_url != board_urls.end(); ++it_url ){
        if( ! ( *it_url ).empty() ) str_board_urls += " \"" + ( *it_url ) + "\"";
    }
    it_url = article_urls.begin();
    for( ; it_url != article_urls.end(); ++it_url ){
        if( ! ( *it_url ).empty() ) str_article_urls += " \"" + ( *it_url ) + "\"";
    }
    it_url = image_urls.begin();
    for( ; it_url != image_urls.end(); ++it_url ){
        if( ! ( *it_url ).empty() ) str_image_urls += " \"" + ( *it_url ) + "\"";
    }

    // 開いているタブのロック状態
    std::string str_board_locked;
    std::string str_article_locked;
    std::string str_image_locked;

    std::list< bool >::iterator it_locked = board_locked.begin();
    for( ; it_locked != board_locked.end(); ++it_locked ){
        if( *it_locked ) str_board_locked += " 1";
        else str_board_locked += " 0";
    }

    it_locked = article_locked.begin();
    for( ; it_locked != article_locked.end(); ++it_locked ){
        if( *it_locked ) str_article_locked += " 1";
        else str_article_locked += " 0";
    }

    it_locked = image_locked.begin();
    for( ; it_locked != image_locked.end(); ++it_locked ){
        if( *it_locked ) str_image_locked += " 1";
        else str_image_locked += " 0";
    }


    // 保存内容作成

    std::ostringstream oss;
    oss << "mode_pane = " << mode_pane << std::endl
        << "mode_online = " << mode_online << std::endl
        << "mode_login2ch = " << mode_login2ch << std::endl
        << "mode_loginbe = " << mode_loginbe << std::endl
        << "x = " << x_win_main << std::endl
        << "y = " << y_win_main << std::endl
        << "width = " << width_win_main << std::endl
        << "height = " << height_win_main << std::endl
        << "maximized = " << maximized_win_main << std::endl
        << "toolbar_pos = " << win_toolbar_pos << std::endl
        << "show_bbslist_toolbar = " << show_bbslist_toolbar << std::endl
        << "show_board_toolbar = " << show_board_toolbar << std::endl
        << "show_article_toolbar = " << show_article_toolbar << std::endl
        << "show_board_tab = " << show_board_tab << std::endl
        << "show_article_tab = " << show_article_tab << std::endl
        << "show_sidebar = " << win_show_sidebar << std::endl
        << "show_menubar = " << win_show_menubar << std::endl
        << "focused_admin = " << win_focused_admin << std::endl
        << "focused_admin_sidebar = " << win_focused_admin_sidebar << std::endl

        << "hpane_main_pos = " << win_hpane_main_pos << std::endl
        << "vpane_main_pos = " << win_vpane_main_pos << std::endl
        << "hpane_main_r_pos = " << win_hpane_main_r_pos << std::endl
        << "vpane_main_mes_pos = " << win_vpane_main_mes_pos << std::endl

        << "notebook_main_page = " << win_notebook_main_page << std::endl

        << "bbslist_page = " << win_bbslist_page << std::endl

        << "board_page = " << win_board_page << std::endl
        << "board_urls = " << str_board_urls << std::endl
        << "board_locked = " << str_board_locked << std::endl

        << "article_page = " << win_article_page << std::endl
        << "article_urls = " << str_article_urls << std::endl
        << "article_locked = " << str_article_locked << std::endl

        << "image_page = " << win_image_page << std::endl
        << "image_urls = " << str_image_urls << std::endl
        << "image_locked = " << str_image_locked << std::endl

        << "items_sidebar = " << items_sidebar_str << std::endl
        << "items_main_toolbar = " << items_main_toolbar_str << std::endl
        << "items_article_toolbar = " << items_article_toolbar_str << std::endl
        << "items_board_toolbar = " << items_board_toolbar_str << std::endl
        << "items_board = " << items_board_str << std::endl
        << "items_msg_toolbar = " << items_msg_toolbar_str << std::endl

        << "col_mark = " << board_col_mark << std::endl
        << "col_id = " << board_col_id << std::endl
        << "col_subject = " << board_col_subject << std::endl
        << "col_number = " << board_col_number << std::endl
        << "col_load = " << board_col_load << std::endl
        << "col_new = " << board_col_new << std::endl
        << "col_since = " << board_col_since << std::endl
        << "col_write = " << board_col_write << std::endl
        << "col_speed = " << board_col_speed << std::endl

        << "embedded_img = " << embedded_img << std::endl
        << "x_win_img = " << x_win_img << std::endl
        << "y_win_img = " << y_win_img << std::endl
        << "width_win_img = " << width_win_img << std::endl
        << "height_win_img = " << height_win_img << std::endl

        << "embedded_mes = " << embedded_mes << std::endl
        << "close_mes = " << close_mes << std::endl
        << "x_win_mes = " << x_win_mes << std::endl
        << "y_win_mes = " << y_win_mes << std::endl
        << "width_win_mes = " << width_win_mes << std::endl
        << "height_win_mes = " << height_win_mes << std::endl

        << "img_dir_dat_save = " << img_dir_dat_save << std::endl
        << "img_dir_img_save = " << img_dir_img_save << std::endl
        << "dir_draft = " << dir_draft << std::endl;

    CACHE::save_rawdata( CACHE::path_session(), oss.str() );

#ifdef _DEBUG
    std::cout << "SESSION::save_session\n" << oss.str() << std::endl;
#endif    

}

// ブート中
const bool SESSION::is_booting(){ return booting; }
void SESSION::set_booting( bool boot ){ booting = boot; }

// 終了中
const bool SESSION::is_quitting(){ return quitting; }
void SESSION::set_quitting( bool quit ){ quitting = quit; }

// ディストリ名取得
const std::string& SESSION::get_distribution_name()
{
    if( distribution_name.empty() ) distribution_name = get_distribution_name_from_environment();
    return distribution_name;
}

// WM 判定
const int SESSION::get_wm(){ return win_manager; }

const int SESSION::get_mode_pane() { return mode_pane; }
void SESSION::set_mode_pane( int mode ){ mode_pane = mode; }

const bool SESSION::is_online(){ return mode_online; }
void SESSION::set_online( bool mode ){ mode_online = mode; }

const bool SESSION::login2ch(){ return mode_login2ch; }
void SESSION::set_login2ch( bool login ){ mode_login2ch = login; }

const bool SESSION::loginbe(){ return mode_loginbe; }
void SESSION::set_loginbe( bool login ){ mode_loginbe = login; }

const bool SESSION::show_sidebar(){ return win_show_sidebar; }

const bool SESSION::show_menubar(){ return win_show_menubar; }
void SESSION::set_show_menubar( bool show ){ win_show_menubar = show; }

const int SESSION::toolbar_pos(){ return win_toolbar_pos; }
void SESSION::set_toolbar_pos( int pos ){ win_toolbar_pos = pos; }

const bool SESSION::get_show_bbslist_toolbar(){ return show_bbslist_toolbar; }
void SESSION::set_show_bbslist_toolbar( bool show ){ show_bbslist_toolbar = show; }

const bool SESSION::get_show_board_toolbar(){ return show_board_toolbar; }
void SESSION::set_show_board_toolbar( bool show ){ show_board_toolbar = show; }

const bool SESSION::get_show_article_toolbar(){ return show_article_toolbar; }
void SESSION::set_show_article_toolbar( bool show ){ show_article_toolbar = show; }

const bool SESSION::get_show_board_tab(){ return show_board_tab; }
void SESSION::set_show_board_tab( bool show ){ show_board_tab = show; }

const bool SESSION::get_show_article_tab(){ return show_article_tab; }
void SESSION::set_show_article_tab( bool show ){ show_article_tab = show; }

const int SESSION::focused_admin(){ return win_focused_admin; }
void SESSION::set_focused_admin( int admin ){ win_focused_admin = admin; }
int SESSION::focused_admin_sidebar(){ return win_focused_admin_sidebar; }
void SESSION::set_focused_admin_sidebar( int admin ){ win_focused_admin_sidebar = admin; }


// 各windowの座標
const int SESSION::get_x_win_main(){ return x_win_main; }
const int SESSION::get_y_win_main(){ return y_win_main; }
void SESSION::set_x_win_main( int x ){ x_win_main = x; }
void SESSION::set_y_win_main( int y ){ y_win_main = y; }

const int SESSION::get_x_win_img(){ return x_win_img; }
const int SESSION::get_y_win_img(){ return y_win_img; }
void SESSION::set_x_win_img( int x ){ x_win_img = x; }
void SESSION::set_y_win_img( int y ){ y_win_img = y; }

const int SESSION::get_x_win_mes(){ return x_win_mes; }
const int SESSION::get_y_win_mes(){ return y_win_mes; }
void SESSION::set_x_win_mes( int x ){ x_win_mes = x; }
void SESSION::set_y_win_mes( int y ){ y_win_mes = y; }


// 各windowのサイズ
const int SESSION::get_width_win_main(){ return width_win_main; }
const int SESSION::get_height_win_main(){ return height_win_main; }
void SESSION::set_width_win_main( int width ){ width_win_main = width; }
void SESSION::set_height_win_main( int height ){ height_win_main = height; }

const int SESSION::get_width_win_img(){ return width_win_img; }
const int SESSION::get_height_win_img(){ return height_win_img; }
void SESSION::set_width_win_img( int width ){ width_win_img = width; }
void SESSION::set_height_win_img( int height ){ height_win_img = height; }

const int SESSION::get_width_win_mes(){ return width_win_mes; }
const int SESSION::get_height_win_mes(){ return height_win_mes; }
void SESSION::set_width_win_mes( int width ){ width_win_mes = width; }
void SESSION::set_height_win_mes( int height ){ height_win_mes = height; }


// 各window がフォーカスされているか
const bool SESSION::is_focus_win_main(){ return focus_win_main; }
void SESSION::set_focus_win_main( bool set ){ focus_win_main = set; }

const bool SESSION::is_focus_win_img(){ return focus_win_img; }
void SESSION::set_focus_win_img( bool set ){ focus_win_img = set; }

const bool SESSION::is_focus_win_mes(){ return focus_win_mes; }
void SESSION::set_focus_win_mes( bool set ){ focus_win_mes = set; }


// 各window が最大化されているか
const bool SESSION::is_maximized_win_main(){ return maximized_win_main; }
void SESSION::set_maximized_win_main( bool set ){ maximized_win_main = set; }

const bool SESSION::is_maximized_win_img(){ return maximized_win_img; }
void SESSION::set_maximized_win_img( bool set ){ maximized_win_img = set; }

bool SESSION::is_maximized_win_mes(){ return maximized_win_mes; }
void SESSION::set_maximized_win_mes( bool set ){ maximized_win_mes = set; }


// 各window が最小化されているか
const bool SESSION::is_iconified_win_main(){ return iconified_win_main; }
void SESSION::set_iconified_win_main( bool set ){ iconified_win_main = set; }

const bool SESSION::is_iconified_win_img(){ return iconified_win_img; }
void SESSION::set_iconified_win_img( bool set ){ iconified_win_img = set; }

const bool SESSION::is_iconified_win_mes(){ return iconified_win_mes; }
void SESSION::set_iconified_win_mes( bool set ){ iconified_win_mes = set; }


// 各window が画面に表示されているか
const bool SESSION::is_shown_win_main(){ return shown_win_main; }
void SESSION::set_shown_win_main( bool set ){ shown_win_main = set; }

const bool SESSION::is_shown_win_img(){ return shown_win_img; }
void SESSION::set_shown_win_img( bool set ){ shown_win_img = set; }

const bool SESSION::is_shown_win_mes(){ return shown_win_mes; }
void SESSION::set_shown_win_mes( bool set ){ shown_win_mes = set; }


// ダイアログ表示中
const bool SESSION::is_dialog_shown(){ return dialog_shown; }
void SESSION::set_dialog_shown( bool set ){ dialog_shown = set; }

void SESSION::set_show_sidebar( bool showurl ){ win_show_sidebar = showurl; }


// メインウィンドウのペインの敷居の位置
int SESSION::hpane_main_pos(){ return win_hpane_main_pos; }
int SESSION::vpane_main_pos(){ return win_vpane_main_pos; }
int SESSION::hpane_main_r_pos(){ return win_hpane_main_r_pos; }
int SESSION::vpane_main_mes_pos(){ return win_vpane_main_mes_pos; }

void SESSION::set_hpane_main_pos( int pos ){ win_hpane_main_pos = pos; }
void SESSION::set_vpane_main_pos( int pos ){ win_vpane_main_pos = pos; }
void SESSION::set_hpane_main_r_pos( int pos ){ win_hpane_main_r_pos = pos; }
void SESSION::set_vpane_main_mes_pos( int pos ){ win_vpane_main_mes_pos = pos; }

// メインnotebookのページ番号
int SESSION::notebook_main_page(){ return win_notebook_main_page; }
void SESSION::set_notebook_main_page( int page ){ win_notebook_main_page = page; }

// bbslistの開いてるページ番号
int SESSION::bbslist_page(){ return win_bbslist_page; }
void SESSION::set_bbslist_page( int page ){ win_bbslist_page = page; }


// 前回閉じたときに開いていたboardのページ番号とURL
int SESSION::board_page(){ return win_board_page; }
void SESSION::set_board_page( int page ){ win_board_page = page; }
std::list< std::string >& SESSION::get_board_URLs(){ return board_urls; }
void SESSION::set_board_URLs( const std::list< std::string >& urls ){ board_urls = urls; }

// スレ一覧のロック状態
const std::list< bool >& SESSION::get_board_locked(){ return board_locked; }
void SESSION::set_board_locked( const std::list< bool >& locked ){ board_locked = locked; }

// 前回閉じたときに開いていたスレタブのページ番号とURL
int SESSION::article_page(){ return win_article_page; }
void SESSION::set_article_page( int page ){ win_article_page = page; }
std::list< std::string >& SESSION::get_article_URLs(){ return article_urls; }
void SESSION::set_article_URLs( const std::list< std::string >& urls ){ article_urls = urls; }

// スレタブのロック状態
const std::list< bool >& SESSION::get_article_locked(){ return article_locked; }
void SESSION::set_article_locked( const std::list< bool >& locked ){ article_locked = locked; }

// 前回閉じたときに開いていたimageのページ番号とURL
int SESSION::image_page(){ return win_image_page; }
void SESSION::set_image_page( int page ){ win_image_page = page; }
const std::list< std::string >& SESSION::image_URLs(){ return image_urls; }
void SESSION::set_image_URLs( const std::list< std::string >& urls ){ image_urls = urls; }

// 画像タブのロック状態
const std::list< bool >& SESSION::get_image_locked(){ return image_locked; }
void SESSION::set_image_locked( const std::list< bool >& locked ){ image_locked = locked; }

// サイドバーのツールバーの項目
const std::string& SESSION::get_items_sidebar_str(){ return items_sidebar_str; }
void SESSION::set_items_sidebar_str( const std::string& items_str )
{
    items_sidebar_str = items_str;
    items_sidebar = parse_items( items_sidebar_str );
}
const int SESSION::get_item_sidebar( const int num ){ return items_sidebar[ num ]; }

// メインツールバーの項目
const std::string& SESSION::get_items_main_toolbar_str(){ return items_main_toolbar_str; }
void SESSION::set_items_main_toolbar_str( const std::string& items_str )
{
    items_main_toolbar_str = items_str;
    items_main_toolbar = parse_items( items_main_toolbar_str );
}
const int SESSION::get_item_main_toolbar( const int num ){ return items_main_toolbar[ num ]; }

// スレビューのツールバーの項目
const std::string& SESSION::get_items_article_toolbar_str(){ return items_article_toolbar_str; }
void SESSION::set_items_article_toolbar_str( const std::string& items_str )
{
    items_article_toolbar_str = items_str;
    items_article_toolbar = parse_items( items_article_toolbar_str );
}
const int SESSION::get_item_article_toolbar( const int num ){ return items_article_toolbar[ num ]; }

// スレ一覧のツールバー項目
const std::string& SESSION::get_items_board_toolbar_str(){ return items_board_toolbar_str; }
void SESSION::set_items_board_toolbar_str( const std::string& items_str )
{
    items_board_toolbar_str = items_str;
    items_board_toolbar = parse_items( items_board_toolbar_str );
}
const int SESSION::get_item_board_toolbar( const int num ){ return items_board_toolbar[ num ]; }

// スレ一覧の項目
const std::string& SESSION::get_items_board_str(){ return items_board_str; }
void SESSION::set_items_board_str( const std::string& items_str )
{
    items_board_str = items_str;
    items_board = parse_items( items_board_str );
}
const int SESSION::get_item_board( const int num ){ return items_board[ num ]; }

// 書き込みビューのツールバー項目
const std::string& SESSION::get_items_msg_toolbar_str(){ return items_msg_toolbar_str; }
void SESSION::set_items_msg_toolbar_str( const std::string& items_str )
{
    items_msg_toolbar_str = items_str;
    items_msg_toolbar = parse_items( items_msg_toolbar_str );
}
const int SESSION::get_item_msg_toolbar( const int num ){ return items_msg_toolbar[ num ]; }


// board ビューの列幅
int SESSION::col_mark(){ return board_col_mark; }
int SESSION::col_id(){ return board_col_id; }
int SESSION::col_subject(){ return board_col_subject; }
int SESSION::col_number(){ return board_col_number; }
int SESSION::col_load(){ return board_col_load; }
int SESSION::col_new(){ return board_col_new; }
int SESSION::col_since(){ return board_col_since; }
int SESSION::col_write(){ return board_col_write; }
int SESSION::col_speed(){ return board_col_speed; }


// 現在開いている bbslist のページ
const int SESSION::get_bbslist_current_page()
{
    return BBSLIST::get_admin()->get_current_page();
}

// 現在開いている bbslist のurl
const std::string SESSION::get_bbslist_current_url()
{
    return BBSLIST::get_admin()->get_current_url();
}


void SESSION::set_col_mark( int width ){ board_col_mark = width; }
void SESSION::set_col_id( int width ){ board_col_id = width; }
void SESSION::set_col_subject( int width ){ board_col_subject = width; }
void SESSION::set_col_number( int width ){ board_col_number = width; }
void SESSION::set_col_load( int width ){ board_col_load = width; }
void SESSION::set_col_new( int width ){ board_col_new = width; }
void SESSION::set_col_since( int width ){ board_col_since = width; }
void SESSION::set_col_write( int width ){ board_col_write = width; }
void SESSION::set_col_speed( int width ){ board_col_speed = width; }


// 現在開いているarticle のurl
const std::string SESSION::get_article_current_url()
{
    return ARTICLE::get_admin()->get_current_url();
}


// 埋め込みimage使用
bool SESSION::get_embedded_img(){ return embedded_img; }
void SESSION::set_embedded_img( bool set ){ embedded_img = set; }


// 埋め込みmessageを使用
bool SESSION::get_embedded_mes(){ return embedded_mes; }
void SESSION::set_embedded_mes( bool set ){ embedded_mes = set; }

// 書き込み後にmessageを閉じる
bool SESSION::get_close_mes(){ return close_mes; }
void SESSION::set_close_mes( bool set ){ close_mes = set; }


// 最後にdatを保存したディレクトリ
const std::string& SESSION::dir_dat_save(){ return img_dir_dat_save; }
void SESSION::set_dir_dat_save( const std::string& dir ){ img_dir_dat_save = dir; }

// 最後に画像を保存したディレクトリ
const std::string& SESSION::dir_img_save(){ return img_dir_img_save; }
void SESSION::set_dir_img_save( const std::string& dir ){ img_dir_img_save = dir; }

// 下書きファイルのディレクトリ
const std::string& SESSION::get_dir_draft(){ return dir_draft; }
void SESSION::set_dir_draft( const std::string& dir ){ dir_draft = dir; }


// ポップアップメニュー表示中
const bool SESSION::is_popupmenu_shown(){ return popupmenu_shown; }
void SESSION::set_popupmenu_shown( bool shown ){ popupmenu_shown = shown; }


// JD終了時に削除するスレのリスト
// 実況などしたスレは削除する。 Core::~Core()を参照
std::vector< std::string >& SESSION::get_delete_list()
{
    return delete_list;
}


void SESSION::append_delete_list( const std::string& url )
{
    std::vector< std::string >::iterator it = delete_list.begin();
    for( ; it != delete_list.end(); ++it ){
        if( ( *it ) == url ) return;
    }

    delete_list.push_back( url );

#ifdef _DEBUG
    std::cout << "SESSION::append_delete_list urls == " << delete_list.size() << " url = " << url << std::endl;
#endif
}


void SESSION::remove_delete_list( const std::string& url )
{
    if( delete_list.empty() ) return;

    std::vector< std::string >::iterator it = delete_list.begin();
    for( ; it != delete_list.end(); ++it ) if( ( *it ) == url ){ delete_list.erase( it ); break; }

#ifdef _DEBUG
    std::cout << "SESSION::remove_delete_list urls == " << delete_list.size() << " url = " << url << std::endl;
#endif
}


// 実況実行中のスレか
const bool SESSION::is_live( const std::string& url )
{
#ifdef _DEBUG
    std::cout << "SESSION::is_live live_urls == " << live_urls.size() << " url = " << url << std::endl;
#endif

    if( live_urls.empty() ) return false;

    std::vector< std::string >::iterator it = live_urls.begin();
    for( ; it != live_urls.end(); ++it ){
#ifdef _DEBUG
        std::cout << (*it) << std::endl;
#endif
        if( ( *it ) == url ) return true;
    }

    return false;
}


void SESSION::append_live( const std::string& url )
{
    if( ! is_live( url ) ) live_urls.push_back( url );

#ifdef _DEBUG
    std::cout << "SESSION::append_live live_urls == " << live_urls.size() << " url = " << url << std::endl;
#endif
}

void SESSION::remove_live( const std::string& url )
{
    if( live_urls.empty() ) return;

    std::vector< std::string >::iterator it = live_urls.begin();
    for( ; it != live_urls.end(); ++it ) if( ( *it ) == url ){ live_urls.erase( it ); break; }

#ifdef _DEBUG
    std::cout << "SESSION::remove_live live_urls == " << live_urls.size() << " url = " << url << std::endl;
#endif
}

