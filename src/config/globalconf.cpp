// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "globalconf.h"
#include "cache.h"

#include "jdlib/confloader.h"
#include "jdlib/miscutil.h"

#define COLOR_SIZE 3

bool restore_board;
bool restore_article;
bool restore_image;

int color_char[ COLOR_SIZE ];
int color_char_age[ COLOR_SIZE ];
int color_sepa[ COLOR_SIZE ];
int color_back[ COLOR_SIZE ];
int color_back_popup[ COLOR_SIZE ];
int color_back_tree[ COLOR_SIZE ];
int color_back_tree_board[ COLOR_SIZE ];
std::string fontname_main;
std::string fontname_popup;
std::string fontname_tree;
std::string fontname_tree_board;
std::string fontname_message;

std::string ref_prefix;
int ref_prefix_space;

std::string url_login2ch;
std::string url_bbsmenu;
std::string path_cacheroot;

std::string agent_for2ch;

bool use_proxy_for2ch;
std::string proxy_for2ch;
int proxy_port_for2ch;

bool use_proxy_for2ch_w;
std::string proxy_for2ch_w;
int proxy_port_for2ch_w;

std::string agent_for_data;

bool use_proxy_for_data;
std::string proxy_for_data;
int proxy_port_for_data;

std::string x_2ch_ua;

int loader_bufsize;
int loader_timeout;
int loader_timeout_post;
int loader_timeout_img;

std::string command_openurl;
int brownsercombo_id;

int imgpopup_width;
int imgpopup_height;
bool use_mosaic;
bool zoom_to_fit;
int del_img_day;
int max_img_size;

bool show_oldarticle;
int newthread_hour;

int tree_scroll_size;
bool open_one_category;
bool always_write_ok;
bool save_postlog;
bool hide_writing_dialog;

int margin_popup;
int mouse_radius;
int history_size;
int instruct_popup;

double adjust_underline_pos;
double adjust_line_space;

bool draw_underline;
bool strict_char_width;

int tab_min_str;

bool show_tab_icon;

std::list< std::string > list_abone_word_thread;
std::list< std::string > list_abone_regex_thread;

std::list< std::string > list_abone_name;
std::list< std::string > list_abone_word;
std::list< std::string > list_abone_regex;

bool abone_transparent;
bool abone_chain;


//
// 初期設定
//
const bool CONFIG::init_config()
{
    JDLIB::ConfLoader cf( CACHE::path_conf(), std::string() );

#ifdef _DEBUG
    std::cout << "CONFIG::init_config empty = " << cf.empty() << std::endl;
#endif

    // 前回開いたviewを復元するか
    restore_board = cf.get_option( "restore_board", false );
    restore_article = cf.get_option( "restore_article", false );
    restore_image = cf.get_option( "restore_image", false );

    // フォント
    fontname_main = cf.get_option( "fontname_main", "Kochi Gothic 12" );
    fontname_popup = cf.get_option( "fontname_popup","Kochi Gothic 9" );
    fontname_tree = cf.get_option( "fontname_tree","Kochi Gothic 10" );
    fontname_tree_board = cf.get_option( "fontname_tree_board",fontname_tree );
    fontname_message = cf.get_option( "fontname_message",fontname_message );

    // レスを参照するときに前に付ける文字
    ref_prefix = cf.get_option( "ref_prefix", ">" );

    // ref_prefix の後のスペースの数
    // JDLIB::ConfLoader の中で MISC::remove_space() が呼ばれて空白が消えるので別設定とした
    ref_prefix_space = cf.get_option( "ref_prefix_space", 1 );
    for( int i = 0; i < ref_prefix_space; ++i ) ref_prefix += " ";

    // キャッシュのルートディレクトリ
    // キャッシュ構造は navi2ch の上位互換なので path_cacheroot = "~/.navi2ch/" とすればnavi2chとキャッシュを共有できる
    path_cacheroot = cf.get_option( "path_cacheroot", "~/.jd/" );

    // 読み込み用プロクシとポート番号
    use_proxy_for2ch = cf.get_option( "use_proxy_for2ch", 0 );
    proxy_for2ch = cf.get_option( "proxy_for2ch", "" );
    proxy_port_for2ch = cf.get_option( "proxy_port_for2ch", 8080 );

    // 書き込み用プロクシとポート番号
    use_proxy_for2ch_w = cf.get_option( "use_proxy_for2ch_w", 0 );
    proxy_for2ch_w = cf.get_option( "proxy_for2ch_w", "" );
    proxy_port_for2ch_w = cf.get_option( "proxy_port_for2ch_w", 8080 );

    // 2chの外にアクセスするときのプロクシとポート番号
    use_proxy_for_data = cf.get_option( "use_proxy_for_data", 0 );
    proxy_for_data = cf.get_option( "proxy_for_data", "" );
    proxy_port_for_data = cf.get_option( "proxy_port_for_data", 8080 );

    // 2ch にアクセスするときのエージェント名
    agent_for2ch = cf.get_option( "agent_for2ch", "Monazilla/1.00 JD" );

    // 2ch外にアクセスするときのエージェント名
    agent_for_data = cf.get_option( "agent_for_data", "Mozilla/5.0 (Windows; U; Windows NT 5.0; ja; rv:1.8) Gecko/20051111 Firefox/1.5" );

    // 2ch にログインするときのX-2ch-UA
    x_2ch_ua = cf.get_option( "x_2ch_ua", "Navigator for 2ch 1.7.5" );

    // ローダのバッファサイズ
    loader_bufsize = cf.get_option( "loader_bufsize", 32 );

    // ローダのタイムアウト値
    loader_timeout = cf.get_option( "loader_timeout", 10 );
    loader_timeout_post = cf.get_option( "loader_timeout_post", 30 ); // ポスト
    loader_timeout_img = cf.get_option( "loader_timeout_img", 30 ); // 画像

    // リンクをクリックしたときに実行するコマンド
    command_openurl = cf.get_option( "command_openurl", "" );

    // ブラウザ設定ダイアログのコンボボックスの番号
    brownsercombo_id = cf.get_option( "brownsercombo_id", 0 );

    // 画像ポップアップサイズ
    imgpopup_width = cf.get_option( "imgpopup_width", 320 );
    imgpopup_height = cf.get_option( "imgpopup_height", 240 );

    // 画像にモザイクかける
    use_mosaic = cf.get_option( "use_mosaic", 1 );

    // 画像をデフォルトでウィンドウサイズに合わせる
    zoom_to_fit = cf.get_option( "zoom_to_fit", 1 );

    // 画像キャッシュ削除の日数
    del_img_day = cf.get_option( "del_img_day", 20 );

    // ダウンロードする画像の最大サイズ(Mbyte)
    max_img_size = cf.get_option( "max_img_size", 16 );

    // 2chの認証サーバ
    url_login2ch = cf.get_option( "url_login2ch", "https://2chv.tora3.net/futen.cgi" );

    // bbsmenu.htmlのURL
    url_bbsmenu = cf.get_option( "url_bbsmenu", "http://menu.2ch.net/bbsmenu.html" );

    // 色 ( RGB の順 ) 範囲は 0 - 65535

    // 文字色
    color_char[ 0 ] = cf.get_option( "color_char_R", 0 );
    color_char[ 1 ] = cf.get_option( "color_char_G", 0 );
    color_char[ 2 ] = cf.get_option( "color_char_B", 0 );

    // ageの時のメール欄
    color_char_age[ 0 ] = cf.get_option( "color_char_age_R", 65000 );
    color_char_age[ 1 ] = cf.get_option( "color_char_age_G", 0 );
    color_char_age[ 2 ] = cf.get_option( "color_char_age_B", 0 );

    // 新着セパレータ
    color_sepa[ 0 ] = cf.get_option( "color_sepa_R", 32000 );
    color_sepa[ 1 ] = cf.get_option( "color_sepa_G", 32000 );
    color_sepa[ 2 ] = cf.get_option( "color_sepa_B", 32000 );

    // 背景色
    color_back[ 0 ] = cf.get_option( "color_back_R", 65000 );
    color_back[ 1 ] = cf.get_option( "color_back_G", 65000 );
    color_back[ 2 ] = cf.get_option( "color_back_B", 63000 );

    // ツリービュー(板一覧)の背景色
    color_back_tree[ 0 ] = cf.get_option( "color_tree_R", 65000 );
    color_back_tree[ 1 ] = cf.get_option( "color_tree_G", 65000 );
    color_back_tree[ 2 ] = cf.get_option( "color_tree_B", 63000 );

    // ツリービュー(スレ一覧)の背景色
    color_back_tree_board[ 0 ] = cf.get_option( "color_tree_board_R", color_back_tree[ 0 ] );
    color_back_tree_board[ 1 ] = cf.get_option( "color_tree_board_G", color_back_tree[ 1 ] );
    color_back_tree_board[ 2 ] = cf.get_option( "color_tree_board_B", color_back_tree[ 2 ] );

    // ポップアップの背景色
    color_back_popup[ 0 ] = cf.get_option( "color_popup_R", 65000 );
    color_back_popup[ 1 ] = cf.get_option( "color_popup_G", 65000 );
    color_back_popup[ 2 ] = cf.get_option( "color_popup_B", 63000 );

    // boardビューで古いスレも表示
    show_oldarticle = cf.get_option( "show_oldarticle", false );

    // スレ一覧で指定した値(時間)よりも後に立てられたスレを新着とみなす
    newthread_hour = cf.get_option( "newthread_hour", 24 );


    /////////////////////////
    // UI 周りの設定

    // ツリービューのスクロール量(行数)
    tree_scroll_size = cf.get_option( "tree_scroll_size", 4 );

    // 板一覧でカテゴリを常にひとつだけ開く
    open_one_category = cf.get_option( "open_one_category", false );

    // 書き込み時に書き込み確認ダイアログを出すかどうか
    always_write_ok = cf.get_option( "always_write_ok", false );

    // 書き込みログを保存
    save_postlog = cf.get_option( "save_postlog", false );

    // 書き込み中のダイアログを表示しない
    hide_writing_dialog = cf.get_option( "hide_writing_dialog", false );

    // ポップアップとカーソルの間のマージン
    margin_popup = cf.get_option( "margin_popup", 30 );

    // マウスジェスチャの判定開始半径
    mouse_radius = cf.get_option( "mouse_radius", 25 );

    // 履歴の保持数
    history_size = cf.get_option( "history_size", 20 );

    // 0以上なら多重ポップアップの説明を表示する
    instruct_popup = cf.get_option( "instruct_popup", 100 );    

    // スレ表示の行間調整
    adjust_underline_pos = cf.get_option( "adjust_underline_pos", 1.0 );
    adjust_line_space = cf.get_option( "adjust_line_space", 1.0 );

    // リンク下線を表示
    draw_underline = cf.get_option( "draw_underline", true );

    // スレビューで文字幅の近似を厳密にする
    strict_char_width = cf.get_option( "strict_char_width", false );

    // タブに表示する文字列の最小値
    tab_min_str = cf.get_option( "tab_min_str", 4 );

    // タブにアイコンを表示するか
    show_tab_icon = cf.get_option( "show_tab_icon", true );

    std::list< std::string > list_tmp;
    std::list< std::string >::iterator it_tmp;
    std::string str_tmp;

    // スレ あぼーん word
    str_tmp = cf.get_option( "abonewordthread", "" );
    if( ! str_tmp.empty() ) list_abone_word_thread = MISC::strtolist( str_tmp );

    // スレ あぼーん regex
    str_tmp = cf.get_option( "aboneregexthread", "" );
    if( ! str_tmp.empty() ) list_abone_regex_thread = MISC::strtolist( str_tmp );

    // あぼーん name
    str_tmp = cf.get_option( "abonename", "" );
    if( ! str_tmp.empty() ) list_abone_name = MISC::strtolist( str_tmp );

    // あぼーん word
    str_tmp = cf.get_option( "aboneword", "" );
    if( ! str_tmp.empty() ) list_abone_word = MISC::strtolist( str_tmp );

    // あぼーん regex
    str_tmp = cf.get_option( "aboneregex", "" );
    if( ! str_tmp.empty() ) list_abone_regex = MISC::strtolist( str_tmp );

    // デフォルトで透明、連鎖あぼーんをするか
    abone_transparent = cf.get_option( "abone_transparent", false );
    abone_chain = cf.get_option( "abone_chain", false );

    return ! cf.empty();
}


//
// コンフィグファイル保存
// 
void CONFIG::save_conf()
{
    JDLIB::ConfLoader cf( CACHE::path_conf(), std::string() );

    cf.update( "restore_board", restore_board );
    cf.update( "restore_article", restore_article );
    cf.update( "restore_image", restore_image );
    cf.update( "url_login2ch", url_login2ch );
    cf.update( "url_bbsmenu", url_bbsmenu );

    cf.update( "fontname_main", fontname_main );
    cf.update( "fontname_popup", fontname_popup );
    cf.update( "fontname_tree", fontname_tree );
    cf.update( "fontname_tree_board", fontname_tree_board );
    cf.update( "fontname_message", fontname_message );

    cf.update( "ref_prefix", ref_prefix );
    cf.update( "ref_prefix_space", ref_prefix_space );

    cf.update( "path_cacheroot", path_cacheroot );

    cf.update( "agent_for2ch", agent_for2ch );

    cf.update( "use_proxy_for2ch", use_proxy_for2ch );
    cf.update( "proxy_for2ch", proxy_for2ch );
    cf.update( "proxy_port_for2ch", proxy_port_for2ch );

    cf.update( "use_proxy_for2ch_w", use_proxy_for2ch_w );
    cf.update( "proxy_for2ch_w", proxy_for2ch_w );
    cf.update( "proxy_port_for2ch_w", proxy_port_for2ch_w );

    cf.update( "agent_for_data", agent_for_data );

    cf.update( "use_proxy_for_data", use_proxy_for_data );
    cf.update( "proxy_for_data", proxy_for_data );
    cf.update( "proxy_port_for_data", proxy_port_for_data );

    cf.update( "x_2ch_ua", x_2ch_ua );

    cf.update( "loader_bufsize", loader_bufsize );
    cf.update( "loader_timeout", loader_timeout );
    cf.update( "loader_timeout_post", loader_timeout_post );
    cf.update( "loader_timeout_img", loader_timeout_img );

    cf.update( "command_openurl", command_openurl );
    cf.update( "brownsercombo_id", brownsercombo_id );

    cf.update( "imgpopup_width", imgpopup_width );
    cf.update( "imgpopup_height", imgpopup_height );
    cf.update( "use_mosaic", use_mosaic );
    cf.update( "zoom_to_fit", zoom_to_fit );
    cf.update( "del_img_day", del_img_day );
    cf.update( "max_img_size", max_img_size );

    cf.update( "color_char_R", color_char[ 0 ] );
    cf.update( "color_char_G", color_char[ 1 ] );
    cf.update( "color_char_B", color_char[ 2 ] );

    cf.update( "color_char_age_R", color_char_age[ 0 ] );
    cf.update( "color_char_age_G", color_char_age[ 1 ] );
    cf.update( "color_char_age_B", color_char_age[ 2 ] );

    cf.update( "color_sepa_R", color_sepa[ 0 ] );
    cf.update( "color_sepa_G", color_sepa[ 1 ] );
    cf.update( "color_sepa_B", color_sepa[ 2 ] );

    cf.update( "color_back_R", color_back[ 0 ] );
    cf.update( "color_back_G", color_back[ 1 ] );
    cf.update( "color_back_B", color_back[ 2 ] );

    cf.update( "color_tree_R", color_back_tree[ 0 ] );
    cf.update( "color_tree_G", color_back_tree[ 1 ] );
    cf.update( "color_tree_B", color_back_tree[ 2 ] );

    cf.update( "color_tree_board_R", color_back_tree_board[ 0 ] );
    cf.update( "color_tree_board_G", color_back_tree_board[ 1 ] );
    cf.update( "color_tree_board_B", color_back_tree_board[ 2 ] );

    cf.update( "color_popup_R", color_back_popup[ 0 ] );
    cf.update( "color_popup_G", color_back_popup[ 1 ] );
    cf.update( "color_popup_B", color_back_popup[ 2 ] );

    cf.update( "show_oldarticle", show_oldarticle );
    cf.update( "newthread_hour", newthread_hour );

    cf.update( "tree_scroll_size", tree_scroll_size );
    cf.update( "open_one_category", open_one_category );
    cf.update( "always_write_ok", always_write_ok );
    cf.update( "save_postlog", save_postlog );
    cf.update( "hide_writing_dialog", hide_writing_dialog );
    cf.update( "margin_popup", margin_popup );
    cf.update( "mouse_radius", mouse_radius );
    cf.update( "history_size", history_size );
    cf.update( "instruct_popup", instruct_popup );

    cf.update( "adjust_underline_pos", adjust_underline_pos );
    cf.update( "adjust_line_space", adjust_line_space );

    cf.update( "draw_underline", draw_underline );
    cf.update( "strict_char_width", strict_char_width );

    cf.update( "tab_min_str", tab_min_str );

    cf.update( "show_tab_icon", show_tab_icon );

    // スレあぼーん情報
    std::string str_abone_word_thread = MISC::listtostr( list_abone_word_thread );
    std::string str_abone_regex_thread = MISC::listtostr( list_abone_regex_thread );

    cf.update( "abonewordthread", str_abone_word_thread );
    cf.update( "aboneregexthread", str_abone_regex_thread );

    // あぼーん情報
    std::string str_abone_name = MISC::listtostr( list_abone_name );
    std::string str_abone_word = MISC::listtostr( list_abone_word );
    std::string str_abone_regex = MISC::listtostr( list_abone_regex );

    cf.update( "abonename", str_abone_name );
    cf.update( "aboneword", str_abone_word );
    cf.update( "aboneregex", str_abone_regex );

    cf.update( "abone_transparent", abone_transparent );
    cf.update( "abone_chain", abone_chain );

    cf.save();
}


const bool CONFIG::get_restore_board(){ return restore_board; }
void CONFIG::set_restore_board( bool restore ){ restore_board = restore; }
const bool CONFIG::get_restore_article(){ return restore_article; }
void CONFIG::set_restore_article( bool restore ){ restore_article = restore; }
const bool CONFIG::get_restore_image(){ return restore_image; }
void CONFIG::set_restore_image( bool restore ){ restore_image = restore; }

const int* CONFIG::get_color_char() { return color_char; }
const int* CONFIG::get_color_char_age() { return color_char_age; }
const int* CONFIG::get_color_separator() { return color_sepa; }
const int* CONFIG::get_color_back() { return color_back; }
const int* CONFIG::get_color_back_popup() { return color_back_popup; }
const int* CONFIG::get_color_back_tree() { return color_back_tree; }
const int* CONFIG::get_color_back_tree_board() { return color_back_tree_board; }

void CONFIG::set_color_char( int* color ) { memcpy( color_char, color, sizeof( int )*COLOR_SIZE ); }
void CONFIG::set_color_char_age( int* color ) { memcpy( color_char_age, color, sizeof( int )*COLOR_SIZE ); }
void CONFIG::set_color_separator( int* color ) { memcpy( color_sepa, color, sizeof( int )*COLOR_SIZE ); }
void CONFIG::set_color_back( int* color ) { memcpy( color_back, color, sizeof( int )*COLOR_SIZE ); }
void CONFIG::set_color_back_popup( int* color ) { memcpy( color_back_popup, color, sizeof( int )*COLOR_SIZE ); }
void CONFIG::set_color_back_tree( int* color ) { memcpy( color_back_tree, color, sizeof( int )*COLOR_SIZE ); }
void CONFIG::set_color_back_tree_board( int* color ) { memcpy( color_back_tree_board, color, sizeof( int )*COLOR_SIZE ); }


const std::string& CONFIG::get_fontname_main() { return fontname_main; }
const std::string& CONFIG::get_fontname_popup() { return fontname_popup; }
const std::string& CONFIG::get_fontname_tree() { return fontname_tree; }
const std::string& CONFIG::get_fontname_tree_board() { return fontname_tree_board; }
const std::string& CONFIG::get_fontname_message() { return fontname_message; }

void CONFIG::set_fontname_main( const std::string& name) { fontname_main = name; }
void CONFIG::set_fontname_popup( const std::string& name) { fontname_popup = name; }
void CONFIG::set_fontname_tree( const std::string& name) { fontname_tree = name; }
void CONFIG::set_fontname_tree_board( const std::string& name) { fontname_tree_board = name; }
void CONFIG::set_fontname_message( const std::string& name) { fontname_message = name; }

const std::string&  CONFIG::get_ref_prefix(){ return ref_prefix; }

const std::string& CONFIG::get_url_login2ch() { return url_login2ch; }
const std::string& CONFIG::get_url_bbsmenu() { return url_bbsmenu; }
const std::string& CONFIG::get_path_cacheroot() { return path_cacheroot; }

const std::string& CONFIG::get_agent_for2ch() { return agent_for2ch; }

const bool CONFIG::get_use_proxy_for2ch() { return use_proxy_for2ch; }
const std::string& CONFIG::get_proxy_for2ch() { return proxy_for2ch; }
const int CONFIG::get_proxy_port_for2ch() { return proxy_port_for2ch; }

void CONFIG::set_use_proxy_for2ch( bool set ){ use_proxy_for2ch = set; }
void CONFIG::set_proxy_for2ch( const std::string& proxy ){ proxy_for2ch = proxy; }
void CONFIG::set_proxy_port_for2ch( int port ){ proxy_port_for2ch = port; }

const bool CONFIG::get_use_proxy_for2ch_w() { return use_proxy_for2ch_w; }
const std::string& CONFIG::get_proxy_for2ch_w() { return proxy_for2ch_w; }
const int CONFIG::get_proxy_port_for2ch_w() { return proxy_port_for2ch_w; }

void CONFIG::set_use_proxy_for2ch_w( bool set ){ use_proxy_for2ch_w = set; }
void CONFIG::set_proxy_for2ch_w( const std::string& proxy ){ proxy_for2ch_w = proxy; }
void CONFIG::set_proxy_port_for2ch_w( int port ){ proxy_port_for2ch_w = port; }

const std::string& CONFIG::get_agent_for_data() { return agent_for_data; }

const bool CONFIG::get_use_proxy_for_data() { return use_proxy_for_data; }
const std::string& CONFIG::get_proxy_for_data() { return proxy_for_data; }
const int CONFIG::get_proxy_port_for_data() { return proxy_port_for_data; }

const std::string& CONFIG::get_x_2ch_ua() { return x_2ch_ua; }

void CONFIG::set_use_proxy_for_data( bool set ){ use_proxy_for_data = set; }
void CONFIG::set_proxy_for_data( const std::string& proxy ){ proxy_for_data = proxy; }
void CONFIG::set_proxy_port_for_data( int port ){ proxy_port_for_data = port; }

const int CONFIG::get_loader_bufsize(){ return loader_bufsize; }
const int CONFIG::get_loader_timeout(){ return loader_timeout; }
const int CONFIG::get_loader_timeout_post(){ return loader_timeout_post; }
const int CONFIG::get_loader_timeout_img(){ return loader_timeout_img; }

const std::string& CONFIG::get_command_openurl() { return command_openurl; }
void CONFIG::set_command_openurl( const std::string& command ){ command_openurl = command; }

const int CONFIG::get_brownsercombo_id(){ return brownsercombo_id; }
void CONFIG::set_brownsercombo_id( int id ){ brownsercombo_id = id; }

const int CONFIG::get_imgpopup_width(){ return imgpopup_width; }
const int CONFIG::get_imgpopup_height(){ return imgpopup_height; }
const bool CONFIG::get_use_mosaic(){ return use_mosaic; }
void CONFIG::set_use_mosaic( bool mosaic ) { use_mosaic = mosaic; }
const bool CONFIG::get_zoom_to_fit(){ return zoom_to_fit; }
void CONFIG::set_zoom_to_fit( bool fit ){ zoom_to_fit = fit; }
const int CONFIG::get_del_img_day(){ return del_img_day; }
void CONFIG::set_del_img_day( int day ){ del_img_day = day; }
const int CONFIG::get_max_img_size(){ return max_img_size; }

const bool CONFIG::get_show_oldarticle(){ return show_oldarticle; }
void CONFIG::set_show_oldarticle( bool showarticle ){ show_oldarticle = showarticle; }

const int CONFIG::get_newthread_hour(){ return newthread_hour; }

const int CONFIG::get_tree_scroll_size(){ return tree_scroll_size; }
const bool CONFIG::get_open_one_category(){ return open_one_category; }
const bool CONFIG::get_always_write_ok() { return always_write_ok; }
void CONFIG::set_always_write_ok( bool write_ok ){ always_write_ok = write_ok; }
const bool CONFIG::get_save_postlog(){ return save_postlog; }
void CONFIG::set_save_postlog( bool save ){ save_postlog = save; }
const bool CONFIG::get_hide_writing_dialog(){ return hide_writing_dialog; }

const int CONFIG::get_margin_popup(){ return margin_popup; }
const int CONFIG::get_mouse_radius(){ return mouse_radius; }
const int CONFIG::get_history_size(){ return history_size; }

// 0以上なら多重ポップアップの説明を表示する
// 呼び出される度に--する
const int CONFIG::get_instruct_popup(){
    if( instruct_popup ) return instruct_popup--;
    return 0;
}

const double CONFIG::get_adjust_underline_pos(){ return adjust_underline_pos; }
const double CONFIG::get_adjust_line_space(){ return adjust_line_space; }

const bool CONFIG::get_draw_underline(){ return draw_underline; }
const bool CONFIG::get_strict_char_width(){ return strict_char_width; }

const int CONFIG::get_tab_min_str(){ return tab_min_str; }

const bool CONFIG::get_show_tab_icon(){ return show_tab_icon; }

std::list< std::string >& CONFIG::get_list_abone_word_thread(){ return list_abone_word_thread; }
std::list< std::string >& CONFIG::get_list_abone_regex_thread(){ return list_abone_regex_thread; }


void CONFIG::set_list_abone_word_thread( std::list< std::string >& word )
{
    // 前後の空白と空白行を除く
    list_abone_word_thread = MISC::remove_space_from_list( word );
    list_abone_word_thread = MISC::remove_nullline_from_list( list_abone_word_thread );
}


void CONFIG::set_list_abone_regex_thread( std::list< std::string >& regex )
{
    // 前後の空白と空白行を除く
    list_abone_regex_thread = MISC::remove_space_from_list( regex );
    list_abone_regex_thread = MISC::remove_nullline_from_list( list_abone_regex_thread );
}


std::list< std::string >& CONFIG::get_list_abone_name(){ return list_abone_name; }
std::list< std::string >& CONFIG::get_list_abone_word(){ return list_abone_word; }
std::list< std::string >& CONFIG::get_list_abone_regex(){ return list_abone_regex; }

void CONFIG::set_list_abone_name( std::list< std::string >& name )
{
    // 前後の空白と空白行を除く
    list_abone_name = MISC::remove_space_from_list( name );
    list_abone_name = MISC::remove_nullline_from_list( list_abone_name );
}

void CONFIG::set_list_abone_word( std::list< std::string >& word )
{
    // 前後の空白と空白行を除く
    list_abone_word = MISC::remove_space_from_list( word );
    list_abone_word = MISC::remove_nullline_from_list( list_abone_word );
}


void CONFIG::set_list_abone_regex( std::list< std::string >& regex )
{
    // 前後の空白と空白行を除く
    list_abone_regex = MISC::remove_space_from_list( regex );
    list_abone_regex = MISC::remove_nullline_from_list( list_abone_regex );
}

const bool CONFIG::get_abone_transparent(){ return abone_transparent; }
void CONFIG::set_abone_transparent( bool set ){ abone_transparent = set; }
const bool CONFIG::get_abone_chain(){ return abone_chain; }
void CONFIG::set_abone_chain( bool set ){ abone_chain = set; }
