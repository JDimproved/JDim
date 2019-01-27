// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "session.h"
#include "cache.h"
#include "global.h"

#include "jdlib/confloader.h"
#include "jdlib/miscutil.h"
#include "jdlib/misctime.h"

#include "bbslist/bbslistadmin.h"
#include "article/articleadmin.h"
#include "article/articleviewbase.h"

#include <sstream>

bool booting = true;
bool quitting = false;
bool tab_operating_article = false;
bool tab_operating_image = false;

int mode_pane;
bool mode_online;
bool mode_login2ch;
bool mode_loginbe;
bool mode_loginp2;

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
std::list< std::string > board_switchhistory;

std::list< std::string > article_urls;
std::list< bool > article_locked;
std::list< std::string > article_switchhistory;

std::list< std::string > image_urls;
std::list< bool > image_locked;

std::string items_sidebar_toolbar_str;
std::vector< int > items_sidebar_toolbar;

std::string items_main_toolbar_str;
std::vector< int > items_main_toolbar;

std::string items_article_toolbar_str;
std::vector< int > items_article_toolbar;

std::string items_search_toolbar_str;
std::vector< int > items_search_toolbar;

std::string items_board_toolbar_str;
std::vector< int > items_board_toolbar;

std::string items_board_col_str;
std::vector< int > items_board_col;

std::string items_msg_toolbar_str;
std::vector< int > items_msg_toolbar;

std::string items_board_menu_str;
std::vector< int > items_board_menu;

std::string items_article_menu_str;
std::vector< int > items_article_menu;

int board_col_mark;
int board_col_id;
int board_col_board;
int board_col_subject;
int board_col_number;
int board_col_load;
int board_col_new;
int board_col_since;
int board_col_write;
int board_col_access;
int board_col_speed;
int board_col_diff;

int board_col_since_time;
int board_col_write_time;
int board_col_access_time;

bool win_show_sidebar;

bool win_show_menubar;

bool show_main_toolbar;
int win_toolbar_pos;

bool show_bbslist_toolbar;
bool show_board_toolbar;
bool show_article_toolbar;

bool show_board_tab;
bool show_article_tab;

bool show_main_statbar;

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

bool full_win_main = false;

bool dialog_shown = false;

bool embedded_img;

bool embedded_mes;
bool close_mes;

std::string dir_dat;
std::string img_dir_img_save;

std::string dir_draft;

bool popupmenu_shown;

std::vector< std::string > delete_list;

std::vector< std::string > live_urls;

int img_fit_mode;

std::string dir_select_favorite;

/////////////////////////////////////


// ツールバー等の項目名 -> ID 変換
int SESSION::parse_item( const std::string& item_name )
{
    int item = ITEM_END;

    if( item_name == ITEM_NAME_BBSLISTVIEW ) item = ITEM_BBSLISTVIEW;
    else if( item_name == ITEM_NAME_FAVORITEVIEW ) item = ITEM_FAVORITEVIEW;
    else if( item_name == ITEM_NAME_BOARDVIEW ) item = ITEM_BOARDVIEW;
    else if( item_name == ITEM_NAME_HISTVIEW ) item = ITEM_HISTVIEW;
    else if( item_name == ITEM_NAME_HIST_BOARDVIEW ) item = ITEM_HIST_BOARDVIEW;
    else if( item_name == ITEM_NAME_HIST_CLOSEVIEW ) item = ITEM_HIST_CLOSEVIEW;
    else if( item_name == ITEM_NAME_HIST_CLOSEBOARDVIEW ) item = ITEM_HIST_CLOSEBOARDVIEW;
    else if( item_name == ITEM_NAME_HIST_CLOSEIMGVIEW ) item = ITEM_HIST_CLOSEIMGVIEW;
    else if( item_name == ITEM_NAME_ARTICLEVIEW ) item = ITEM_ARTICLEVIEW;
    else if( item_name == ITEM_NAME_IMAGEVIEW ) item = ITEM_IMAGEVIEW;
    else if( item_name == ITEM_NAME_URL ) item = ITEM_URL;
    else if( item_name == ITEM_NAME_GO ) item = ITEM_GO;
    else if( item_name == ITEM_NAME_SEPARATOR ) item = ITEM_SEPARATOR;

    else if( item_name == ITEM_NAME_MARK ) item = ITEM_MARK;
    else if( item_name == ITEM_NAME_ID ) item = ITEM_ID;
    else if( item_name == ITEM_NAME_BOARD ) item = ITEM_BOARD;
    else if( item_name == ITEM_NAME_NAME ) item = ITEM_NAME;
    else if( item_name == ITEM_NAME_RES ) item = ITEM_RES;
    else if( item_name == ITEM_NAME_LOAD ) item = ITEM_LOAD;
    else if( item_name == ITEM_NAME_NEW ) item = ITEM_NEW;
    else if( item_name == ITEM_NAME_SINCE ) item = ITEM_SINCE;
    else if( item_name == ITEM_NAME_LASTWRITE ) item = ITEM_LASTWRITE;
    else if( item_name == ITEM_NAME_ACCESS ) item = ITEM_ACCESS;
    else if( item_name == ITEM_NAME_SPEED ) item = ITEM_SPEED;
    else if( item_name == ITEM_NAME_DIFF ) item = ITEM_DIFF;

    else if( item_name == ITEM_NAME_WRITEMSG ) item = ITEM_WRITEMSG;
    else if( item_name == ITEM_NAME_OPENBOARD ) item = ITEM_OPENBOARD;
    else if( item_name == ITEM_NAME_OPENARTICLETAB ) item = ITEM_OPENARTICLETAB;
    else if( item_name == ITEM_NAME_REGETARTICLE ) item = ITEM_REGETARTICLE;
    else if( item_name == ITEM_NAME_BOOKMARK ) item = ITEM_BOOKMARK;
    else if( item_name == ITEM_NAME_SEARCH ) item = ITEM_SEARCH;
    else if( item_name == ITEM_NAME_DRAWOUT ) item = ITEM_DRAWOUT;
    else if( item_name == ITEM_NAME_RELOAD ) item = ITEM_RELOAD;
    else if( item_name == ITEM_NAME_STOPLOADING ) item = ITEM_STOPLOADING;
    else if( item_name == ITEM_NAME_APPENDFAVORITE ) item = ITEM_APPENDFAVORITE;
    else if( item_name == ITEM_NAME_FAVORITE_ARTICLE ) item = ITEM_FAVORITE_ARTICLE;
    else if( item_name == ITEM_NAME_CHECK_UPDATE_ROOT ) item = ITEM_CHECK_UPDATE_ROOT;
    else if( item_name == ITEM_NAME_CHECK_UPDATE_OPEN_ROOT ) item = ITEM_CHECK_UPDATE_OPEN_ROOT;

    else if( item_name == ITEM_NAME_COPY ) item = ITEM_COPY;
    else if( item_name == ITEM_NAME_COPY_URL ) item = ITEM_COPY_URL;
    else if( item_name == ITEM_NAME_COPY_TITLE_URL ) item = ITEM_COPY_TITLE_URL;
    else if( item_name == ITEM_NAME_COPY_TITLE_URL_THREAD ) item = ITEM_COPY_TITLE_URL_THREAD;
    else if( item_name == ITEM_NAME_COPY_THREAD_INFO ) item = ITEM_COPY_THREAD_INFO;

    else if( item_name == ITEM_NAME_DELETE ) item = ITEM_DELETE;
    else if( item_name == ITEM_NAME_QUIT ) item = ITEM_QUIT;
    else if( item_name == ITEM_NAME_BACK ) item = ITEM_BACK;
    else if( item_name == ITEM_NAME_FORWARD ) item = ITEM_FORWARD;
    else if( item_name == ITEM_NAME_LOCK ) item = ITEM_LOCK;
    else if( item_name == ITEM_NAME_LIVE ) item = ITEM_LIVE;

    else if( item_name == ITEM_NAME_NEWARTICLE ) item = ITEM_NEWARTICLE;
    else if( item_name == ITEM_NAME_SEARCHBOX ) item = ITEM_SEARCHBOX;
    else if( item_name == ITEM_NAME_SEARCH_NEXT ) item = ITEM_SEARCH_NEXT;
    else if( item_name == ITEM_NAME_SEARCH_PREV ) item = ITEM_SEARCH_PREV;
    else if( item_name == ITEM_NAME_NEXTARTICLE ) item = ITEM_NEXTARTICLE;
    else if( item_name == ITEM_NAME_CLEAR_HIGHLIGHT ) item = ITEM_CLEAR_HIGHLIGHT;

    else if( item_name == ITEM_NAME_INSERTTEXT ) item = ITEM_INSERTTEXT;
    else if( item_name == ITEM_NAME_LOCK_MESSAGE ) item = ITEM_LOCK_MESSAGE;
    else if( item_name == ITEM_NAME_PREVIEW ) item = ITEM_PREVIEW;

    else if( item_name == ITEM_NAME_UNDO ) item = ITEM_UNDO;
    else if( item_name == ITEM_NAME_REDO ) item = ITEM_REDO;

    else if( item_name == ITEM_NAME_NGWORD ) item = ITEM_NGWORD;
    else if( item_name == ITEM_NAME_ABONE_SELECTION ) item = ITEM_ABONE_SELECTION;
    else if( item_name == ITEM_NAME_ABONE_ARTICLE ) item = ITEM_ABONE_ARTICLE;
    else if( item_name == ITEM_NAME_QUOTE_SELECTION ) item = ITEM_QUOTE_SELECTION;
    else if( item_name == ITEM_NAME_OPEN_BROWSER ) item = ITEM_OPEN_BROWSER;
    else if( item_name == ITEM_NAME_OPEN_CACHE_BROWSER ) item = ITEM_OPEN_CACHE_BROWSER;
    else if( item_name == ITEM_NAME_USER_COMMAND ) item = ITEM_USER_COMMAND;
    else if( item_name == ITEM_NAME_ETC ) item = ITEM_ETC;
    else if( item_name == ITEM_NAME_SAVE_DAT ) item = ITEM_SAVE_DAT;
    else if( item_name == ITEM_NAME_SELECTIMG ) item = ITEM_SELECTIMG;
    else if( item_name == ITEM_NAME_SELECTDELIMG ) item = ITEM_SELECTDELIMG;
    else if( item_name == ITEM_NAME_SELECTABONEIMG ) item = ITEM_SELECTABONEIMG;

    else if( item_name == ITEM_NAME_PREF_BOARD ) item = ITEM_PREF_BOARD;
    else if( item_name == ITEM_NAME_PREF_THREAD ) item = ITEM_PREF_THREAD;

    return item;
}


std::vector< int > parse_items( const std::string& items_str )
{
    std::vector< int > items;
    const std::list< std::string > list_order = MISC::split_line( items_str );
    std::list< std::string >::const_iterator it = list_order.begin();
    for( ; it != list_order.end(); ++it ){

        const int item = SESSION::parse_item( *it );
        if( item != ITEM_END ) items.push_back( item );
    }
    items.push_back( ITEM_END );

    return items;
}


void read_list_urls( JDLIB::ConfLoader& cf, const std::string& id_urls,  std::list< std::string >& list_urls )
{
    list_urls.clear();

    std::string str_tmp;
    std::list< std::string > list_tmp;
    std::list< std::string >::iterator it_tmp;

    str_tmp = cf.get_option_str( id_urls, "" );
    if( ! str_tmp.empty() ){
        list_tmp = MISC::split_line( str_tmp );
        it_tmp = list_tmp.begin();
        for( ; it_tmp != list_tmp.end(); ++it_tmp ) if( !(*it_tmp).empty() ) list_urls.push_back( (*it_tmp));
    }
}


void read_list_locked( JDLIB::ConfLoader& cf, const std::string& id_locked, std::list< bool >& list_locked )
{
    list_locked.clear();

    std::string str_tmp;
    std::list< std::string > list_tmp;
    std::list< std::string >::iterator it_tmp;

    str_tmp = cf.get_option_str( id_locked, "" );
    if( ! str_tmp.empty() ){
        list_tmp = MISC::split_line( str_tmp );
        it_tmp = list_tmp.begin();
        for( ; it_tmp != list_tmp.end(); ++it_tmp ){
            if( ( *it_tmp ) == "1" ) list_locked.push_back( true );
            else list_locked.push_back( false );
        }
    }
}



// セッション情報読み込み
void SESSION::init_session()
{
#ifdef _DEBUG
    std::cout << "SESSION::init_session\n";
#endif

    JDLIB::ConfLoader cf( CACHE::path_session(), std::string() );

    // オンライン
    mode_online = cf.get_option_bool( "mode_online", true );

    // 2chログイン
    mode_login2ch = cf.get_option_bool( "mode_login2ch", false );

    // beログイン
    mode_loginbe = cf.get_option_bool( "mode_loginbe", false );

    // p2ログイン
    mode_loginp2 = cf.get_option_bool( "mode_loginp2", false );

    // paneのモード
    mode_pane = cf.get_option_int( "mode_pane", MODE_2PANE, 0, MODE_PANE_NUM -1 );

    x_win_main = cf.get_option_int( "x", 0, 0, 8192 );
    y_win_main = cf.get_option_int( "y", 0, 0, 8192 );
    width_win_main = cf.get_option_int( "width", 800, 80, 8192 );
    height_win_main = cf.get_option_int( "height", 600, 60, 8192 );
    maximized_win_main = cf.get_option_bool( "maximized", false );
    full_win_main = cf.get_option_bool( "full_win_main", false );

    win_show_sidebar = cf.get_option_bool( "show_sidebar", true );

    win_show_menubar = cf.get_option_bool( "show_menubar", true );

    show_main_toolbar = cf.get_option_bool( "show_main_toolbar", true );
    win_toolbar_pos = cf.get_option_int( "toolbar_pos", TOOLBAR_POS_NORMAL, 0, TOOLBAR_POS_NUM -1 );

    show_bbslist_toolbar = cf.get_option_bool( "show_bbslist_toolbar", true );
    show_board_toolbar = cf.get_option_bool( "show_board_toolbar", true );
    show_article_toolbar = cf.get_option_bool( "show_article_toolbar", true );

    show_board_tab = cf.get_option_bool( "show_board_tab", true );
    show_article_tab = cf.get_option_bool( "show_article_tab", true );

    show_main_statbar = cf.get_option_bool( "show_main_statbar", true );

    win_focused_admin = cf.get_option_int( "focused_admin", FOCUS_NOT, 0, FOCUS_NUM -1 );
    win_focused_admin_sidebar = cf.get_option_int( "focused_admin_sidebar", FOCUS_NOT, 0, FOCUS_NUM -1 );

    win_hpane_main_pos = cf.get_option_int( "hpane_main_pos", 190, 0, 8192 );
    win_vpane_main_pos = cf.get_option_int( "vpane_main_pos", 200, 0, 8192 );
    win_hpane_main_r_pos = cf.get_option_int( "hpane_main_r_pos", 300, 0, 8192 );
    win_vpane_main_mes_pos = cf.get_option_int( "vpane_main_mes_pos", 400, 0, 8192 );

    // メインnotebookのページ番号
    win_notebook_main_page = cf.get_option_int( "notebook_main_page", PAGE_ARTICLE, 0, PAGE_NUM -1 );

    // 各ビューの開いてるページ番号
    win_bbslist_page = cf.get_option_int( "bbslist_page", 0, 0, 1024 );
    win_board_page = cf.get_option_int( "board_page", 0, 0, 1024 );
    win_article_page = cf.get_option_int( "article_page", 0, 0, 1024 );
    win_image_page = cf.get_option_int( "image_page", 0, 0, 1024 );

    // スレ一覧のURLとロック状態と切り替え履歴
    read_list_urls( cf, "board_urls", board_urls );
    read_list_locked( cf, "board_locked", board_locked );
    read_list_urls( cf, "board_switchhistory", board_switchhistory );

    // スレタブのURLとロック状態と切り替え履歴    
    read_list_urls( cf, "article_urls", article_urls );
    read_list_locked( cf, "article_locked", article_locked );
    read_list_urls( cf, "article_switchhistory", article_switchhistory );

    // 画像タブのURLとロック状態
    read_list_urls( cf, "image_urls", image_urls );
    read_list_locked( cf, "image_locked", image_locked );

    // サイドバーのツールバー項目
    items_sidebar_toolbar_str = cf.get_option_str( "items_sidebar", get_items_sidebar_toolbar_default_str() );
    items_sidebar_toolbar =  parse_items( items_sidebar_toolbar_str );

    // メインツールバーの項目
    items_main_toolbar_str = cf.get_option_str( "items_main_toolbar", get_items_main_toolbar_default_str() );
    items_main_toolbar =  parse_items( items_main_toolbar_str );

    // スレビューのツールバーの項目
    items_article_toolbar_str = cf.get_option_str( "items_article_toolbar", get_items_article_toolbar_default_str() );
    items_article_toolbar =  parse_items( items_article_toolbar_str );

    // 検索ビューのツールバーの項目
    items_search_toolbar_str = cf.get_option_str( "items_search_toolbar", get_items_search_toolbar_default_str() );
    items_search_toolbar =  parse_items( items_search_toolbar_str );

    // スレ一覧のツールバー項目
    items_board_toolbar_str = cf.get_option_str( "items_board_toolbar", get_items_board_toolbar_default_str() );
    items_board_toolbar =  parse_items( items_board_toolbar_str );

    // 書き込みビューのツールバー項目
    items_msg_toolbar_str = cf.get_option_str( "items_msg_toolbar", get_items_msg_toolbar_default_str() );
    items_msg_toolbar =  parse_items( items_msg_toolbar_str );

    // スレ一覧の列項目
    items_board_col_str = cf.get_option_str( "items_board", get_items_board_col_default_str() );
    items_board_col =  parse_items( items_board_col_str );

    // スレ一覧のコンテキストメニュー項目
    items_board_menu_str = cf.get_option_str( "items_board_menu", get_items_board_menu_default_str() );
    items_board_menu =  parse_items( items_board_menu_str );

    // スレビューのコンテキストメニュー項目
    items_article_menu_str = cf.get_option_str( "items_article_menu", get_items_article_menu_default_str() );
    items_article_menu =  parse_items( items_article_menu_str );

    // board ビューの列幅
    board_col_mark = cf.get_option_int( "col_mark", 30, 4, 8192 );
    board_col_id = cf.get_option_int( "col_id", 45, 4, 8192 );
    board_col_board = cf.get_option_int( "col_board", 70, 4, 8192 );
    board_col_subject = cf.get_option_int( "col_subject", 190, 4, 8192 );
    board_col_number = cf.get_option_int( "col_number", 45, 4, 8192 );
    board_col_load = cf.get_option_int( "col_load", 45, 4, 8192 );
    board_col_new = cf.get_option_int( "col_new", 45, 4, 8192 );
    board_col_since = cf.get_option_int( "col_since", 70, 4, 8192 );
    board_col_write = cf.get_option_int( "col_write", 70, 4, 8192 );
    board_col_access = cf.get_option_int( "col_access", 70, 4, 8192 );
    board_col_speed = cf.get_option_int( "col_speed", 45, 4, 8192 );
    board_col_diff = cf.get_option_int( "col_diff", 45, 4, 8192 );

    // スレ一覧の since の表示モード
    board_col_since_time = cf.get_option_int( "col_since_time", MISC::TIME_NORMAL, 0, MISC::TIME_NUM-1 );

    // スレ一覧の 最終書込 の表示モード
    board_col_write_time = cf.get_option_int( "col_write_time", MISC::TIME_NORMAL, 0, MISC::TIME_NUM-1 );

    // スレ一覧の 最終取得 の表示モード
    board_col_access_time = cf.get_option_int( "col_access_time", MISC::TIME_NORMAL, 0, MISC::TIME_NUM-1 );

    embedded_img = cf.get_option_bool( "embedded_img", true );

    x_win_img = cf.get_option_int( "x_win_img", 0, 0, 8192 );
    y_win_img = cf.get_option_int( "y_win_img", 0, 0, 8192 );
    width_win_img = cf.get_option_int( "width_win_img", 600, 60, 8192 );
    height_win_img = cf.get_option_int( "height_win_img", 400, 40, 8192 );

    embedded_mes = cf.get_option_bool( "embedded_mes", false );

    // 書き込み後にmessageを閉じる
    close_mes = cf.get_option_bool( "close_mes", true );

    x_win_mes = cf.get_option_int( "x_win_mes", 0, 0, 8192 );
    y_win_mes = cf.get_option_int( "y_win_mes", 0, 0, 8192 );
    width_win_mes = cf.get_option_int( "width_win_mes", 600, 60, 8192 );
    height_win_mes = cf.get_option_int( "height_win_mes", 400, 40, 8192 );

    dir_dat = cf.get_option_str( "dir_dat", "" );
    img_dir_img_save = cf.get_option_str( "img_dir_img_save", "" );

    dir_draft = cf.get_option_str( "dir_draft", "" );

    popupmenu_shown = false;

    // 画像のfitモード
    img_fit_mode = cf.get_option_int( "img_fit_mode", IMG_FIT_NORMAL, 0, IMG_FIT_NUM -1 );

    dir_select_favorite = cf.get_option_str( "dir_select_favorite", "" );

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

    std::list< std::string >::iterator it_tmp = board_urls.begin();
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
              << board_col_board << std::endl
              << board_col_subject << std::endl
              << board_col_number << std::endl
              << board_col_load << std::endl
              << board_col_new << std::endl
              << board_col_since << std::endl
              << board_col_write << std::endl
              << board_col_speed << std::endl
              << board_col_diff << std::endl

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

    // タブの切り替え履歴
    std::string str_board_switchhistory;
    std::string str_article_switchhistory;

    it_url = board_switchhistory.begin();
    for( ; it_url != board_switchhistory.end(); ++it_url ){
        if( ! ( *it_url ).empty() ) str_board_switchhistory += " \"" + ( *it_url ) + "\"";
    }

    it_url = article_switchhistory.begin();
    for( ; it_url != article_switchhistory.end(); ++it_url ){
        if( ! ( *it_url ).empty() ) str_article_switchhistory += " \"" + ( *it_url ) + "\"";
    }

    // 保存内容作成

    std::ostringstream oss;
    oss << "mode_pane = " << mode_pane << std::endl
        << "mode_online = " << mode_online << std::endl
        << "mode_login2ch = " << mode_login2ch << std::endl
        << "mode_loginbe = " << mode_loginbe << std::endl
        << "mode_loginp2 = " << mode_loginp2 << std::endl
        << "x = " << x_win_main << std::endl
        << "y = " << y_win_main << std::endl
        << "width = " << width_win_main << std::endl
        << "height = " << height_win_main << std::endl
        << "maximized = " << maximized_win_main << std::endl
        << "full_win_main = " << full_win_main << std::endl
        << "show_main_toolbar = " << show_main_toolbar << std::endl
        << "toolbar_pos = " << win_toolbar_pos << std::endl
        << "show_bbslist_toolbar = " << show_bbslist_toolbar << std::endl
        << "show_board_toolbar = " << show_board_toolbar << std::endl
        << "show_article_toolbar = " << show_article_toolbar << std::endl
        << "show_board_tab = " << show_board_tab << std::endl
        << "show_article_tab = " << show_article_tab << std::endl
        << "show_sidebar = " << win_show_sidebar << std::endl
        << "show_menubar = " << win_show_menubar << std::endl
        << "show_main_statbar = " << show_main_statbar << std::endl
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
        << "board_switchhistory = " << str_board_switchhistory << std::endl

        << "article_page = " << win_article_page << std::endl
        << "article_urls = " << str_article_urls << std::endl
        << "article_locked = " << str_article_locked << std::endl
        << "article_switchhistory = " << str_article_switchhistory << std::endl

        << "image_page = " << win_image_page << std::endl
        << "image_urls = " << str_image_urls << std::endl
        << "image_locked = " << str_image_locked << std::endl

        << "items_sidebar = " << items_sidebar_toolbar_str << std::endl
        << "items_main_toolbar = " << items_main_toolbar_str << std::endl
        << "items_article_toolbar = " << items_article_toolbar_str << std::endl
        << "items_search_toolbar = " << items_search_toolbar_str << std::endl
        << "items_board_toolbar = " << items_board_toolbar_str << std::endl
        << "items_msg_toolbar = " << items_msg_toolbar_str << std::endl
        << "items_board = " << items_board_col_str << std::endl
        << "items_board_menu = " << items_board_menu_str << std::endl
        << "items_article_menu = " << items_article_menu_str << std::endl

        << "col_mark = " << board_col_mark << std::endl
        << "col_id = " << board_col_id << std::endl
        << "col_board = " << board_col_board << std::endl    
        << "col_subject = " << board_col_subject << std::endl
        << "col_number = " << board_col_number << std::endl
        << "col_load = " << board_col_load << std::endl
        << "col_new = " << board_col_new << std::endl
        << "col_since = " << board_col_since << std::endl
        << "col_write = " << board_col_write << std::endl
        << "col_access = " << board_col_access << std::endl
        << "col_speed = " << board_col_speed << std::endl
        << "col_diff = " << board_col_diff << std::endl

        << "col_since_time = " << board_col_since_time << std::endl
        << "col_write_time = " << board_col_write_time << std::endl
        << "col_access_time = " << board_col_access_time << std::endl

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

        << "dir_dat = " << dir_dat << std::endl
        << "img_dir_img_save = " << img_dir_img_save << std::endl
        << "dir_draft = " << dir_draft << std::endl

        << "img_fit_mode = " << img_fit_mode << std::endl

        << "dir_select_favorite = " << dir_select_favorite << std::endl;

    CACHE::save_rawdata( CACHE::path_session(), oss.str() );

#ifdef _DEBUG
    std::cout << "SESSION::save_session\n" << oss.str() << std::endl;
#endif    

}

// ブート中
bool SESSION::is_booting(){ return booting; }
void SESSION::set_booting( const bool boot ){ booting = boot; }

// 終了中
bool SESSION::is_quitting(){ return quitting; }
void SESSION::set_quitting( const bool quit ){ quitting = quit; }

// 入れ替えなどのタブ操作中
// ビューの再描画などを禁止する
bool SESSION::is_tab_operating( const std::string& url_admin )
{
    if( url_admin == URL_ARTICLEADMIN ) return tab_operating_article;
    if( url_admin == URL_IMAGEADMIN ) return tab_operating_image;

    return false;
}

void SESSION::set_tab_operating( const std::string& url_admin, const bool operating )
{
    if( url_admin == URL_ARTICLEADMIN ) tab_operating_article = operating;
    if( url_admin == URL_IMAGEADMIN ) tab_operating_image = operating;
}

int SESSION::get_mode_pane() { return mode_pane; }
void SESSION::set_mode_pane( const int mode ){ mode_pane = mode; }

bool SESSION::is_online(){ return mode_online; }
void SESSION::set_online( const bool mode ){ mode_online = mode; }

bool SESSION::login2ch(){ return mode_login2ch; }
void SESSION::set_login2ch( const bool login ){ mode_login2ch = login; }

bool SESSION::loginbe(){ return mode_loginbe; }
void SESSION::set_loginbe( const bool login ){ mode_loginbe = login; }

bool SESSION::loginp2(){ return mode_loginp2; }
void SESSION::set_loginp2( const bool login ){ mode_loginp2 = login; }

bool SESSION::show_sidebar(){ return win_show_sidebar; }

bool SESSION::show_menubar(){ return win_show_menubar; }
void SESSION::set_show_menubar( const bool show ){ win_show_menubar = show; }

bool SESSION::get_show_main_toolbar(){ return show_main_toolbar; }
void SESSION::set_show_main_toolbar( const bool show ){ show_main_toolbar = show; }
int SESSION::get_toolbar_pos(){ return win_toolbar_pos; }
void SESSION::set_toolbar_pos( const int pos ){ win_toolbar_pos = pos; }

bool SESSION::get_show_bbslist_toolbar(){ return show_bbslist_toolbar; }
void SESSION::set_show_bbslist_toolbar( const bool show ){ show_bbslist_toolbar = show; }

bool SESSION::get_show_board_toolbar(){ return show_board_toolbar; }
void SESSION::set_show_board_toolbar( const bool show ){ show_board_toolbar = show; }

bool SESSION::get_show_article_toolbar(){ return show_article_toolbar; }
void SESSION::set_show_article_toolbar( const bool show ){ show_article_toolbar = show; }

bool SESSION::get_show_board_tab(){ return show_board_tab; }
void SESSION::set_show_board_tab( const bool show ){ show_board_tab = show; }

bool SESSION::get_show_article_tab(){ return show_article_tab; }
void SESSION::set_show_article_tab( const bool show ){ show_article_tab = show; }

bool SESSION::get_show_main_statbar(){ return show_main_statbar; }
void SESSION::set_show_main_statbar( const bool show ){ show_main_statbar = show; }

int SESSION::focused_admin(){ return win_focused_admin; }
void SESSION::set_focused_admin( const int admin ){ win_focused_admin = admin; }
int SESSION::focused_admin_sidebar(){ return win_focused_admin_sidebar; }
void SESSION::set_focused_admin_sidebar( const int admin ){ win_focused_admin_sidebar = admin; }


// 各windowの座標
int SESSION::get_x_win_main(){ return x_win_main; }
int SESSION::get_y_win_main(){ return y_win_main; }
void SESSION::set_x_win_main( const int x ){ x_win_main = x; }
void SESSION::set_y_win_main( const int y ){ y_win_main = y; }

int SESSION::get_x_win_img(){ return x_win_img; }
int SESSION::get_y_win_img(){ return y_win_img; }
void SESSION::set_x_win_img( const int x ){ x_win_img = x; }
void SESSION::set_y_win_img( const int y ){ y_win_img = y; }

int SESSION::get_x_win_mes(){ return x_win_mes; }
int SESSION::get_y_win_mes(){ return y_win_mes; }
void SESSION::set_x_win_mes( const int x ){ x_win_mes = x; }
void SESSION::set_y_win_mes( const int y ){ y_win_mes = y; }


// 各windowのサイズ
int SESSION::get_width_win_main(){ return width_win_main; }
int SESSION::get_height_win_main(){ return height_win_main; }
void SESSION::set_width_win_main( const int width ){ width_win_main = width; }
void SESSION::set_height_win_main( const int height ){ height_win_main = height; }

int SESSION::get_width_win_img(){ return width_win_img; }
int SESSION::get_height_win_img(){ return height_win_img; }
void SESSION::set_width_win_img( const int width ){ width_win_img = width; }
void SESSION::set_height_win_img( const int height ){ height_win_img = height; }

int SESSION::get_width_win_mes(){ return width_win_mes; }
int SESSION::get_height_win_mes(){ return height_win_mes; }
void SESSION::set_width_win_mes( const int width ){ width_win_mes = width; }
void SESSION::set_height_win_mes( const int height ){ height_win_mes = height; }


// 各window がフォーカスされているか
bool SESSION::is_focus_win_main(){ return focus_win_main; }
void SESSION::set_focus_win_main( const bool set ){ focus_win_main = set; }

bool SESSION::is_focus_win_img(){ return focus_win_img; }
void SESSION::set_focus_win_img( const bool set ){ focus_win_img = set; }

bool SESSION::is_focus_win_mes(){ return focus_win_mes; }
void SESSION::set_focus_win_mes( const bool set ){ focus_win_mes = set; }


// 各window が最大化されているか
bool SESSION::is_maximized_win_main(){ return maximized_win_main; }
void SESSION::set_maximized_win_main( const bool set ){ maximized_win_main = set; }

bool SESSION::is_maximized_win_img(){ return maximized_win_img; }
void SESSION::set_maximized_win_img( const bool set ){ maximized_win_img = set; }

bool SESSION::is_maximized_win_mes(){ return maximized_win_mes; }
void SESSION::set_maximized_win_mes( const bool set ){ maximized_win_mes = set; }


// 各window が最小化されているか
bool SESSION::is_iconified_win_main(){ return iconified_win_main; }
void SESSION::set_iconified_win_main( const bool set ){ iconified_win_main = set; }

bool SESSION::is_iconified_win_img(){ return iconified_win_img; }
void SESSION::set_iconified_win_img( const bool set ){ iconified_win_img = set; }

bool SESSION::is_iconified_win_mes(){ return iconified_win_mes; }
void SESSION::set_iconified_win_mes( const bool set ){ iconified_win_mes = set; }


// 各window が画面に表示されているか
bool SESSION::is_shown_win_main(){ return shown_win_main; }
void SESSION::set_shown_win_main( const bool set ){ shown_win_main = set; }

bool SESSION::is_shown_win_img(){ return shown_win_img; }
void SESSION::set_shown_win_img( const bool set ){ shown_win_img = set; }

bool SESSION::is_shown_win_mes(){ return shown_win_mes; }
void SESSION::set_shown_win_mes( const bool set ){ shown_win_mes = set; }


// windowがフルスクリーンか
bool SESSION::is_full_win_main(){ return full_win_main; }
void SESSION::set_full_win_main( const bool set ){ full_win_main = set; }


// ダイアログ表示中
bool SESSION::is_dialog_shown(){ return dialog_shown; }
void SESSION::set_dialog_shown( const bool set ){ dialog_shown = set; }

void SESSION::set_show_sidebar( const bool showurl ){ win_show_sidebar = showurl; }


// メインウィンドウのペインの敷居の位置
int SESSION::hpane_main_pos(){ return win_hpane_main_pos; }
int SESSION::vpane_main_pos(){ return win_vpane_main_pos; }
int SESSION::hpane_main_r_pos(){ return win_hpane_main_r_pos; }
int SESSION::vpane_main_mes_pos(){ return win_vpane_main_mes_pos; }

void SESSION::set_hpane_main_pos( const int pos ){ win_hpane_main_pos = pos; }
void SESSION::set_vpane_main_pos( const int pos ){ win_vpane_main_pos = pos; }
void SESSION::set_hpane_main_r_pos( const int pos ){ win_hpane_main_r_pos = pos; }
void SESSION::set_vpane_main_mes_pos( const int pos ){ win_vpane_main_mes_pos = pos; }

// メインnotebookのページ番号
int SESSION::notebook_main_page(){ return win_notebook_main_page; }
void SESSION::set_notebook_main_page( const int page ){ win_notebook_main_page = page; }

// bbslistの開いてるページ番号
int SESSION::bbslist_page(){ return win_bbslist_page; }
void SESSION::set_bbslist_page( const int page ){ win_bbslist_page = page; }


// 前回閉じたときに開いていたboardのページ番号とURL
int SESSION::board_page(){ return win_board_page; }
void SESSION::set_board_page( const int page ){ win_board_page = page; }
const std::list< std::string >& SESSION::get_board_URLs(){ return board_urls; }
void SESSION::set_board_URLs( const std::list< std::string >& urls ){ board_urls = urls; }

// スレ一覧のロック状態
const std::list< bool >& SESSION::get_board_locked(){ return board_locked; }
void SESSION::set_board_locked( const std::list< bool >& locked ){ board_locked = locked; }

// スレ一覧の切り替え履歴    
const std::list< std::string >& SESSION::get_board_switchhistory(){ return board_switchhistory; }
void SESSION::set_board_switchhistory( const std::list< std::string >& hist ){ board_switchhistory = hist; }

// 前回閉じたときに開いていたスレタブのページ番号とURL
int SESSION::article_page(){ return win_article_page; }
void SESSION::set_article_page( const int page ){ win_article_page = page; }
const std::list< std::string >& SESSION::get_article_URLs(){ return article_urls; }
void SESSION::set_article_URLs( const std::list< std::string >& urls ){ article_urls = urls; }

// スレタブのロック状態
const std::list< bool >& SESSION::get_article_locked(){ return article_locked; }
void SESSION::set_article_locked( const std::list< bool >& locked ){ article_locked = locked; }

// スレタブの切り替え履歴    
const std::list< std::string >& SESSION::get_article_switchhistory(){ return article_switchhistory; }
void SESSION::set_article_switchhistory( const std::list< std::string >& hist ){ article_switchhistory = hist; }

// 前回閉じたときに開いていたimageのページ番号とURL
int SESSION::image_page(){ return win_image_page; }
void SESSION::set_image_page( const int page ){ win_image_page = page; }
const std::list< std::string >& SESSION::image_URLs(){ return image_urls; }
void SESSION::set_image_URLs( const std::list< std::string >& urls ){ image_urls = urls; }

// 画像タブのロック状態
const std::list< bool >& SESSION::get_image_locked(){ return image_locked; }
void SESSION::set_image_locked( const std::list< bool >& locked ){ image_locked = locked; }

// サイドバーのツールバーの項目
const std::string& SESSION::get_items_sidebar_toolbar_str(){ return items_sidebar_toolbar_str; }
const std::string SESSION::get_items_sidebar_toolbar_default_str()
{
    return
    ITEM_NAME_SEARCHBOX + std::string ( " " ) +
    ITEM_NAME_SEARCH_NEXT + std::string ( " " ) +
    ITEM_NAME_SEARCH_PREV;

}
void SESSION::set_items_sidebar_toolbar_str( const std::string& items_str )
{
    items_sidebar_toolbar_str = items_str;
    items_sidebar_toolbar = parse_items( items_sidebar_toolbar_str );
}
int SESSION::get_item_sidebar_toolbar( const int num ){ return items_sidebar_toolbar[ num ]; }

// メインツールバーの項目
const std::string& SESSION::get_items_main_toolbar_str(){ return items_main_toolbar_str; }
const std::string SESSION::get_items_main_toolbar_default_str()
{
    return
    ITEM_NAME_BBSLISTVIEW + std::string ( " " ) +
    ITEM_NAME_FAVORITEVIEW + std::string ( " " ) +
    ITEM_NAME_BOARDVIEW + std::string ( " " ) +
    ITEM_NAME_ARTICLEVIEW + std::string ( " " ) +
    ITEM_NAME_IMAGEVIEW + std::string ( " " ) +
    ITEM_NAME_SEPARATOR + std::string ( " " ) +
    ITEM_NAME_URL + std::string ( " " ) +
    ITEM_NAME_GO;
}
void SESSION::set_items_main_toolbar_str( const std::string& items_str )
{
    items_main_toolbar_str = items_str;
    items_main_toolbar = parse_items( items_main_toolbar_str );
}
int SESSION::get_item_main_toolbar( const int num ){ return items_main_toolbar[ num ]; }

// スレビューのツールバーの項目
const std::string& SESSION::get_items_article_toolbar_str(){ return items_article_toolbar_str; }
const std::string SESSION::get_items_article_toolbar_default_str()
{
    return
    ITEM_NAME_WRITEMSG + std::string ( " " ) +
    ITEM_NAME_OPENBOARD + std::string ( " " ) +
    ITEM_NAME_NAME + std::string ( " " ) +
    ITEM_NAME_SEARCH + std::string ( " " ) +
    ITEM_NAME_RELOAD + std::string ( " " ) +
    ITEM_NAME_STOPLOADING + std::string ( " " ) +
    ITEM_NAME_APPENDFAVORITE + std::string ( " " ) +
    ITEM_NAME_DELETE + std::string ( " " ) +
    ITEM_NAME_QUIT;
}
void SESSION::set_items_article_toolbar_str( const std::string& items_str )
{
    items_article_toolbar_str = items_str;
    items_article_toolbar = parse_items( items_article_toolbar_str );
}
int SESSION::get_item_article_toolbar( const int num ){ return items_article_toolbar[ num ]; }

// 検索ビューのツールバーの項目
const std::string& SESSION::get_items_search_toolbar_str(){ return items_search_toolbar_str; }
const std::string SESSION::get_items_search_toolbar_default_str()
{
    return
    ITEM_NAME_NAME + std::string ( " " ) +
    ITEM_NAME_SEARCH + std::string ( " " ) +
    ITEM_NAME_RELOAD + std::string ( " " ) +
    ITEM_NAME_STOPLOADING + std::string ( " " ) +
    ITEM_NAME_QUIT;
}
void SESSION::set_items_search_toolbar_str( const std::string& items_str )
{
    items_search_toolbar_str = items_str;
    items_search_toolbar = parse_items( items_search_toolbar_str );
}
int SESSION::get_item_search_toolbar( const int num ){ return items_search_toolbar[ num ]; }

// スレ一覧のツールバー項目
const std::string& SESSION::get_items_board_toolbar_str(){ return items_board_toolbar_str; }
const std::string SESSION::get_items_board_toolbar_default_str()
{
    return
    ITEM_NAME_NEWARTICLE + std::string ( " " ) +
    ITEM_NAME_SEARCHBOX + std::string ( " " ) +
    ITEM_NAME_SEARCH_NEXT + std::string ( " " ) +
    ITEM_NAME_SEARCH_PREV + std::string ( " " ) +
    ITEM_NAME_RELOAD + std::string ( " " ) +
    ITEM_NAME_STOPLOADING + std::string ( " " ) +
    ITEM_NAME_APPENDFAVORITE + std::string ( " " ) +
    ITEM_NAME_DELETE + std::string ( " " ) +
    ITEM_NAME_QUIT;
}
void SESSION::set_items_board_toolbar_str( const std::string& items_str )
{
    items_board_toolbar_str = items_str;
    items_board_toolbar = parse_items( items_board_toolbar_str );
}
int SESSION::get_item_board_toolbar( const int num ){ return items_board_toolbar[ num ]; }

// 書き込みビューのツールバー項目
const std::string& SESSION::get_items_msg_toolbar_str(){ return items_msg_toolbar_str; }
const std::string SESSION::get_items_msg_toolbar_default_str()
{
    return
    ITEM_NAME_PREVIEW + std::string ( " " ) +
    ITEM_NAME_WRITEMSG+ std::string ( " " ) +
    ITEM_NAME_NAME + std::string ( " " ) +
    ITEM_NAME_UNDO + std::string ( " " ) +
    ITEM_NAME_INSERTTEXT + std::string ( " " ) +
    ITEM_NAME_LOCK_MESSAGE + std::string ( " " ) +
    ITEM_NAME_QUIT;
}
void SESSION::set_items_msg_toolbar_str( const std::string& items_str )
{
    items_msg_toolbar_str = items_str;
    items_msg_toolbar = parse_items( items_msg_toolbar_str );
}
int SESSION::get_item_msg_toolbar( const int num ){ return items_msg_toolbar[ num ]; }

// スレ一覧の列項目
const std::string& SESSION::get_items_board_col_str(){ return items_board_col_str; }
const std::string SESSION::get_items_board_col_default_str()
{
    return
    ITEM_NAME_MARK + std::string ( " " ) +
    ITEM_NAME_ID + std::string ( " " ) +
    ITEM_NAME_NAME + std::string ( " " ) +
    ITEM_NAME_RES + std::string ( " " ) +
    ITEM_NAME_LOAD + std::string ( " " ) +
    ITEM_NAME_NEW + std::string ( " " ) +
    ITEM_NAME_SINCE + std::string ( " " ) +
    ITEM_NAME_LASTWRITE + std::string ( " " ) +
    ITEM_NAME_SPEED;
}
void SESSION::set_items_board_col_str( const std::string& items_str )
{
    items_board_col_str = items_str;
    items_board_col = parse_items( items_board_col_str );
}
int SESSION::get_item_board_col( const int num ){ return items_board_col[ num ]; }

// スレ一覧のコンテキストメニュー項目
const std::string& SESSION::get_items_board_menu_str(){ return items_board_menu_str; }
const std::string SESSION::get_items_board_menu_default_str()
{
    return
    ITEM_NAME_BOOKMARK + std::string ( " " ) +
    ITEM_NAME_SEPARATOR + std::string ( " " ) +
    ITEM_NAME_OPENARTICLETAB + std::string ( " " ) +
    ITEM_NAME_OPEN_BROWSER + std::string ( " " ) +
    ITEM_NAME_SEPARATOR + std::string ( " " ) +
    ITEM_NAME_COPY_URL + std::string ( " " ) +
    ITEM_NAME_COPY_TITLE_URL_THREAD + std::string ( " " ) +
    ITEM_NAME_SEPARATOR + std::string ( " " ) +
    ITEM_NAME_ABONE_ARTICLE + std::string ( " " ) +
    ITEM_NAME_SEPARATOR + std::string ( " " ) +
    ITEM_NAME_DELETE + std::string ( " " ) +
    ITEM_NAME_SEPARATOR + std::string ( " " ) +
    ITEM_NAME_ETC + std::string ( " " ) +
    ITEM_NAME_SEPARATOR + std::string ( " " ) +
    ITEM_NAME_PREF_THREAD + std::string ( " " ) +
    ITEM_NAME_PREF_BOARD;
}
void SESSION::set_items_board_menu_str( const std::string& items_str )
{
    items_board_menu_str = items_str;
    items_board_menu = parse_items( items_board_menu_str );
}
int SESSION::get_item_board_menu( const int num ){ return items_board_menu[ num ]; }


// スレビューのコンテキストメニュー項目
const std::string& SESSION::get_items_article_menu_str(){ return items_article_menu_str; }
const std::string SESSION::get_items_article_menu_default_str()
{
    return
    ITEM_NAME_DRAWOUT + std::string ( " " ) +
    ITEM_NAME_GO + std::string ( " " ) +
    ITEM_NAME_SEARCH + std::string ( " " ) +
    ITEM_NAME_NGWORD + std::string ( " " ) +
    ITEM_NAME_SEPARATOR + std::string ( " " ) +
    ITEM_NAME_QUOTE_SELECTION + std::string ( " " ) +
    ITEM_NAME_SEPARATOR + std::string ( " " ) +
    ITEM_NAME_OPEN_BROWSER + std::string ( " " ) +
    ITEM_NAME_USER_COMMAND + std::string ( " " ) +
    ITEM_NAME_SEPARATOR + std::string ( " " ) +
    ITEM_NAME_COPY_URL + std::string ( " " ) +
    ITEM_NAME_COPY + std::string ( " " ) +
    ITEM_NAME_SEPARATOR + std::string ( " " ) +
    ITEM_NAME_ETC + std::string ( " " ) +
    ITEM_NAME_SEPARATOR + std::string ( " " ) +
    ITEM_NAME_PREF_THREAD;
}
void SESSION::set_items_article_menu_str( const std::string& items_str )
{
    items_article_menu_str = items_str;
    items_article_menu = parse_items( items_article_menu_str );
}
int SESSION::get_item_article_menu( const int num ){ return items_article_menu[ num ]; }

// board ビューの列幅
int SESSION::col_mark(){ return board_col_mark; }
int SESSION::col_id(){ return board_col_id; }
int SESSION::col_board(){ return board_col_board; }
int SESSION::col_subject(){ return board_col_subject; }
int SESSION::col_number(){ return board_col_number; }
int SESSION::col_load(){ return board_col_load; }
int SESSION::col_new(){ return board_col_new; }
int SESSION::col_since(){ return board_col_since; }
int SESSION::col_write(){ return board_col_write; }
int SESSION::col_access(){ return board_col_access; }
int SESSION::col_speed(){ return board_col_speed; }
int SESSION::col_diff(){ return board_col_diff; }

// スレ一覧の since の表示モード
int SESSION::get_col_since_time() { return board_col_since_time; }
void SESSION::set_col_since_time( const int mode ){ board_col_since_time = mode; }

// スレ一覧の 最終書込 の表示モード
int SESSION::get_col_write_time() { return board_col_write_time; }
void SESSION::set_col_write_time( const int mode ){ board_col_write_time = mode; }

// スレ一覧の 最終取得 の表示モード
int SESSION::get_col_access_time() { return board_col_access_time; }
void SESSION::set_col_access_time( const int mode ){ board_col_access_time = mode; }

// 現在開いているサイドバーのページ
int SESSION::get_sidebar_current_page()
{
    return BBSLIST::get_admin()->get_current_page();
}

// 現在開いているサイドバーのurl
const std::string SESSION::get_sidebar_current_url()
{
    return BBSLIST::get_admin()->get_current_url();
}


void SESSION::set_col_mark( const int width ){ board_col_mark = width; }
void SESSION::set_col_id( const int width ){ board_col_id = width; }
void SESSION::set_col_board( const int width ){ board_col_board = width; }
void SESSION::set_col_subject( const int width ){ board_col_subject = width; }
void SESSION::set_col_number( const int width ){ board_col_number = width; }
void SESSION::set_col_load( const int width ){ board_col_load = width; }
void SESSION::set_col_new( const int width ){ board_col_new = width; }
void SESSION::set_col_since( const int width ){ board_col_since = width; }
void SESSION::set_col_write( const int width ){ board_col_write = width; }
void SESSION::set_col_access( const int width ){ board_col_access = width; }
void SESSION::set_col_speed( const int width ){ board_col_speed = width; }
void SESSION::set_col_diff( const int width ){ board_col_diff = width; }


// 現在開いているarticle の ARTICLE::DrawAreaBase
ARTICLE::DrawAreaBase* SESSION::get_base_drawarea()
{
    ARTICLE::ArticleViewBase* base_view = NULL;
    base_view = dynamic_cast< ARTICLE::ArticleViewBase* >( ARTICLE::get_admin()->get_current_view() );

    if( base_view == NULL ) return NULL;

    ARTICLE::DrawAreaBase* base_drawarea = NULL;
    base_drawarea = base_view->drawarea();

    return base_drawarea;
}


// 現在開いているarticle のurl
const std::string SESSION::get_article_current_url()
{
    return ARTICLE::get_admin()->get_current_url();
}


// 埋め込みimage使用
bool SESSION::get_embedded_img(){ return embedded_img; }
void SESSION::set_embedded_img( const bool set ){ embedded_img = set; }


// 埋め込みmessageを使用
bool SESSION::get_embedded_mes(){ return embedded_mes; }
void SESSION::set_embedded_mes( const bool set ){ embedded_mes = set; }

// 書き込み後にmessageを閉じる
bool SESSION::get_close_mes(){ return close_mes; }
void SESSION::set_close_mes( const bool set ){ close_mes = set; }


// 最後にdatを読み書きしたディレクトリ
const std::string& SESSION::get_dir_dat(){ return dir_dat; }
void SESSION::set_dir_dat( const std::string& dir ){ dir_dat = dir; }

// 最後に画像を保存したディレクトリ
const std::string& SESSION::dir_img_save(){ return img_dir_img_save; }
void SESSION::set_dir_img_save( const std::string& dir ){ img_dir_img_save = dir; }

// 下書きファイルのディレクトリ
const std::string& SESSION::get_dir_draft(){ return dir_draft; }
void SESSION::set_dir_draft( const std::string& dir ){ dir_draft = dir; }


// ポップアップメニュー表示中
bool SESSION::is_popupmenu_shown(){ return popupmenu_shown; }
void SESSION::set_popupmenu_shown( const bool shown ){ popupmenu_shown = shown; }


// JD終了時に削除するスレのリスト
// 実況などしたスレは削除する。 Core::~Core()を参照
const std::vector< std::string >& SESSION::get_delete_list()
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
bool SESSION::is_live( const std::string& url )
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


// 画像のfitモード
int SESSION::get_img_fit_mode()
{
    return img_fit_mode;
}


void SESSION::toggle_img_fit_mode()
{
    if( img_fit_mode == IMG_FIT_NORMAL ) img_fit_mode = IMG_FIT_WIDTH;
    else img_fit_mode = IMG_FIT_NORMAL;
}


// お気に入り挿入ダイアログで最後に保存したディレクトリ名
const std::string& SESSION::get_dir_select_favorite()
{
    return dir_select_favorite;
}

void SESSION::set_dir_select_favorite( const std::string& dir )
{
    dir_select_favorite = dir;
}


// 各履歴を取得
void SESSION::get_history( const std::string& url, CORE::DATA_INFO_LIST& info_list )
{
    return BBSLIST::get_admin()->get_history( url, info_list );
}


// サイドバーの指定したidのディレクトリに含まれるスレのアドレスを取得
void SESSION::get_sidebar_threads( const std::string& url, const int dirid, std::vector< std::string >& list_url )
{
    BBSLIST::get_admin()->get_threads( url, dirid, list_url );
}


// サイドバーの指定したidのディレクトリの名前を取得
const std::string SESSION::get_sidebar_dirname( const std::string& url, const int dirid )
{
    return BBSLIST::get_admin()->get_dirname( url, dirid );
}
