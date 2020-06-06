// ライセンス: GPL2

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

//#define _DEBUG
#include "jddebug.h"

#include "globalconf.h"
#include "configitems.h"

#include "jdlib/miscutil.h"


CONFIG::ConfigItems* instance_confitem = nullptr;
CONFIG::ConfigItems* instance_confitem_bkup = nullptr;


CONFIG::ConfigItems* CONFIG::get_confitem()
{
    return instance_confitem;
}


void CONFIG::delete_confitem()
{
    if( instance_confitem ) delete instance_confitem;
    instance_confitem = nullptr;

    if( instance_confitem_bkup ) delete instance_confitem_bkup;
    instance_confitem_bkup = nullptr;
}


bool CONFIG::load_conf()
{
    if( ! instance_confitem ) instance_confitem = new CONFIG::ConfigItems();
    return get_confitem()->load();
}


void CONFIG::save_conf()
{
    get_confitem()->save();
}


void CONFIG::bkup_conf()
{
    if( ! instance_confitem_bkup ) instance_confitem_bkup = new CONFIG::ConfigItems();
    *instance_confitem_bkup = * instance_confitem;
}


void CONFIG::restore_conf()
{
    if( ! instance_confitem_bkup ) return;
    *instance_confitem = * instance_confitem_bkup;
}


//////////////////////////////////////////////////////////////



bool CONFIG::get_restore_board(){ return get_confitem()->restore_board; }
void CONFIG::set_restore_board( const bool restore ){ get_confitem()->restore_board = restore; }
bool CONFIG::get_restore_article(){ return get_confitem()->restore_article; }
void CONFIG::set_restore_article( const bool restore ){ get_confitem()->restore_article = restore; }
bool CONFIG::get_restore_image(){ return get_confitem()->restore_image; }
void CONFIG::set_restore_image( const bool restore ){ get_confitem()->restore_image = restore; }

bool CONFIG::get_manage_winpos(){ return get_confitem()->manage_winpos; }
void CONFIG::set_manage_winpos( const bool manage ){ get_confitem()->manage_winpos = manage; }


// 色
const std::string& CONFIG::get_color( const int id )
{
    return get_confitem()->str_color[ id ];
}

void CONFIG::set_color( const int id, const std::string& color )
{
    get_confitem()->str_color[ id ] = color;
}

void CONFIG::reset_colors(){ get_confitem()->reset_colors(); }

bool CONFIG::get_use_message_gtktheme() { return get_confitem()->use_message_gtktheme; }
void CONFIG::set_use_message_gtktheme( const bool use ) { get_confitem()->use_message_gtktheme = use; }

bool CONFIG::get_use_tree_gtkrc(){ return get_confitem()->use_tree_gtkrc; }
void CONFIG::set_use_tree_gtkrc( const bool use ){ get_confitem()->use_tree_gtkrc = use; }

bool CONFIG::get_use_select_gtkrc(){ return get_confitem()->use_select_gtkrc; }
void CONFIG::set_use_select_gtkrc( const bool use ){ get_confitem()->use_select_gtkrc = use; }

// ツリービューの行間スペース
int CONFIG::get_tree_ypad(){ return get_confitem()->tree_ypad; }

// ツリービューにエクスパンダを表示
bool CONFIG::get_tree_show_expanders(){ return get_confitem()->tree_show_expanders; }

// ツリービューのレベルインデント調整量(ピクセル)
int CONFIG::get_tree_level_indent(){ return get_confitem()->tree_level_indent; }

// カテゴリやディレクトリを開いたときにツリービューをスクロールする
bool CONFIG::get_scroll_tree(){ return get_confitem()->scroll_tree; }

// ツリービューの選択を表示中のビューと同期する ( 0: 同期しない 1: 同期する 2: 同期する(フォルダを開く) )
int CONFIG::get_select_item_sync(){ return get_confitem()->select_item_sync; }
void CONFIG::set_select_item_sync( const int sync ){ get_confitem()->select_item_sync = sync; }

int CONFIG::get_view_margin(){ return get_confitem()->view_margin; }

// スクロールバーを左に配置
bool CONFIG::get_left_scrbar(){ return get_confitem()->left_scrbar; }

// スレ一覧で古いスレも表示 	
bool CONFIG::get_show_oldarticle(){ return get_confitem()->show_oldarticle; }

// フォント
const std::string& CONFIG::get_fontname( const int id )
{
    return get_confitem()->fontname[ id ];
}

void CONFIG::set_fontname( const int id, const std::string& fontname )
{
    get_confitem()->fontname[ id ] = fontname;
}

void CONFIG::reset_fonts(){ get_confitem()->reset_fonts(); }

bool CONFIG::get_aafont_enabled(){ return get_confitem()->aafont_enabled; }


std::string CONFIG::get_ref_prefix(){ return get_confitem()->ref_prefix + get_confitem()->ref_prefix_space_str; }
int CONFIG::ref_prefix_space(){ return get_confitem()->ref_prefix_space; }

// レスにアスキーアートがあると判定する正規表現
std::string CONFIG::get_regex_res_aa(){
    std::string str = get_confitem()->regex_res_aa;
    int size = str.size();

    // ダブルクオートの削除
    if( size >= 2 && str[ 0 ] == '"' && str[ size - 1 ] == '"' ){
        str = str.substr( 1, size - 2 );
    }
    return str;
}
void CONFIG::set_regex_res_aa( const std::string& regex ){
    get_confitem()->regex_res_aa = "\"" + regex + "\"";
}

const std::string& CONFIG::get_url_jdhp() { return get_confitem()->url_jdhp; }
const std::string& CONFIG::get_url_jdimhp() { return get_confitem()->url_jdimhp; }

// 2chの認証サーバのアドレス
const std::string& CONFIG::get_url_login2ch() { return get_confitem()->url_login2ch; }

// BEの認証サーバのアドレス
const std::string& CONFIG::get_url_loginbe() { return get_confitem()->url_loginbe; }

const std::string& CONFIG::get_url_bbsmenu() { return get_confitem()->url_bbsmenu; }

bool CONFIG::use_link_as_board(){ return get_confitem()->use_link_as_board; }

bool CONFIG::get_show_movediag(){ return get_confitem()->show_movediag; }
void CONFIG::set_show_movediag( const bool show ){ get_confitem()->show_movediag = show; }

const std::string& CONFIG::get_menu_search_title(){ return get_confitem()->menu_search_title; }
const std::string& CONFIG::get_url_search_title(){ return get_confitem()->url_search_title; }
const std::string& CONFIG::get_regex_search_title(){ return get_confitem()->regex_search_title; }

const std::string& CONFIG::get_menu_search_web(){ return get_confitem()->menu_search_web; }
const std::string& CONFIG::get_url_search_web(){ return get_confitem()->url_search_web; }

const std::string& CONFIG::get_agent_for2ch() { return get_confitem()->agent_for2ch; }

bool CONFIG::get_use_proxy_for2ch() { return get_confitem()->use_proxy_for2ch; }
bool CONFIG::get_send_cookie_to_proxy_for2ch() { return get_confitem()->send_cookie_to_proxy_for2ch; }
const std::string& CONFIG::get_proxy_for2ch() { return get_confitem()->proxy_for2ch; }
int CONFIG::get_proxy_port_for2ch() { return get_confitem()->proxy_port_for2ch; }
const std::string& CONFIG::get_proxy_basicauth_for2ch() { return get_confitem()->proxy_basicauth_for2ch; }

void CONFIG::set_use_proxy_for2ch( bool set ){ get_confitem()->use_proxy_for2ch = set; }
void CONFIG::set_send_cookie_to_proxy_for2ch( bool set ){ get_confitem()->send_cookie_to_proxy_for2ch = set; }
void CONFIG::set_proxy_for2ch( const std::string& proxy ){ get_confitem()->set_proxy_for2ch( proxy ); }
void CONFIG::set_proxy_port_for2ch( int port ){ get_confitem()->proxy_port_for2ch = port; }

bool CONFIG::get_use_proxy_for2ch_w() { return get_confitem()->use_proxy_for2ch_w; }
bool CONFIG::get_send_cookie_to_proxy_for2ch_w() { return get_confitem()->send_cookie_to_proxy_for2ch_w; }
const std::string& CONFIG::get_proxy_for2ch_w() { return get_confitem()->proxy_for2ch_w; }
int CONFIG::get_proxy_port_for2ch_w() { return get_confitem()->proxy_port_for2ch_w; }
const std::string& CONFIG::get_proxy_basicauth_for2ch_w() { return get_confitem()->proxy_basicauth_for2ch_w; }

void CONFIG::set_use_proxy_for2ch_w( bool set ){ get_confitem()->use_proxy_for2ch_w = set; }
void CONFIG::set_send_cookie_to_proxy_for2ch_w( bool set ){ get_confitem()->send_cookie_to_proxy_for2ch_w = set; }
void CONFIG::set_proxy_for2ch_w( const std::string& proxy ){ get_confitem()->set_proxy_for2ch_w( proxy ); }
void CONFIG::set_proxy_port_for2ch_w( int port ){ get_confitem()->proxy_port_for2ch_w = port; }

const std::string& CONFIG::get_agent_for_data() { return get_confitem()->agent_for_data; }

bool CONFIG::get_use_proxy_for_data() { return get_confitem()->use_proxy_for_data; }
bool CONFIG::get_send_cookie_to_proxy_for_data() { return get_confitem()->send_cookie_to_proxy_for_data; }
const std::string& CONFIG::get_proxy_for_data() { return get_confitem()->proxy_for_data; }
int CONFIG::get_proxy_port_for_data() { return get_confitem()->proxy_port_for_data; }
const std::string& CONFIG::get_proxy_basicauth_for_data() { return get_confitem()->proxy_basicauth_for_data; }

const std::string& CONFIG::get_x_2ch_ua() { return get_confitem()->x_2ch_ua; }

void CONFIG::set_use_proxy_for_data( bool set ){ get_confitem()->use_proxy_for_data = set; }
void CONFIG::set_send_cookie_to_proxy_for_data( bool set ){ get_confitem()->send_cookie_to_proxy_for_data = set; }
void CONFIG::set_proxy_for_data( const std::string& proxy ){ get_confitem()->set_proxy_for_data( proxy ); }
void CONFIG::set_proxy_port_for_data( int port ){ get_confitem()->proxy_port_for_data = port; }

int CONFIG::get_loader_bufsize(){ return get_confitem()->loader_bufsize; }
int CONFIG::get_loader_bufsize_board(){ return get_confitem()->loader_bufsize_board; }

int CONFIG::get_loader_timeout(){ return get_confitem()->loader_timeout; }
int CONFIG::get_loader_timeout_post(){ return get_confitem()->loader_timeout_post; }
int CONFIG::get_loader_timeout_data(){ return get_confitem()->loader_timeout_img; } // 旧 get_loader_timeout_img() 関数
int CONFIG::get_loader_timeout_checkupdate(){ return get_confitem()->loader_timeout_checkupdate; }

bool CONFIG::get_use_ipv6(){ return get_confitem()->use_ipv6; }
void CONFIG::set_use_ipv6( const bool set ){ get_confitem()->use_ipv6 = set; }

// 同一ホストに対する最大コネクション数( 1 または 2 )
int CONFIG::get_connection_num(){ return get_confitem()->connection_num; }

// 2chのクッキー (互換性のため設定名は旧名称を使う)
bool CONFIG::get_use_cookie_hap(){ return get_confitem()->use_cookie_hap; }
const std::string& CONFIG::get_cookie_hap(){ return get_confitem()->cookie_hap; }
const std::string& CONFIG::get_cookie_hap_bbspink(){ return get_confitem()->cookie_hap_bbspink; }
void CONFIG::set_cookie_hap( const std::string& cookie_hap ){ get_confitem()->cookie_hap = cookie_hap; }
void CONFIG::set_cookie_hap_bbspink( const std::string& cookie_hap ){ get_confitem()->cookie_hap_bbspink = cookie_hap; }

const std::string& CONFIG::get_command_openurl() { return get_confitem()->command_openurl; }
void CONFIG::set_command_openurl( const std::string& command ){ get_confitem()->command_openurl = command; }

int CONFIG::get_browsercombo_id(){ return get_confitem()->browsercombo_id; }
void CONFIG::set_browsercombo_id( const int id ){ get_confitem()->browsercombo_id = id; }

bool CONFIG::get_refpopup_by_mo(){ return get_confitem()->refpopup_by_mo; }
bool CONFIG::get_namepopup_by_mo(){ return get_confitem()->namepopup_by_mo; }
bool CONFIG::get_idpopup_by_mo(){ return get_confitem()->idpopup_by_mo; }

int CONFIG::get_imgemb_interp(){ return get_confitem()->imgemb_interp; }
int CONFIG::get_imgmain_interp(){ return get_confitem()->imgmain_interp; }
int CONFIG::get_imgpopup_interp(){ return get_confitem()->imgpopup_interp; }

int CONFIG::get_imgpopup_width(){ return get_confitem()->imgpopup_width; }
int CONFIG::get_imgpopup_height(){ return get_confitem()->imgpopup_height; }

bool CONFIG::get_use_image_popup(){ return get_confitem()->use_image_popup; }
void CONFIG::set_use_image_popup( const bool use ){ get_confitem()->use_image_popup = use; }

bool CONFIG::get_use_image_view(){ return get_confitem()->use_image_view; }
void CONFIG::set_use_image_view( const bool image_view ){ get_confitem()->use_image_view = image_view; }

bool CONFIG::get_use_inline_image(){ return get_confitem()->use_inline_image; }
void CONFIG::set_use_inline_image( const bool inline_img ){ get_confitem()->use_inline_image = inline_img; }

bool CONFIG::get_show_ssspicon(){ return get_confitem()->show_ssspicon; }
void CONFIG::set_show_sssp_icon( const bool show ){ get_confitem()->show_ssspicon = show; }

// インライン画像の最大幅と高さ
int CONFIG::get_embimg_width(){ return get_confitem()->embimg_width; }
int CONFIG::get_embimg_height(){ return get_confitem()->embimg_height; }

// 埋め込み画像ビューを閉じたときにタブも閉じる
bool CONFIG::get_hide_imagetab(){ return get_confitem()->hide_imagetab; }

// 画像ビューでdeleteを押したときに確認ダイアログを表示する
bool CONFIG::get_show_delimgdiag(){ return get_confitem()->show_delimgdiag; }
void CONFIG::set_show_delimgdiag( const bool show ){ get_confitem()->show_delimgdiag = show; }

bool CONFIG::get_use_mosaic(){ return get_confitem()->use_mosaic; }
void CONFIG::set_use_mosaic( const bool mosaic ) { get_confitem()->use_mosaic = mosaic; }
int CONFIG::get_mosaic_size(){ return get_confitem()->mosaic_size; }
bool CONFIG::get_zoom_to_fit(){ return get_confitem()->zoom_to_fit; }
void CONFIG::set_zoom_to_fit( const bool fit ){ get_confitem()->zoom_to_fit = fit; }
int CONFIG::get_del_img_day(){ return get_confitem()->del_img_day; }
void CONFIG::set_del_img_day( const int day ){ get_confitem()->del_img_day = day; }
int CONFIG::get_del_imgabone_day(){ return get_confitem()->del_imgabone_day; }
void CONFIG::set_del_imgabone_day( const int day ){ get_confitem()->del_imgabone_day = day; }
int CONFIG::get_max_img_size(){ return get_confitem()->max_img_size; }
int CONFIG::get_max_img_pixel(){ return get_confitem()->max_img_pixel; }
int CONFIG::get_imgcache_size(){ return get_confitem()->imgcache_size; }

int CONFIG::get_newthread_hour(){ return get_confitem()->newthread_hour; }

bool CONFIG::get_inc_search_board(){ return get_confitem()->inc_search_board; }

bool CONFIG::get_show_deldiag(){ return get_confitem()->show_deldiag; }
void CONFIG::set_show_deldiag( const bool show ){ get_confitem()->show_deldiag = show; }

// スレ一覧をロードする前にキャッシュにある一覧を表示
bool CONFIG::get_show_cached_board(){ return get_confitem()->show_cached_board; }

// スレ一覧でお知らせスレ(924)のアイコンを表示する
bool CONFIG::get_show_924(){ return get_confitem()->show_924; }

int CONFIG::get_tree_scroll_size(){ return get_confitem()->tree_scroll_size; }
int CONFIG::get_scroll_size(){ return get_confitem()->scroll_size; }
int CONFIG::get_key_scroll_size(){ return get_confitem()->key_scroll_size; }
int CONFIG::get_key_fastscroll_size(){ return get_confitem()->key_fastscroll_size; }

bool CONFIG::get_jump_after_reload(){ return get_confitem()->jump_after_reload; }
void CONFIG::set_jump_after_reload( const bool set ){ get_confitem()->jump_after_reload = set; }
bool CONFIG::get_jump_new_after_reload(){ return get_confitem()->jump_new_after_reload; }
void CONFIG::set_jump_new_after_reload( const bool set ){ get_confitem()->jump_new_after_reload = set; }

int CONFIG::get_live_mode(){ return get_confitem()->live_mode; }
void CONFIG::set_live_mode( const int mode ) { get_confitem()->live_mode = mode; }
int CONFIG::get_live_speed(){ return get_confitem()->live_speed; }
void CONFIG::set_live_speed( const int speed ){ get_confitem()->live_speed = speed; }
int CONFIG::get_live_threshold(){ return get_confitem()->live_threshold; }
void CONFIG::set_live_threshode( const int th ){ get_confitem()->live_threshold = th; }

bool CONFIG::get_open_one_category(){ return get_confitem()->open_one_category; }
bool CONFIG::get_open_one_favorite(){ return get_confitem()->open_one_favorite; }

// デフォルトの書き込み名
std::string CONFIG::get_write_name(){ return get_confitem()->write_name; }

// デフォルトのメールアドレス
std::string CONFIG::get_write_mail(){ return get_confitem()->write_mail; }

bool CONFIG::get_always_write_ok() { return get_confitem()->always_write_ok; }
void CONFIG::set_always_write_ok( const bool write_ok ){ get_confitem()->always_write_ok = write_ok; }

bool CONFIG::get_save_post_log(){ return get_confitem()->save_postlog; }
void CONFIG::set_save_post_log( const bool save ){ get_confitem()->save_postlog = save; }
size_t CONFIG::get_maxsize_post_log(){ return get_confitem()->maxsize_postlog; }

// 書き込み履歴を保存
bool CONFIG::get_save_post_history(){ return get_confitem()->save_posthist; }
void CONFIG::set_save_post_history( const bool save ){ get_confitem()->save_posthist = save; }

bool CONFIG::get_hide_writing_dialog(){ return get_confitem()->hide_writing_dialog; }

// 編集中のメッセージの保存確認ダイアログを表示する
bool CONFIG::get_show_savemsgdiag(){ return get_confitem()->show_savemsgdiag; }
void CONFIG::set_show_savemsgdiag( const bool show ){ get_confitem()->show_savemsgdiag = show; }

// 書き込みビューでテキストを折り返す
bool CONFIG::get_message_wrap(){ return get_confitem()->message_wrap; }
void CONFIG::set_message_wrap( const bool wrap ){ get_confitem()->message_wrap = wrap; }

bool CONFIG::get_fold_message(){ return get_confitem()->fold_message; }
void CONFIG::set_fold_message( const bool fold ){ get_confitem()->fold_message = fold; }

bool CONFIG::get_fold_image(){ return get_confitem()->fold_image; }

bool CONFIG::get_keep_im_status(){ return get_confitem()->keep_im_status; }

int CONFIG::get_margin_popup(){ return get_confitem()->margin_popup; }
void CONFIG::set_margin_popup( const int margin ){ get_confitem()->margin_popup = margin; }

// 画像ポップアップとカーソルの間のマージン
int CONFIG::get_margin_imgpopup_x(){ return get_confitem()->margin_imgpopup_x; }
int CONFIG::get_margin_imgpopup(){ return get_confitem()->margin_imgpopup; }

// ポップアップが消えるまでの時間(ミリ秒)
int CONFIG::get_hide_popup_msec(){ return get_confitem()->hide_popup_msec; }

// マウスジェスチャを有効
bool CONFIG::get_enable_mg(){ return get_confitem()->enable_mg; }

// マウスジェスチャの判定開始半径
int CONFIG::get_mouse_radius(){ return get_confitem()->mouse_radius; }

// 数字入力ジャンプのウェイト(ミリ秒)
int CONFIG::get_numberjmp_msec(){ return get_confitem()->numberjmp_msec; }

int CONFIG::get_history_size(){ return get_confitem()->history_size; }
int CONFIG::get_historyview_size(){ return get_confitem()->historyview_size; }
int CONFIG::get_aahistory_size(){ return get_confitem()->aahistory_size; }

// 0以上なら多重ポップアップの説明を表示する
// 呼び出される度に--する
int CONFIG::get_instruct_popup(){
    if( get_confitem()->instruct_popup ) return get_confitem()->instruct_popup--;
    return 0;
}


bool CONFIG::get_instruct_tglart(){

    if( get_confitem()->instruct_tglart_end ) return false;

    get_confitem()->instruct_tglart_end = true; // 一度表示したら表示しない
    return get_confitem()->instruct_tglart;
}
void CONFIG::set_instruct_tglart( const bool tgl ){ get_confitem()->instruct_tglart = tgl; }

bool CONFIG::get_instruct_tglimg(){

    if( get_confitem()->instruct_tglimg_end ) return false;

    get_confitem()->instruct_tglimg_end = true; // 一度表示したら表示しない
    return get_confitem()->instruct_tglimg;
}

void CONFIG::set_instruct_tglimg( bool tgl ){ get_confitem()->instruct_tglimg = tgl; }


// スレビューでdeleteを押したときに確認ダイアログを表示する
bool CONFIG::get_show_delartdiag(){ return get_confitem()->show_delartdiag; }
void CONFIG::set_show_delartdiag( const bool show ){ get_confitem()->show_delartdiag = show; }


double CONFIG::get_adjust_underline_pos(){ return get_confitem()->adjust_underline_pos; }
void CONFIG::set_adjust_underline_pos( const double pos ){ get_confitem()->adjust_underline_pos = pos; }

double CONFIG::get_adjust_line_space(){ return get_confitem()->adjust_line_space; }
void CONFIG::set_adjust_line_space( const double space ){ get_confitem()->adjust_line_space = space; }

bool CONFIG::get_draw_underline(){ return get_confitem()->draw_underline; }

bool CONFIG::get_strict_char_width(){ return get_confitem()->strict_char_width; }
void CONFIG::set_strict_char_width( const bool strictwidth ){ get_confitem()->strict_char_width = strictwidth; }

bool CONFIG::get_check_id(){ return get_confitem()->check_id; }

int CONFIG::get_num_reference_high(){ return get_confitem()->num_reference_high; }
int CONFIG::get_num_reference_low(){ return get_confitem()->num_reference_low; }
int CONFIG::get_num_id_high(){ return get_confitem()->num_id_high; }
int CONFIG::get_num_id_low(){ return get_confitem()->num_id_low; }

bool CONFIG::get_loose_url(){ return get_confitem()->loose_url; }

bool CONFIG::get_hide_usrcmd(){ return get_confitem()->hide_usrcmd; }
void CONFIG::set_hide_usrcmd( const bool hide ){ get_confitem()->hide_usrcmd = hide; }

bool CONFIG::get_reload_allthreads(){ return get_confitem()->reload_allthreads; }

int CONFIG::get_tab_min_str(){ return get_confitem()->tab_min_str; }

bool CONFIG::get_show_tab_icon(){ return get_confitem()->show_tab_icon; }

// タブ上でマウスホイールを回転してタブを切り替える
bool CONFIG::get_switchtab_wheel(){ return get_confitem()->switchtab_wheel; }

// 他のビューを開くときのタブの位置 ( 0: 一番右端 1:右隣 2:左隣 )
int CONFIG::get_newtab_pos(){  return get_confitem()->newtab_pos; }

// ツリービューで選択したビューを開くときのタブの位置 ( 0: 一番右端 1:右隣 2:左隣 )
int CONFIG::get_opentab_pos(){  return get_confitem()->opentab_pos; }

// 次スレ検索を開くときのタブの位置 ( 0: 次スレ検索タブ 1:新しいタブ 2:アクティブなタブを置き換え )
int CONFIG::get_boardnexttab_pos(){  return get_confitem()->boardnexttab_pos; }

bool CONFIG::get_show_post_mark(){ return get_confitem()->show_post_mark; }
void CONFIG::set_show_post_mark( const bool show ){ get_confitem()->show_post_mark = show; }

bool CONFIG::get_flat_button(){ return get_confitem()->flat_button; }
void CONFIG::set_flat_button( const bool set ){ get_confitem()->flat_button = set; }

// ツールバーの背景描画
bool CONFIG::get_draw_toolbarback(){ return get_confitem()->draw_toolbarback; }
void CONFIG::set_draw_toolbarback( const bool set ){ get_confitem()->draw_toolbarback = set; }

std::list< std::string >& CONFIG::get_list_abone_word_thread(){ return get_confitem()->list_abone_word_thread; }
std::list< std::string >& CONFIG::get_list_abone_regex_thread(){ return get_confitem()->list_abone_regex_thread; }


void CONFIG::set_list_abone_word_thread( std::list< std::string >& word )
{
    // 前後の空白と空白行を除く
    get_confitem()->list_abone_word_thread = MISC::remove_space_from_list( word );
    get_confitem()->list_abone_word_thread = MISC::remove_nullline_from_list( get_confitem()->list_abone_word_thread );
}


void CONFIG::set_list_abone_regex_thread( std::list< std::string >& regex )
{
    // 前後の空白と空白行を除く
    get_confitem()->list_abone_regex_thread = MISC::remove_space_from_list( regex );
    get_confitem()->list_abone_regex_thread = MISC::remove_nullline_from_list( get_confitem()->list_abone_regex_thread );
}


int CONFIG::get_remove_old_abone_thread(){ return get_confitem()->remove_old_abone_thread; }
void CONFIG::set_remove_old_abone_thread( const int remove ){ get_confitem()->remove_old_abone_thread = remove; }

int CONFIG::get_abone_number_thread(){ return get_confitem()->abone_number_thread; }
void CONFIG::set_abone_number_thread( const int number ){ get_confitem()->abone_number_thread = number; }

int CONFIG::get_abone_hour_thread(){ return get_confitem()->abone_hour_thread; }
void CONFIG::set_abone_hour_thread( const int hour ){ get_confitem()->abone_hour_thread = hour; }


const std::list< std::string >& CONFIG::get_list_abone_name(){ return get_confitem()->list_abone_name; }
const std::list< std::string >& CONFIG::get_list_abone_word(){ return get_confitem()->list_abone_word; }
const std::list< std::string >& CONFIG::get_list_abone_regex(){ return get_confitem()->list_abone_regex; }

void CONFIG::set_list_abone_name( const std::list< std::string >& name )
{
    // 前後の空白と空白行を除く
    get_confitem()->list_abone_name = MISC::remove_space_from_list( name );
    get_confitem()->list_abone_name = MISC::remove_nullline_from_list( get_confitem()->list_abone_name );
}

void CONFIG::set_list_abone_word( const std::list< std::string >& word )
{
    // 前後の空白と空白行を除く
    get_confitem()->list_abone_word = MISC::remove_space_from_list( word );
    get_confitem()->list_abone_word = MISC::remove_nullline_from_list( get_confitem()->list_abone_word );
}


void CONFIG::set_list_abone_regex( const std::list< std::string >& regex )
{
    // 前後の空白と空白行を除く
    get_confitem()->list_abone_regex = MISC::remove_space_from_list( regex );
    get_confitem()->list_abone_regex = MISC::remove_nullline_from_list( get_confitem()->list_abone_regex );
}


// デフォルトで透明、連鎖あぼーんをするか
bool CONFIG::get_abone_transparent(){ return get_confitem()->abone_transparent; }
void CONFIG::set_abone_transparent( const bool set ){ get_confitem()->abone_transparent = set; }

bool CONFIG::get_abone_chain(){ return get_confitem()->abone_chain; }
void CONFIG::set_abone_chain( const bool set ){ get_confitem()->abone_chain = set; }

// NG正規表現によるあぼーん時に大小と全半角文字の違いを無視
bool CONFIG::get_abone_icase(){ return get_confitem()->abone_icase; }
void CONFIG::set_abone_icase( const bool set ){ get_confitem()->abone_icase = set; }

bool CONFIG::get_abone_wchar(){ return get_confitem()->abone_wchar; }
void CONFIG::set_abone_wchar( const bool set ){ get_confitem()->abone_wchar = set; }


bool CONFIG::get_expand_sidebar(){ return get_confitem()->expand_sidebar; }

bool CONFIG::get_expand_rpane(){ return get_confitem()->expand_rpane; }

// ペーンの境界をクリックしてサイドバーを開け閉めする
bool CONFIG::get_open_sidebar_by_click(){ return get_confitem()->open_sidebar_by_click; }

// 次スレ検索の類似度のしきい値
int CONFIG::get_threshold_next(){ return get_confitem()->threshold_next; }

// 次スレを開いたときにお気に入りのアドレスと名前を自動更新
int CONFIG::get_replace_favorite_next(){ return get_confitem()->replace_favorite_next; }
void CONFIG::set_replace_favorite_next( const int mode ){ get_confitem()->replace_favorite_next = mode; }

// お気に入りの自動更新をするかダイアログを出す
bool CONFIG::show_diag_replace_favorite(){ return get_confitem()->show_diag_replace_favorite; }
void CONFIG::set_show_diag_replace_favorite( const bool show ){ get_confitem()->show_diag_replace_favorite = show; }

// スレをお気に入りに追加したときにしおりをセットする
bool CONFIG::get_bookmark_drop(){ return get_confitem()->bookmark_drop; }

// お気に入りの更新チェック時に板の更新もチェックする
bool CONFIG::get_check_update_board(){ return get_confitem()->check_update_board; }

// 起動時にお気に入りを自動でチェックする
bool CONFIG::get_check_update_boot(){ return get_confitem()->check_update_boot; }

// お気に入り登録時に重複項目を登録するか ( 0: 登録する 1: ダイアログ表示 2: 登録しない )
int CONFIG::get_check_favorite_dup(){ return get_confitem()->check_favorite_dup; }
void CONFIG::set_check_favorite_dup( const int check ){ get_confitem()->check_favorite_dup = check; }

// お気に入り登録時に挿入先ダイアログを表示する ( 0 : 表示する 1: 表示せず先頭に追加 2: 表示せず最後に追加 )
int CONFIG::get_show_favorite_select_diag(){ return get_confitem()->show_favorite_select_diag; }

// Ctrl+qでウィンドウを閉じない ( 2.8.6以前と互換性を保つため残す )
bool CONFIG::get_disable_close(){ return get_confitem()->disable_close; }
void CONFIG::set_disable_close( const bool disable ){ get_confitem()->disable_close = disable; }

// メニューバーを非表示にした時にダイアログを表示
bool CONFIG::get_show_hide_menubar_diag(){ return get_confitem()->show_hide_menubar_diag; }
void CONFIG::set_show_hide_menubar_diag( const bool set ){ get_confitem()->show_hide_menubar_diag = set; }

// 状態変更時にメインステータスバーの色を変える
bool CONFIG::get_change_stastatus_color(){ return get_confitem()->change_stastatus_color; }

// まちBBSの取得に offlaw.cgi を使用する
bool CONFIG::get_use_machi_offlaw(){ return get_confitem()->use_machi_offlaw; }
void CONFIG::set_use_machi_offlaw( const bool set ){ get_confitem()->use_machi_offlaw = set; }

// 書き込み履歴のあるスレを削除する時にダイアログを表示
bool CONFIG::get_show_del_written_thread_diag(){ return get_confitem()->show_del_written_thread_diag; }
void CONFIG::set_del_written_thread_diag( const bool set ){ get_confitem()->show_del_written_thread_diag = set; }

// スレを削除する時に画像キャッシュも削除する ( 0: ダイアログ表示 1: 削除 2: 削除しない )
int CONFIG::get_delete_img_in_thread(){ return get_confitem()->delete_img_in_thread; }
void CONFIG::set_delete_img_in_thread( const int set ){ get_confitem()->delete_img_in_thread = set; }

//最大表示可能レス数
int CONFIG::get_max_resnumber(){ return get_confitem()->max_resnumber; }
void CONFIG::set_max_resnumber( const int set ){ get_confitem()->max_resnumber = set; }


// FIFOの作成などにエラーがあったらダイアログを表示する
bool CONFIG::get_show_diag_fifo_error(){ return get_confitem()->show_diag_fifo_error; }
void CONFIG::set_show_diag_fifo_error( const bool set ){ get_confitem()->show_diag_fifo_error = set; }

// 指定した分ごとにセッションを自動保存 (0: 保存しない)
int CONFIG::get_save_session(){ return get_confitem()->save_session; }

#ifdef HAVE_MIGEMO_H
const std::string& CONFIG::get_migemodict_path() { return get_confitem()->migemodict_path; }
#endif
