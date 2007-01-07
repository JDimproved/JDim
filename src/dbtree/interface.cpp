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
DBTREE::Root *instance_dbtree_root = NULL;


void DBTREE::create_root()
{
    if( ! instance_dbtree_root ) instance_dbtree_root = new DBTREE::Root();
}


void DBTREE::delete_root()
{
    if( instance_dbtree_root ) delete instance_dbtree_root;
}


//////////////////////////////////////
//
// ツリーの構成要素のポインタ取得
//
// root から葉っぱに向かって順に取得していく
//

DBTREE::Root* DBTREE::get_root()
{
    assert( instance_dbtree_root != NULL );
    return instance_dbtree_root;
}

DBTREE::BoardBase* DBTREE::get_board( const std::string& url )
{
    DBTREE::BoardBase* board = DBTREE::get_root()->get_board( url );
    assert( board != NULL );
    return board;
}

DBTREE::ArticleBase* DBTREE::get_article( const std::string& url )
{
    DBTREE::ArticleBase* article = DBTREE::get_board( url )->get_article_fromURL( url );
    assert( article != NULL );
    return article;
}

//////////////////////////////////////

const std::string DBTREE::url_subject( const std::string& url )
{
    return DBTREE::get_board( url )->url_subject();
}


const std::string DBTREE::url_root( const std::string& url )
{
    return DBTREE::get_board( url )->url_root();
}


const std::string DBTREE::url_boardbase( const std::string& url )
{
    return DBTREE::get_board( url )->url_boardbase();
}


const std::string DBTREE::url_datbase( const std::string& url )
{
    return DBTREE::get_board( url )->url_datbase();
}


// urlをdat型のurlに変換
const std::string DBTREE::url_dat( const std::string& url, int& num_from, int& num_to )
{
    return DBTREE::get_board( url )->url_dat( url, num_from, num_to );
}


// urlをdat型のurlに変換(簡易版)
const std::string DBTREE::url_dat( const std::string& url )
{
    int num_from, num_to;
    return url_dat( url, num_from, num_to );
}


// url を read.cgi型のurlに変換
const std::string DBTREE::url_readcgi( const std::string& url, int num_from, int num_to )
{
    return DBTREE::get_board( url )->url_readcgi( url, num_from, num_to );
}


const std::string DBTREE::url_bbscgibase( const std::string& url )
{
    return DBTREE::get_board( url )->url_bbscgibase();
}

const std::string DBTREE::url_subbbscgibase( const std::string& url )
{
    return DBTREE::get_board( url )->url_subbbscgibase();
}


const std::string DBTREE::url_bbscgi( const std::string& url )
{
    return DBTREE::get_article( url )->url_bbscgi();
}

const std::string DBTREE::url_subbbscgi( const std::string& url )
{
    return DBTREE::get_article( url )->url_subbbscgi();
}

const std::string DBTREE::url_bbscgi_new( const std::string& url )
{
    return DBTREE::get_board( url )->url_bbscgi_new();
}

const std::string DBTREE::url_subbbscgi_new( const std::string& url )
{
    return DBTREE::get_board( url )->url_subbbscgi_new();
}


const std::string  DBTREE::is_board_moved( const std::string& url )
{
    return get_root()->is_board_moved( url );
}


const std::string& DBTREE::get_xml_bbsmenu()
{
    return get_root()->xml_bbsmenu();
}


const std::string& DBTREE::get_xml_etc()
{
    return get_root()->xml_etc();
}


void DBTREE::download_bbsmenu()
{
    get_root()->download_bbsmenu();
}


const std::string DBTREE::board_path( const std::string& url )
{
    return DBTREE::get_board( url )->get_path_board();
}


const std::string DBTREE::board_id( const std::string& url )
{
    return DBTREE::get_board( url )->get_id();
}

// 更新時間
const time_t DBTREE::board_time_modified( const std::string& url )
{
    return DBTREE::get_board( url )->time_modified();
}


const std::string DBTREE::board_name( const std::string& url )
{
    return DBTREE::get_board( url )->get_name();
}


const std::string DBTREE::board_subjecttxt( const std::string& url )
{
    return DBTREE::get_board( url )->get_subjecttxt();
}


const std::string DBTREE::board_charset( const std::string& url )
{
    return DBTREE::get_board( url )->get_charset();
}


const std::string DBTREE::board_cookie_for_write( const std::string& url )
{
    return DBTREE::get_board( url )->cookie_for_write();
}


const std::list< std::string >& DBTREE::board_list_cookies_for_write( const std::string& url )
{
    return DBTREE::get_board( url )->list_cookies_for_write();
}


void DBTREE::board_set_list_cookies_for_write( const std::string& url, const std::list< std::string>& list_cookies )
{
    DBTREE::get_board( url )->set_list_cookies_for_write( list_cookies );
}


const std::string DBTREE::board_hana_for_write( const std::string& url )
{
    return DBTREE::get_board( url )->hana_for_write();
}


void DBTREE::board_set_hana_for_write( const std::string& url, const std::string& hana )
{
    DBTREE::get_board( url )->set_hana_for_write( hana );
}


const std::string DBTREE::board_basicauth( const std::string& url )
{
    return DBTREE::get_board( url )->get_basicauth();
}


const std::string DBTREE::board_ext( const std::string& url )
{
    return DBTREE::get_board( url )->get_ext();
}

const int DBTREE::board_code( const std::string& url )
{
    return DBTREE::get_board( url )->get_code();
}


const std::string DBTREE::board_str_code( const std::string& url )
{
    return DBTREE::get_board( url )->get_str_code();
}


void DBTREE::board_save_info( const std::string& url )
{
    DBTREE::get_board( url )->save_info();
}


void DBTREE::board_download_subject( const std::string& url )
{
    DBTREE::get_board( url )->download_subject();
}

void DBTREE::board_stop_load( const std::string& url )
{
    DBTREE::get_board( url )->stop_load();
}


std::list< DBTREE::ArticleBase* >& DBTREE::board_list_subject( const std::string& url )
{
    return DBTREE::get_board( url )->get_list_subject();
}


const int DBTREE::board_view_sort_column( const std::string& url )
{
    return DBTREE::get_board( url )->get_view_sort_column();
}


void DBTREE::board_set_view_sort_column( const std::string& url, int column )
{
    DBTREE::get_board( url )->set_view_sort_column( column );
}


const int DBTREE::board_view_sort_mode( const std::string& url )
{
    return DBTREE::get_board( url )->get_view_sort_mode();
}


void DBTREE::board_set_view_sort_mode( const std::string& url, int mode )
{
    DBTREE::get_board( url )->set_view_sort_mode( mode );
}


const int DBTREE::board_view_sort_pre_column( const std::string& url )
{
    return DBTREE::get_board( url )->get_view_sort_pre_column();
}


void DBTREE::board_set_view_sort_pre_column( const std::string& url, int column )
{
    DBTREE::get_board( url )->set_view_sort_pre_column( column );
}


const int DBTREE::board_view_sort_pre_mode( const std::string& url )
{
    return DBTREE::get_board( url )->get_view_sort_pre_mode();
}


void DBTREE::board_set_view_sort_pre_mode( const std::string& url, int mode )
{
    DBTREE::get_board( url )->set_view_sort_pre_mode( mode );
}

const bool DBTREE::board_check_noname( const std::string& url )
{
    return DBTREE::get_board( url )->get_check_noname();
}


void DBTREE::board_set_check_noname( const std::string& url, bool check )
{
    DBTREE::get_board( url )->set_check_noname( check );
}

const int DBTREE::board_get_mode_local_proxy( const std::string& url )
{
    return DBTREE::get_board( url )->get_mode_local_proxy();
}

const std::string& DBTREE::board_get_local_proxy( const std::string& url )
{
    return DBTREE::get_board( url )->get_local_proxy();
}

const int DBTREE::board_get_local_proxy_port( const std::string& url )
{
    return DBTREE::get_board( url )->get_local_proxy_port();
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

const int DBTREE::board_get_mode_local_proxy_w( const std::string& url )
{
    return DBTREE::get_board( url )->get_mode_local_proxy_w();
}

const std::string& DBTREE::board_get_local_proxy_w( const std::string& url )
{
    return DBTREE::get_board( url )->get_local_proxy_w();
}

const int DBTREE::board_get_local_proxy_port_w( const std::string& url )
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


void DBTREE::read_boardinfo_all()
{
    DBTREE::get_root()->read_boardinfo_all();
}

std::list< std::string > DBTREE::search_cache_all( const std::string& url, const std::string& query, bool mode_or, bool& stop )
{
    return DBTREE::get_root()->search_cache( query, mode_or, stop );
}

std::list< std::string > DBTREE::search_cache( const std::string& url, const std::string& query, bool mode_or, bool& stop )
{
    return DBTREE::get_board( url )->search_cache( query, mode_or, stop );
}


void DBTREE::update_abone_all_board()
{
    DBTREE::get_root()->update_abone_all_board();

}

void DBTREE::update_abone_board( const std::string& url )
{
    DBTREE::get_board( url )->update_abone_thread();
}

std::list< std::string > DBTREE::get_abone_list_thread( const std::string& url )
{
    return DBTREE::get_board( url )->get_abone_list_thread();
}

std::list< std::string > DBTREE::get_abone_list_word_thread( const std::string& url )
{
    return DBTREE::get_board( url )->get_abone_list_word_thread();
}

std::list< std::string > DBTREE::get_abone_list_regex_thread( const std::string& url )
{
    return DBTREE::get_board( url )->get_abone_list_regex_thread();
}


void DBTREE::reset_abone_thread( const std::string& url,
                                 std::list< std::string >& threads, std::list< std::string >& words, std::list< std::string >& regexs )
{
    DBTREE::get_board( url )->reset_abone_thread( threads, words, regexs );
}


const bool DBTREE::article_is_cached( const std::string& url )
{
    return DBTREE::get_article( url )->is_cached();
}


// 拡張子付き
const std::string DBTREE::article_id( const std::string& url )
{
    std::string id =  DBTREE::get_article( url )->get_id();
    return id;
}


// idから拡張子を取ったもの
const std::string DBTREE::article_key( const std::string& url )
{
    std::string key = DBTREE::get_article( url )->get_key();
    return key;
}


const time_t DBTREE::article_since_time( const std::string& url )
{
    return DBTREE::get_article( url )->get_since_time();
}


const std::string DBTREE::article_since_date( const std::string& url )
{
    return DBTREE::get_article( url )->get_since_date();
}


// スレの更新時間( size_t )
const time_t DBTREE::article_time_modified( const std::string& url )
{
    return DBTREE::get_article( url )->get_time_modified();
}


// スレの更新時間( 文字列 )
const std::string DBTREE::article_date_modified( const std::string& url )
{
    return DBTREE::get_article( url )->get_date_modified();
}


const int  DBTREE::article_hour( const std::string& url )
{
    return DBTREE::get_article( url )->get_hour();
}


// 最終書き込み時刻
const time_t DBTREE::article_write_time( const std::string& url )
{
    return DBTREE::get_article( url )->get_write_time();
}

// 最終書き込み時刻(文字列)
const std::string DBTREE::article_write_date( const std::string& url )
{
    return DBTREE::get_article( url )->get_write_date();
}



const int DBTREE::article_status( const std::string& url )
{
    return DBTREE::get_article( url )->get_status();
}


void DBTREE::article_reset_status( const std::string& url )
{
    DBTREE::get_article( url )->reset_status();
}


const int DBTREE::article_code( const std::string& url )
{
    return DBTREE::get_article( url )->get_code();
}

const std::string DBTREE::article_str_code( const std::string& url )
{
    return DBTREE::get_article( url )->get_str_code();
}

const std::string DBTREE::article_ext_err( const std::string& url )
{
    return DBTREE::get_article( url )->get_ext_err();
}

const std::string DBTREE::article_subject( const std::string& url )
{
    return DBTREE::get_article( url )->get_subject();
}


const int DBTREE::article_number( const std::string& url )
{
    return DBTREE::get_article( url )->get_number();
}

const int DBTREE::article_number_load( const std::string& url )
{
    return DBTREE::get_article( url )->get_number_load();
}

const int DBTREE::article_number_seen( const std::string& url )
{
    return DBTREE::get_article( url )->get_number_seen();
}

void DBTREE::article_set_number_seen( const std::string& url, int seen )
{
    DBTREE::get_article( url )->set_number_seen( seen );
}

const int DBTREE::article_number_new( const std::string& url )
{
    return DBTREE::get_article( url )->get_number_new();
}

void DBTREE::article_download_dat( const std::string& url )
{
    DBTREE::get_article( url )->download_dat();
}


const int DBTREE::article_get_speed( const std::string& url )
{
    return DBTREE::get_article( url )->get_speed();
}


const std::string& DBTREE::get_agent( const std::string& url )
{
    return DBTREE::get_board( url )->get_agent();
}
    
const std::string DBTREE::get_proxy_host( const std::string& url )
{
    return DBTREE::get_board( url )->get_proxy_host();
}
    
const int DBTREE::get_proxy_port( const std::string& url )
{
    return DBTREE::get_board( url )->get_proxy_port();
}

const std::string DBTREE::get_proxy_host_w( const std::string& url )
{
    return DBTREE::get_board( url )->get_proxy_host_w();
}
    
const int DBTREE::get_proxy_port_w( const std::string& url )
{
    return DBTREE::get_board( url )->get_proxy_port_w();
}

const std::string DBTREE::settingtxt( const std::string& url )
{
    return DBTREE::get_board( url )->settingtxt();
}


const std::string DBTREE::default_noname( const std::string& url )
{
    return DBTREE::get_board( url )->default_noname();
}

const int DBTREE::line_number( const std::string& url )
{
    return DBTREE::get_board( url )->line_number();
}

const int DBTREE::message_count( const std::string& url )
{
    return DBTREE::get_board( url )->message_count();
}

const std::string& DBTREE::write_name( const std::string& url )
{
    return DBTREE::get_article( url )->get_write_name();
}
    
void DBTREE::set_write_name( const std::string& url, const std::string& str )
{
    DBTREE::get_article( url )->set_write_name( str );
}

const bool DBTREE::write_fixname( const std::string& url )
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

const bool DBTREE::write_fixmail( const std::string& url )
{
    return DBTREE::get_article( url )->get_write_fixmail();
}

void DBTREE::set_write_fixmail( const std::string& url, bool set )
{
    DBTREE::get_article( url )->set_write_fixmail( set );
}


const std::string DBTREE::create_write_message( const std::string& url,
                                        const std::string& name, const std::string& mail, const std::string& msg )
{
    return DBTREE::get_article( url )->create_write_message( name, mail, msg );
}


const std::string DBTREE::create_newarticle_message( const std::string& url, const std::string& subject,
                                                     const std::string& name, const std::string& mail, const std::string& msg )
{
    return DBTREE::get_board( url )->create_newarticle_message( subject, name, mail, msg );
}


// キャッシュ削除
void DBTREE::delete_article( const std::string& url )
{
    DBTREE::get_article( url )->delete_cache();
}


// キャッシュ保存
bool DBTREE::article_save_dat( const std::string& url, const std::string& path_to )
{
    return DBTREE::get_article( url )->save_dat( path_to );
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


std::list< std::string > DBTREE::get_abone_list_id( const std::string& url )
{
    return DBTREE::get_article( url )->get_abone_list_id();
}


std::list< std::string > DBTREE::get_abone_list_name( const std::string& url )
{
    return DBTREE::get_article( url )->get_abone_list_name();
}

std::list< std::string > DBTREE::get_abone_list_word( const std::string& url )
{
    return DBTREE::get_article( url )->get_abone_list_word();
}

std::list< std::string > DBTREE::get_abone_list_regex( const std::string& url )
{
    return DBTREE::get_article( url )->get_abone_list_regex();
}


std::vector< char > DBTREE::get_abone_vec_res( const std::string& url )
{
    return DBTREE::get_article( url )->get_abone_vec_res();
}


void DBTREE::reset_abone( const std::string& url, std::list< std::string >& ids, std::list< std::string >& names
                          , std::list< std::string >& words, std::list< std::string >& regexs
                          , std::vector< char >& vec_abone_res
                          , bool transparent, bool chain)
{
    DBTREE::get_article( url )->reset_abone( ids, names, words, regexs, vec_abone_res, transparent, chain );
}


void DBTREE::set_abone_res( const std::string& url, const int number, const bool set )
{
    DBTREE::get_article( url )->set_abone_res( number, set );
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


const bool DBTREE::get_abone_transparent( const std::string& url )
{
    return DBTREE::get_article( url )->get_abone_transparent();
}


void DBTREE::set_abone_transparent( const std::string& url, bool set )
{
    DBTREE::get_article( url )->set_abone_transparent( set );
}


const bool DBTREE::get_abone_chain( const std::string& url )
{
    return DBTREE::get_article( url )->get_abone_chain();
}


void DBTREE::set_abone_chain( const std::string& url, bool set )
{
    DBTREE::get_article( url )->set_abone_chain( set );
}


const bool DBTREE::is_bookmarked( const std::string& url, int number )
{
    return DBTREE::get_article( url )->is_bookmarked( number );
}


void DBTREE::set_bookmark( const std::string& url, int number, bool set )
{
    DBTREE::get_article( url )->set_bookmark( number, set );
}

