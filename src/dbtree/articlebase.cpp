// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "articlebase.h"
#include "nodetreebase.h"
#include "interface.h"

#include "skeleton/msgdiag.h"

#include "jdlib/miscutil.h"
#include "jdlib/misctime.h"
#include "jdlib/miscmsg.h"
#include "jdlib/jdregex.h"
#include "jdlib/tfidf.h"

#include "dbimg/imginterface.h"

#include "config/globalconf.h"

#include "httpcode.h"
#include "command.h"
#include "cache.h"
#include "global.h"
#include "login2ch.h"
#include "session.h"
#include "updatemanager.h"

#include <sstream>


using namespace DBTREE;


// 情報ファイルのパスをセット
// デストラクタの中でCACHE::path_article_ext_info()などを呼ぶとabortするので
// メンバ変数としてパスを取得しておく
#define SET_INFOPATH() \
do{ \
m_path_article_info = CACHE::path_article_info( m_url, m_id ); \
m_path_article_ext_info = CACHE::path_article_ext_info( m_url, m_id ); \
} while( false )


// 情報ファイルから値を読み込み
// JDLIB::ConfLoader では遅いので別に作成。オプションの順番に注意すること
#define GET_INFOVALUE(target,targetname) \
do { \
target = std::string(); \
const int n = strlen( targetname ); \
it2 = it; \
while( it2 != lines.end() && ( ( *it2 ).c_str()[ 0 ] != targetname[ 0 ] || strncmp( ( *it2 ).c_str(), targetname, n ) != 0 ) ) ++it2; \
if( it2 != lines.end() ){ \
    target = ( *it2 ).substr( n ); \
    it = ++it2; \
} } while( false )




ArticleBase::ArticleBase( const std::string& datbase, const std::string& id, bool cached )
    : SKELETON::Lockable(),
      m_id ( id ),
      m_key( std::string() ),
      m_since_time( 0 ),
      m_since_date( std::string() ),
      m_code( HTTP_INIT ),
      m_str_code( std::string() ),
      m_status( STATUS_UNKNOWN ),
      m_subject( std::string() ),
      m_number( 0 ),
      m_number_diff( 0 ),
      m_number_new( 0 ),
      m_number_load( 0 ),
      m_number_before_load( 0 ),
      m_number_seen( 0 ),
      m_number_max( 0 ),
      m_write_fixname( 0 ),
      m_write_fixmail( 0 ),
      m_abone_transparent( false ),
      m_abone_chain( false ),
      m_abone_age( false ),
      m_abone_board( true ),
      m_abone_global( true ),
      m_bookmarked_thread( false ),
      m_cached( cached ),
      m_read_info( 0 ),
      m_save_info( false ),
      m_924( false )
{
#ifdef _DEBUG
//    std::cout << "ArticleBase::ArticleBase : " << m_id << std::endl;
#endif

    memset( &m_access_time, 0, sizeof( struct timeval ) );
    memset( &m_check_update_time, 0, sizeof( struct timeval ) );
    memset( &m_write_time, 0, sizeof( struct timeval ) );

    // m_url にURLセット
    update_datbase( datbase );

    // この段階では移転前の旧ホスト名は分からないのでとりあえず現在のホスト名をセットしておく
    // あとで BoardBase::url_dat()でURLを変換する時に旧ホスト名を教えてもらってinfoファイルに保存しておく。
    // BoardBase::url_dat()も参照せよ
    m_org_host = MISC::get_hostname( m_url );
}


ArticleBase::~ArticleBase()
{
#ifdef _DEBUG
//    std::cout << "ArticleBase::~ArticleBase : " << m_id << std::endl;
#endif

    // 参照ロックが外れていない
    assert( get_lock() == 0 );

    // nodetreeのクリアと情報保存
    unlock_impl();
}


bool ArticleBase::empty()
{
    return  m_url.empty();
}


// ID がこのスレのものかどうか
bool ArticleBase::equal( const std::string& datbase, const std::string& id )
{
    return ( id == m_id );
}


//
// 移転する前のオリジナルのURL
//
std::string ArticleBase::get_org_url()
{
    std::string newhost = MISC::get_hostname( m_url );
    return m_org_host + m_url.substr( newhost.length() );
}


// スレ立て時刻( string型 )
const std::string& ArticleBase::get_since_date()
{
    if( m_since_date.empty()
        || SESSION::get_col_since_time() == MISC::TIME_PASSED ){

        m_since_date = MISC::timettostr( get_since_time(), SESSION::get_col_since_time() );
    }

    return m_since_date;
}


// スレ速度
int ArticleBase::get_speed()
{
    time_t current_t = time( nullptr );
    return ( get_number() * 60 * 60 * 24 ) / MAX( 1, current_t - get_since_time() );
}


// キャッシュにあるdatファイルのサイズ
size_t ArticleBase::get_lng_dat()
{
    return get_nodetree()->get_lng_dat();
}



//
// nodetree の number 番のレスのヘッダノードのポインタを返す
//
NODE* ArticleBase::res_header( int number )
{
    return get_nodetree()->res_header( number );
}


//
// number番の名前
//
std::string ArticleBase::get_name( int number )
{
    return get_nodetree()->get_name( number );
}


//
// number番の名前の重複数( = 発言数 )
//
int ArticleBase::get_num_name( int number )
{
    return get_nodetree()->get_num_name( number );
}


//
// 指定した発言者の名前のレス番号をリストにして取得
//
std::list< int > ArticleBase::get_res_name( const std::string& name )
{
    return get_nodetree()->get_res_name( name );
}


//
// number番のレスの時刻を文字列で取得
// 内部で regex　を使っているので遅い
//
std::string ArticleBase::get_time_str( int number )
{
    return get_nodetree()->get_time_str( number );
}


//
// number番の発言者ID
//
std::string ArticleBase::get_id_name( int number )
{
    return get_nodetree()->get_id_name( number );
}


// 指定した発言者ID の重複数( = 発言数 )
// 下のnum_id_name( int number )と違って検索するので遅い
int ArticleBase::get_num_id_name( const std::string& id )
{
    return get_nodetree()->get_num_id_name( id );
}



// number番の発言者ID の重複数( = 発言数 )
int ArticleBase::get_num_id_name( int number )
{
    return get_nodetree()->get_num_id_name( number );
}


// 指定した発言者IDを持つレス番号をリストにして取得
std::list< int > ArticleBase::get_res_id_name( const std::string& id_name )
{
    return get_nodetree()->get_res_id_name( id_name );
}


// str_num で指定したレス番号をリストにして取得
// str_num は "from-to"　の形式 (例) 3から10をセットしたいなら "3-10"
// list_jointは出力で true のスレは前のスレに連結される (例) "3+4" なら 4が3に連結
std::list< int > ArticleBase::get_res_str_num( const std::string& str_num, std::list< bool >& list_joint )
{
    if( empty() ){ std::list< int > tmp; return tmp; }

    return get_nodetree()->get_res_str_num( str_num, list_joint );
}


// ブックマークをつけたレス番号をリストにして取得
std::list< int > ArticleBase::get_res_bm()
{
    std::list< int > list_resnum;          
    for( int i = 1; i <= m_number_load ; ++i ){
        if( is_bookmarked( i ) ) list_resnum.push_back( i );
    }

    // あぼーんしていてもリストから取り除かない
    return list_resnum;
}


// 書き込みしたレス番号をリストにして取得
std::list< int > ArticleBase::get_res_posted()
{
    std::list< int > list_resnum;          
    for( int i = 1; i <= m_number_load ; ++i ){
        if( is_posted( i ) ) list_resnum.push_back( i );
    }

    // あぼーんしていてもリストから取り除かない
    return list_resnum;
}

// 高参照レスをリストにして取得
std::list< int > ArticleBase::get_highly_referenced_res()
{
    return get_nodetree()->get_highly_referened_res();
}

//
// number番のレスを参照しているレス番号をリストにして取得
//
std::list< int > ArticleBase::get_res_reference( const int number )
{
    return get_nodetree()->get_res_reference( number );
}


//
// res_num に含まれるレスを参照しているレス番号をリストにして取得
//
std::list< int > ArticleBase::get_res_reference( const std::list< int >& res_num )
{
    return get_nodetree()->get_res_reference( res_num );
}


//
// URL を含むレス番号をリストにして取得
//
std::list< int > ArticleBase::get_res_with_url()
{
    if( empty() ){ std::list< int > tmp; return tmp; }

    return get_nodetree()->get_res_with_url();
}


//
// query を含むレス番号をリストにして取得
//
// mode_or == true なら OR抽出
//
std::list< int > ArticleBase::get_res_query( const std::string& query, const bool mode_or )
{
    if( empty() ){ std::list< int > tmp; return tmp; }

    return get_nodetree()->get_res_query( query, mode_or );
}


//
// number番のレスの文字列を返す
// ref == true なら先頭に ">" を付ける        
//
std::string ArticleBase::get_res_str( int number, bool ref )
{
    return get_nodetree()->get_res_str( number, ref );
}


//
// 更新時刻
//
time_t ArticleBase::get_time_modified()
{
    time_t time_out;
    time_out = MISC::datetotime( m_date_modified );
    if( time_out == 0 ) time_out = time( nullptr ) - 600;
    return time_out; 
}



// スレが立ってからの経過時間( 時間 )
int ArticleBase::get_hour()
{
    return ( time( nullptr ) - get_since_time() ) / ( 60 * 60 );
}


//
// url の更新
//
// 移転があったときなどに上位クラスのboardbaseから呼ばれる
//
void ArticleBase::update_datbase( const std::string& datbase )
{
    if( m_id.empty() ) return;

#ifdef _DEBUG
    std::string old_url = m_url;
#endif

    // URL 更新
    m_datbase = datbase;
    m_url = datbase + m_id;

    // info ファイルのパスも更新
    if( ! m_path_article_info.empty() ) SET_INFOPATH();

#ifdef _DEBUG
    if( !old_url.empty() ) std::cout << "ArticleBase::update_datbase from "  << old_url
                                     << " to " << m_url << std::endl;
#endif

    if( m_nodetree ) m_nodetree->update_url( m_url );
}



//
// 移転前のオリジナルのホスト名をセット
//
void ArticleBase::set_org_host( const std::string& host )
{
    if( host != m_org_host ){

#ifdef _DEBUG
    std::cout << "ArticleBase::set_org_host : " << m_id << std::endl
              << "m_url = " << m_url << std::endl
              << "org_host = " << m_org_host
              << " -> " << host << std::endl;
#endif

        m_org_host = host;
        m_save_info = true;
    }
}




//
// access_time を 文字列に変換して返す
//
std::string ArticleBase::get_access_time_str()
{
    return MISC::timevaltostr( m_access_time );
}


//
// ユーザが最後にロードした月日( string型 )
//
const std::string& ArticleBase::get_access_date()
{
    if( m_access_time.tv_sec ){

        if( m_access_date.empty() 
            || SESSION::get_col_access_time() == MISC::TIME_PASSED ){
        
            m_access_date = MISC::timettostr( m_access_time.tv_sec, SESSION::get_col_access_time() );
        }
    }

    return m_access_date;
}


void ArticleBase::reset_status()
{
#ifdef _DEBUG
    std::cout << "ArticleBase::reset_status\n";
#endif

    m_status = STATUS_UNKNOWN;
}


void ArticleBase::set_subject( const std::string& subject )
{
    if( subject.empty() ) return;

    // 特殊文字の置き換え
    if( subject.find( "&" ) != std::string::npos ){

        std::string subject_tmp = MISC::html_unescape( subject );

        if( subject_tmp != m_subject ){
            m_subject = subject_tmp;
            m_save_info = true;
        }
    }
    else if( subject != m_subject ){

        m_subject = subject;
        m_save_info = true;
    }
}


void ArticleBase::set_number( const int number, const bool is_online )
{
    if( ! number ) return;

    m_number_diff = 0;
    m_status &= ~STATUS_BROKEN_SUBJECT;

    if( number > m_number ){

        m_number_diff = number - m_number;
        m_number = number;

        // キャッシュがあって更新可能になった場合は
        // お気に入りとスレビューのタブのアイコンに更新マークを表示
        if( is_cached() && !( m_status & STATUS_UPDATE ) && m_number_load < m_number ) show_updateicon( true );
    }

    // subject.txt に示されたレス数よりも実際の取得数の方が多い
    else if( is_online && number < m_number ){
#ifdef _DEBUG
        std::cout << "ArticleBase::set_number : broken_subject " << get_subject() << " "
                  << number << " / " << m_number << std::endl;
#endif
        m_status |= STATUS_BROKEN_SUBJECT;
    }
}


void ArticleBase::set_number_load( const int number_load )
{
    if( number_load && number_load != m_number_load ) m_number_load = number_load;
}


void ArticleBase::set_number_seen( const int number_seen )
{
    if( number_seen && number_seen != m_number_seen ){
        m_number_seen = number_seen;
        m_save_info = true;
    }
}


// 最終書き込み時間( string型 )
const std::string& ArticleBase::get_write_date()
{
    if( m_write_time.tv_sec ){

        if( m_write_time_date.empty() 
            || SESSION::get_col_write_time() == MISC::TIME_PASSED ){
        
            m_write_time_date = MISC::timettostr( m_write_time.tv_sec, SESSION::get_col_write_time() );
        }
    }

    return m_write_time_date;
}


//
// 書き込み時間更新
//
void ArticleBase::update_writetime()
{
    struct timeval tv;
    struct timezone tz;
    if( CONFIG::get_save_post_history() && gettimeofday( &tv, &tz ) == 0 ){

        m_write_time = tv;
        m_write_time_date = std::string();

#ifdef _DEBUG
        std::cout << "ArticleBase::update_writetime : " << m_write_time.tv_sec << " " << m_write_time_date << std::endl;
#endif
        m_save_info = true;

        // BoardViewの行を更新
        CORE::core_set_command( "update_board_item", DBTREE::url_subject( m_url ), m_id );
    }

    // 板の書き込み時間を更新
    DBTREE::board_update_writetime( m_url );
}


//
// 書き込み数
//
int ArticleBase::get_num_posted()
{
    return m_posts.size();
}


//
// スレ自体のスレ一覧でのブックマーク
//
void ArticleBase::set_bookmarked_thread( const bool bookmarked )
{
    m_bookmarked_thread = bookmarked;
    save_info( true );

    // BoardViewの行を更新
    CORE::core_set_command( "update_board_item", DBTREE::url_subject( m_url ), m_id );
}


//
// キャッシュがあって、かつ新着の読み込みが可能
//
bool ArticleBase::enable_load()
{
    return ( is_cached() && ( m_status & STATUS_UPDATE ) && ! ( m_status & STATUS_OLD ) );
}


//
// キャッシュはあるが規定のレス数を越えていて、かつ全てのレスが既読
//
bool ArticleBase::is_finished()
{
    if( is_cached() && ! enable_load() &&  m_number_max && get_number_seen() >= m_number_max ){

#ifdef _DEBUG
        std::cout << "ArticleBase::is_finished :  seen = " << get_number_seen() << " max = " << m_number_max << " : " << get_subject() << std::endl;
#endif

        return true;
    }

    return false;
}


//
// 透明あぼーん
//
bool ArticleBase::get_abone_transparent()
{
    if( CONFIG::get_abone_transparent() ) return true;

    return m_abone_transparent;
}


//
// 連鎖あぼーん
//
bool ArticleBase::get_abone_chain()
{
    if( CONFIG::get_abone_chain() ) return true;

    return m_abone_chain;
}


//
// あぼーんしてるか
//
bool ArticleBase::get_abone( int number )
{
    return get_nodetree()->get_abone( number );
}



//
// 全レスのあぼーん状態の更新
//
// あぼーん情報を変更したら呼び出す
//
// あぼーんしたスレの発言数や参照はカウントしないので、発言数や参照数も更新する
//
void ArticleBase::update_abone()
{
    // nodetreeが作られていないときは更新しない
    if( ! m_nodetree ) return;

    get_nodetree()->copy_abone_info( m_list_abone_id, m_list_abone_name, m_list_abone_word, m_list_abone_regex, m_abone_reses,
                                     m_abone_transparent, m_abone_chain, m_abone_age, m_abone_board, m_abone_global );

    get_nodetree()->update_abone_all();
}



//
// あぼーん状態のリセット(情報セットと状態更新)
//
void ArticleBase::reset_abone( const std::list< std::string >& ids,
                               const std::list< std::string >& names,
                               const std::list< std::string >& words,
                               const std::list< std::string >& regexs,
                               const std::vector< char >& vec_abone_res,
                               const bool transparent, const bool chain, const bool age,
                               const bool board, const bool global
    )
{
    if( empty() ) return;

#ifdef _DEBUG
    std::cout << "ArticleBase::reset_abone\n";
#endif

    // 前後の空白と空白行を除く

    m_list_abone_id = MISC::remove_space_from_list( ids );
    m_list_abone_id = MISC::remove_nullline_from_list( m_list_abone_id );

    m_list_abone_name = MISC::remove_space_from_list( names );
    m_list_abone_name = MISC::remove_nullline_from_list( m_list_abone_name );

    m_list_abone_word = MISC::remove_space_from_list( words );
    m_list_abone_word = MISC::remove_nullline_from_list( m_list_abone_word );

    m_list_abone_regex = MISC::remove_space_from_list( regexs );
    m_list_abone_regex = MISC::remove_nullline_from_list( m_list_abone_regex );

    if( vec_abone_res.size() ){

        for( int i = 1; i <= MIN( m_number_load, (int)vec_abone_res.size() ) ; ++i ){
            if( vec_abone_res[ i ] ) {
                m_abone_reses.insert( i );
            }
            else {
                m_abone_reses.erase( i );
            }
        }
    }
    
    m_abone_transparent = transparent;
    m_abone_chain = chain;
    m_abone_age = age;
    m_abone_board = board;
    m_abone_global = global;

    update_abone();

    m_save_info = true;
}



//
// あぼーんID追加
//
void ArticleBase::add_abone_id( const std::string& id )
{
    if( empty() ) return;
    if( id.empty() ) return;

#ifdef _DEBUG
    std::cout << "ArticleBase::add_abone_id : " << id << std::endl;
#endif    

    std::string id_tmp = id.substr( strlen( PROTO_ID ) );

    m_list_abone_id.push_back( id_tmp );

    update_abone();

    m_save_info = true;
}


//
// あぼーん名前追加
//
void ArticleBase::add_abone_name( const std::string& name )
{
    if( empty() ) return;
    if( name.empty() ) return;

#ifdef _DEBUG
    std::cout << "ArticleBase::add_abone_name : " << name << std::endl;
#endif    

    m_list_abone_name.push_back( name );

    update_abone();

    m_save_info = true;
}



//
// あぼーん文字列追加
//
void ArticleBase::add_abone_word( const std::string& word )
{
    if( empty() ) return;
    if( word.empty() ) return;

#ifdef _DEBUG
    std::cout << "ArticleBase::add_abone_word : " << word << std::endl;
#endif    

    m_list_abone_word.push_back( word );

    update_abone();

    m_save_info = true;
}


//
// レスあぼーんのセット
//
void ArticleBase::set_abone_res( const int num_from, const int num_to, const bool set )
{
    if( empty() ) return;
    if( num_from > num_to ) return;
    if( num_from <= 0 || num_to > CONFIG::get_max_resnumber() ) return;

#ifdef _DEBUG
    std::cout << "ArticleBase::set_abone_res num_from = " << num_from << " num_to = " << num_to << " set = " << set << std::endl;
#endif    

    if( set ) {
        for( int i = num_from; i <= num_to; ++i ) m_abone_reses.insert( i );
    }
    else {
        for( int i = num_from; i <= num_to; ++i ) m_abone_reses.erase( i );
    }

    update_abone();

    m_save_info = true;
}



//
// 透明あぼーん更新
//
void ArticleBase::set_abone_transparent( const bool set )
{
    if( empty() ) return;

    m_abone_transparent = set;

    update_abone();

    m_save_info = true;
} 


//
// 連鎖あぼーん更新
//
void ArticleBase::set_abone_chain( const bool set )
{
    if( empty() ) return;

    m_abone_chain = set;

    update_abone();

    m_save_info = true;
} 


//
// ageあぼーん更新
//
void ArticleBase::set_abone_age( const bool set )
{
    if( empty() ) return;

    m_abone_age = set;

    update_abone();

    m_save_info = true;
} 


//
// 板レベルでのあぼーんを有効にする
//
void ArticleBase::set_abone_board( const bool set )
{
    if( empty() ) return;

    m_abone_board = set;

    update_abone();

    m_save_info = true;
} 


//
// 全体レベルでのあぼーんを有効にする
//
void ArticleBase::set_abone_global( const bool set )
{
    if( empty() ) return;

    m_abone_global = set;

    update_abone();

    m_save_info = true;
} 


//
// ブックマークの数
//
int ArticleBase::get_num_bookmark()
{
    return m_bookmarks.size();
}


//
// ブックマークされているか
//
bool ArticleBase::is_bookmarked( const int number )
{
    if( number <= 0 || number > m_number_load ) return false;

    // まだnodetreeが作られてなくてブックマークの情報が得られてないのでnodetreeを作って情報取得
    if( m_bookmarks.empty() ) get_nodetree();

    return ( m_bookmarks.find( number ) != m_bookmarks.end() );
}


//
// ブックマークセット
//
void ArticleBase::set_bookmark( const int number, const bool set )
{
    if( m_bookmarks.empty() ) get_nodetree();
    if( number <= 0 || number > CONFIG::get_max_resnumber() ) return;

    m_save_info = true;
    if( set ) {
        m_bookmarks.insert( number );
    }
    else {
        m_bookmarks.erase( number );
    }
}


//
// 自分が書き込んだレスか
//
bool ArticleBase::is_posted( const int number )
{
    if( number <= 0 || number > m_number_load ) return false;

    // まだnodetreeが作られてなくて情報が得られてないのでnodetreeを作って情報取得
    if( m_posts.empty() ) get_nodetree();

    return ( m_posts.find( number ) != m_posts.end() );
}


// 自分の書き込みにレスしたか
bool ArticleBase::is_refer_posted( const int number )
{
    return get_nodetree()->is_refer_posted( number );
}


// 書き込みマークセット
void ArticleBase::set_posted( const int number, const bool set )
{
    if( number <= 0 || number > m_number_load ) return;

    // まだnodetreeが作られてなくて情報が得られてないのでnodetreeを作って情報取得
    if( m_posts.empty() ) get_nodetree();

    m_save_info = true;
    if( set ) {
        m_posts.insert( number );
    }
    else {
        m_posts.erase( number );
    }

    // nodetreeに情報反映
    m_nodetree->set_posted( number, set );
}


// 書き込み履歴のリセット
void ArticleBase::clear_post_history()
{
    if( empty() ) return;
    if( ! is_cached() ) return;

    read_info();
    if( !m_posts.empty() || m_write_time.tv_sec || m_write_time.tv_usec ){

#ifdef _DEBUG
        std::cout << "ArticleBase::clear_post_history size = " << m_posts.size()
                  << " time = " << m_write_time_date
                  << " subject = " << m_subject << std::endl;
#endif
        m_posts.clear();
        memset( &m_write_time, 0, sizeof( struct timeval ) );
        m_write_time_date = std::string();

        // nodetreeが作られている時はnodetreeもリセット
        if( m_nodetree ) m_nodetree->clear_post_history();

        m_write_name = std::string();
        m_write_mail = std::string();
        m_write_fixname = false;
        m_write_fixmail = false;

        save_info( true );
    }
}


// 新着返信レス取得
const std::set<int>& ArticleBase::get_refer_posts_from_newres ()
{
    return get_nodetree()->get_refer_posts_from_newres();
}


//
// NodeTree作成
//
// もしNodeTreeが作られていなかったらここでNewする
//
// this の参照が無くなったら ArticleBase::unlock_impl()が呼ばれて m_nodetree は自動クリアされる
//
JDLIB::ConstPtr< NodeTreeBase >& ArticleBase::get_nodetree()
{
    assert( !empty() );

    if( ! m_nodetree ){

#ifdef _DEBUG
        std::cout << "ArticleBase::get_nodetree create " << m_url << std::endl;
#endif
    
        m_nodetree = create_nodetree();
        assert( m_nodetree );

        // あぼーん情報のコピー
        m_nodetree->copy_abone_info( m_list_abone_id, m_list_abone_name, m_list_abone_word, m_list_abone_regex, m_abone_reses,
                                     m_abone_transparent, m_abone_chain, m_abone_age, m_abone_board, m_abone_global );

        // 書き込み情報のコピー
        m_nodetree->copy_post_info( m_posts );

        m_nodetree->sig_updated().connect( sigc::mem_fun( *this, &ArticleBase::slot_node_updated ) );
        m_nodetree->sig_finished().connect( sigc::mem_fun( *this, &ArticleBase::slot_load_finished ) );

        // キャッシュ読み込み
        m_number_load = 0; // 読み込み数リセット
        m_nodetree->load_cache();
    }
    
    return m_nodetree;
}



//
// this の参照ロックが外れたときに呼ばれる
//
// m_nodetree を deleteする
//
void ArticleBase::unlock_impl()
{
    if( !m_nodetree ) return;

#ifdef _DEBUG
    std::cout << "ArticleBase::unlock_impl  url = " << m_url << std::endl;
#endif

    m_nodetree->terminate_load(); // deleteする前にスレッド停止

    // スレ情報保存 	 
    save_info( false );

    m_nodetree.clear();
}





//
// ロード中か
//
bool ArticleBase::is_loading() const
{
    if( ! m_nodetree ) return false;
    return m_nodetree->is_loading();
}


//
// 更新チェック中か
//
bool ArticleBase::is_checking_update() const
{
    if( ! is_loading() ) return false;

    return m_nodetree->is_checking_update();
}


//
// ロード停止
//
void ArticleBase::stop_load()
{
    if( ! m_nodetree ) return;
    m_nodetree->stop_load();
}



//
// スレッドのロード開始
//
// DAT落ちの場合はロードしないので、強制的にリロードしたいときは reset_status() で
// ステータスをリセットしてからロードする
//
// check_update == true の時はHEADによる更新チェックをおこなう
//
void ArticleBase::download_dat( const bool check_update )
{
    if( empty() ) return;

#ifdef _DEBUG
    std::cout << "ArticleBase::download_dat " << m_url << " status = " << m_status
              << " checkupdate = " << check_update << std::endl
              << "url_pre_article = " << m_url_pre_article << std::endl;
#endif

    struct timeval tv;
    struct timezone tz;
    if( gettimeofday( &tv, &tz ) != 0 ) tv.tv_sec = 0;

    // 更新チェック可能か判定する
    if( check_update ){

        // 一度更新チェックしたらしばらくは再チェックできないようにする
        time_t passed = 0;
        if( tv.tv_sec ) passed = MAX( 0, tv.tv_sec - m_check_update_time.tv_sec );

        if( ! SESSION::is_online()
            || ! enable_check_update()
            || is_loading()
            || enable_load() // 既に新着あり状態の時はチェックしない
            || ( m_status & STATUS_OLD )
            || ( passed <= CHECKUPDATE_MINSEC )
            )
        {
#ifdef _DEBUG
            std::cout << "skipped : passed = " << passed << " enable_load = " << enable_load()
                      << " loading = " << is_loading()
                      << std::endl;
#endif

            // スレビューのタブとサイドバーのアイコン表示を更新

            // JDが異常終了すると、お気に入りに+が付いてないのにスレが更新可能状態になっていて
            // 更新チェックが行われてなくなるときがある。このバグは次の様にして再現出来た
            // (1) 端末からJDを起動する
            // (2) 適当な板のスレ一覧を開く
            // (3) お気に入りに適当なスレを登録
            // (4) そのスレを読み込む。読み込んだらスレビューは閉じないままにしておく
            // (5) スレ一覧を更新してそのスレに + マークを付ける。お気に入りにも + が付く
            // (6) そのスレのスレビューを閉じる ( スレ情報を保存 )
            // (7) 端末で Ctrl+c を押してJDを強制終了
            // (8) 再起動してお気に入りの復元をしない
            // (9) お気に入りの + が消えている。以後更新チェックも行われない

            CORE::core_set_command( "toggle_article_icon", m_url);
            CORE::core_set_command( "toggle_sidebar_articleicon", m_url );

            return;
        }
    }

    if( SESSION::is_online() && tv.tv_sec ) m_check_update_time = tv;

    // DAT落ちしていてロードしない場合
    if( ( m_status & STATUS_OLD ) && ! is_load_olddat() ) {
#ifdef _DEBUG
        std::cout << "old !\n";
#endif       
        CORE::core_set_command( "toggle_sidebar_articleicon", m_url );

        // update_article_finish コマンドを送らないとキャッシュが無くて
        // dat落ちしているスレのタブが空白になる
        CORE::core_set_command( "update_article_finish", m_url );
        return;
    }

#ifdef _DEBUG
    std::cout << "start\n";
#endif       
    get_nodetree()->download_dat( check_update );
}


//
// 前スレのアドレスの指定
//
// 前スレのアドレスをセットしてからdownload_dat()を呼び出すと
// ロード終了時( slot_load_finished() )に次スレ移行チェックをする
//
void ArticleBase::set_url_pre_article( const std::string& url_pre_article )
{
#ifdef _DEBUG
    std::cout << "ArticleBase::set_url_pre_article url = " << url_pre_article << std::endl;
#endif

    m_url_pre_article = url_pre_article;

    if( ! m_url_pre_article.empty() ){

        // 既読スレや板違いのスレに対しては移行チェックはしない
        if( m_number_load
            || m_datbase != DBTREE::url_datbase( m_url_pre_article )
            || get_since_time() < DBTREE::article_since_time( m_url_pre_article )
            ) m_url_pre_article = std::string();
    }

    // TFIDFによる類似度判定のためオフラインに切り替えてからキャッシュにあるsubject.txtを読み込む
    if( ! m_url_pre_article.empty() && ! DBTREE::board_list_subject( m_url ).size() ){

#ifdef _DEBUG
        std::cout << "load subjects\n";
#endif

        const bool online = SESSION::is_online();
        SESSION::set_online( false );

        DBTREE::board_download_subject( m_url, std::string() );

        SESSION::set_online( online );
    }
}


//
// url_src で示されるスレの情報を引き継ぐ
//
void ArticleBase::copy_article_info( const std::string& url_src )
{
    if( url_src.empty() ) return;
    if( ! DBTREE::article_is_cached( url_src ) ) return;

    // 名前、メール
    m_write_fixname = DBTREE::write_fixname( url_src );
    m_write_name = DBTREE::write_name( url_src );

    m_write_fixmail = DBTREE::write_fixmail( url_src );
    m_write_mail = DBTREE::write_mail( url_src );

    // あぼーん関係
    std::list< std::string > ids;
    std::list< std::string > names = DBTREE::get_abone_list_name( url_src );
    std::list< std::string > words = DBTREE::get_abone_list_word( url_src );
    std::list< std::string > regexs = DBTREE::get_abone_list_regex( url_src );
    std::vector< char > vec_abone_res;
    const bool transparent = DBTREE::get_abone_transparent( url_src );
    const bool chain = DBTREE::get_abone_chain( url_src );
    const bool age = DBTREE::get_abone_age( url_src );
    const bool board = DBTREE::get_abone_board( url_src );
    const bool global = DBTREE::get_abone_global( url_src );

    reset_abone( ids, names ,words, regexs, vec_abone_res, transparent, chain, age, board, global );
}


//
// ロード中など nodetree の構造が変わったときにnodetreeから呼ばれる slot
//
void ArticleBase::slot_node_updated()
{
    assert( m_nodetree );

    // 更新チェック中
    if( m_nodetree->is_checking_update() ) return;

#ifdef _DEBUG
    std::cout << "ArticleBase::slot_node_updated" << std::endl;
#endif

    // nodetreeから情報取得
    if( ! m_nodetree->get_subject().empty() ) set_subject( m_nodetree->get_subject() );

    // スレが更新している場合
    if( m_number_load != m_nodetree->get_res_number() ){

        // スレの読み込み数更新
        m_number_load = m_nodetree->get_res_number();

        // 対応するarticleビューを更新
        CORE::core_set_command( "update_article", m_url );
    }
}


//
// ロード終了後に nodetree から呼ばれる slot
// nodetree から情報を取得する
//
void ArticleBase::slot_load_finished()
{
    assert( m_nodetree );

#ifdef _DEBUG
    std::cout << "ArticleBase::slot_load_finished" << std::endl;
#endif

    slot_node_updated();

    // HTTPコード取得
    const int old_code = m_code;
    m_code = m_nodetree->get_code();
    m_status &= ~STATUS_UPDATED;

    // 状態更新
    const int old_status = m_status;
    if( m_code != HTTP_ERR ){

        // DAT落ち
        if( m_code == HTTP_MOVED_PERM || m_code == HTTP_REDIRECT || m_code == HTTP_NOT_FOUND || m_code == HTTP_OLD ){
            m_status &= ~STATUS_NORMAL;
            m_status |= STATUS_OLD;
            CORE::core_set_command( "toggle_sidebar_articleicon", m_url );
        }

        // 既にDAT落ち状態では無いときは通常状態にする
        else if( ! (m_status & STATUS_OLD ) ){
            m_status |= STATUS_NORMAL;
            m_status &= ~STATUS_OLD;
        }
    }

    // 壊れている
    if( m_nodetree->is_broken() ) m_status |= STATUS_BROKEN;

    // レス数が最大表示可能数以上か
    if( get_number_load() >= CONFIG::get_max_resnumber() )
        m_status |= STATUS_OVERFLOW;
    else {
        m_status &= ~STATUS_OVERFLOW;
    }

    // 状態が変わっていたら情報保存
    if( old_status != m_status ) m_save_info = true;

    // 更新チェック
    if( m_nodetree->is_checking_update() ){

#ifdef _DEBUG
        std::cout << "check_update code = " << m_code << std::endl;
#endif

        // スレタブとお気に入りとスレ一覧のアイコンに更新マークをつける
        if( m_code == HTTP_OK // まちBBSは206が返らない(200か304のみ)
            || m_code == HTTP_PARTIAL_CONTENT ){

            show_updateicon( true );

            // このスレが所属する板を更新可能状態にしてお気に入りやスレ一覧のタブのアイコンに更新マークを表示
            DBTREE::board_show_updateicon( m_url, true );

            // スレ一覧の ! 行のアイコンを更新マークにする
            CORE::core_set_command( "update_board_item", DBTREE::url_subject( m_url ), m_id );
        }

        // code と modified を戻しておく
        m_code = old_code;
        m_nodetree->set_date_modified( m_date_modified );

#ifdef _DEBUG
        std::cout << "check_update done\n";
#endif

        // 更新チェック時間を保存
        save_info( true );

        // 次のスレを更新チェック
        CORE::get_checkupdate_manager()->pop_front();

        return;
    }

    // nodetreeから情報取得
    m_str_code = m_nodetree->get_str_code();
    std::string m_old_modified = m_date_modified;
    m_date_modified = m_nodetree->get_date_modified();
    if( m_number_before_load < m_number_load ) m_number_new = m_number_load - m_number_before_load;
    else m_number_new = 0;

    // 書き込み情報
    const auto& node_posts = m_nodetree->get_posts();
    if( m_number_new && node_posts.size() ) {

        const auto end = m_posts.end();
        const auto node_end = node_posts.end();
        (void)end; // _DEBUGが定義されていないときの警告抑制
        for( int i = m_number_before_load +1; i <= m_number_load; ++i ){

            if( node_posts.find( i ) != node_end ) {
                m_posts.insert( i );
            }
            else {
                m_posts.erase( i );
            }
#ifdef _DEBUG
            if( m_posts.find( i ) != end ) std::cout << "posted no = " << i << std::endl;
#endif
        }
    }

    // 次スレチェック
    bool relayout = false;
    if( m_number_load && ! m_url_pre_article.empty() && ! m_subject.empty() ){

        const std::string pre_subject = DBTREE::article_subject( m_url_pre_article );
        if( ! pre_subject.empty() ){

#ifdef _DEBUG
            std::cout << "check next\n";
#endif
            int value = 0;

            const std::vector< DBTREE::ArticleBase* >& list_subject = DBTREE::board_list_subject( m_url );

            // subjectがキャッシュにある場合は TFIDF を使って類似度チェック
            if( list_subject.size() ){
#ifdef _DEBUG
                std::cout << "use tfidf\n";
#endif
                // 単語ベクトル作成
                MISC::VEC_WORDS vec_words;
                MISC::tfidf_create_vec_words( vec_words, pre_subject );

                // IDFベクトル計算
                MISC::VEC_IDF vec_idf;
                MISC::tfidf_create_vec_idf_from_board( vec_idf, pre_subject, list_subject, vec_words );

                // TFIDFベクトル計算
                MISC::VEC_TFIDF vec_tfidf_src;
                MISC::VEC_TFIDF vec_tfidf;
                vec_tfidf_src.resize( vec_words.size() );
                vec_tfidf.resize( vec_words.size() );
                MISC::tfidf_calc_vec_tfifd( vec_tfidf_src, pre_subject, vec_idf, vec_words );
                MISC::tfidf_calc_vec_tfifd( vec_tfidf, m_subject, vec_idf, vec_words );

                value = ( int )( MISC::tfidf_cos_similarity( vec_tfidf_src, vec_tfidf ) * 10 + .5 );
            }

            // subject がキャッシュに無い場合はレーベンシュタイン距離を使って類似度チェック
            else{
#ifdef _DEBUG
                std::cout << "use leven\n";
#endif
                const int MAXSTR = 256;
                std::vector< std::vector< int > > dist( MAXSTR, std::vector< int >( MAXSTR ) );
                value = 10 - ( int )( MISC::leven( dist, pre_subject, m_subject ) * 10 + .5 );
            }

#ifdef _DEBUG
            std::cout << "pre_subject = " << pre_subject << std::endl
                      << "subject = " << m_subject << std::endl
                      << "value = " << value << std::endl;
#endif

            // このスレは m_url_pre_article の次スレとみなして情報を引き継ぐ
            if( value >= CONFIG::get_threshold_next() ){

#ifdef _DEBUG
                std::cout << "hit!\n";
#endif
                copy_article_info( m_url_pre_article );

                // お気に入りのアドレスと名前を自動更新
                CORE::core_set_command( "replace_favorite_thread", "", m_url_pre_article, m_url );

                // 前スレにしおりがセットされていたらしおりをつける
                if( DBTREE::is_bookmarked_thread( m_url_pre_article ) ){
                    set_bookmarked_thread( true );
                }

                relayout = true;
            }
#ifdef _DEBUG
            else std::cout << "not hit\n";
#endif
        }

        m_url_pre_article = std::string();
    }

    m_number_before_load = m_number_load;
    m_ext_err = m_nodetree->get_ext_err();

    // スレの数が0ならスレ情報はセーブしない
    if( ! m_number_load ) m_cached = false;

    else{

        // スレ情報を更新
        if( m_number_new // スレが更新している場合

            || m_date_modified != m_old_modified // ときどき modified が誤って返るときがあるので最新の値を保存しておく

            || ( SESSION::is_online() && ( m_status & STATUS_UPDATE )
                 && ( m_code == HTTP_OK || m_code == HTTP_NOT_MODIFIED ) )  // 間違って更新可能マークが付いている場合はマークを消す

            ){

            m_cached = true;
            m_read_info = true;
            m_save_info = true;

            struct timeval tv;
            struct timezone tz;
            if( gettimeofday( &tv, &tz ) == 0 ) {
                m_access_time = tv;
                m_access_date = std::string();
            }

            if( m_number < m_number_load ) m_number = m_number_load;

            m_status |= STATUS_UPDATED;
            show_updateicon( false );

            // 情報ファイルのパスをセット
            if( m_path_article_info.empty() ) SET_INFOPATH();
        }
    }

#ifdef _DEBUG
    std::cout << "ArticleBase::slot_load_finished " << std::endl
              << "subject = " << m_subject << std::endl
              << "load = " << m_number_load << std::endl
              << "number = " << m_number << std::endl
              << "new = " << m_number_new << std::endl
              << "date = " << m_date_modified << std::endl
              << "access-time = " << get_access_time_str() << std::endl
              << "code = " << m_code << std::endl
              << "status = " << m_status << std::endl

    ;
#endif

    // 対応するBoardビューの行を更新
    CORE::core_set_command( "update_board_item", DBTREE::url_subject( m_url ), m_id );
    
    // articleビューに終了を知らせる
    CORE::core_set_command( "update_article", m_url );
    CORE::core_set_command( "update_article_finish", m_url );

    // あぼーん情報が前スレよりコピーされたので再レイアウト指定
    if( relayout ) CORE::core_set_command( "relayout_article", m_url );
}



//
// お気に入りのアイコンとスレビューのタブのアイコンに更新マークを表示
//
// update == true の時に表示。falseなら戻す
//
void ArticleBase::show_updateicon( const bool update )
{
#ifdef _DEBUG
    std::cout << "ArticleBase::show_updateicon url = " << m_url
              << " update = " << update << " status = " << ( m_status & STATUS_UPDATE ) << std::endl;
#endif

    struct timeval tv;
    struct timezone tz;
    if( gettimeofday( &tv, &tz ) == 0 ) m_check_update_time = tv;

    if( update ){

        if( ! ( m_status & STATUS_UPDATE ) ){

#ifdef _DEBUG
            std::cout << "toggle_icon on\n";
#endif

            m_save_info = true;
            m_status |= STATUS_UPDATE;

            // スレビューのタブとサイドバーのアイコン表示を更新
            CORE::core_set_command( "toggle_article_icon", m_url);
            CORE::core_set_command( "toggle_sidebar_articleicon", m_url );
        }
    }
    else{

        // この if をコメントアウトしないと
        //
        // スレ一覧を開いてお気に入りにあるスレに更新マークを付ける → スレ一覧を閉じる
        // 更新マークを付けたスレを開かないでJD終了(※) → 再起動してお気に入りで更新マークを付けたスレをクリック
        // → お気に入りのアイコン表示が戻らない
        //
        // という問題が生じる( ※ の所でスレ情報が保存されていないので再起動すると STATUS_UPDATE が外れるため。
        // 終了時にスレ情報を保存しようとすると終了処理が重くなる。)
//        if( m_status & STATUS_UPDATE ){

#ifdef _DEBUG
            std::cout << "toggle_icon off\n";
#endif

            m_save_info = true;
            m_status &= ~STATUS_UPDATE;

            // サイドバーのアイコン表示を戻す
            // スレビューのタブのアイコンはArticleViewがロード終了時に自動的に戻す
            CORE::core_set_command( "toggle_sidebar_articleicon", m_url );
//        }
    }
}


//
// キャッシュ削除
//
// cache_only == true の時はキャッシュだけ削除してスレ情報は消さない
//
void ArticleBase::delete_cache( const bool cache_only )
{
#ifdef _DEBUG
    std::cout << "ArticleBase::delete_cache  url = " << m_url << std::endl;
#endif

    if( empty() ) return;


    if( ! cache_only ){

        if( m_bookmarked_thread ){

            const std::string msg = "「" + get_subject() +
            "」にはしおりが付けられています。\n\nスレを削除しますか？\n\nしおりを解除するにはスレの上で右クリックしてしおり解除を選択してください。";

            SKELETON::MsgDiag mdiag( nullptr, msg, false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_YES_NO );
            mdiag.set_default_response( Gtk::RESPONSE_YES );
            if( mdiag.run() != Gtk::RESPONSE_YES ) return;
        }

        if( CONFIG::get_show_del_written_thread_diag() && m_write_time.tv_sec ){

            const std::string msg = "「" + get_subject() + "」には書き込み履歴が残っています。\n\nスレを削除しますか？";

            SKELETON::MsgCheckDiag mdiag( nullptr, msg,
                                          "今後表示しない(常に削除)(_D)",
                                          Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_YES_NO );

            if( mdiag.run() != Gtk::RESPONSE_YES ) return;

            if( mdiag.get_chkbutton().get_active() ) CONFIG::set_del_written_thread_diag( false );
        }

        // スレ内の画像キャッシュ削除
        if( CONFIG::get_delete_img_in_thread() != 2 ){

            bool delete_img_cache = false;

            std::list< std::string > list_urls = get_nodetree()->get_urls();
            std::list< std::string >::iterator it = list_urls.begin();
            for( ; it != list_urls.end(); ++it ){

                if( DBIMG::get_type_ext( *it ) != DBIMG::T_UNKNOWN && DBIMG::is_cached( *it ) ){
                    delete_img_cache = true;
                    break;
                }
            }

            if( delete_img_cache ){

                if( CONFIG::get_delete_img_in_thread() == 0 ){

                    const std::string msg = "「" + get_subject() + "」には画像が貼られています。\n\n画像のキャッシュも削除しますか？";

                    SKELETON::MsgCheckDiag mdiag( nullptr, msg,
                                                  "今後表示しない(常に削除しない)(_D)",
                                                  Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_NONE );

                    mdiag.add_button( "スレ削除中止(_C)", Gtk::RESPONSE_CANCEL );
                    mdiag.add_button( Gtk::Stock::YES, Gtk::RESPONSE_YES );
                    Gtk::Button button( Gtk::Stock::NO );
                    mdiag.add_default_button( &button, Gtk::RESPONSE_NO );

                    const int ret = mdiag.run();
                    if( ret == Gtk::RESPONSE_CANCEL ) return;
                    if( ret != Gtk::RESPONSE_YES ){

                        if( mdiag.get_chkbutton().get_active() ) CONFIG::set_delete_img_in_thread( 2 );
                        delete_img_cache = false;
                    }
                }

                if( delete_img_cache ){

                    it = list_urls.begin();
                    for( ; it != list_urls.end(); ++it ){

                        if( DBIMG::get_type_ext( *it ) != DBIMG::T_UNKNOWN && DBIMG::is_cached( *it ) ){

#ifdef _DEBUG
                            std::cout << "delete " << *it << std::endl;
#endif
                            DBIMG::delete_cache( *it );
                        }
                    }
                }

            }
        }
    }

    m_number_load = m_number_seen = m_number_before_load = 0;
    m_cached = false;
    reset_status();
    m_date_modified.clear();
    memset( &m_access_time, 0, sizeof( struct timeval ) );
    memset( &m_check_update_time, 0, sizeof( struct timeval ) );

    if( ! cache_only ){

        memset( &m_write_time, 0, sizeof( struct timeval ) );
        m_write_time_date.clear();

        m_code =  HTTP_INIT;
        m_str_code =  std::string();

        m_write_name.clear();
        m_write_mail.clear();
        m_write_fixname = false;
        m_write_fixmail = false;

        m_bookmarks.clear();
        m_posts.clear();
        m_list_abone_id.clear();
        m_list_abone_name.clear();
        m_list_abone_word.clear();
        m_list_abone_regex.clear();
        m_abone_reses.clear();
        m_abone_transparent = false;
        m_abone_chain = false;
        m_abone_age = false;
        m_abone_board = true;
        m_abone_global = true;
        m_read_info = false;
        m_save_info = false;
        m_bookmarked_thread = false;
    
        // info 削除
        if( CACHE::file_exists( m_path_article_info ) == CACHE::EXIST_FILE ) unlink( to_locale_cstr( m_path_article_info ) );

        // 拡張info 削除
        if( CACHE::file_exists( m_path_article_ext_info ) == CACHE::EXIST_FILE ) unlink( to_locale_cstr( m_path_article_ext_info ) );

        // お気に入りから削除
        CORE::core_set_command( "remove_favorite", m_url );
    }

    // キャッシュ削除
    std::string path_dat = CACHE::path_dat( m_url );
    if( CACHE::file_exists( path_dat ) == CACHE::EXIST_FILE ) unlink( to_locale_cstr( path_dat ) );

    // BoardViewの行を更新
    CORE::core_set_command( "update_board_item", DBTREE::url_subject( m_url ), m_id );

    // サイドバーのアイコン表示を戻す
    CORE::core_set_command( "toggle_sidebar_articleicon", m_url );
}



//
// キャッシュを名前を付けて保存
//
// path_to はデフォルトのファイル名
//
bool ArticleBase::save_dat( const std::string& path_to )
{
    if( is_loading() ) return false;

    std::string dir = MISC::get_dir( path_to );
    if( dir.empty() ) dir = SESSION::get_dir_dat();

    std::string name = MISC::get_filename( path_to );
    if( name.empty() ) name = get_id();

    std::string save_to = CACHE::copy_file( nullptr, CACHE::path_dat( m_url ), dir + name, CACHE::FILE_TYPE_DAT );

    if( ! save_to.empty() ){
        SESSION::set_dir_dat( MISC::get_dir( save_to ) );
        return true;
    }

    return false;
}



//
// infoファイル読み込み
//
// インスタンスが出来るたびに呼んでいると重くなるので、BoardBase::get_article_fromURL()
// で初めて参照されたときや、Boardビューに表示するときに一回だけ読み込む
//
void ArticleBase::read_info()
{
    if( m_read_info ) return; // 一度読んだら2度読みしない
    if( empty() ) return;
    if( ! is_cached() ) return;  // キャッシュがないなら読まない

#ifdef _DEBUG
    std::cout << "ArticleBase::read_info :  url = " << m_url << std::endl;
#endif

    m_read_info = true;

    const int status_old = m_status;
    bool saveinfo = false;

    // 情報ファイルのパスをセット
    if( m_path_article_info.empty() ) SET_INFOPATH();

    int ret = CACHE::file_exists( m_path_article_ext_info );
    if( ret == CACHE::EXIST_FILE ){

        std::string str_info, str_tmp;
        std::list< std::string > list_tmp;
        std::list< std::string >::iterator it_tmp;
        CACHE::load_rawdata( m_path_article_ext_info, str_info );

        std::list< std::string > lines = MISC::get_lines( str_info );
        std::list < std::string >::iterator it = lines.begin(), it2;

        // subject
        GET_INFOVALUE( m_subject, "subject = " );

        // 旧ホスト名
        GET_INFOVALUE( m_org_host, "org_host = " );
        if( m_org_host.empty() ) m_org_host = MISC::get_hostname( m_url );

        // 取得数
        m_number_load = 0;
        GET_INFOVALUE( str_tmp, "load = " );
        if( ! str_tmp.empty() ) m_number_load = std::stoi( str_tmp );
        m_number_before_load = m_number_load;

        // 見た場所
        m_number_seen = 0;
        GET_INFOVALUE( str_tmp, "seen = " );
        if( ! str_tmp.empty() ) m_number_seen = std::stoi( str_tmp );

        // 更新時間 (time)
        GET_INFOVALUE( m_date_modified, "modified = " );

        // access time
        GET_INFOVALUE( str_tmp, "access = " );
        if( ! str_tmp.empty() ){
            list_tmp = MISC::split_line( str_tmp );
            if( list_tmp.size() == 3 ){
                it_tmp = list_tmp.begin();
                m_access_time.tv_sec = ( atoi( ( *(it_tmp++) ).c_str() ) << 16 ) + atoi( ( *(it_tmp++) ).c_str() );
                m_access_time.tv_usec = atoi( ( *(it_tmp++) ).c_str() );
            }
        }

        // write time
        GET_INFOVALUE( str_tmp, "writetime = " );
        if( ! str_tmp.empty() ){
            list_tmp = MISC::split_line( str_tmp );
            if( list_tmp.size() == 3 ){
                it_tmp = list_tmp.begin();
                m_write_time.tv_sec = ( atoi( ( *(it_tmp++) ).c_str() ) << 16 ) + atoi( ( *(it_tmp++) ).c_str() );
                m_write_time.tv_usec = atoi( ( *(it_tmp++) ).c_str() );
            }
        }

        // write name
        m_write_name = std::string();
        GET_INFOVALUE( m_write_name, "writename = " );

        // write mail
        m_write_mail = std::string();
        GET_INFOVALUE( m_write_mail, "writemail = " );

        // name 固定
        m_write_fixname = false;
        GET_INFOVALUE( str_tmp, "writefixname = " );
        if( ! str_tmp.empty() ) m_write_fixname = atoi( str_tmp.c_str() );

        // mail 固定
        m_write_fixmail = false;
        GET_INFOVALUE( str_tmp, "writefixmail = " );
        if( ! str_tmp.empty() ) m_write_fixmail = atoi( str_tmp.c_str() );

        // 状態
        reset_status();
        GET_INFOVALUE( str_tmp, "status = " );
        if( ! str_tmp.empty() ) m_status = atoi( str_tmp.c_str() );

        // あぼーん ID
        GET_INFOVALUE( str_tmp, "aboneid = " );
        if( ! str_tmp.empty() ) m_list_abone_id = MISC::strtolist( str_tmp );

        // あぼーん name
        GET_INFOVALUE( str_tmp, "abonename = " );
        if( ! str_tmp.empty() ) m_list_abone_name = MISC::strtolist( str_tmp );

        // レスブックマーク
        GET_INFOVALUE( str_tmp, "bookmark = " );
        if( ! str_tmp.empty() ){

            list_tmp = MISC::split_line( str_tmp );
            for( const std::string& num_str : list_tmp ) {
                if( !num_str.empty() ) {
                    m_bookmarks.insert( std::stoi( num_str ) );
                }
            }
        }

        // あぼーん word
        GET_INFOVALUE( str_tmp, "aboneword = " );
        if( ! str_tmp.empty() ) m_list_abone_word = MISC::strtolist( str_tmp );

        // あぼーん regex
        GET_INFOVALUE( str_tmp, "aboneregex = " );
        if( ! str_tmp.empty() ) m_list_abone_regex = MISC::strtolist( str_tmp );

        // 透明あぼーん
        m_abone_transparent = false;
        GET_INFOVALUE( str_tmp, "abonetrp = " );
        if( ! str_tmp.empty() ) m_abone_transparent = atoi( str_tmp.c_str() );

        // 連鎖あぼーん
        m_abone_chain = false;
        GET_INFOVALUE( str_tmp, "abonechain = " );
        if( ! str_tmp.empty() ) m_abone_chain = atoi( str_tmp.c_str() );

        // レス番号あぼーん
        m_abone_reses.clear();
        GET_INFOVALUE( str_tmp, "aboneres = " );
        if( ! str_tmp.empty() ){

            list_tmp = MISC::split_line( str_tmp );
            for( const std::string& num_str : list_tmp ) {
                if( !num_str.empty() ) {
                    m_abone_reses.insert( std::stoi( num_str ) );
                }
            }
        }

        // スレ一覧でブックマークされているか
        m_bookmarked_thread = false;
        GET_INFOVALUE( str_tmp, "bkmark_thread = " );
        if( ! str_tmp.empty() ) m_bookmarked_thread = atoi( str_tmp.c_str() );

        // 書き込みしたレス番号
        GET_INFOVALUE( str_tmp, "posted = " );
        if( ! str_tmp.empty() ){

            list_tmp = MISC::split_line( str_tmp );
            for( const std::string& num_str : list_tmp ) {
                if( !num_str.empty() ) {
                    m_posts.insert( std::stoi( num_str ) );
                }
            }
        }

        // ageあぼーん
        m_abone_age = false;
        GET_INFOVALUE( str_tmp, "aboneage = " );
        if( ! str_tmp.empty() ) m_abone_age = atoi( str_tmp.c_str() );

        // 最終更新チェック時間
        GET_INFOVALUE( str_tmp, "checktime = " );
        if( ! str_tmp.empty() ){
            list_tmp = MISC::split_line( str_tmp );
            if( list_tmp.size() == 2 ){
                it_tmp = list_tmp.begin();
                m_check_update_time.tv_sec = ( atoi( ( *(it_tmp++) ).c_str() ) << 16 ) + atoi( ( *(it_tmp++) ).c_str() );
            }
        }

        // 板レベルでのあぼーんを有効にする
        m_abone_board = true;
        GET_INFOVALUE( str_tmp, "aboneboard = " );
        if( ! str_tmp.empty() ) m_abone_board = atoi( str_tmp.c_str() );

        // 全体レベルでのあぼーんを有効にする
        m_abone_global = true;
        GET_INFOVALUE( str_tmp, "aboneglobal = " );
        if( ! str_tmp.empty() ) m_abone_global = atoi( str_tmp.c_str() );
    }

    // キャッシュはあるけど情報ファイルが無い場合
    // 一時的にnodetreeを作って情報を取得して保存
    else{

#ifdef _DEBUG
        std::cout << "ArticleBase::read_info : update info " << m_url << std::endl;
        std::cout << "load = " << m_number_load << " subject = " << m_subject << std::endl;
        std::cout << "ret = " << ret << std::endl;
        std::cout << "path = " << m_path_article_ext_info << std::endl;
#endif

        CORE::core_set_command( "set_status","", "スレ情報更新中・・・しばらくお待ち下さい" );
        MISC::MSG( "updating " + m_url );

        reset_status();
        set_subject( get_nodetree()->get_subject() );
        m_number_load = get_nodetree()->get_res_number();

        if( !m_number_load ){
            m_number_load = 1;
            m_status |= STATUS_BROKEN;
            MISC::MSG( "updating failed" );
        }
        
        m_number_before_load = m_number_load;
        saveinfo = true;
        unlock_impl();

#ifdef _DEBUG
        std::cout << "\nArticleBase::read_info : update done.\n\n";
#endif
    }

    if( m_number < m_number_load ) m_number = m_number_load;

    // infoファイル読み込み前に既にDAT落ち状態になっていた場合は
    // 状態を DAT 落ちに戻しておく
    if( ( status_old & STATUS_OLD ) && ( m_status & STATUS_NORMAL ) ){
        m_status &= ~STATUS_NORMAL;
        m_status |= STATUS_OLD;
        saveinfo = true;
    }
    if( saveinfo ) save_info( true );

#ifdef _DEBUG
    std::cout << "ArticleBase::read_info file = " << m_path_article_ext_info << std::endl;

    std::cout << "subject = " << m_subject << std::endl
              << "org_host = " << m_org_host << std::endl
              << "load = " << m_number_load << std::endl
              << "seen = " << m_number_seen << std::endl
              << "modified = " << m_date_modified << std::endl
              << "writetime = " << m_write_time_date << std::endl
              << "writename = " << m_write_name << std::endl
              << "writemail = " << m_write_mail << std::endl
              << "writefixname = " << m_write_fixname << std::endl
              << "writefixmail = " << m_write_fixmail << std::endl
              << "status = " << m_status << std::endl
              << "transparent_abone = " << m_abone_transparent << std::endl
              << "bookmarked_thread = " << m_bookmarked_thread << std::endl
    ;

    std::cout << "abone-id\n"; std::list < std::string >::iterator it = m_list_abone_id.begin();
    for( ; it != m_list_abone_id.end(); ++it ) std::cout << (*it) << std::endl;
    std::cout << "abone-name\n"; it = m_list_abone_name.begin();
    for( ; it != m_list_abone_name.end(); ++it ) std::cout << (*it) << std::endl;
    std::cout << "abone-word\n"; it = m_list_abone_word.begin();
    for( ; it != m_list_abone_word.end(); ++it ) std::cout << (*it) << std::endl;
    std::cout << "abone-regex\n"; it = m_list_abone_regex.begin();
    for( ; it != m_list_abone_regex.end(); ++it ) std::cout << (*it) << std::endl;

    if( !m_abone_reses.empty() ) {
        std::cout << "abone-res =";
        const auto end = m_abone_reses.end();
        for( int i = 1; i <= m_number_load; ++i ) {
            if( m_abone_reses.find( i ) != end ) std::cout << ' ' << i;
        }
    }

    if( !m_bookmarks.empty() ) {
        std::cout << "bookmark = ";
        const auto end = m_bookmarks.end();
        for( int i = 1; i <= m_number_load; ++i ) {
            if( m_bookmarks.find( i ) != end ) std::cout << ' ' << i;
        }
        std::cout << std::endl;
    }

    if( !m_posts.empty() ) {
        std::cout << "posted =";
        const auto end = m_posts.end();
        for( int i = 1; i <= m_number_load; ++i ) {
            if( m_posts.find( i ) != end ) std::cout << ' ' << i;
        }
        std::cout << std::endl;
    }
#endif
}


//
// infoファイル書き込み
//
// キャッシュがある( is_cached() == true )　かつ
// m_save_info = true かつ nodetree が作られている時に保存。
// save_info()を呼ぶ前にm_save_infoをセットすること。
//
// キャッシュがあって、force = true の時は強制書き込み
void ArticleBase::save_info( const bool force )
{
    if( empty() ) return;
    if( ! is_cached() ) return;
    if( ! force ){

        if( ! m_save_info ) return;
        if( ! m_nodetree ) return;
    }

    m_save_info = false;

    if( m_path_article_ext_info.empty() ) return;
    if( ! CACHE::mkdir_boardroot( m_url ) ) return;

#ifdef _DEBUG
    std::cout << "ArticleBase::save_info force = " << force << std::endl;
    std::cout << "path_article_info = " << m_path_article_info << std::endl;
    std::cout << "path_article_ext_info = " << m_path_article_ext_info << std::endl;
    std::cout << "subject = " << m_subject << std::endl;
#endif

    // 書き込み時間
    std::ostringstream ss_write;
    if( m_write_time.tv_sec ) ss_write << ( m_write_time.tv_sec >> 16 ) << " " << ( m_write_time.tv_sec & 0xffff ) << " " << m_write_time.tv_usec;

    // 更新チェック時間
    std::ostringstream ss_check;
    if( m_check_update_time.tv_sec ) ss_check << ( m_check_update_time.tv_sec >> 16 ) << " " << ( m_check_update_time.tv_sec & 0xffff );

    // あぼーん情報
    std::string str_abone_id = MISC::listtostr( m_list_abone_id );
    std::string str_abone_name = MISC::listtostr( m_list_abone_name );
    std::string str_abone_word = MISC::listtostr( m_list_abone_word );
    std::string str_abone_regex = MISC::listtostr( m_list_abone_regex );

    // レスあぼーん
    std::ostringstream ss_abone_res;
    if( !m_abone_reses.empty() ) {
        const auto end = m_abone_reses.end();
        for( int i = 1; i <= m_number_load; ++i ) {
            if( m_abone_reses.find( i ) != end ) ss_abone_res << ' ' << i;
        }
    }

    // レスのブックマーク
    std::ostringstream ss_bookmark;
    if( !m_bookmarks.empty() ) {
        const auto end = m_bookmarks.end();
        for( int i = 1; i <= m_number_load; ++i ) {
            if( m_bookmarks.find( i ) != end ) ss_bookmark << ' ' << i;
        }
    }

    // 書き込み
    std::ostringstream ss_posted;
    if( !m_posts.empty() ) {
        const auto end = m_posts.end();
        for( int i = 1; i <= m_number_load; ++i ) {
            if( m_posts.find( i ) != end ) ss_posted << ' ' << i;
        }
    }

    std::ostringstream sstr;
    sstr << "subject = " << m_subject << std::endl
         << "org_host = " << m_org_host << std::endl
         << "load = " << m_number_load << std::endl
         << "seen = " << m_number_seen << std::endl
         << "modified = " << m_date_modified << std::endl
         << "access = " << get_access_time_str() << std::endl
         << "writetime = " << ss_write.str() << std::endl
         << "writename = " << m_write_name << std::endl
         << "writemail = " << m_write_mail << std::endl
         << "writefixname = " << m_write_fixname << std::endl
         << "writefixmail = " << m_write_fixmail << std::endl
         << "status = " << m_status << std::endl
         << "aboneid = " << str_abone_id << std::endl
         << "abonename = " << str_abone_name << std::endl
         << "bookmark = " << ss_bookmark.str() << std::endl
         << "aboneword = " << str_abone_word << std::endl
         << "aboneregex = " << str_abone_regex << std::endl
         << "abonetrp = " << m_abone_transparent << std::endl
         << "abonechain = " << m_abone_chain << std::endl
         << "aboneres = " << ss_abone_res.str() << std::endl
         << "bkmark_thread = " << m_bookmarked_thread << std::endl
         << "posted = " << ss_posted.str() << std::endl
         << "aboneage = " << m_abone_age << std::endl
         << "checktime = " << ss_check.str() << std::endl
         << "aboneboard = " << m_abone_board << std::endl
         << "aboneglobal = " << m_abone_global << std::endl
    ;

#ifdef _DEBUG
    std::cout << "ArticleBase::save_info file = " << m_path_article_ext_info << std::endl;
    std::cout << sstr.str() << std::endl;
#endif

    CACHE::save_rawdata( m_path_article_ext_info, sstr.str() );

    // 互換性のため
    save_navi2ch_info();
}



//
// navi2ch互換情報ファイル書き込み
//
// 互換性のため書き出すだけで実際にはこの中の情報は使わない
//
void ArticleBase::save_navi2ch_info()
{
    if( empty() ) return;
    if( ! is_cached() ) return;
    if( m_path_article_info.empty() ) return;

    std::string name = "nil";
    std::string hide = "nil";
    std::string important = "nil";
    std::string unfilter = "nil";
    std::string mail = "nil";
    std::string kako = "nil";

    // 保存してあるinfoから扱ってない情報をコピー
    if( CACHE::file_exists( m_path_article_info ) == CACHE::EXIST_FILE ){

        std::string str_info;
        CACHE::load_rawdata( m_path_article_info, str_info );
#ifdef _DEBUG
        std::cout << "str_info " << str_info << std::endl;
#endif

        std::list< std::string > lists = MISC::get_elisp_lists( str_info );
        std::list< std::string >::iterator it = lists.begin();

        do{
            ++it;
            if( it == lists.end() ) break;
            name = *( it++ );

            if( it == lists.end() ) break;
            ++it;
            if( it == lists.end() ) break;
            hide = *( it++ );

            if( it == lists.end() ) break;
            important = *( it++ );

            if( it == lists.end() ) break;
            unfilter = *( it++ );

            if( it == lists.end() ) break;
            mail = *( it++ );

            if( it == lists.end() ) break;
            kako = *( it++ );

        } while(0);
    }

    std::ostringstream sstr;
    sstr << "("
         << "(number . " << m_number_load << ")"
         << " " << name
         << " " << "(time . \"" << m_date_modified  << "\")"
         << " " << hide
         << " " << important
         << " " << unfilter
         << " " << mail
         << " " << kako
         << ")";

#ifdef _DEBUG
    std::cout << "ArticleBase::save_navi2ch_info file = " << m_path_article_info << std::endl;
    std::cout << sstr.str() << std::endl;
#endif

    CACHE::save_rawdata( m_path_article_info, sstr.str() );
}
