// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "session.h"
#include "cache.h"

#include "jdlib/confloader.h"
#include "jdlib/miscutil.h"

#include <sstream>

int mode_pane;
bool mode_online;
bool mode_login2ch;

int win_manager;
int win_x;
int win_y;
int win_width;
int win_height;
bool win_maximized;

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
std::list< std::string > article_urls;
std::list< std::string > image_urls;

int board_col_mark;
int board_col_id;
int board_col_subject;
int board_col_number;
int board_col_load;
int board_col_new;
int board_col_since;
int board_col_write;
int board_col_speed;

bool img_shown;

bool win_show_sidebar;

int win_toolbar_pos;

int win_focused_admin;
int win_focused_admin_sidebar;

bool focus_win_main;
bool focus_win_img;

bool iconified_win_main = false;
bool iconified_win_img = false;

bool dialog_shown = false;

bool embedded_img;

int img_x;
int img_y;
int img_width;
int img_height;

bool embedded_mes;
bool close_mes;

int win_mes_x;
int win_mes_y;
int win_mes_width;
int win_mes_height;
bool win_mes_maximized;

std::string img_dir_dat_save;
std::string img_dir_img_save;

bool popupmenu_shown;


/////////////////////////////////////


// セッション情報読み込み
void SESSION::init_session()
{
#ifdef _DEBUG
    std::cout << "SESSION::init_session\n";
#endif
    std::string str_tmp;
    std::list< std::string > list_tmp;
    std::list< std::string >::iterator it_tmp;

    JDLIB::ConfLoader cf( CACHE::path_session(), std::string() );

    // オンライン
    mode_online = cf.get_option( "mode_online", true );

    // 2chログイン
    mode_login2ch = cf.get_option( "mode_login2ch", false );

    // paneのモード
    mode_pane = cf.get_option( "mode_pane", 0 );

    win_x = cf.get_option( "x", 0 );
    win_y = cf.get_option( "y", 0 );
    win_width = cf.get_option( "width", 800 );
    win_height = cf.get_option( "height", 600 );
    win_maximized = cf.get_option( "maximized", false );

    win_show_sidebar = cf.get_option( "show_sidebar", true );

    win_toolbar_pos = cf.get_option( "toolbar_pos", 0 );

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

    str_tmp = cf.get_option( "board_urls", "");
    if( ! str_tmp.empty() ){
        list_tmp = MISC::split_line( str_tmp );
        it_tmp = list_tmp.begin();
        for( ; it_tmp != list_tmp.end(); ++it_tmp ) if( !(*it_tmp).empty() ) board_urls.push_back( (*it_tmp));
    }

    str_tmp = cf.get_option( "article_urls", "");
    if( ! str_tmp.empty() ){
        list_tmp = MISC::split_line( str_tmp );
        it_tmp = list_tmp.begin();
        for( ; it_tmp != list_tmp.end(); ++it_tmp ) if( !(*it_tmp).empty() ) article_urls.push_back( (*it_tmp));
    }

    str_tmp = cf.get_option( "image_urls", "");
    if( ! str_tmp.empty() ){
        list_tmp = MISC::split_line( str_tmp );
        it_tmp = list_tmp.begin();
        for( ; it_tmp != list_tmp.end(); ++it_tmp ) if( !(*it_tmp).empty() ) image_urls.push_back( (*it_tmp));
    }

    board_col_mark = cf.get_option( "col_mark", 30 );
    board_col_id = cf.get_option( "col_id", 30 );
    board_col_subject = cf.get_option(  "col_subject", 350 );
    board_col_number = cf.get_option( "col_number", 50 );
    board_col_load = cf.get_option( "col_load", 50 );
    board_col_new = cf.get_option( "col_new", 50 );
    board_col_since = cf.get_option( "col_since", 100 );
    board_col_write = cf.get_option( "col_write", 100 );
    board_col_speed = cf.get_option( "col_speed", 50 );

    img_shown = false;

    embedded_img = cf.get_option( "embedded_img", true );

    img_x = cf.get_option( "img_x", 0 );
    img_y = cf.get_option( "img_y", 0 );
    img_width = cf.get_option( "img_width", 600 );
    img_height = cf.get_option( "img_height", 500 );

    embedded_mes = cf.get_option( "embedded_mes", false );
    close_mes = cf.get_option( "close_mes", true );

    win_mes_x = cf.get_option( "mes_x", 0 );
    win_mes_y = cf.get_option( "mes_y", 0 );
    win_mes_width = cf.get_option( "mes_width", 300 );
    win_mes_height = cf.get_option( "mes_height", 300 );
    win_mes_maximized = cf.get_option( "mes_maximized", false );

    img_dir_dat_save = cf.get_option( "img_dir_dat_save", "" );
    img_dir_img_save = cf.get_option( "img_dir_img_save", "" );

    popupmenu_shown = false;

    // WM 判定
    // TODO: 環境変数で判定できない場合の判定方法を考える
    win_manager = WM_UNKNON;
    std::string str_wm;
    if( getenv( "DESKTOP_SESSION" ) ) str_wm = getenv( "DESKTOP_SESSION" );
    if( str_wm.find( "xfce" ) != std::string::npos ) win_manager = WM_XFCE;
    else if( str_wm.find( "gnome" ) != std::string::npos ) win_manager = WM_GNOME;
    if( win_manager == WM_UNKNON ){
        if( getenv( "GNOME_DESKTOP_SESSION_ID" ) ) win_manager = WM_GNOME;
    }


#ifdef _DEBUG
    std::cout << "x=" << win_x << std::endl
              << "y=" << win_y << std::endl
              << "w=" << win_width << std::endl
              << "h=" << win_height << std::endl
              << "m=" << win_maximized << std::endl
              << "toolbar_pos=" << win_toolbar_pos << std::endl
              << "sidebar=" << win_show_sidebar << std::endl
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
              << "close_mes = " << close_mes << std::endl;
              << "wx=" << win_mes_x << std::endl
              << "wy=" << win_mes_y << std::endl
              << "ww=" << win_mes_width << std::endl
              << "wh=" << win_mes_height << std::endl
              << "wm=" << win_mes_maximized << std::endl;
#endif    
}



// セッション情報保存
void SESSION::save_session()
{
    std::string str_board_urls;
    std::string str_article_urls;
    std::string str_image_urls;

    std::list< std::string >::iterator it = board_urls.begin();
    for( ; it != board_urls.end(); ++it ){
        if( ! ( *it ).empty() ) str_board_urls += " \"" + ( *it ) + "\"";
    }
    it = article_urls.begin();
    for( ; it != article_urls.end(); ++it ){
        if( ! ( *it ).empty() ) str_article_urls += " \"" + ( *it ) + "\"";
    }
    it = image_urls.begin();
    for( ; it != image_urls.end(); ++it ){
        if( ! ( *it ).empty() ) str_image_urls += " \"" + ( *it ) + "\"";
    }

    std::ostringstream oss;
    oss << "mode_pane = " << mode_pane << std::endl
        << "mode_online = " << mode_online << std::endl
        << "mode_login2ch = " << mode_login2ch << std::endl
        << "x = " << win_x << std::endl
        << "y = " << win_y << std::endl
        << "width = " << win_width << std::endl
        << "height = " << win_height << std::endl
        << "maximized = " << win_maximized << std::endl
        << "toolbar_pos = " << win_toolbar_pos << std::endl
        << "show_sidebar = " << win_show_sidebar << std::endl
        << "focused_admin = " << win_focused_admin << std::endl
        << "focused_admin_sidebar = " << win_focused_admin_sidebar << std::endl

        << "hpane_main_pos = " << win_hpane_main_pos << std::endl
        << "vpane_main_pos = " << win_vpane_main_pos << std::endl
        << "hpane_main_r_pos = " << win_hpane_main_r_pos << std::endl
        << "vpane_main_mes_pos = " << win_vpane_main_mes_pos << std::endl

        << "notebook_main_page = " << win_notebook_main_page << std::endl

        << "bbslist_page = " << win_bbslist_page << std::endl
        << "board_page = " << win_board_page << std::endl
        << "article_page = " << win_article_page << std::endl
        << "image_page = " << win_image_page << std::endl

        << "board_urls = " << str_board_urls << std::endl
        << "article_urls = " << str_article_urls << std::endl
        << "image_urls = " << str_image_urls << std::endl
        
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
        << "img_x = " << img_x << std::endl
        << "img_y = " << img_y << std::endl
        << "img_width = " << img_width << std::endl
        << "img_height = " << img_height << std::endl

        << "embedded_mes = " << embedded_mes << std::endl
        << "close_mes = " << close_mes << std::endl
        << "mes_x = " << win_mes_x << std::endl
        << "mes_y = " << win_mes_y << std::endl
        << "mes_width = " << win_mes_width << std::endl
        << "mes_height = " << win_mes_height << std::endl
        << "mes_maximized = " << win_mes_maximized << std::endl

        << "img_dir_dat_save = " << img_dir_dat_save << std::endl
        << "img_dir_img_save = " << img_dir_img_save << std::endl;

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

int SESSION::x(){ return win_x; }
int SESSION::y(){ return win_y; }
int SESSION::width(){ return win_width; }
int SESSION::height(){ return win_height; }
bool SESSION::maximized(){ return win_maximized; }
bool SESSION::show_sidebar(){ return win_show_sidebar; }

int SESSION::toolbar_pos(){ return win_toolbar_pos; }
void SESSION::set_toolbar_pos( int pos ){ win_toolbar_pos = pos; }

int SESSION::focused_admin(){ return win_focused_admin; }
void SESSION::set_focused_admin( int admin ){ win_focused_admin = admin; }
int SESSION::focused_admin_sidebar(){ return win_focused_admin_sidebar; }
void SESSION::set_focused_admin_sidebar( int admin ){ win_focused_admin_sidebar = admin; }


// 各window がフォーカスされているか
const bool SESSION::is_focus_win_main(){ return focus_win_main; }
void SESSION::set_focus_win_main( bool set ){ focus_win_main = set; }

const bool SESSION::is_focus_win_img(){ return focus_win_img; }
void SESSION::set_focus_win_img( bool set ){ focus_win_img = set; }


    // 各window が最小化されているか
const bool SESSION::is_iconified_win_main(){ return iconified_win_main; }
void SESSION::set_iconified_win_main( bool set ){ iconified_win_main = set; }

const bool SESSION::is_iconified_win_img(){ return iconified_win_img; }
void SESSION::set_iconified_win_img( bool set ){ iconified_win_img = set; }

// ダイアログ表示中
const bool SESSION::is_dialog_shown(){ return dialog_shown; }
void SESSION::set_dialog_shown( bool set ){ dialog_shown = set; }


void SESSION::set_x( int x ){ win_x = x; }
void SESSION::set_y( int y ){ win_y = y; }
void SESSION::set_width( int width ){ win_width = width; }
void SESSION::set_height( int height ){ win_height = height; }
void SESSION::set_maximized( bool maximized ){ win_maximized = maximized; }
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


// 前回閉じたときに開いていたarticleのページ番号とURL
int SESSION::article_page(){ return win_article_page; }
void SESSION::set_article_page( int page ){ win_article_page = page; }
const std::list< std::string >& SESSION::article_URLs(){ return article_urls; }
void SESSION::set_article_URLs( const std::list< std::string >& urls ){ article_urls = urls; }


// 前回閉じたときに開いていたimageのページ番号とURL
int SESSION::image_page(){ return win_image_page; }
void SESSION::set_image_page( int page ){ win_image_page = page; }
const std::list< std::string >& SESSION::image_URLs(){ return image_urls; }
void SESSION::set_image_URLs( const std::list< std::string >& urls ){ image_urls = urls; }


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

void SESSION::set_col_mark( int width ){ board_col_mark = width; }
void SESSION::set_col_id( int width ){ board_col_id = width; }
void SESSION::set_col_subject( int width ){ board_col_subject = width; }
void SESSION::set_col_number( int width ){ board_col_number = width; }
void SESSION::set_col_load( int width ){ board_col_load = width; }
void SESSION::set_col_new( int width ){ board_col_new = width; }
void SESSION::set_col_since( int width ){ board_col_since = width; }
void SESSION::set_col_write( int width ){ board_col_write = width; }
void SESSION::set_col_speed( int width ){ board_col_speed = width; }

bool SESSION::is_img_shown(){ return img_shown; }
void SESSION::set_img_shown( bool set ){ img_shown = set; }


// 埋め込みimage使用
bool SESSION::get_embedded_img(){ return embedded_img; }
void SESSION::set_embedded_img( bool set ){ embedded_img = set; }

int SESSION::get_img_x(){ return img_x; }
int SESSION::get_img_y(){ return img_y; }
int SESSION::get_img_width(){ return img_width; }
int SESSION::get_img_height(){ return img_height; }

void SESSION::set_img_x( int x ){ img_x = x; }
void SESSION::set_img_y( int y ){ img_y = y; }
void SESSION::set_img_width( int width ){ img_width = width; }
void SESSION::set_img_height( int height ){ img_height = height; }


// 埋め込みmessageを使用
bool SESSION::get_embedded_mes(){ return embedded_mes; }
void SESSION::set_embedded_mes( bool set ){ embedded_mes = set; }

// 書き込み後にmessageを閉じる
bool SESSION::get_close_mes(){ return close_mes; }
void SESSION::set_close_mes( bool set ){ close_mes = set; }


// message ウィンドウの位置
int SESSION::mes_x(){ return win_mes_x; }
int SESSION::mes_y(){ return win_mes_y; }
int SESSION::mes_width(){ return win_mes_width; }
int SESSION::mes_height(){ return win_mes_height; }
bool SESSION::mes_maximized(){ return win_mes_maximized; }

void SESSION::set_mes_x( int x ){ win_mes_x = x; }
void SESSION::set_mes_y( int y ){ win_mes_y = y; }
void SESSION::set_mes_width( int width ){ win_mes_width = width; }
void SESSION::set_mes_height( int height ){ win_mes_height = height; }
void SESSION::set_mes_maximized( bool maximized ){ win_mes_maximized = maximized; }


// 最後にdatを保存したディレクトリ
const std::string& SESSION::dir_dat_save(){ return img_dir_dat_save; }
void SESSION::set_dir_dat_save( const std::string& dir ){ img_dir_dat_save = dir; }

// 最後に画像を保存したディレクトリ
const std::string& SESSION::dir_img_save(){ return img_dir_img_save; }
void SESSION::set_dir_img_save( const std::string& dir ){ img_dir_img_save = dir; }

// ポップアップメニュー表示中
const bool SESSION::is_popupmenu_shown(){ return popupmenu_shown; }
void SESSION::set_popupmenu_shown( bool shown ){ popupmenu_shown = shown; }
