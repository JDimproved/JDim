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
void CONFIG::set_restore_board( bool restore ){ get_confitem()->restore_board = restore; }
const bool CONFIG::get_restore_article(){ return get_confitem()->restore_article; }
void CONFIG::set_restore_article( bool restore ){ get_confitem()->restore_article = restore; }
const bool CONFIG::get_restore_image(){ return get_confitem()->restore_image; }
void CONFIG::set_restore_image( bool restore ){ get_confitem()->restore_image = restore; }


// 色
const std::string& CONFIG::get_color( int id )
{
    return get_confitem()->str_color[ id ];
}

void CONFIG::set_color( int id, const std::string& color )
{
    get_confitem()->str_color[ id ] = color;
}

const bool CONFIG::get_use_tree_gtkrc(){ return get_confitem()->use_tree_gtkrc; }

const int CONFIG::get_tree_ypad(){ return get_confitem()->tree_ypad; }


// フォント
const std::string& CONFIG::get_fontname( int id )
{
    return get_confitem()->fontname[ id ];
}

void CONFIG::set_fontname( int id, const std::string& fontname )
{
    get_confitem()->fontname[ id ] = fontname;
}

const std::string&  CONFIG::get_ref_prefix(){ return get_confitem()->ref_prefix; }

const std::string& CONFIG::get_url_jdhp() { return get_confitem()->url_jdhp; }

const std::string& CONFIG::get_url_login2ch() { return get_confitem()->url_login2ch; }
const std::string& CONFIG::get_url_bbsmenu() { return get_confitem()->url_bbsmenu; }

const bool CONFIG::use_link_as_board(){ return get_confitem()->use_link_as_board; }

const std::string& CONFIG::get_agent_for2ch() { return get_confitem()->agent_for2ch; }

const bool CONFIG::get_use_proxy_for2ch() { return get_confitem()->use_proxy_for2ch; }
const std::string& CONFIG::get_proxy_for2ch() { return get_confitem()->proxy_for2ch; }
const int CONFIG::get_proxy_port_for2ch() { return get_confitem()->proxy_port_for2ch; }

void CONFIG::set_use_proxy_for2ch( bool set ){ get_confitem()->use_proxy_for2ch = set; }
void CONFIG::set_proxy_for2ch( const std::string& proxy ){ get_confitem()->proxy_for2ch = proxy; }
void CONFIG::set_proxy_port_for2ch( int port ){ get_confitem()->proxy_port_for2ch = port; }

const bool CONFIG::get_use_proxy_for2ch_w() { return get_confitem()->use_proxy_for2ch_w; }
const std::string& CONFIG::get_proxy_for2ch_w() { return get_confitem()->proxy_for2ch_w; }
const int CONFIG::get_proxy_port_for2ch_w() { return get_confitem()->proxy_port_for2ch_w; }

void CONFIG::set_use_proxy_for2ch_w( bool set ){ get_confitem()->use_proxy_for2ch_w = set; }
void CONFIG::set_proxy_for2ch_w( const std::string& proxy ){ get_confitem()->proxy_for2ch_w = proxy; }
void CONFIG::set_proxy_port_for2ch_w( int port ){ get_confitem()->proxy_port_for2ch_w = port; }

const std::string& CONFIG::get_agent_for_data() { return get_confitem()->agent_for_data; }

const bool CONFIG::get_use_proxy_for_data() { return get_confitem()->use_proxy_for_data; }
const std::string& CONFIG::get_proxy_for_data() { return get_confitem()->proxy_for_data; }
const int CONFIG::get_proxy_port_for_data() { return get_confitem()->proxy_port_for_data; }

const std::string& CONFIG::get_x_2ch_ua() { return get_confitem()->x_2ch_ua; }

void CONFIG::set_use_proxy_for_data( bool set ){ get_confitem()->use_proxy_for_data = set; }
void CONFIG::set_proxy_for_data( const std::string& proxy ){ get_confitem()->proxy_for_data = proxy; }
void CONFIG::set_proxy_port_for_data( int port ){ get_confitem()->proxy_port_for_data = port; }

const int CONFIG::get_loader_bufsize(){ return get_confitem()->loader_bufsize; }
const int CONFIG::get_loader_timeout(){ return get_confitem()->loader_timeout; }
const int CONFIG::get_loader_timeout_post(){ return get_confitem()->loader_timeout_post; }
const int CONFIG::get_loader_timeout_img(){ return get_confitem()->loader_timeout_img; }
const bool CONFIG::get_use_ipv6(){ return get_confitem()->use_ipv6; }

const std::string& CONFIG::get_command_openurl() { return get_confitem()->command_openurl; }
void CONFIG::set_command_openurl( const std::string& command ){ get_confitem()->command_openurl = command; }

const int CONFIG::get_browsercombo_id(){ return get_confitem()->browsercombo_id; }
void CONFIG::set_browsercombo_id( int id ){ get_confitem()->browsercombo_id = id; }

const bool CONFIG::get_refpopup_by_mo(){ return get_confitem()->refpopup_by_mo; }
const bool CONFIG::get_namepopup_by_mo(){ return get_confitem()->namepopup_by_mo; }
const bool CONFIG::get_idpopup_by_mo(){ return get_confitem()->idpopup_by_mo; }

const int CONFIG::get_imgpopup_width(){ return get_confitem()->imgpopup_width; }
const int CONFIG::get_imgpopup_height(){ return get_confitem()->imgpopup_height; }

const bool CONFIG::get_use_image_view(){ return get_confitem()->use_image_view; }
void CONFIG::set_use_image_view( bool image_view ){ get_confitem()->use_image_view = image_view; }

const bool CONFIG::get_use_inline_image(){ return get_confitem()->use_inline_image; }
void CONFIG::set_use_inline_image( bool inline_img ){ get_confitem()->use_inline_image = inline_img; }

const bool CONFIG::get_use_mosaic(){ return get_confitem()->use_mosaic; }
void CONFIG::set_use_mosaic( bool mosaic ) { get_confitem()->use_mosaic = mosaic; }
const bool CONFIG::get_zoom_to_fit(){ return get_confitem()->zoom_to_fit; }
void CONFIG::set_zoom_to_fit( bool fit ){ get_confitem()->zoom_to_fit = fit; }
const int CONFIG::get_del_img_day(){ return get_confitem()->del_img_day; }
void CONFIG::set_del_img_day( int day ){ get_confitem()->del_img_day = day; }
const int CONFIG::get_max_img_size(){ return get_confitem()->max_img_size; }

const bool CONFIG::get_show_oldarticle(){ return get_confitem()->show_oldarticle; }
void CONFIG::set_show_oldarticle( bool showarticle ){ get_confitem()->show_oldarticle = showarticle; }

const int CONFIG::get_newthread_hour(){ return get_confitem()->newthread_hour; }

const int CONFIG::get_tree_scroll_size(){ return get_confitem()->tree_scroll_size; }
const int CONFIG::get_scroll_size(){ return get_confitem()->scroll_size; }
const int CONFIG::get_key_scroll_size(){ return get_confitem()->key_scroll_size; }

const bool CONFIG::get_open_one_category(){ return get_confitem()->open_one_category; }
const bool CONFIG::get_always_write_ok() { return get_confitem()->always_write_ok; }
void CONFIG::set_always_write_ok( bool write_ok ){ get_confitem()->always_write_ok = write_ok; }
const bool CONFIG::get_save_postlog(){ return get_confitem()->save_postlog; }
void CONFIG::set_save_postlog( bool save ){ get_confitem()->save_postlog = save; }
const bool CONFIG::get_hide_writing_dialog(){ return get_confitem()->hide_writing_dialog; }

const int CONFIG::get_margin_popup(){ return get_confitem()->margin_popup; }
void CONFIG::set_margin_popup( int margin ){ get_confitem()->margin_popup = margin; }

const int CONFIG::get_margin_imgpopup(){ return get_confitem()->margin_imgpopup; }

const int CONFIG::get_mouse_radius(){ return get_confitem()->mouse_radius; }
const int CONFIG::get_history_size(){ return get_confitem()->history_size; }
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
void CONFIG::set_instruct_tglart( bool tgl ){ get_confitem()->instruct_tglart = tgl; }

const bool CONFIG::get_instruct_tglimg(){

    if( get_confitem()->instruct_tglimg_end ) return false;

    get_confitem()->instruct_tglimg_end = true; // 一度表示したら表示しない
    return get_confitem()->instruct_tglimg;
}

void CONFIG::set_instruct_tglimg( bool tgl ){ get_confitem()->instruct_tglimg = tgl; }


const double CONFIG::get_adjust_underline_pos(){ return get_confitem()->adjust_underline_pos; }
void CONFIG::set_adjust_underline_pos( double pos ){ get_confitem()->adjust_underline_pos = pos; }

const double CONFIG::get_adjust_line_space(){ return get_confitem()->adjust_line_space; }
void CONFIG::set_adjust_line_space( double space ){ get_confitem()->adjust_line_space = space; }

const bool CONFIG::get_draw_underline(){ return get_confitem()->draw_underline; }

const bool CONFIG::get_strict_char_width(){ return get_confitem()->strict_char_width; }
void CONFIG::set_strict_char_width( bool strictwidth ){ get_confitem()->strict_char_width = strictwidth; }

const bool CONFIG::get_loose_url(){ return get_confitem()->loose_url; }

const bool CONFIG::get_hide_usrcmd(){ return get_confitem()->hide_usrcmd; }
const int CONFIG::get_max_show_usrcmd(){ return get_confitem()->max_show_usrcmd; }

const bool CONFIG::get_reload_allthreads(){ return get_confitem()->reload_allthreads; }

const int CONFIG::get_tab_min_str(){ return get_confitem()->tab_min_str; }

const bool CONFIG::get_show_tab_icon(){ return get_confitem()->show_tab_icon; }

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


std::list< std::string >& CONFIG::get_list_abone_name(){ return get_confitem()->list_abone_name; }
std::list< std::string >& CONFIG::get_list_abone_word(){ return get_confitem()->list_abone_word; }
std::list< std::string >& CONFIG::get_list_abone_regex(){ return get_confitem()->list_abone_regex; }

void CONFIG::set_list_abone_name( std::list< std::string >& name )
{
    // 前後の空白と空白行を除く
    get_confitem()->list_abone_name = MISC::remove_space_from_list( name );
    get_confitem()->list_abone_name = MISC::remove_nullline_from_list( get_confitem()->list_abone_name );
}

void CONFIG::set_list_abone_word( std::list< std::string >& word )
{
    // 前後の空白と空白行を除く
    get_confitem()->list_abone_word = MISC::remove_space_from_list( word );
    get_confitem()->list_abone_word = MISC::remove_nullline_from_list( get_confitem()->list_abone_word );
}


void CONFIG::set_list_abone_regex( std::list< std::string >& regex )
{
    // 前後の空白と空白行を除く
    get_confitem()->list_abone_regex = MISC::remove_space_from_list( regex );
    get_confitem()->list_abone_regex = MISC::remove_nullline_from_list( get_confitem()->list_abone_regex );
}

const bool CONFIG::get_abone_transparent(){ return get_confitem()->abone_transparent; }
void CONFIG::set_abone_transparent( bool set ){ get_confitem()->abone_transparent = set; }
const bool CONFIG::get_abone_chain(){ return get_confitem()->abone_chain; }
void CONFIG::set_abone_chain( bool set ){ get_confitem()->abone_chain = set; }

const bool CONFIG::get_expand_sidebar(){ return get_confitem()->expand_sidebar; }

#ifdef HAVE_MIGEMO_H
const std::string& CONFIG::get_migemodict_path() { return get_confitem()->migemodict_path; }
#endif
