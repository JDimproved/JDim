// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "interface.h"
#include "root.h"
#include "boardbase.h"
#include "articlebase.h"

#include "jdlib/miscutil.h"

#include "global.h"

// インスタンスは Core でひとつだけ作って、Coreのデストラクタでdeleteする
DBTREE::Root *instance_dbtree_root = nullptr;


void DBTREE::create_root()
{
    if( ! instance_dbtree_root ) instance_dbtree_root = new DBTREE::Root();
}


void DBTREE::delete_root()
{
    if( instance_dbtree_root ){
        instance_dbtree_root->terminate_load();
        delete instance_dbtree_root;
    }
}


//////////////////////////////////////
//
// ツリーの構成要素のポインタ取得
//
// root から葉っぱに向かって順に取得していく
//

DBTREE::Root* DBTREE::get_root()
{
    assert( instance_dbtree_root != nullptr );
    return instance_dbtree_root;
}

DBTREE::BoardBase* DBTREE::get_board( const std::string& url )
{
    DBTREE::BoardBase* board = DBTREE::get_root()->get_board( url );
    assert( board != nullptr );
    return board;
}

DBTREE::ArticleBase* DBTREE::get_article( const std::string& url )
{
    DBTREE::ArticleBase* article = DBTREE::get_board( url )->get_article_fromURL( url );
    assert( article != nullptr );
    return article;
}

//////////////////////////////////////

std::string DBTREE::url_subject( const std::string& url )
{
    return DBTREE::get_board( url )->url_subject();
}


std::string DBTREE::url_root( const std::string& url )
{
    return DBTREE::get_board( url )->url_root();
}


std::string DBTREE::url_boardbase( const std::string& url )
{
    return DBTREE::get_board( url )->url_boardbase();
}


std::string DBTREE::url_datbase( const std::string& url )
{
    return DBTREE::get_board( url )->url_datbase();
}


// urlをdat型のurlに変換
std::string DBTREE::url_dat( const std::string& url, int& num_from, int& num_to, std::string& num_str )
{
    return DBTREE::get_board( url )->url_dat( url, num_from, num_to, num_str );
}


// urlをdat型のurlに変換(簡易版)
std::string DBTREE::url_dat( const std::string& url )
{
    int num_from, num_to;
    std::string num_str;
    return url_dat( url, num_from, num_to, num_str );
}


// url を read.cgi型のurlに変換
std::string DBTREE::url_readcgi( const std::string& url, int num_from, int num_to )
{
    return DBTREE::get_board( url )->url_readcgi( url, num_from, num_to );
}


std::string DBTREE::url_bbscgibase( const std::string& url )
{
    return DBTREE::get_board( url )->url_bbscgibase();
}

std::string DBTREE::url_subbbscgibase( const std::string& url )
{
    return DBTREE::get_board( url )->url_subbbscgibase();
}


std::string DBTREE::url_bbscgi( const std::string& url )
{
    return DBTREE::get_article( url )->url_bbscgi();
}

std::string DBTREE::url_subbbscgi( const std::string& url )
{
    return DBTREE::get_article( url )->url_subbbscgi();
}

std::string DBTREE::url_bbscgi_new( const std::string& url )
{
    return DBTREE::get_board( url )->url_bbscgi_new();
}

std::string DBTREE::url_subbbscgi_new( const std::string& url )
{
    return DBTREE::get_board( url )->url_subbbscgi_new();
}


// 簡易版
std::string DBTREE::is_board_moved( const std::string& url )
{
    return get_root()->is_board_moved( url );
}

std::string DBTREE::is_board_moved( const std::string& url,
                                    std::string& old_root,
                                    std::string& old_path_board,
                                    std::string& new_root,
                                    std::string& new_path_board )
{
    return get_root()->is_board_moved( url, old_root, old_path_board, new_root, new_path_board );
}


bool DBTREE::move_board( const std::string& url_old, const std::string& url_new )
{
    return get_root()->move_board( url_old, url_new, false );
}

void DBTREE::set_enable_save_movetable( const bool set )
{
    get_root()->set_enable_save_movetable( set );
}

void DBTREE::save_movetable()
{
    get_root()->save_movetable();
}

const XML::Document& DBTREE::get_xml_document()
{
    return get_root()->xml_document();
}


const std::list< DBTREE::ETCBOARDINFO >& DBTREE::get_etcboards()
{
    return get_root()->get_etcboards();
}


bool DBTREE::add_etc( const std::string& url, const std::string& name, const std::string& basicauth, const std::string& id )
{
    return get_root()->add_etc( url, name, basicauth, id );
}


bool DBTREE::move_etc( const std::string& url_old, const std::string& url_new,
               const std::string& name_old, const std::string& name_new,
               const std::string& basicauth, const std::string& boardid )
{
    return get_root()->move_etc( url_old, url_new,
                                 name_old, name_new,
                                 basicauth, boardid );
}


bool DBTREE::remove_etc( const std::string& url, const std::string& name )
{
    return get_root()->remove_etc( url, name );
}


void DBTREE::save_etc()
{
    get_root()->save_etc();
}


void DBTREE::download_bbsmenu()
{
    get_root()->download_bbsmenu();
}


// bbsmenuの更新時間( 文字列 )
std::string DBTREE::get_date_modified()
{
    return get_root()->get_date_modified();
}


// bbsmenuの更新時間( time_t )
time_t DBTREE::get_time_modified()
{
    return get_root()->get_time_modified();
}


std::string DBTREE::board_path( const std::string& url )
{
    return DBTREE::get_board( url )->get_path_board();
}


std::string DBTREE::board_id( const std::string& url )
{
    return DBTREE::get_board( url )->get_id();
}

// 更新時間( time_t )
time_t DBTREE::board_time_modified( const std::string& url )
{
    return DBTREE::get_board( url )->get_time_modified();
}

// 板の更新時間( 文字列 )
std::string DBTREE::board_date_modified( const std::string& url )
{
    return DBTREE::get_board( url )->get_date_modified();
}

// 板の更新時間( 文字列 )をセット
void DBTREE::board_set_date_modified( const std::string& url, const std::string& date )
{
    DBTREE::get_board( url )->set_date_modified( date );
}

const std::string& DBTREE::board_get_modified_localrule( const std::string& url )
{
    return DBTREE::get_board( url )->get_modified_localrule();
}

void DBTREE::board_set_modified_localrule( const std::string& url, const std::string& modified )
{
    DBTREE::get_board( url )->set_modified_localrule( modified );
}

const std::string& DBTREE::board_get_modified_setting( const std::string& url )
{
    return DBTREE::get_board( url )->get_modified_setting();
}

void DBTREE::board_set_modified_setting( const std::string& url, const std::string& modified )
{
    DBTREE::get_board( url )->set_modified_setting( modified );
}


std::string DBTREE::board_name( const std::string& url )
{
    return DBTREE::get_board( url )->get_name();
}


std::string DBTREE::board_subjecttxt( const std::string& url )
{
    return DBTREE::get_board( url )->get_subjecttxt();
}


std::string DBTREE::board_charset( const std::string& url )
{
    return DBTREE::get_board( url )->get_charset();
}


std::string DBTREE::board_cookie_by_host( const std::string& url )
{
    return DBTREE::get_board( url )->cookie_by_host();
}


std::string DBTREE::board_cookie_for_request( const std::string& url )
{
    return DBTREE::get_board( url )->cookie_for_request();
}


std::string DBTREE::board_cookie_for_post( const std::string& url )
{
    return DBTREE::get_board( url )->cookie_for_post();
}


void DBTREE::board_set_list_cookies( const std::string& url, const std::list< std::string>& list_cookies )
{
    DBTREE::get_board( url )->set_list_cookies( list_cookies );
}

void DBTREE::board_delete_cookies( const std::string& url )
{
    DBTREE::get_board( url )->delete_cookies();
}

std::string DBTREE::board_keyword_for_write( const std::string& url )
{
    return DBTREE::get_board( url )->get_keyword_for_write();
}


void DBTREE::board_set_keyword_for_write( const std::string& url, const std::string& keyword )
{
    DBTREE::get_board( url )->set_keyword_for_write( keyword );
}


void DBTREE::board_analyze_keyword_for_write( const std::string& url, const std::string& html )
{
    DBTREE::get_board( url )->analyze_keyword_for_write( html );
}


std::string DBTREE::board_keyword_for_newarticle( const std::string& url )
{
    return DBTREE::get_board( url )->get_keyword_for_newarticle();
}


void DBTREE::board_set_keyword_for_newarticle( const std::string& url, const std::string& keyword )
{
    DBTREE::get_board( url )->set_keyword_for_newarticle( keyword );
}


void DBTREE::board_analyze_keyword_for_newarticle( const std::string& url, const std::string& html )
{
    DBTREE::get_board( url )->analyze_keyword_for_newarticle( html );
}


std::string DBTREE::board_basicauth( const std::string& url )
{
    return DBTREE::get_board( url )->get_basicauth();
}


std::string DBTREE::board_ext( const std::string& url )
{
    return DBTREE::get_board( url )->get_ext();
}


int DBTREE::board_status( const std::string& url )
{
    return DBTREE::get_board( url )->get_status();
}


int DBTREE::board_code( const std::string& url )
{
    return DBTREE::get_board( url )->get_code();
}


std::string DBTREE::board_str_code( const std::string& url )
{
    return DBTREE::get_board( url )->get_str_code();
}


void DBTREE::board_save_info( const std::string& url )
{
    DBTREE::get_board( url )->save_info();
}


void DBTREE::board_download_subject( const std::string& url, const std::string& url_update_view )
{
    DBTREE::get_board( url )->download_subject( url_update_view, false );
}

void DBTREE::board_read_subject_from_cache( const std::string& url )
{
    DBTREE::get_board( url )->download_subject( std::string(), true );
}

bool DBTREE::board_is_loading( const std::string& url )
{
    return DBTREE::get_board( url )->is_loading();
}

void DBTREE::board_stop_load( const std::string& url )
{
    DBTREE::get_board( url )->stop_load();
}


std::vector< DBTREE::ArticleBase* >& DBTREE::board_list_subject( const std::string& url )
{
    return DBTREE::get_board( url )->get_list_subject();
}


int DBTREE::board_view_sort_column( const std::string& url )
{
    return DBTREE::get_board( url )->get_view_sort_column();
}


void DBTREE::board_set_view_sort_column( const std::string& url, int column )
{
    DBTREE::get_board( url )->set_view_sort_column( column );
}


int DBTREE::board_view_sort_mode( const std::string& url )
{
    return DBTREE::get_board( url )->get_view_sort_mode();
}


void DBTREE::board_set_view_sort_mode( const std::string& url, int mode )
{
    DBTREE::get_board( url )->set_view_sort_mode( mode );
}


int DBTREE::board_view_sort_pre_column( const std::string& url )
{
    return DBTREE::get_board( url )->get_view_sort_pre_column();
}


void DBTREE::board_set_view_sort_pre_column( const std::string& url, int column )
{
    DBTREE::get_board( url )->set_view_sort_pre_column( column );
}


int DBTREE::board_view_sort_pre_mode( const std::string& url )
{
    return DBTREE::get_board( url )->get_view_sort_pre_mode();
}


void DBTREE::board_set_view_sort_pre_mode( const std::string& url, int mode )
{
    DBTREE::get_board( url )->set_view_sort_pre_mode( mode );
}

bool DBTREE::board_check_noname( const std::string& url )
{
    return DBTREE::get_board( url )->get_check_noname();
}

void DBTREE::board_set_check_noname( const std::string& url, const bool check )
{
    DBTREE::get_board( url )->set_check_noname( check );
}

bool DBTREE::board_show_oldlog( const std::string& url )
{
    return DBTREE::get_board( url )->get_show_oldlog();
}

void DBTREE::board_set_show_oldlog( const std::string& url, const bool show )
{
    DBTREE::get_board( url )->set_show_oldlog( show );
}


int DBTREE::board_get_mode_local_proxy( const std::string& url )
{
    return DBTREE::get_board( url )->get_mode_local_proxy();
}

const std::string& DBTREE::board_get_local_proxy( const std::string& url )
{
    return DBTREE::get_board( url )->get_local_proxy();
}

int DBTREE::board_get_local_proxy_port( const std::string& url )
{
    return DBTREE::get_board( url )->get_local_proxy_port();
}

const std::string& DBTREE::board_get_local_proxy_basicauth( const std::string& url )
{
    return DBTREE::get_board( url )->get_local_proxy_basicauth();
}

void DBTREE::board_set_mode_local_proxy( const std::string& url, int mode )
{
    DBTREE::get_board( url )->set_mode_local_proxy( mode );
}

void DBTREE::board_set_local_proxy( const std::string& url, const std::string& proxy )
{
    DBTREE::get_board( url )->set_local_proxy( proxy );
}

void DBTREE::board_set_local_proxy_port( const std::string& url, int port )
{
    DBTREE::get_board( url )->set_local_proxy_port( port );
}

int DBTREE::board_get_mode_local_proxy_w( const std::string& url )
{
    return DBTREE::get_board( url )->get_mode_local_proxy_w();
}

const std::string& DBTREE::board_get_local_proxy_w( const std::string& url )
{
    return DBTREE::get_board( url )->get_local_proxy_w();
}

const std::string& DBTREE::board_get_local_proxy_basicauth_w( const std::string& url )
{
    return DBTREE::get_board( url )->get_local_proxy_basicauth_w();
}

int DBTREE::board_get_local_proxy_port_w( const std::string& url )
{
    return DBTREE::get_board( url )->get_local_proxy_port_w();
}

void DBTREE::board_set_mode_local_proxy_w( const std::string& url, int mode )
{
    DBTREE::get_board( url )->set_mode_local_proxy_w( mode );
}

void DBTREE::board_set_local_proxy_w( const std::string& url, const std::string& proxy )
{
    DBTREE::get_board( url )->set_local_proxy_w( proxy );
}

void DBTREE::board_set_local_proxy_port_w( const std::string& url, int port )
{
    DBTREE::get_board( url )->set_local_proxy_port_w( port );
}

const std::string& DBTREE::board_get_write_name( const std::string& url )
{
    return DBTREE::get_board( url )->get_write_name();
}

const std::string& DBTREE::board_get_write_mail( const std::string& url )
{
    return DBTREE::get_board( url )->get_write_mail();
}

void DBTREE::board_set_write_name( const std::string& url, const std::string& name )
{
    DBTREE::get_board( url )->set_write_name( name );
}

void DBTREE::board_set_write_mail( const std::string& url, const std::string& mail )
{
    DBTREE::get_board( url )->set_write_mail( mail );
}


// 全スレの書き込み履歴のリセット
void DBTREE::clear_all_post_history()
{
    DBTREE::get_root()->clear_all_post_history();
}


void DBTREE::read_boardinfo_all()
{
    DBTREE::get_root()->read_boardinfo_all();
}

void DBTREE::search_cache_all( std::vector< DBTREE::ArticleBase* >& list_article,
                               const std::string& query, const bool mode_or, const bool bm, const bool stop )
{
    DBTREE::get_root()->search_cache( list_article, query, mode_or, bm, stop );
}

void DBTREE::search_cache( const std::string& url, std::vector< DBTREE::ArticleBase* >& list_article, const std::string& query,
                           const bool mode_or, const bool bm, const bool stop )
{
    DBTREE::get_board( url )->search_cache( list_article, query, mode_or, bm, stop );
}

void DBTREE::board_update_writetime( const std::string& url )
{
    DBTREE::get_board( url )->update_writetime();
}

time_t DBTREE::board_write_time( const std::string& url )
{
    return DBTREE::get_board( url )->get_write_time();
}

time_t DBTREE::board_write_pass( const std::string& url )
{
    return DBTREE::get_board( url )->get_write_pass();
}

time_t DBTREE::board_samba_sec( const std::string& url )
{
    return DBTREE::get_board( url )->get_samba_sec();
}

void DBTREE::board_set_samba_sec( const std::string& url, time_t sec )
{
    DBTREE::get_board( url )->set_samba_sec( sec );
}

time_t DBTREE::board_write_leftsec( const std::string& url )
{
    return DBTREE::get_board( url )->get_write_leftsec();
}

void DBTREE::board_show_updateicon( const std::string& url, const bool update )
{
    DBTREE::get_board( url )->show_updateicon( update );
}

std::list< std::string > DBTREE::board_get_check_update_articles( const std::string& url )
{
    return DBTREE::get_board( url )->get_check_update_articles();
}

// datファイルのインポート
std::string DBTREE::board_import_dat( const std::string& url, const std::string& filename )
{
    return DBTREE::get_board( url )->import_dat( filename );
}

// 板に属する全スレの書き込み履歴のリセット
void DBTREE::board_clear_all_post_history( const std::string& url )
{
    DBTREE::get_board( url )->clear_all_post_history();
}


int DBTREE::board_get_number_max_res( const std::string& url )
{
    return DBTREE::get_board( url )->get_number_max_res();
}

void DBTREE::board_set_number_max_res( const std::string& url, const int number )
{
    DBTREE::get_board( url )->set_number_max_res( number );
}

// datの最大サイズ(Kバイト)
int DBTREE::board_get_max_dat_lng( const std::string& url )
{
    return DBTREE::get_board( url )->get_max_dat_lng();
}

time_t DBTREE::board_get_live_sec( const std::string& url )
{
    return DBTREE::get_board( url )->get_live_sec();
}

void DBTREE::board_set_live_sec( const std::string& url, time_t sec )
{
    DBTREE::get_board( url )->set_live_sec( sec );
}

time_t DBTREE::board_last_access_time( const std::string& url )
{
    return DBTREE::get_board( url )->get_last_access_time();
}


/////////////////////////////////////////////////


const std::list< std::string >& DBTREE::get_abone_list_id_board( const std::string& url )
{
    return DBTREE::get_board( url )->get_abone_list_id_board();
}

const std::list< std::string >& DBTREE::get_abone_list_name_board( const std::string& url )
{
    return DBTREE::get_board( url )->get_abone_list_name_board();
}

const std::list< std::string >& DBTREE::get_abone_list_word_board( const std::string& url )
{
    return DBTREE::get_board( url )->get_abone_list_word_board();
}

const std::list< std::string >& DBTREE::get_abone_list_regex_board( const std::string& url )
{
    return DBTREE::get_board( url )->get_abone_list_regex_board();
}

void DBTREE::reset_abone_board( const std::string& url,
                                const std::list< std::string >& ids,
                                const std::list< std::string >& names,
                                const std::list< std::string >& words,
                                const std::list< std::string >& regexs )
{
    DBTREE::get_board( url )->reset_abone_board( ids, names, words, regexs );
}


void DBTREE::add_abone_id_board( const std::string& url, const std::string& id )
{
    DBTREE::get_board( url )->add_abone_id_board( id );
}

void DBTREE::add_abone_name_board( const std::string& url, const std::string& name )
{
    DBTREE::get_board( url )->add_abone_name_board( name );
}

void DBTREE::add_abone_word_board( const std::string& url, const std::string& word )
{
    DBTREE::get_board( url )->add_abone_word_board( word );
}


/////////////////////////////////////////////////


const std::list< std::string >& DBTREE::get_abone_list_thread( const std::string& url )
{
    return DBTREE::get_board( url )->get_abone_list_thread();
}

const std::list< std::string >& DBTREE::get_abone_list_thread_remove( const std::string& url )
{
    return DBTREE::get_board( url )->get_abone_list_thread_remove();
}

const std::list< std::string >& DBTREE::get_abone_list_word_thread( const std::string& url )
{
    return DBTREE::get_board( url )->get_abone_list_word_thread();
}

const std::list< std::string >& DBTREE::get_abone_list_regex_thread( const std::string& url )
{
    return DBTREE::get_board( url )->get_abone_list_regex_thread();
}

int DBTREE::get_abone_number_thread( const std::string& url )
{
    return DBTREE::get_board( url )->get_abone_number_thread();
}

int DBTREE::get_abone_hour_thread( const std::string& url )
{
    return DBTREE::get_board( url )->get_abone_hour_thread();
}


void DBTREE::remove_old_abone_thread( const std::string& url )
{
    DBTREE::get_board( url )->remove_old_abone_thread();
}

void DBTREE::update_abone_thread()
{
    DBTREE::get_root()->update_abone_thread();
}

void DBTREE::reset_abone_thread( const std::string& url,
                                 const std::list< std::string >& threads,
                                 const std::list< std::string >& words,
                                 const std::list< std::string >& regexs,
                                 const int number,
                                 const int hour,
                                 const bool redraw
    )
{
    DBTREE::get_board( url )->reset_abone_thread( threads, words, regexs, number, hour, redraw );
}

/////////////////////////////////////////////////


bool DBTREE::article_is_cached( const std::string& url )
{
    return DBTREE::get_article( url )->is_cached();
}


// 拡張子付き
std::string DBTREE::article_id( const std::string& url )
{
    return DBTREE::get_article( url )->get_id();
}


// idから拡張子を取ったもの
std::string DBTREE::article_key( const std::string& url )
{
    return DBTREE::get_article( url )->get_key();
}


// 移転する前のオリジナルのURL
std::string DBTREE::article_org_host( const std::string& url )
{
    return DBTREE::get_article( url )->get_org_host();
}


time_t DBTREE::article_since_time( const std::string& url )
{
    return DBTREE::get_article( url )->get_since_time();
}


std::string DBTREE::article_since_date( const std::string& url )
{
    return DBTREE::get_article( url )->get_since_date();
}


// スレの更新時間( time_t )
time_t DBTREE::article_time_modified( const std::string& url )
{
    return DBTREE::get_article( url )->get_time_modified();
}


// スレの更新時間( 文字列 )
std::string DBTREE::article_date_modified( const std::string& url )
{
    return DBTREE::get_article( url )->get_date_modified();
}

// スレの更新時間( 文字列 )をセット
void DBTREE::article_set_date_modified( const std::string& url, const std::string& date )
{
    DBTREE::get_article( url )->set_date_modified( date );
}

int  DBTREE::article_hour( const std::string& url )
{
    return DBTREE::get_article( url )->get_hour();
}


// 最終書き込み時刻
time_t DBTREE::article_write_time( const std::string& url )
{
    return DBTREE::get_article( url )->get_write_time();
}

// 最終書き込み時刻(文字列)
std::string DBTREE::article_write_date( const std::string& url )
{
    return DBTREE::get_article( url )->get_write_date();
}


int DBTREE::article_status( const std::string& url )
{
    return DBTREE::get_article( url )->get_status();
}


int DBTREE::article_code( const std::string& url )
{
    return DBTREE::get_article( url )->get_code();
}

std::string DBTREE::article_str_code( const std::string& url )
{
    return DBTREE::get_article( url )->get_str_code();
}

std::string DBTREE::article_ext_err( const std::string& url )
{
    return DBTREE::get_article( url )->get_ext_err();
}

std::string DBTREE::article_subject( const std::string& url )
{
    return DBTREE::get_article( url )->get_subject();
}


int DBTREE::article_number( const std::string& url )
{
    return DBTREE::get_article( url )->get_number();
}

int DBTREE::article_number_load( const std::string& url )
{
    return DBTREE::get_article( url )->get_number_load();
}

int DBTREE::article_number_seen( const std::string& url )
{
    return DBTREE::get_article( url )->get_number_seen();
}

void DBTREE::article_set_number_seen( const std::string& url, int seen )
{
    DBTREE::get_article( url )->set_number_seen( seen );
}

int DBTREE::article_number_new( const std::string& url )
{
    return DBTREE::get_article( url )->get_number_new();
}

bool DBTREE::article_is_loading( const std::string& url )
{
    return DBTREE::get_article( url )->is_loading();
}

bool DBTREE::article_is_checking_update( const std::string& url )
{
    return DBTREE::get_article( url )->is_checking_update();
}

void DBTREE::article_download_dat( const std::string& url, const bool check_update )
{
    DBTREE::get_article( url )->download_dat( check_update );
}

void DBTREE::article_set_url_pre_article( const std::string& url, const std::string& url_pre_article )
{
    DBTREE::get_article( url )->set_url_pre_article( url_pre_article );
}

void DBTREE::article_copy_article_info( const std::string& url, const std::string& url_src )
{
    DBTREE::get_article( url )->copy_article_info( url_src );
}

void DBTREE::article_stop_load( const std::string& url )
{
    DBTREE::get_article( url )->stop_load();
}


int DBTREE::article_get_speed( const std::string& url )
{
    return DBTREE::get_article( url )->get_speed();
}


// 書き込み履歴のリセット
void DBTREE::article_clear_post_history( const std::string& url )
{
    DBTREE::get_article( url )->clear_post_history();
}


// ユーザーエージェント
// ダウンロード用
const std::string& DBTREE::get_agent( const std::string& url )
{
    return DBTREE::get_board( url )->get_agent();
}

// 書き込み用
const std::string& DBTREE::get_agent_w( const std::string& url )
{
    return DBTREE::get_board( url )->get_agent_w();
}

    
std::string DBTREE::get_proxy_host( const std::string& url )
{
    return DBTREE::get_board( url )->get_proxy_host();
}
    
int DBTREE::get_proxy_port( const std::string& url )
{
    return DBTREE::get_board( url )->get_proxy_port();
}

std::string DBTREE::get_proxy_basicauth( const std::string& url )
{
    return DBTREE::get_board( url )->get_proxy_basicauth();
}

std::string DBTREE::get_proxy_host_w( const std::string& url )
{
    return DBTREE::get_board( url )->get_proxy_host_w();
}
    
int DBTREE::get_proxy_port_w( const std::string& url )
{
    return DBTREE::get_board( url )->get_proxy_port_w();
}

std::string DBTREE::get_proxy_basicauth_w( const std::string& url )
{
    return DBTREE::get_board( url )->get_proxy_basicauth_w();
}


std::string DBTREE::localrule( const std::string& url )
{
    return DBTREE::get_board( url )->localrule();
}


std::string DBTREE::settingtxt( const std::string& url )
{
    return DBTREE::get_board( url )->settingtxt();
}


std::string DBTREE::default_noname( const std::string& url )
{
    return DBTREE::get_board( url )->default_noname();
}

int DBTREE::line_number( const std::string& url )
{
    return DBTREE::get_board( url )->line_number();
}

int DBTREE::message_count( const std::string& url )
{
    return DBTREE::get_board( url )->message_count();
}

// 特殊文字書き込み可能か( pass なら可能、 change なら不可 )
std::string DBTREE::get_unicode( const std::string& url )
{
    return DBTREE::get_board( url )->get_unicode();
}

const std::string& DBTREE::write_name( const std::string& url )
{
    return DBTREE::get_article( url )->get_write_name();
}
    
void DBTREE::set_write_name( const std::string& url, const std::string& str )
{
    DBTREE::get_article( url )->set_write_name( str );
}

bool DBTREE::write_fixname( const std::string& url )
{
    return DBTREE::get_article( url )->get_write_fixname();
}

void DBTREE::set_write_fixname( const std::string& url, bool set )
{
    DBTREE::get_article( url )->set_write_fixname( set );
}

const std::string& DBTREE::write_mail( const std::string& url )
{
    return DBTREE::get_article( url )->get_write_mail();
}

void DBTREE::set_write_mail( const std::string& url, const std::string& str )
{
    DBTREE::get_article( url )->set_write_mail( str );
}

bool DBTREE::write_fixmail( const std::string& url )
{
    return DBTREE::get_article( url )->get_write_fixmail();
}

void DBTREE::set_write_fixmail( const std::string& url, bool set )
{
    DBTREE::get_article( url )->set_write_fixmail( set );
}


std::string DBTREE::create_write_message( const std::string& url, const std::string& name, const std::string& mail,
                                          const std::string& msg )
{
    return DBTREE::get_article( url )->create_write_message( name, mail, msg );
}


std::string DBTREE::create_newarticle_message( const std::string& url, const std::string& subject,
                                               const std::string& name, const std::string& mail,
                                               const std::string& msg )
{
    return DBTREE::get_board( url )->create_newarticle_message( subject, name, mail, msg );
}


std::string DBTREE::get_write_referer( const std::string& url )
{
    return DBTREE::get_board( url )->get_write_referer();
}


// キャッシュ削除
void DBTREE::delete_article( const std::string& url, const bool cache_only )
{
    DBTREE::get_article( url )->delete_cache( cache_only );
}


// キャッシュ保存
bool DBTREE::article_save_dat( const std::string& url, const std::string& path_to )
{
    return DBTREE::get_article( url )->save_dat( path_to );
}


// 全スレ情報の保存
void DBTREE::save_articleinfo_all()
{
    DBTREE::get_root()->save_articleinfo_all();
}


void DBTREE::article_update_writetime( const std::string& url )
{
    DBTREE::get_article( url )->update_writetime();
}


size_t DBTREE::article_lng_dat( const std::string& url )
{
    return DBTREE::get_article( url )->get_lng_dat();
}



void DBTREE::update_abone_all_article()
{
    DBTREE::get_root()->update_abone_all_article();
}


// 全articlebaseクラスの書き込み時間とスレ立て時間の文字列をリセット
void DBTREE::reset_all_since_date()
{
    DBTREE::get_root()->reset_all_since_date();
}

void DBTREE::reset_all_write_date()
{
    DBTREE::get_root()->reset_all_write_date();
}

void DBTREE::reset_all_access_date()
{
    DBTREE::get_root()->reset_all_access_date();
}


const std::list< std::string >& DBTREE::get_abone_list_id( const std::string& url )
{
    return DBTREE::get_article( url )->get_abone_list_id();
}


const std::list< std::string >& DBTREE::get_abone_list_name( const std::string& url )
{
    return DBTREE::get_article( url )->get_abone_list_name();
}

const std::list< std::string >& DBTREE::get_abone_list_word( const std::string& url )
{
    return DBTREE::get_article( url )->get_abone_list_word();
}

const std::list< std::string >& DBTREE::get_abone_list_regex( const std::string& url )
{
    return DBTREE::get_article( url )->get_abone_list_regex();
}


const std::unordered_set< int >& DBTREE::get_abone_reses( const std::string& url )
{
    return DBTREE::get_article( url )->get_abone_reses();
}


void DBTREE::reset_abone( const std::string& url,
                          const std::list< std::string >& ids,
                          const std::list< std::string >& names,
                          const std::list< std::string >& words,
                          const std::list< std::string >& regexs,
                          const std::vector< char >& vec_abone_res,
                          const bool transparent, const bool chain, const bool age,
                          const bool board, const bool global
    )
{
    DBTREE::get_article( url )->reset_abone( ids, names, words, regexs, vec_abone_res, transparent, chain, age, board, global );
}


void DBTREE::set_abone_res( const std::string& url, const int num_from, const int num_to, const bool set )
{
    DBTREE::get_article( url )->set_abone_res( num_from, num_to, set );
}


void DBTREE::add_abone_id( const std::string& url, const std::string& id )
{
    DBTREE::get_article( url )->add_abone_id( id );
}


void DBTREE::add_abone_name( const std::string& url, const std::string& name )
{
    DBTREE::get_article( url )->add_abone_name( name );
}

void DBTREE::add_abone_word( const std::string& url, const std::string& word )
{
    DBTREE::get_article( url )->add_abone_word( word );
}


// 透明あぼーん
bool DBTREE::get_abone_transparent( const std::string& url )
{
    return DBTREE::get_article( url )->get_abone_transparent();
}


void DBTREE::set_abone_transparent( const std::string& url, const bool set )
{
    DBTREE::get_article( url )->set_abone_transparent( set );
}


// 連鎖あぼーん
bool DBTREE::get_abone_chain( const std::string& url )
{
    return DBTREE::get_article( url )->get_abone_chain();
}


void DBTREE::set_abone_chain( const std::string& url, const bool set )
{
    DBTREE::get_article( url )->set_abone_chain( set );
}


// ageあぼーん
bool DBTREE::get_abone_age( const std::string& url )
{
    return DBTREE::get_article( url )->get_abone_age();
}


void DBTREE::set_abone_age( const std::string& url, const bool set )
{
    DBTREE::get_article( url )->set_abone_age( set );
}


// 板レベルでのあぼーん
bool DBTREE::get_abone_board( const std::string& url )
{
    return DBTREE::get_article( url )->get_abone_board();
}


void DBTREE::set_abone_board( const std::string& url, const bool set )
{
    DBTREE::get_article( url )->set_abone_board( set );
}


// 全体レベルでのあぼーん
bool DBTREE::get_abone_global( const std::string& url )
{
    return DBTREE::get_article( url )->get_abone_global();
}


void DBTREE::set_abone_global( const std::string& url, const bool set )
{
    DBTREE::get_article( url )->set_abone_global( set );
}


bool DBTREE::is_bookmarked_thread( const std::string& url )
{
    return DBTREE::get_article( url )->is_bookmarked_thread();
}

void DBTREE::set_bookmarked_thread( const std::string& url, const bool bookmarked )
{
    DBTREE::get_article( url )->set_bookmarked_thread( bookmarked );
}


int DBTREE::get_num_bookmark( const std::string& url )
{
    return DBTREE::get_article( url )->get_num_bookmark();
}

bool DBTREE::is_bookmarked( const std::string& url, const int number )
{
    return DBTREE::get_article( url )->is_bookmarked( number );
}


void DBTREE::set_bookmark( const std::string& url, const int number, const bool set )
{
    DBTREE::get_article( url )->set_bookmark( number, set );
}

