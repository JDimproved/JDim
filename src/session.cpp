// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "session.h"
#include "cache.h"
#include "global.h"

#include "jdlib/confloader.h"
#include "jdlib/miscutil.h"

#include "bbslist/bbslistadmin.h"
#include "article/articleadmin.h"

#include <sstream>

int mode_pane;
bool mode_online;
bool mode_login2ch;

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

std::string items_board;

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


/////////////////////////////////////


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

    items_board = cf.get_option( "items_board",
                                 COLUMN_TITLE_MARK + std::string ( " " ) +
                                 COLUMN_TITLE_ID + std::string ( " " ) +
                                 COLUMN_TITLE_NAME + std::string ( " " ) +
                                 COLUMN_TITLE_RES + std::string ( " " ) +
                                 COLUMN_TITLE_LOAD + std::string ( " " ) +
                                 COLUMN_TITLE_NEW + std::string ( " " ) +
                                 COLUMN_TITLE_SINCE + std::string ( " " ) +
                                 COLUMN_TITLE_WRITE + std::string ( " " ) +
                                 COLUMN_TITLE_SPEED + std::string ( " " ) );

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
    std::string str_wm;

    if( getenv( "DESKTOP_SESSION" ) ) str_wm = getenv( "DESKTOP_SESSION" );
    if( str_wm.find( "xfce" ) != std::string::npos ) win_manager = WM_XFCE;
    else if( str_wm.find( "gnome" ) != std::string::npos ) win_manager = WM_GNOME;
    else if( str_wm.find( "kde" ) != std::string::npos ) win_manager = WM_KDE;

    if( win_manager == WM_UNKNON ){
        if( getenv( "GNOME_DESKTOP_SESSION_ID" ) ) win_manager = WM_GNOME;
        else{

            if( getenv( "KDE_FULL_SESSION" ) ) str_wm = getenv( "KDE_FULL_SESSION" );
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
        << "x = " << x_win_main << std::endl
        << "y = " << y_win_main << std::endl
        << "width = " << width_win_main << std::endl
        << "height = " << height_win_main << std::endl
        << "maximized = " << maximized_win_main << std::endl
        << "toolbar_pos = " << win_toolbar_pos << std::endl
        << "show_bbslist_toolbar = " << show_bbslist_toolbar << std::endl
        << "show_board_toolbar = " << show_board_toolbar << std::endl
        << "show_article_toolbar = " << show_article_toolbar << std::endl
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

        << "items_board = " << items_board << std::endl
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


// WM 判定
const int SESSION::get_wm(){ return win_manager; }

const int SESSION::get_mode_pane() { return mode_pane; }
void SESSION::set_mode_pane( int mode ){ mode_pane = mode; }

const bool SESSION::is_online(){ return mode_online; }
void SESSION::set_online( bool mode ){ mode_online = mode; }

const bool SESSION::login2ch(){ return mode_login2ch; }
void SESSION::set_login2ch( bool login ){ mode_login2ch = login; }

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
const std::list< std::string >& SESSION::board_URLs(){ return board_urls; }
void SESSION::set_board_URLs( const std::list< std::string >& urls ){ board_urls = urls; }

// スレ一覧のロック状態
const std::list< bool >& SESSION::get_board_locked(){ return board_locked; }
void SESSION::set_board_locked( const std::list< bool >& locked ){ board_locked = locked; }

// 前回閉じたときに開いていたスレタブのページ番号とURL
int SESSION::article_page(){ return win_article_page; }
void SESSION::set_article_page( int page ){ win_article_page = page; }
const std::list< std::string >& SESSION::get_article_URLs(){ return article_urls; }
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

// スレ一覧の項目
const std::string& SESSION::get_items_board(){ return items_board; }
void SESSION::set_items_board( const std::string& items ){ items_board = items; }

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
