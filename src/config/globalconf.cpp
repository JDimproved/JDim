// ライセンス: GPL2

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

//#define _DEBUG
#include "jddebug.h"

#include "globalconf.h"
#include "configitems.h"

#include "jdlib/miscutil.h"


CONFIG::ConfigItems* instance_confitem = NULL;
CONFIG::ConfigItems* instance_confitem_bkup = NULL;


CONFIG::ConfigItems* CONFIG::get_confitem()
{
    if( ! instance_confitem ) instance_confitem = new CONFIG::ConfigItems();
    return instance_confitem;
}


void CONFIG::delete_confitem()
{
    if( instance_confitem ) delete instance_confitem;
    instance_confitem = NULL;

    if( instance_confitem_bkup ) delete instance_confitem_bkup;
    instance_confitem_bkup = NULL;
}


const bool CONFIG::load_conf()
{
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



const bool CONFIG::get_restore_board(){ return get_confitem()->restore_board; }
void CONFIG::set_restore_board( const bool restore ){ get_confitem()->restore_board = restore; }
const bool CONFIG::get_restore_article(){ return get_confitem()->restore_article; }
void CONFIG::set_restore_article( const bool restore ){ get_confitem()->restore_article = restore; }
const bool CONFIG::get_restore_image(){ return get_confitem()->restore_image; }
void CONFIG::set_restore_image( const bool restore ){ get_confitem()->restore_image = restore; }

const bool CONFIG::get_manage_winpos(){ return get_confitem()->manage_winpos; }
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

const bool CONFIG::get_use_tree_gtkrc(){ return get_confitem()->use_tree_gtkrc; }
void CONFIG::set_use_tree_gtkrc( const bool use ){ get_confitem()->use_tree_gtkrc = use; }

const bool CONFIG::get_use_select_gtkrc(){ return get_confitem()->use_select_gtkrc; }
void CONFIG::set_use_select_gtkrc( const bool use ){ get_confitem()->use_select_gtkrc = use; }

const int CONFIG::get_tree_ypad(){ return get_confitem()->tree_ypad; }

// カテゴリを開いたときにツリービューをスクロールする
const bool CONFIG::get_scroll_tree(){ return get_confitem()->scroll_tree; }

const int CONFIG::get_view_margin(){ return get_confitem()->view_margin; }

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


const std::string CONFIG::get_ref_prefix(){ return get_confitem()->ref_prefix + get_confitem()->ref_prefix_space_str; }
const int CONFIG::ref_prefix_space(){ return get_confitem()->ref_prefix_space; }

const std::string& CONFIG::get_url_jdhp() { return get_confitem()->url_jdhp; }

const std::string& CONFIG::get_url_login2ch() { return get_confitem()->url_login2ch; }
const std::string& CONFIG::get_url_loginp2() { return get_confitem()->url_loginp2; }

const std::string& CONFIG::get_url_bbsmenu() { return get_confitem()->url_bbsmenu; }

const bool CONFIG::use_link_as_board(){ return get_confitem()->use_link_as_board; }

const bool CONFIG::get_show_movediag(){ return get_confitem()->show_movediag; }
void CONFIG::set_show_movediag( const bool show ){ get_confitem()->show_movediag = show; }

const std::string& CONFIG::get_menu_search_title(){ return get_confitem()->menu_search_title; }
const std::string& CONFIG::get_url_search_title(){ return get_confitem()->url_search_title; }
const std::string& CONFIG::get_regex_search_title(){ return get_confitem()->regex_search_title; }

const std::string& CONFIG::get_menu_search_web(){ return get_confitem()->menu_search_web; }
const std::string& CONFIG::get_url_search_web(){ return get_confitem()->url_search_web; }

const std::string& CONFIG::get_url_writep2(){ return get_confitem()->url_writep2; }
const std::string& CONFIG::get_url_resp2(){ return get_confitem()->url_resp2; }

const std::string& CONFIG::get_agent_for2ch() { return get_confitem()->agent_for2ch; }

const bool CONFIG::get_use_proxy_for2ch() { return get_confitem()->use_proxy_for2ch; }
const std::string& CONFIG::get_proxy_for2ch() { return get_confitem()->proxy_for2ch; }
const int CONFIG::get_proxy_port_for2ch() { return get_confitem()->proxy_port_for2ch; }
const std::string& CONFIG::get_proxy_basicauth_for2ch() { return get_confitem()->proxy_basicauth_for2ch; }

void CONFIG::set_use_proxy_for2ch( bool set ){ get_confitem()->use_proxy_for2ch = set; }
void CONFIG::set_proxy_for2ch( const std::string& proxy ){ get_confitem()->set_proxy_for2ch( proxy ); }
void CONFIG::set_proxy_port_for2ch( int port ){ get_confitem()->proxy_port_for2ch = port; }

const bool CONFIG::get_use_proxy_for2ch_w() { return get_confitem()->use_proxy_for2ch_w; }
const std::string& CONFIG::get_proxy_for2ch_w() { return get_confitem()->proxy_for2ch_w; }
const int CONFIG::get_proxy_port_for2ch_w() { return get_confitem()->proxy_port_for2ch_w; }
const std::string& CONFIG::get_proxy_basicauth_for2ch_w() { return get_confitem()->proxy_basicauth_for2ch_w; }

void CONFIG::set_use_proxy_for2ch_w( bool set ){ get_confitem()->use_proxy_for2ch_w = set; }
void CONFIG::set_proxy_for2ch_w( const std::string& proxy ){ get_confitem()->set_proxy_for2ch_w( proxy ); }
void CONFIG::set_proxy_port_for2ch_w( int port ){ get_confitem()->proxy_port_for2ch_w = port; }

const std::string& CONFIG::get_agent_for_data() { return get_confitem()->agent_for_data; }

const bool CONFIG::get_use_proxy_for_data() { return get_confitem()->use_proxy_for_data; }
const std::string& CONFIG::get_proxy_for_data() { return get_confitem()->proxy_for_data; }
const int CONFIG::get_proxy_port_for_data() { return get_confitem()->proxy_port_for_data; }
const std::string& CONFIG::get_proxy_basicauth_for_data() { return get_confitem()->proxy_basicauth_for_data; }

const std::string& CONFIG::get_x_2ch_ua() { return get_confitem()->x_2ch_ua; }

void CONFIG::set_use_proxy_for_data( bool set ){ get_confitem()->use_proxy_for_data = set; }
void CONFIG::set_proxy_for_data( const std::string& proxy ){ get_confitem()->set_proxy_for_data( proxy ); }
void CONFIG::set_proxy_port_for_data( int port ){ get_confitem()->proxy_port_for_data = port; }

const int CONFIG::get_loader_bufsize(){ return get_confitem()->loader_bufsize; }
const int CONFIG::get_loader_bufsize_board(){ return get_confitem()->loader_bufsize_board; }

const int CONFIG::get_loader_timeout(){ return get_confitem()->loader_timeout; }
const int CONFIG::get_loader_timeout_post(){ return get_confitem()->loader_timeout_post; }
const int CONFIG::get_loader_timeout_data(){ return get_confitem()->loader_timeout_img; } // 旧 get_loader_timeout_img() 関数
const int CONFIG::get_loader_timeout_checkupdate(){ return get_confitem()->loader_timeout_checkupdate; }

const bool CONFIG::get_use_ipv6(){ return get_confitem()->use_ipv6; }
void CONFIG::set_use_ipv6( const bool set ){ get_confitem()->use_ipv6 = set; }

const std::string& CONFIG::get_command_openurl() { return get_confitem()->command_openurl; }
void CONFIG::set_command_openurl( const std::string& command ){ get_confitem()->command_openurl = command; }

const int CONFIG::get_browsercombo_id(){ return get_confitem()->browsercombo_id; }
void CONFIG::set_browsercombo_id( const int id ){ get_confitem()->browsercombo_id = id; }

const bool CONFIG::get_refpopup_by_mo(){ return get_confitem()->refpopup_by_mo; }
const bool CONFIG::get_namepopup_by_mo(){ return get_confitem()->namepopup_by_mo; }
const bool CONFIG::get_idpopup_by_mo(){ return get_confitem()->idpopup_by_mo; }

const int CONFIG::get_imgemb_interp(){ return get_confitem()->imgemb_interp; }
const int CONFIG::get_imgmain_interp(){ return get_confitem()->imgmain_interp; }
const int CONFIG::get_imgpopup_interp(){ return get_confitem()->imgpopup_interp; }

const int CONFIG::get_imgpopup_width(){ return get_confitem()->imgpopup_width; }
const int CONFIG::get_imgpopup_height(){ return get_confitem()->imgpopup_height; }

const bool CONFIG::get_use_image_popup(){ return get_confitem()->use_image_popup; }
void CONFIG::set_use_image_popup( const bool use ){ get_confitem()->use_image_popup = use; }

const bool CONFIG::get_use_image_view(){ return get_confitem()->use_image_view; }
void CONFIG::set_use_image_view( const bool image_view ){ get_confitem()->use_image_view = image_view; }

const bool CONFIG::get_use_inline_image(){ return get_confitem()->use_inline_image; }
void CONFIG::set_use_inline_image( const bool inline_img ){ get_confitem()->use_inline_image = inline_img; }

const bool CONFIG::get_show_ssspicon(){ return get_confitem()->show_ssspicon; }
void CONFIG::set_show_sssp_icon( const bool show ){ get_confitem()->show_ssspicon = show; }

const bool CONFIG::get_use_mosaic(){ return get_confitem()->use_mosaic; }
void CONFIG::set_use_mosaic( const bool mosaic ) { get_confitem()->use_mosaic = mosaic; }
const int CONFIG::get_mosaic_size(){ return get_confitem()->mosaic_size; }
const bool CONFIG::get_zoom_to_fit(){ return get_confitem()->zoom_to_fit; }
void CONFIG::set_zoom_to_fit( const bool fit ){ get_confitem()->zoom_to_fit = fit; }
const int CONFIG::get_del_img_day(){ return get_confitem()->del_img_day; }
void CONFIG::set_del_img_day( const int day ){ get_confitem()->del_img_day = day; }
const int CONFIG::get_del_imgabone_day(){ return get_confitem()->del_imgabone_day; }
void CONFIG::set_del_imgabone_day( const int day ){ get_confitem()->del_imgabone_day = day; }
const int CONFIG::get_max_img_size(){ return get_confitem()->max_img_size; }
const int CONFIG::get_max_img_pixel(){ return get_confitem()->max_img_pixel; }
const int CONFIG::get_imgcache_size(){ return get_confitem()->imgcache_size; }

const bool CONFIG::get_show_oldarticle(){ return get_confitem()->show_oldarticle; }
void CONFIG::set_show_oldarticle( const bool showarticle ){ get_confitem()->show_oldarticle = showarticle; }

const int CONFIG::get_newthread_hour(){ return get_confitem()->newthread_hour; }

const bool CONFIG::get_inc_search_board(){ return get_confitem()->inc_search_board; }

const bool CONFIG::get_show_deldiag(){ return get_confitem()->show_deldiag; }
void CONFIG::set_show_deldiag( const bool show ){ get_confitem()->show_deldiag = show; }

// スレ一覧をロードする前にキャッシュにある一覧を表示
const bool CONFIG::get_show_cached_board(){ return get_confitem()->show_cached_board; }

// スレ一覧でお知らせスレ(924)のアイコンを表示する
const bool CONFIG::get_show_924(){ return get_confitem()->show_924; }

const int CONFIG::get_tree_scroll_size(){ return get_confitem()->tree_scroll_size; }
const int CONFIG::get_scroll_size(){ return get_confitem()->scroll_size; }
const int CONFIG::get_key_scroll_size(){ return get_confitem()->key_scroll_size; }
const int CONFIG::get_key_fastscroll_size(){ return get_confitem()->key_fastscroll_size; }

const bool CONFIG::get_jump_after_reload(){ return get_confitem()->jump_after_reload; }
void CONFIG::set_jump_after_reload( const bool set ){ get_confitem()->jump_after_reload = set; }
const bool CONFIG::get_jump_new_after_reload(){ return get_confitem()->jump_new_after_reload; }
void CONFIG::set_jump_new_after_reload( const bool set ){ get_confitem()->jump_new_after_reload = set; }

const int CONFIG::get_live_mode(){ return get_confitem()->live_mode; }
void CONFIG::set_live_mode( const int mode ) { get_confitem()->live_mode = mode; }
const int CONFIG::get_live_speed(){ return get_confitem()->live_speed; }
void CONFIG::set_live_speed( const int speed ){ get_confitem()->live_speed = speed; }
const int CONFIG::get_live_threshold(){ return get_confitem()->live_threshold; }
void CONFIG::set_live_threshode( const int th ){ get_confitem()->live_threshold = th; }

const bool CONFIG::get_open_one_category(){ return get_confitem()->open_one_category; }
const bool CONFIG::get_open_one_favorite(){ return get_confitem()->open_one_favorite; }
const bool CONFIG::get_always_write_ok() { return get_confitem()->always_write_ok; }
void CONFIG::set_always_write_ok( const bool write_ok ){ get_confitem()->always_write_ok = write_ok; }

const bool CONFIG::get_save_post_log(){ return get_confitem()->save_postlog; }
void CONFIG::set_save_post_log( const bool save ){ get_confitem()->save_postlog = save; }
const size_t CONFIG::get_maxsize_post_log(){ return get_confitem()->maxsize_postlog; }

// 書き込み履歴を保存
const bool CONFIG::get_save_post_history(){ return get_confitem()->save_posthist; }
void CONFIG::set_save_post_history( const bool save ){ get_confitem()->save_posthist = save; }

const bool CONFIG::get_hide_writing_dialog(){ return get_confitem()->hide_writing_dialog; }

const bool CONFIG::get_fold_message(){ return get_confitem()->fold_message; }
void CONFIG::set_fold_message( const bool fold ){ get_confitem()->fold_message = fold; }

const bool CONFIG::get_keep_im_status(){ return get_confitem()->keep_im_status; }

const int CONFIG::get_margin_popup(){ return get_confitem()->margin_popup; }
void CONFIG::set_margin_popup( const int margin ){ get_confitem()->margin_popup = margin; }

const int CONFIG::get_margin_imgpopup(){ return get_confitem()->margin_imgpopup; }

const int CONFIG::get_mouse_radius(){ return get_confitem()->mouse_radius; }
const int CONFIG::get_history_size(){ return get_confitem()->history_size; }
const int CONFIG::get_historyview_size(){ return get_confitem()->historyview_size; }
const int CONFIG::get_aahistory_size(){ return get_confitem()->aahistory_size; }

// 0以上なら多重ポップアップの説明を表示する
// 呼び出される度に--する
const int CONFIG::get_instruct_popup(){
    if( get_confitem()->instruct_popup ) return get_confitem()->instruct_popup--;
    return 0;
}


const bool CONFIG::get_instruct_tglart(){

    if( get_confitem()->instruct_tglart_end ) return false;

    get_confitem()->instruct_tglart_end = true; // 一度表示したら表示しない
    return get_confitem()->instruct_tglart;
}
void CONFIG::set_instruct_tglart( const bool tgl ){ get_confitem()->instruct_tglart = tgl; }

const bool CONFIG::get_instruct_tglimg(){

    if( get_confitem()->instruct_tglimg_end ) return false;

    get_confitem()->instruct_tglimg_end = true; // 一度表示したら表示しない
    return get_confitem()->instruct_tglimg;
}

void CONFIG::set_instruct_tglimg( bool tgl ){ get_confitem()->instruct_tglimg = tgl; }


const double CONFIG::get_adjust_underline_pos(){ return get_confitem()->adjust_underline_pos; }
void CONFIG::set_adjust_underline_pos( const double pos ){ get_confitem()->adjust_underline_pos = pos; }

const double CONFIG::get_adjust_line_space(){ return get_confitem()->adjust_line_space; }
void CONFIG::set_adjust_line_space( const double space ){ get_confitem()->adjust_line_space = space; }

const bool CONFIG::get_draw_underline(){ return get_confitem()->draw_underline; }

const bool CONFIG::get_strict_char_width(){ return get_confitem()->strict_char_width; }
void CONFIG::set_strict_char_width( const bool strictwidth ){ get_confitem()->strict_char_width = strictwidth; }

const bool CONFIG::get_check_id(){ return get_confitem()->check_id; }

const int CONFIG::get_num_reference_high(){ return get_confitem()->num_reference_high; }
const int CONFIG::get_num_reference_low(){ return get_confitem()->num_reference_low; }
const int CONFIG::get_num_id_high(){ return get_confitem()->num_id_high; }
const int CONFIG::get_num_id_low(){ return get_confitem()->num_id_low; }

const bool CONFIG::get_loose_url(){ return get_confitem()->loose_url; }

const bool CONFIG::get_hide_usrcmd(){ return get_confitem()->hide_usrcmd; }
void CONFIG::set_hide_usrcmd( const bool hide ){ get_confitem()->hide_usrcmd = hide; }

const bool CONFIG::get_reload_allthreads(){ return get_confitem()->reload_allthreads; }

const int CONFIG::get_tab_min_str(){ return get_confitem()->tab_min_str; }

const bool CONFIG::get_show_tab_icon(){ return get_confitem()->show_tab_icon; }
const bool CONFIG::get_show_post_mark(){ return get_confitem()->show_post_mark; }
void CONFIG::set_show_post_mark( const bool show ){ get_confitem()->show_post_mark = show; }

const bool CONFIG::get_flat_button(){ return get_confitem()->flat_button; }
void CONFIG::set_flat_button( const bool set ){ get_confitem()->flat_button = set; }

// ツールバーの背景描画
const bool CONFIG::get_draw_toolbarback(){ return get_confitem()->draw_toolbarback; }
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


const int CONFIG::get_abone_number_thread(){ return get_confitem()->abone_number_thread; } 
void CONFIG::set_abone_number_thread( const int number ){ get_confitem()->abone_number_thread = number; }

const int CONFIG::get_abone_hour_thread(){ return get_confitem()->abone_hour_thread; } 
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

const bool CONFIG::get_abone_transparent(){ return get_confitem()->abone_transparent; }
void CONFIG::set_abone_transparent( const bool set ){ get_confitem()->abone_transparent = set; }
const bool CONFIG::get_abone_chain(){ return get_confitem()->abone_chain; }
void CONFIG::set_abone_chain( const bool set ){ get_confitem()->abone_chain = set; }

const bool CONFIG::get_expand_sidebar(){ return get_confitem()->expand_sidebar; }

const bool CONFIG::get_expand_rpane(){ return get_confitem()->expand_rpane; }


// 次スレ検索の類似度のしきい値
const int CONFIG::get_threshold_next(){ return get_confitem()->threshold_next; }

// 次スレを開いたときにお気に入りのアドレスと名前を自動更新
const int CONFIG::get_replace_favorite_next(){ return get_confitem()->replace_favorite_next; }
void CONFIG::set_replace_favorite_next( const int mode ){ get_confitem()->replace_favorite_next = mode; }

// お気に入りの自動更新をするかダイアログを出す
const bool CONFIG::show_diag_replace_favorite(){ return get_confitem()->show_diag_replace_favorite; }
void CONFIG::set_show_diag_replace_favorite( const bool show ){ get_confitem()->show_diag_replace_favorite = show; }

// スレをお気に入りに追加したときにしおりをセットする
const bool CONFIG::get_bookmark_drop(){ return get_confitem()->bookmark_drop; }

// お気に入りの更新チェック時に板の更新もチェックする
const bool CONFIG::get_check_update_board(){ return get_confitem()->check_update_board; }

// 起動時にお気に入りを自動でチェックする
const bool CONFIG::get_check_update_boot(){ return get_confitem()->check_update_boot; }

// Ctrl+qでウィンドウを閉じない
const bool CONFIG::get_disable_close(){ return get_confitem()->disable_close; }

// まちBBSの取得に offlaw.cgi を使用する
const bool CONFIG::get_use_machi_offlaw(){ return get_confitem()->use_machi_offlaw; }
void CONFIG::set_use_machi_offlaw( const bool set ){ get_confitem()->use_machi_offlaw = set; }

// 書き込み履歴のあるスレを削除する時にダイアログを表示
const bool CONFIG::get_show_del_written_thread_diag(){ return get_confitem()->show_del_written_thread_diag; }
void CONFIG::set_del_written_thread_diag( const bool set ){ get_confitem()->show_del_written_thread_diag = set; }

// FIFOの作成などにエラーがあったらダイアログを表示する
const bool CONFIG::get_show_diag_fifo_error(){ return get_confitem()->show_diag_fifo_error; }
void CONFIG::set_show_diag_fifo_error( const bool set ){ get_confitem()->show_diag_fifo_error = set; }

#ifdef HAVE_MIGEMO_H
const std::string& CONFIG::get_migemodict_path() { return get_confitem()->migemodict_path; }
#endif
