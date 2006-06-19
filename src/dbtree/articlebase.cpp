// ライセンス: 最新のGPL

//#define _DEBUG
#include "jddebug.h"

#include "articlebase.h"
#include "nodetreebase.h"
#include "interface.h"
#include "global.h"

#include "jdlib/miscutil.h"
#include "jdlib/misctime.h"
#include "jdlib/miscmsg.h"

#include "httpcode.h"
#include "command.h"
#include "cache.h"
#include "global.h"
#include "login2ch.h"

#include <sstream>


using namespace DBTREE;


// 情報ファイルから値を読み込み
// JDLIB::ConfLoader では遅いので別に作成。オプションの順番に注意すること
#define GET_INFOVALUE(target,targetname) \
do { \
target = std::string(); \
option = targetname; \
it2 = it; \
while( it2 != lines.end() && ( *it2 ).find( option ) != 0 ) ++it2; \
if( it2 != lines.end() ){ \
    target = ( *it2 ).substr( option.length() ); \
    it = ++it2; \
} } while( false )




ArticleBase::ArticleBase( const std::string& datbase, const std::string& id, bool cached )
    : SKELETON::Lockable(),
      m_heap( MAX_RESNUMBER ),
      m_id ( id ),
      m_key( std::string() ),
      m_since_time( 0 ),
      m_since_date( std::string() ),
      m_str_code( std::string() ),
      m_lng_dat( 0 ),
      m_status( STATUS_UNKNOWN ),
      m_subject( std::string() ),
      m_number( 0 ),
      m_number_new( 0 ),
      m_number_load( 0 ),
      m_number_before_load( 0 ),
      m_number_seen( 0 ),
      m_write_fixname( 0 ),
      m_write_fixmail( 0 ),
      m_cached( cached ),
      m_read_info( 0 ),
      m_current( 0 ),
      m_save_info( 0 )
{
#ifdef _DEBUG
    std::cout << "ArticleBase::ArticleBase : " << m_id << std::endl;
#endif

    memset( &m_access_time, 0, sizeof( struct timeval ) );
    memset( &m_write_time, 0, sizeof( struct timeval ) );

    // m_url にURLセット
    update_url( datbase );

    // この段階では移転前の旧ホスト名は分からないのでとりあえず現在のホスト名をセットしておく
    // あとで Root::url_dat()でURLを変換する時に旧ホスト名を教えてもらってinfoファイルに保存しておく。
    // Root::url_dat()も参照せよ
    m_org_host = MISC::get_hostname( m_url );
}


ArticleBase::~ArticleBase()
{
#ifdef _DEBUG
    std::cout << "ArticleBase::~ArticleBase : " << m_id << std::endl;
#endif

    // 参照ロックが外れていない
    assert( get_lock() == 0 );
    assert( !m_nodetree );
}


const bool ArticleBase::empty()
{
    return  m_url.empty();
}


//
// 移転する前のオリジナルのURL
//
const std::string ArticleBase::get_org_url()
{
    std::string newhost = MISC::get_hostname( m_url );
    return m_org_host + m_url.substr( newhost.length() );
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
const std::string ArticleBase::get_name( int number )
{
    return get_nodetree()->get_name( number );
}


//
// number番のID
//
const std::string ArticleBase::get_id_name( int number )
{
    return get_nodetree()->get_id_name( number );
}


// 指定したID の重複数( = 発言数 )
// 下のnum_id_name( int number )と違って検索するので遅い
int ArticleBase::get_num_id_name( const std::string& id )
{
    return get_nodetree()->get_num_id_name( id );
}



// number番のID の重複数( = 発言数 )
int ArticleBase::get_num_id_name( int number )
{
    return get_nodetree()->get_num_id_name( number );
}



//
// number番のレスを参照しているレス番号をリストにして取得
//
std::list< int > ArticleBase::get_reference( int number )
{
    return get_nodetree()->get_reference( number );
}


//
// URL を含むレス番号をリストにして取得
//
std::list< int > ArticleBase::get_res_with_url()
{
    return get_nodetree()->get_res_with_url();
}


//
// number番のレスの文字列を返す
// ref == true なら先頭に ">" を付ける        
//
const std::string ArticleBase::get_res_str( int number, bool ref )
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
    if( time_out == 0 ) time_out = time( NULL ) - 600;
    return time_out; 
}


//
// url の更新
//
// 移転があったときなどに上位クラスから呼ばれる
//
void ArticleBase::update_url( const std::string& datbase )
{
    if( m_id.empty() ) return;

#ifdef _DEBUG
    std::string old_url = m_url;
#endif

    m_url = datbase + m_id;

#ifdef _DEBUG
    if( !old_url.empty() ) std::cout << "ArticleBase::update_url from "  << old_url
                                     << " to " << m_url << std::endl;
#endif
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
              << "host = " << host << std::endl
              << "org_host = " << m_org_host << std::endl;
#endif

        m_org_host = host;
        m_save_info = true;
    }
}




//
// access_time を 文字列に変換して返す
//
const std::string ArticleBase::get_access_time_str()
{
    return MISC::timevaltostr( m_access_time );
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
    if( ! subject.empty() && subject != m_subject ){
        m_subject = subject;
        m_save_info = true;
    }
}


void ArticleBase::set_number( int number )
{
    if( number && number != m_number ) m_number = number;
}


void ArticleBase::set_number_load( int number_load )
{
    if( number_load && number_load != m_number_load ) m_number_load = number_load;
}


void ArticleBase::set_number_seen( int number_seen )
{
    if( number_seen && number_seen != m_number_seen ){
        m_number_seen = number_seen;
        m_save_info = true;
    }
}

//
// 書き込み時間更新
//
void ArticleBase::update_writetime()
{
    struct timeval tv;
    struct timezone tz;
    if( gettimeofday( &tv, &tz ) == 0 ){

        m_write_time = tv;
        m_write_time_date = MISC::timettostr( m_write_time.tv_sec );

#ifdef _DEBUG
        std::cout << "ArticleBase::update_writetime : " << m_write_time.tv_sec << " " << m_write_time_date << std::endl;
#endif
        m_save_info = true;

        // BoardViewの行を更新
        CORE::core_set_command( "update_board_item", DBTREE::url_subject( m_url ), m_id );
    }
}



//
// あぼーんしてるか
//
const bool ArticleBase::abone( int number )
{
    if( number <= 0 || number > m_number_load ) return false;
    if( empty() ) return false;

    // まだnodetreeが作られてなくてあぼーんの情報が得られてないのでnodetreeを作って情報取得
    if( !m_abone ) get_nodetree();
    assert( m_abone );

    return m_abone[ number ];
}



//
// あぼーん状態の更新
//
// あぼーん情報を変更したら呼び出す
//
void ArticleBase::update_abone()
{
    // まだnodetreeが作られてなくてあぼーん状態が得られてないときは何もしない
    if( !m_abone ) return;

    check_abone( 1, m_number_load );
}



//
// あぼーん状態のリセット(情報セットと状態更新)
//
void ArticleBase::reset_abone( std::list< std::string >& ids, std::list< std::string >& names
                               , std::list< std::string >& words )
{
    if( empty() ) return;

#ifdef _DEBUG
    std::cout << "ArticleBase::reset_abone\n";
#endif

    m_list_abone_id.clear();
    m_list_abone_name.clear();
    m_list_abone_word.clear();

    std::string tmp_str;
    std::list< std::string >::iterator it;    

    // 空白行を除きつつ情報を更新していく
    
    for( it = ids.begin(); it != ids.end(); ++it ){
        std::string tmp_str = MISC::remove_space( (*it) );
        if( ! tmp_str.empty() ) m_list_abone_id.push_back( *it );
    }

    for( it = names.begin(); it != names.end(); ++it ){
        std::string tmp_str = MISC::remove_space( (*it) );
        if( ! tmp_str.empty() ) m_list_abone_name.push_back( *it );
    }

    for( it = words.begin(); it != words.end(); ++it ){
        std::string tmp_str = MISC::remove_space( (*it) );
        if( ! tmp_str.empty() ) m_list_abone_word.push_back( *it );
    }

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
// あぼーん判定を更新
//
void ArticleBase::check_abone( int from_number, int to_number )
{
    if( empty() ) return;
    assert( m_abone );

#ifdef _DEBUG
    std::cout << "ArticleBase::check_abone from " << from_number << " to " << to_number << std::endl;
#endif
    if( to_number < from_number ) return;

    for( int i = from_number ; i <= to_number; ++i ){

        m_abone[ i ] = false;

        // ローカルあぼーん
        if( get_nodetree()->check_abone_id( i, m_list_abone_id ) ){
            m_abone[ i ] = true;
            continue;
        }
        if( get_nodetree()->check_abone_name( i, m_list_abone_name ) ){
            m_abone[ i ] = true;
            continue;
        }
        if( get_nodetree()->check_abone_word( i, m_list_abone_word ) ){
            m_abone[ i ] = true;
            continue;
        }
    }

#ifdef _DEBUG
    for( int i = from_number ; i <= to_number; ++i ) if( m_abone[ i ] ){
        std::cout << i << " " << get_nodetree()->get_id_name( i ) << " " << get_nodetree()->get_name( i ) << std::endl;
    }
#endif 

}




//
// ブックマークの数
//
int ArticleBase::get_num_bookmark()
{
    if( !m_bookmark ) return 0;

    int ret = 0;
    for( int i = 1; i < MAX_RESNUMBER; ++i ) if( is_bookmarked( i ) ) ++ret;
    return ret;
}


//
// ブックマークされているか
//
bool ArticleBase::is_bookmarked( int number )
{
    if( number <= 0 || number > m_number_load ) return false;

    // まだnodetreeが作られてなくてブックマークとあぼーんの情報が得られてないのでnodetreeを作って情報取得
    if( !m_bookmark || !m_abone ) get_nodetree();
    assert( m_abone );
    assert( m_bookmark );

    return ( !m_abone[ number ] && m_bookmark[ number ] );
}


//
// ブックマークセット
//
void ArticleBase::set_bookmark( int number, bool set )
{
    if( ! m_bookmark ) get_nodetree();
    if( number <= 0 || number > MAX_RESNUMBER ) return;

    m_save_info = true;
    m_bookmark[ number ] = set;
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

        if( ! m_abone ) m_abone = ( char* ) m_heap.heap_alloc( MAX_RESNUMBER );
        if( ! m_bookmark ) m_bookmark = ( char* ) m_heap.heap_alloc( MAX_RESNUMBER );

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

    m_nodetree.clear();

    // スレ情報保存
    save_info();
}





//
// ロード中か
//
const bool ArticleBase::is_loading()
{
    if( ! m_nodetree ) return false;
    return m_nodetree->is_loading();
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
void ArticleBase::download_dat()
{
    if( empty() ) return;

#ifdef _DEBUG
    std::cout << "ArticleBase::download_dat " << m_url << std::endl;;
#endif

    // DAT落ちしていてログイン中で無い時はロードしない
    if( ! ( ( m_status & STATUS_OLD ) && ! LOGIN::get_login2ch()->login_now() ) ){

#ifdef _DEBUG
        std::cout << "start\n";
#endif       
        get_nodetree()->download_dat();
    }
}




//
// ロード中など nodetree の構造が変わったときにnodetreeから呼ばれる slot
//
void ArticleBase::slot_node_updated()
{
    assert( m_nodetree );

#ifdef _DEBUG
    std::cout << "ArticleBase::slot_node_updated" << std::endl;
#endif

    // nodetreeから情報取得
    if( ! m_nodetree->get_subject().empty() ) m_subject = m_nodetree->get_subject();
    m_lng_dat = m_nodetree->lng_dat();

    // スレが更新している場合
    if( m_number_load != m_nodetree->get_res_number() ){

        // あぼーん判定更新
        check_abone( m_number_load + 1, m_nodetree->get_res_number() );

        m_number_load = m_nodetree->get_res_number();

        // ビュー更新
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

    // nodetreeから情報取得
    m_code = m_nodetree->get_code();
    m_str_code = m_nodetree->get_str_code();
    m_date_modified = m_nodetree->date_modified();
    if( m_number_before_load < m_number_load ) m_number_new = m_number_load - m_number_before_load;
    else m_number_new = 0;
    m_number_before_load = m_number_load;
    m_ext_err = m_nodetree->get_ext_err();

    // 状態更新
    int old_status = m_status;
    if( m_code != HTTP_ERR ){

        m_status = STATUS_NORMAL;

        // DAT落ち
        if( m_code == HTTP_REDIRECT || m_code == HTTP_NOT_FOUND ) m_status = STATUS_OLD;
    }

    // 壊れている
    if( m_nodetree->is_broken() ) m_status |= STATUS_BROKEN;
    else m_status &= ~STATUS_BROKEN;

    // 状態が変わっていたら情報保存
    if( old_status != m_status ) m_save_info = true;

    // スレの数が0ならスレ情報はセーブしない
    if( ! m_number_load ) m_cached = false;

    // スレが更新している場合はスレ情報を更新
    else if( m_number_new ){

        struct timeval tv;
        struct timezone tz;
        if( gettimeofday( &tv, &tz ) == 0 ) m_access_time = tv;

        if( m_number < m_number_load ) m_number = m_number_load;
        m_number_seen = m_number_load;

        m_cached = true;
        m_read_info = true;
        m_save_info = true;
    }

#ifdef _DEBUG
    std::cout << "ArticleBase::slot_load_finished " << std::endl
              << "subject = " << m_subject << std::endl
              << "load = " << m_number_load << std::endl
              << "number = " << m_number << std::endl
              << "new = " << m_number_new << std::endl
              << "date = " << m_date_modified << std::endl
              << "access-time = " << get_access_time_str() << std::endl
              << "lng = " << m_lng_dat << std::endl
              << "code = " << m_code << std::endl
              << "status = " << m_status << std::endl

    ;
#endif

    // 対応するBoardビューの行を更新
    CORE::core_set_command( "update_board_item", DBTREE::url_subject( m_url ), m_id );
    
    // articleビューに終了を知らせる
    CORE::core_set_command( "update_article", m_url );
    CORE::core_set_command( "update_article_finish", m_url );
}




//
// キャッシュ削除
//
void ArticleBase::delete_cache()
{
#ifdef _DEBUG
    std::cout << "ArticleBase::delete_cache  url = " << m_url << std::endl;
#endif

    if( empty() ) return;

    m_number_load = m_number_seen = m_number_before_load = 0;
    m_status = STATUS_UNKNOWN;
    m_date_modified.clear();
    memset( &m_access_time, 0, sizeof( struct timeval ) );
    memset( &m_write_time, 0, sizeof( struct timeval ) );
    m_write_time_date.clear();

    m_heap.clear();
    m_abone.reset();
    m_bookmark.reset();
    m_list_abone_id.clear();
    m_list_abone_name.clear();
    m_list_abone_word.clear();
    m_cached = false;
    m_read_info = false;
    m_save_info = false;
    
    // キャッシュ
    std::string path = CACHE::path_dat( m_url );
    if( CACHE::is_file_exists( path ) == CACHE::EXIST_FILE ) unlink( path.c_str() );

    // info
    path = CACHE::path_article_info( m_url, m_id );
    if( CACHE::is_file_exists( path ) == CACHE::EXIST_FILE ) unlink( path.c_str() );

    // 拡張info
    path = CACHE::path_article_ext_info( m_url, m_id );
    if( CACHE::is_file_exists( path ) == CACHE::EXIST_FILE ) unlink( path.c_str() );

    // BoardViewの行を更新
    CORE::core_set_command( "update_board_item", DBTREE::url_subject( m_url ), m_id );
}



//
// infoファイル読み込み
//
// インスタンスが出来るたびに呼んでいると重くなるので、BoardBase::get_article_fromURL()
// で初めて参照されたときや、Boardビューに表示するときに一回だけ読み込む
//
void ArticleBase::read_info()
{
    if( empty() ) return;
    if( ! m_cached ) return;  // キャッシュがないなら読まない
    if( m_read_info ) return; // 一度読んだら2度読みしない
    m_read_info = true;

#ifdef _DEBUG
    std::cout << "ArticleBase::read_info :  url = " << m_url << std::endl;
#endif

    std::string path_info = CACHE::path_article_ext_info( m_url, m_id );
    if( CACHE::is_file_exists( path_info ) == CACHE::EXIST_FILE ){

        std::string str_info, str_tmp;
        std::list< std::string > list_tmp;
        std::list< std::string >::iterator it_tmp;
        CACHE::load_rawdata( path_info, str_info );

        std::list< std::string > lines = MISC::get_lines( str_info );
        std::list < std::string >::iterator it = lines.begin(), it2;
        std::string option; // GET_INFOVALUE　で使用

        // subject
        GET_INFOVALUE( m_subject, "subject = " );

        // 旧ホスト名
        GET_INFOVALUE( m_org_host, "org_host = " );
        if( m_org_host.empty() ) m_org_host = MISC::get_hostname( m_url );

        // 取得数
        m_number_load = 0;
        GET_INFOVALUE( str_tmp, "load = " );
        if( ! str_tmp.empty() ) m_number_load = atoi( str_tmp.c_str() );
        m_number_before_load = m_number_load;

        // 見た場所
        m_number_seen = 0;
        GET_INFOVALUE( str_tmp, "seen = " );
        if( ! str_tmp.empty() ) m_number_seen = atoi( str_tmp.c_str() );

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

            m_write_time_date = MISC::timettostr( m_write_time.tv_sec );
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
        m_status = STATUS_UNKNOWN;
        GET_INFOVALUE( str_tmp, "status = " );
        if( ! str_tmp.empty() ) m_status = atoi( str_tmp.c_str() );

        // あぼーん ID
        GET_INFOVALUE( str_tmp, "aboneid = " );
        if( ! str_tmp.empty() ){
            list_tmp = MISC::split_line( str_tmp );
            it_tmp = list_tmp.begin();
            for( ; it_tmp != list_tmp.end(); ++it_tmp ){
                if( !(*it_tmp).empty() ) m_list_abone_id.push_back( MISC::recover_quot( ( *it_tmp ) ) );
            }
        }

        // あぼーん name
        GET_INFOVALUE( str_tmp, "abonename = " );
        if( ! str_tmp.empty() ){
            list_tmp = MISC::split_line( str_tmp );
            it_tmp = list_tmp.begin();
            for( ; it_tmp != list_tmp.end(); ++it_tmp ){
                if( !(*it_tmp).empty() ) m_list_abone_name.push_back( MISC::recover_quot( ( *it_tmp ) ) );
            }
        }

        // ブックマーク
        GET_INFOVALUE( str_tmp, "bookmark = " );
        if( ! str_tmp.empty() ){

            // ブックマーク領域作成
            if( ! m_bookmark ) m_bookmark = ( char* ) m_heap.heap_alloc( MAX_RESNUMBER );

            list_tmp = MISC::split_line( str_tmp );
            it_tmp = list_tmp.begin();
            for( ; it_tmp != list_tmp.end(); ++it_tmp ) if( !(*it_tmp).empty() ) m_bookmark[ atoi( (*it_tmp).c_str() ) ] = true;
        }

        // あぼーん word
        GET_INFOVALUE( str_tmp, "aboneword = " );
        if( ! str_tmp.empty() ){
            list_tmp = MISC::split_line( str_tmp );
            it_tmp = list_tmp.begin();
            for( ; it_tmp != list_tmp.end(); ++it_tmp ){
                if( !(*it_tmp).empty() ) m_list_abone_word.push_back( MISC::recover_quot( ( *it_tmp ) ) );
            }
        }

    }

    // キャッシュはあるけど情報ファイルが無い場合
    // 一時的にnodetreeを作って情報を取得して保存
    if( ! m_number_load || m_subject.empty() ){

#ifdef _DEBUG
        std::cout << "ArticleBase::read_info : update info " << m_url << std::endl;
        std::cout << "load = " << m_number_load << " subject = " << m_subject << std::endl;
#endif

        CORE::core_set_command( "set_status","", "スレ情報更新中・・・しばらくお待ち下さい" );
        MISC::MSG( "updating " + m_url );

        m_subject = get_nodetree()->get_subject();
        m_number_load = get_nodetree()->get_res_number();

        if( m_subject.empty() ) {
            m_subject = "壊れています";
            m_status |= STATUS_BROKEN;
        }
        if( !m_number_load ){
            m_number_load = 1;
            m_status |= STATUS_BROKEN;
        }
        
        m_number_before_load = m_number_load;
        m_save_info = true;
        unlock_impl();

#ifdef _DEBUG
        std::cout << "\nArticleBase::read_info : update done.\n\n";
#endif
    }

    if( m_number < m_number_load ) m_number = m_number_load;

#ifdef _DEBUG
    std::cout << "ArticleBase::read_info file = " << path_info << std::endl;

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
    ;

    std::cout << "abone-id\n"; std::list < std::string >::iterator it = m_list_abone_id.begin();
    for( ; it != m_list_abone_id.end(); ++it ) std::cout << (*it) << std::endl;
    std::cout << "abone-name\n"; it = m_list_abone_name.begin();
    for( ; it != m_list_abone_name.end(); ++it ) std::cout << (*it) << std::endl;
    std::cout << "abone-word\n"; it = m_list_abone_word.begin();
    for( ; it != m_list_abone_word.end(); ++it ) std::cout << (*it) << std::endl;

    if( m_bookmark ){
        std::cout << "bookmark =";
        for( int i = 1; i <= m_number_load; ++i ) if( m_bookmark[ i ] ) std::cout << " " << i;
        std::cout << std::endl;
    }
#endif

}


//
// infoファイル書き込み
//
// キャッシュがある( m_cached = true )　かつ
// m_save_info = true の時に保存。save_info()を呼ぶ前にm_save_infoをセットすること。
//
void ArticleBase::save_info()
{
    if( empty() ) return;
    if( ! m_cached ) return;
    if( ! m_save_info ) return;
    m_save_info = false;

    if( ! CACHE::mkdir_boardroot( m_url ) ) return;
    
    std::string path_info = CACHE::path_article_ext_info( m_url, m_id );
    if( path_info.empty() ) return;

    // 書き込み時間
    std::ostringstream ss_write;
    ss_write << ( m_write_time.tv_sec >> 16 ) << " " << ( m_write_time.tv_sec & 0xffff ) << " " << m_write_time.tv_usec;

    // あぼーん情報
    std::string str_abone_id, str_abone_name, str_abone_word;
    std::list< std::string >::iterator it = m_list_abone_id.begin();
    for( ; it != m_list_abone_id.end(); ++it ){
        if( ! ( *it ).empty() ) str_abone_id += " \"" + MISC::replace_quot( ( *it ) )  + "\"";
    }
    it = m_list_abone_name.begin();
    for( ; it != m_list_abone_name.end(); ++it ){
        if( ! ( *it ).empty() ) str_abone_name += " \"" + MISC::replace_quot( ( *it ) )  + "\"";
    }
    it = m_list_abone_word.begin();
    for( ; it != m_list_abone_word.end(); ++it ){
        if( ! ( *it ).empty() ) str_abone_word += " \"" + MISC::replace_quot( ( *it ) )  + "\"";
    }

    // スレのブックマーク
    std::ostringstream ss_bookmark;
    if( m_bookmark ){
        for( int i = 1; i <= m_number_load; ++i ) if( m_bookmark[ i ] ) ss_bookmark << " " << i;
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
    ;

#ifdef _DEBUG
    std::cout << "ArticleBase::save_info file = " << path_info << std::endl;
    std::cout << sstr.str() << std::endl;
#endif

    CACHE::save_rawdata( path_info, sstr.str() );

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
    if( ! m_cached ) return;

    if( ! CACHE::mkdir_boardroot( m_url ) ) return;
    
    std::string path_info = CACHE::path_article_info( m_url, m_id );

    std::string name = "nil";
    std::string hide = "nil";
    std::string important = "nil";
    std::string unfilter = "nil";
    std::string mail = "nil";
    std::string kako = "nil";

    // 保存してあるinfoから扱ってない情報をコピー
    if( CACHE::is_file_exists( path_info ) == CACHE::EXIST_FILE ){

        std::string str_info;
        CACHE::load_rawdata( path_info, str_info );
#ifdef _DEBUG
        std::cout << "str_info " << str_info << std::endl;
#endif

        std::list< std::string > lists = MISC::get_elisp_lists( str_info );
        std::list< std::string >::iterator it = lists.begin();
        ++it;
        name = *( it++ );
        ++it;
        hide = *( it++ );
        important = *( it++ );
        unfilter = *( it++ );
        mail = *( it++ );
        kako = *( it++ );        
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
    std::cout << "ArticleBase::save_navi2ch_info file = " << path_info << std::endl;
    std::cout << sstr.str() << std::endl;
#endif

    CACHE::save_rawdata( path_info, sstr.str() );
}
