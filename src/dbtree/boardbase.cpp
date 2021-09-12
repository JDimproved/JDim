// ライセンス: GPL2

//#define _DEBUG
//#define _TEST_CACHE
#include "jddebug.h"

#include "boardbase.h"
#include "articlebase.h"
#include "articlehash.h"
#include "interface.h"

#include "skeleton/msgdiag.h"

#include "jdlib/cookiemanager.h"
#include "jdlib/jdiconv.h"
#include "jdlib/jdregex.h"
#include "jdlib/miscutil.h"
#include "jdlib/miscmsg.h"
#include "jdlib/loaderdata.h"
#include "jdlib/confloader.h"


#include "global.h"
#include "httpcode.h"
#include "command.h"
#include "cache.h"
#include "config/globalconf.h"
#include "session.h"
#include "boardcolumnsid.h"

#include <glib/gi18n.h>

#include <algorithm>
#include <cstring>
#include <sstream>


#ifdef _TEST_CACHE
int cache_hit_art = 0;
int cache_nohit_art = 0;
#endif


enum
{
    SIZE_OF_RAWDATA = 2 * 1024 * 1024
};

using namespace DBTREE;


BoardBase::BoardBase( const std::string& root, const std::string& path_board, const std::string& name )
    : SKELETON::Loadable()
    , m_status( STATUS_UNKNOWN )
    , m_view_sort_column( -1 )
    , m_view_sort_mode( -1 )
    , m_view_sort_pre_column( -1 )
    , m_view_sort_pre_mode( -1 )
    , m_root( root )
    , m_path_board( path_board )
    , m_name( name )
{
    clear_load_data();

    // 板情報はクラスが作られた時点ではまだ読まない
    // BoardBase::read_info() の説明を見ること
}


//
// デストラクタで子ArticleBaseクラスを全部削除
//
BoardBase::~BoardBase()
{
#ifdef _DEBUG
    if( m_hash_article.size() ) std::cout << "BoardBase::~BoardBase : " << url_boardbase() << std::endl;
#endif

    clear();

#ifdef _TEST_CACHE
    if( m_hash_article.size() ){
        std::cout << "article cache\n"
                  << "hit = " << cache_hit_art << std::endl
                  << "nohit = " << cache_nohit_art << std::endl
                  << "hit/total*100 = " << (double)(cache_hit_art)/(cache_hit_art+cache_nohit_art)*100. << std::endl;
    }
#endif    
}


ArticleBase* BoardBase::get_article_null()
{
    if( ! m_article_null ) m_article_null = std::make_unique<DBTREE::ArticleBase>( "", "", false );
    return m_article_null.get();
}


bool BoardBase::empty() const
{
    return m_root.empty();
}


//
// url がこの板のものかどうか
//
bool BoardBase::equal( const std::string& url ) const
{
    if( url.rfind( get_root(), 0 ) == 0
        && url.find( get_path_board() + "/", get_root().size() ) != std::string::npos ) return true;

    return false;
}


//
// 名前を変更
//
void BoardBase::update_name( const std::string& name )
{
    if( m_name != name ){

        m_name = name;

       // 表示中のviewの板名表示更新
        CORE::core_set_command( "update_boardname", url_boardbase() );
    }
}


// ユーザエージェント
// ダウンロード用
const std::string& BoardBase::get_agent() const
{
    return CONFIG::get_agent_for_data();
}

// 書き込み用
const std::string& BoardBase::get_agent_w() const
{
    return get_agent();
}


// 読み込み用プロキシ
std::string BoardBase::get_proxy_host() const
{
    const int mode = get_mode_local_proxy();

    if( mode == DBTREE::PROXY_GLOBAL ){

        if( CONFIG::get_use_proxy_for_data() ) return CONFIG::get_proxy_for_data();
    }
    else if( mode == DBTREE::PROXY_LOCAL ) return get_local_proxy();

    return std::string();
}

int BoardBase::get_proxy_port() const
{
    const int mode = get_mode_local_proxy();

    if( mode == DBTREE::PROXY_GLOBAL ) return CONFIG::get_proxy_port_for_data();
    else if( mode == DBTREE::PROXY_LOCAL ) return get_local_proxy_port();

    return 0;
}

std::string BoardBase::get_proxy_basicauth() const
{
    const int mode = get_mode_local_proxy();

    if( mode == DBTREE::PROXY_GLOBAL ) return CONFIG::get_proxy_basicauth_for_data();
    else if( mode == DBTREE::PROXY_LOCAL ) return get_local_proxy_basicauth();

    return std::string();
}


// 書き込み用プロキシ
std::string BoardBase::get_proxy_host_w() const
{
    const int mode = get_mode_local_proxy_w();

    if( mode == DBTREE::PROXY_GLOBAL ){

        if( CONFIG::get_use_proxy_for_data() ) return CONFIG::get_proxy_for_data();
    }
    else if( mode == DBTREE::PROXY_LOCAL ) return get_local_proxy_w();

    return std::string();
}

int BoardBase::get_proxy_port_w() const
{
    const int mode = get_mode_local_proxy_w();

    if( mode == DBTREE::PROXY_GLOBAL ) return CONFIG::get_proxy_port_for_data();
    else if( mode == DBTREE::PROXY_LOCAL ) return get_local_proxy_port_w();

    return 0;
}


std::string BoardBase::get_proxy_basicauth_w() const
{
    const int mode = get_mode_local_proxy_w();

    if( mode == DBTREE::PROXY_GLOBAL ) return CONFIG::get_proxy_basicauth_for_data();
    else if( mode == DBTREE::PROXY_LOCAL ) return get_local_proxy_basicauth_w();

    return std::string();
}


// ローカルルール
std::string BoardBase::localrule() const
{
    return "利用できません";
}


// setting.txt
std::string BoardBase::settingtxt() const
{
    return "利用できません";
}

// デフォルトの名無し名
std::string BoardBase::default_noname() const
{
    return "???";
}


// 最大改行数/2
int BoardBase::line_number() const
{
    return 0;
}


// 最大書き込みバイト数
int BoardBase::message_count() const
{
    return 0;
}


// 特殊文字書き込み可能か( pass なら可能、 change なら不可 )
std::string BoardBase::get_unicode() const
{
    return {};
}


// 板のホストを指定してクッキーを取得
std::string BoardBase::cookie_by_host() const
{
    const JDLIB::CookieManager* cookie_manager = JDLIB::get_cookie_manager();
    const std::string hostname = MISC::get_hostname( get_root(), false );

    return cookie_manager->get_cookie_by_host( hostname );
}


//
// 読み込み用クッキー作成
//
// プロキシの読み込み用設定がoffのとき
// またはプロキシにクッキーを送る設定がonのときはサイトにcookieを送信する
//
std::string BoardBase::cookie_for_request() const
{
    std::string cookie;

    if( ! CONFIG::get_use_proxy_for_data() || CONFIG::get_send_cookie_to_proxy_for_data() ) {
        cookie = cookie_by_host();
    }
    return cookie;
}


//
// 書き込み用クッキー作成
//
// プロキシの書き込み用設定がoffのとき
// またはプロキシにクッキーを送る設定がonのときはサイトにcookieを送信する
//
std::string BoardBase::cookie_for_post() const
{
    std::string cookie;

    if( ! CONFIG::get_use_proxy_for_data() || CONFIG::get_send_cookie_to_proxy_for_data() ) {
        cookie = cookie_by_host();
    }
    return cookie;
}


// 板のホストを指定してクッキーを追加
void BoardBase::set_list_cookies( const std::list< std::string >& list_cookies )
{
    JDLIB::CookieManager* cookie_manager = JDLIB::get_cookie_manager();
    const std::string hostname = MISC::get_hostname( get_root(), false );

    for( const std::string& input : list_cookies ) {
        cookie_manager->feed( hostname, MISC::remove_space( input ) );
    }

    update_hap();
}


// 板のホストを指定してクッキーを削除
void BoardBase::delete_cookies()
{
    JDLIB::CookieManager* cookie_manager = JDLIB::get_cookie_manager();
    const std::string hostname = MISC::get_hostname( get_root(), false );
    cookie_manager->delete_cookie_by_host( hostname );
}


void BoardBase::clear()
{
    m_rawdata.clear();
    m_rawdata.shrink_to_fit();

    m_rawdata_left.clear();
    m_rawdata_left.shrink_to_fit();

    m_get_article_url = std::string();

    m_iconv.reset();

    m_list_artinfo.clear();
}


//
// m_url_update_views に登録されている view に update_board コマンドを送る
//
void BoardBase::send_update_board()
{
#ifdef _DEBUG
    std::cout << "BoardBase::send_update_board\n";
#endif

    // ダウンロードを開始したビュー以外のビューの内容を更新する
    //
    // "update_board" コマンドの後に"update_board_item"を送ると
    // ローディングが終了しているため行を二回更新してしまうので注意
    // 詳しくは BoardViewBase::update_item() を参照
    CORE::core_set_command( "update_board_item", url_boardbase(),
                            std::string() // IDとして空文字を送る
        );


    // ダウンロードを開始したビューの内容を更新する
    std::list< std::string >::iterator it = m_url_update_views.begin();
    for( ; it != m_url_update_views.end(); ++it ){
#ifdef _DEBUG
        std::cout << "update : " << *it << std::endl;
#endif 
        CORE::core_set_command( "update_board", *it );
    }
    m_url_update_views.clear();
}


//
// 新しくArticleBaseクラスを追加してそのポインタを返す
//
ArticleBase* BoardBase::append_article( const std::string& datbase, const std::string& id, const bool cached )
{
    // ベースクラスでは何もしない
    return get_article_null();
}


//
// 書き込み時間更新
//
void BoardBase::update_writetime()
{
    const std::time_t current = std::time( nullptr );
    if( current != std::time_t(-1) ) {
        m_write_time = current;
    }

#ifdef _DEBUG
    std::cout << "BoardBase::update_writetime : " << m_write_time << std::endl;
#endif
}


//
// 経過時間(秒)
//
time_t BoardBase::get_write_pass() const
{
    std::time_t current;
    if( m_write_time && std::time( &current ) != std::time_t(-1) ) {

        return std::max<std::time_t>( 0, current - m_write_time );
    }
    return 0;
}


//
// 書き込み可能までの残り秒
//
time_t BoardBase::get_write_leftsec() const
{
    const time_t mrg = 2;

    if( ! m_samba_sec ) return 0;
    if( ! get_write_pass() ) return 0;

    // ログイン中は書き込み規制無し
    if( SESSION::login2ch() ) return 0;

    return MAX( 0, m_samba_sec + mrg - get_write_pass() );
}


//
// 全書き込み履歴クリア
//
void BoardBase::clear_all_post_history()
{
    // キャッシュにあるレスをデータベースに登録
    append_all_article_in_cache();
    if( m_hash_article.size() == 0 ) return;

    for( ArticleBase* a : m_hash_article ) a->clear_post_history();
}


//
// 全スレの書き込み時間とスレ立て時間の文字列をリセット
//
void BoardBase::reset_all_since_date()
{
    for( ArticleBase* a : m_hash_article ) a->reset_since_date();
}

void BoardBase::reset_all_write_date()
{
    for( ArticleBase* a : m_hash_article ) a->reset_write_date();
}

void BoardBase::reset_all_access_date()
{
    for( ArticleBase* a : m_hash_article ) a->reset_access_date();
}


//
// 最大レス数をセット
//
void BoardBase::set_number_max_res( const int number )
{
#ifdef _DEBUG
    std::cout << "BoardBase::set_number_max_res " << m_number_max_res << " -> " << number << std::endl;
#endif

    m_number_max_res = MAX( 0, MIN( CONFIG::get_max_resnumber(), number ) );

    for( ArticleBase* a : m_hash_article ) a->set_number_max( m_number_max_res );
}



//
// 板情報の取得
//
// コンストラクタで呼ぶと起動時に全ての板の情報を読まなければならなくなるので、
// Root::get_board()で初めて参照されたときに一度だけ実行
//
void BoardBase::read_info()
{
    if( empty() ) return;

    if( ! m_read_info ){ // 一度読んだらもう処理しない

#ifdef _DEBUG
        std::cout << "BoardBase::read_info : " << m_id << std::endl;
#endif
        
        m_read_info = true; 

        // 板の情報ファイル読み込み
        read_board_info();

        // キャッシュからローカルルールとSETTING.TXT のロード
        load_rule_setting();
    }
}



//
// 移転などで板のルートやパスを変更する
//
void BoardBase::update_url( const std::string& root, const std::string& path_board )
{
#ifdef _DEBUG
    std::cout << "BoardBase::update_url\n"
              << m_root << " -> " << root << std::endl
              << m_path_board << " -> " << path_board << std::endl;
#endif

    m_root = root;
    m_path_board = path_board;

    m_query_dat.clear();
    m_query_cgi.clear();
    m_query_kako.clear();

    // modified 時刻をリセット
    // 自動移転処理後に bbsmenu.html を読み込んだときに、bbsmenu.html の
    // アドレスが古いと二度と自動移転処理しなくなる
    set_date_modified( std::string() );

    // 配下の ArticleBase にも知らせてあげる
    const std::string datbase = url_datbase();
    for( ArticleBase* a : m_hash_article ) a->update_datbase( datbase );
}


//
// スレの urlをdat型のurlに変換
//
// url がスレッドのURLで無い時はempty()が返る
// もしurlが移転前の旧ホストのものだったら対応するarticlebaseクラスに旧ホスト名を知らせる
//
// (例) url =  "http://www.hoge2ch.net/test/read.cgi/hogeboard/12345/12-15"のとき、
//
// 戻り値 : "http://www.hoge2ch.net/hogeboard/dat/12345.dat",  num_from = 12, num_to = 15, num_str = "12-15"
//
std::string BoardBase::url_dat( const std::string& url, int& num_from, int& num_to, std::string& num_str )
{
    if( empty() ) return std::string();

    JDLIB::Regex regex;
    const size_t offset = 0;

    std::string id; // スレッドのID

#ifdef _DEBUG
    std::cout << "BoardBase::url_dat : url = " << url << std::endl;
#endif
    
    num_from = num_to = 0;

    if( ! m_query_dat.compiled() ){

        constexpr bool icase = false;
        constexpr bool newline = true;
        constexpr bool usemigemo = false;
        constexpr bool wchar = false;

        // dat 型
        const std::string datpath = MISC::replace_str( url_datpath(), "?", "\\?" );
        const std::string reg_dat = "^ *(https?://.+" + datpath  + ")([0-9]+" + get_ext() + ") *$";
        m_query_dat.set( reg_dat, icase, newline, usemigemo, wchar );

        // read.cgi型
        const std::string cgipath = MISC::replace_str( url_readcgipath(), "?", "\\?" );
        const std::string reg_cgi = "^ *(https?://.+" + cgipath + ")([0-9]+)/?r?(l50)?([0-9]+)?(-)?([0-9]+)?.*$";
        m_query_cgi.set( reg_cgi, icase, newline, usemigemo, wchar );

        // 過去ログかどうか
        const std::string pathboard = MISC::replace_str( m_path_board, "?", "\\?" );
        const std::string reg_kako = "^ *(https?://.+)" + pathboard  + "/kako(/[0-9]+)?/[0-9]+/([0-9]+).html *$";
        m_query_kako.set( reg_kako, icase, newline, usemigemo, wchar );

#ifdef _DEBUG
        std::cout << "reg_dat = " << reg_dat << std::endl;
        std::cout << "reg_cgi = " << reg_cgi << std::endl;
        std::cout << "reg_kako = " << reg_kako << std::endl;
#endif
    }

    if( regex.match( m_query_dat, url, offset ) ) id = regex.str( 2 );

    else if( regex.match( m_query_cgi, url, offset ) ){

        id = regex.str( 2 ) + get_ext(); 

        if( regex.length( 3 ) ){ // l50
            num_from = 1;
            num_to = 50;
        }
        else{

            num_from = atoi( regex.str( 4 ).c_str() );
            num_to = atoi( regex.str( 6 ).c_str() );
        }

        if( num_from != 0 ){

            num_from = MAX( 1, num_from );

            // 12- みたいな場合はとりあえず大きい数字を入れとく
            if( regex.length( 5 ) && !num_to ) num_to = CONFIG::get_max_resnumber() + 1;
        }

        // -15 みたいな場合
        else if( num_to != 0 ) num_from = 1;

        num_to = MAX( num_from, num_to );
        num_str = MISC::get_filename( url );
    }

    // どちらでもない(スレのURLでない)場合
    else{

        if( regex.match( m_query_kako, url, offset ) ){

            std::string url_tmp = regex.str( 1 ) + url_datpath() + regex.str( 3 ) + get_ext();
#ifdef _DEBUG
            std::cout << "kako log -> " << url_tmp << std::endl;
#endif
            return url_dat( url_tmp, num_from, num_to, num_str );
        }

        // 外部板の移転の場合は path_board も変わるときがあるので
        // 移転した場合はurlを置換してからもう一度試す
        std::string old_root;
        std::string old_path_board;
        std::string new_root;
        std::string new_path_board;
        std::string new_url = DBTREE::is_board_moved( url, old_root, old_path_board, new_root, new_path_board );

#ifdef _DEBUG
        std::cout << "old_root = " << old_root << " new_root = " << new_root << std::endl;
        std::cout << "old_path_board = " << old_path_board << " new_path_board = " << new_path_board << std::endl;
#endif

        if( ! new_url.empty() ){

            std::string url_tmp = MISC::replace_str( url, old_root, new_root );
            url_tmp = MISC::replace_str( url_tmp, old_path_board + "/", new_path_board + "/" );

#ifdef _DEBUG
            std::cout << "moved -> " << url_tmp << std::endl;
#endif
            return url_dat( url_tmp, num_from, num_to, num_str );
        }


#ifdef _DEBUG
        std::cout << "not found\n";
#endif
        return std::string();
    }        

    const std::string datbase = url_datbase();

#ifdef _DEBUG
    std::cout << "BoardBase::url_dat result : " << url << " ->\n";
    std::cout << "datbase = " << datbase << " id = " << id << " from " << num_from << " to " << num_to
              << " num = " << num_str << std::endl;
#endif

    // もしurl(スレッドのURL)が移転前の旧URLのものだったら対応するarticlebaseクラスに旧ホスト名を教えてあげる
    // ( offlaw による dat落ちスレの読み込み時に使用する )
    if( m_root.rfind( MISC::get_hostname( url ), 0 ) != 0 ){
#ifdef _DEBUG
        std::cout << "org_host : " << MISC::get_hostname( url ) << std::endl;
#endif
        get_article_create( datbase, id )->set_org_host( MISC::get_hostname( url ) );
    }

    return datbase + id;
}



//
// スレの url を read.cgi型のurlに変換
//
// url がスレッドのURLで無い時はempty()が返る
// num_from と num_to が 0 で無い時はスレ番号を付ける
//
// (例) "http://www.hoge2ch.net/hogeboard/dat/12345.dat",  num_from = 12, num_to = 15 のとき
//
// 戻り値 : "http://www.hoge2ch.net/test/read.cgi/hogeboard/12345/12-15"
//
std::string BoardBase::url_readcgi( const std::string& url, int num_from, int num_to )
{
    if( empty() ) return std::string();

#ifdef _DEBUG
    std::cout << "BoardBase::url_readcgi : " << url  << " from " << num_from << " to " << num_to << std::endl;
#endif

    ArticleBase* article = get_article_fromURL( url );
    if( !article ) return std::string();
    if( article->empty() ) return std::string();

    std::string readcgi = url_readcgibase() + article->get_key() + "/";

#ifdef _DEBUG
    std::cout << "BoardBase::::url_readcgi : to " << readcgi << std::endl;
#endif

    if( num_from > 0 || num_to > 0 ){

        std::ostringstream ss;
        ss << readcgi;

        if( num_from ) ss << num_from;
        if( num_to > num_from ) ss << "-" << num_to;
        return ss.str();
    }

    return readcgi;
}



//
// subject.txt の URLを取得
//
// (例) "http://www.hoge2ch.net/hogeboard/subject.txt"
//
std::string BoardBase::url_subject() const
{
    if( empty() ) return std::string();

    return url_boardbase() + m_subjecttxt;
}



//
//  ルートアドレス
//
// (例) "http://www.hoge2ch.net/"  (最後に '/' がつく)
//
std::string BoardBase::url_root() const
{
    if( empty() ) return std::string();

    return m_root + "/";
}


//
//  板のベースアドレス
//
// (例) "http://www.hoge2ch.net/hogeboard/"  (最後に '/' がつく)
//
std::string BoardBase::url_boardbase() const
{
    if( empty() ) return std::string();

    return m_root + m_path_board + "/";
}


//
// dat ファイルのURLのベースアドレスを返す
//
// (例) "http://www.hoge2ch.net/hogeboard/dat/" (最後に '/' がつく)
//
std::string BoardBase::url_datbase() const
{
    if( empty() ) return std::string();

    return m_root + url_datpath();
}


//
// dat ファイルのURLのパスを返す
//
// (例) "/hogeboard/dat/"  (最初と最後に '/' がつく)
//
std::string BoardBase::url_datpath() const
{
    if( empty() ) return std::string();

    return m_path_board + m_path_dat + "/";
}



//
// read.cgi のURLのベースアドレスを返す
//
// (例) "http://www.hoge2ch.net/test/read.cgi/hogeboard/"  (最後に '/' がつく)
//
std::string BoardBase::url_readcgibase() const
{
    if( empty() ) return std::string();

    return m_root + url_readcgipath();
}


//
// read.cgi のURLのパスを返す
//
// (例) "/test/read.cgi/hogeboard/"   (最初と最後に '/' がつく)
//
std::string BoardBase::url_readcgipath() const
{
    if( empty() ) return std::string();

    return m_path_readcgi + m_path_board + "/";
}


//
// bbscgi のURLのベースアドレス
//
// (例) "http://www.hoge2ch.net/test/bbs.cgi/" ( 最後に '/' がつく )
//
//
std::string BoardBase::url_bbscgibase() const
{
    if( empty() ) return std::string();

    return m_root + m_path_bbscgi + "/";
}


//
// subbbscgi のURLのベースアドレス
//
// (例) "http://www.hoge2ch.net/test/subbbs.cgi/"  ( 最後に '/' がつく )
//
std::string BoardBase::url_subbbscgibase() const
{
    if( empty() ) return std::string();

    return m_root + m_path_subbbscgi + "/";
}



//
// article のID を渡してハッシュから article のポインタを検索して返すだけ
//
// 無ければNullクラスを返す
//
ArticleBase* BoardBase::get_article( const std::string& datbase, const std::string& id )
{
    if( id.empty() ) return get_article_null();

    // キャッシュにあるレスをデータベースに登録
    append_all_article_in_cache();

    ArticleBase* art = m_hash_article.find( datbase, id );
    if( art ) return art;

    return get_article_null();
}



//
// articleの IDを渡して ハッシュから article のポインタを検索して返す
//
// ポインタがあった場合は情報ファイルを読み込む
// さらにデータベースにArticleBaseクラスが登録されてない場合はクラスを作成して登録する
//
ArticleBase* BoardBase::get_article_create( const std::string& datbase, const std::string& id )
{
#ifdef _DEBUG
    std::cout << "BoardBase::get_article_create id = " << id << std::endl;
#endif

    ArticleBase* art = get_article( datbase, id );

    // データベースに無いので新規登録
    // get_article() の中で append_all_article_in_cache() を
    // 呼び出しているので、スレがキャッシュ内にない場合 art->empty() == TRUE になる
    if( art->empty() ){

#ifdef _DEBUG        
        std::cout << "BoardBase::get_article_create : append_article id =  " << id << std::endl;
#endif
        art = append_article( datbase, id,
                              false // キャッシュ無し
            );

        assert( art );
    }

    if( art->is_cached() ) art->read_info();

    return art;
}


ArticleBase* BoardBase::insert( std::unique_ptr<ArticleBase> article )
{
    return m_hash_article.insert( std::move( article ) );
}


//
// article の URL を渡してハッシュから　article のポインタを検索して返す
//
// さらにデータベースにArticleBaseクラスが登録されてない場合はクラスを作成して登録する
//
ArticleBase* BoardBase::get_article_fromURL( const std::string& url )
{
    if( empty() ) return get_article_null();

    // キャッシュ
    if( url == m_get_article_url ){

#ifdef _TEST_CACHE
        ++cache_hit_art;
#endif
        return m_get_article;
    }

    m_get_article_url = url;
    m_get_article = get_article_null();

#ifdef _DEBUG
    std::cout << "BoardBase::get_article_fromURL url = " << url << std::endl;
#endif

    // dat型のURLにしてdatbaseとidを取得
    int num_from, num_to;
    std::string num_str;
    const std::string urldat = url_dat( url, num_from, num_to, num_str );
    if( urldat.empty() ) return m_get_article;

    const std::string datbase = url_datbase();
    const std::string id = urldat.substr( datbase.length() );
    if( id.empty() ) return m_get_article;

#ifdef _DEBUG
    std::cout << "datbase = " << datbase << std::endl
              << "id = " << id << std::endl;
#endif

#ifdef _TEST_CACHE
    ++cache_nohit_art;
#endif

    // get_article_create() 経由で ArticleBase::read_info() から get_article_fromURL()が
    // 再帰呼び出しされることもあるので m_get_article_url を空にしておく
    m_get_article_url = std::string(); 

    m_get_article = get_article_create( datbase, id );
    m_get_article_url = url;
    return m_get_article;
}



//
// subject.txt ダウンロード
//
// url_update_view : CORE::core_set_command( "update_board" ) を送信するビューのアドレス
// read_from_cache : まだスレ一覧を開いていないときにキャッシュのsubject.txtを読み込む
//
void BoardBase::download_subject( const std::string& url_update_view, const bool read_from_cache )
{
#ifdef _DEBUG
    std::cout << "BoardBase::download_subject " << url_boardbase() << std::endl
              << "url_update_view = " << url_update_view << std::endl
              << "read_from_cache = " << read_from_cache << std::endl
              << "empty = " << empty() << std::endl
              << "loading = " << is_loading() << std::endl
              << "views = " << m_url_update_views.size() << std::endl;
#endif

    // ダウンロード中に他のビューから再びダウンロード依頼が来たら
    // ダウンロード終了時にまとめてビューにupdateコマンドを送る
    if( ! url_update_view.empty() ) m_url_update_views.push_back( url_update_view );
    if( m_url_update_views.size() >= 2 ) return;

    if( empty() ) return;
    if( is_loading() ) return;
    if( read_from_cache && m_list_subject_created ) return;

    clear();
    m_read_url_boardbase = false;

    if( read_from_cache ) m_is_online = false;
    else m_is_online = SESSION::is_online();

    m_is_booting = SESSION::is_booting();

    // オフライン
    if( ! m_is_online  ){

        // まだスレ一覧を開いていないときはディスパッチャを通さないで直接subject.txtを読み込む
        if( read_from_cache ) receive_finish();

        // 一度でもスレ一覧を開いている場合はディスパッチャ経由で receive_finish() を呼び出す
        else{

            // HTTP コードは Loadable::callback_dispatch() の中で HTTP_INIT にセットされる
            set_str_code( "" );

            finish();
        }

        return;
    }

    // オンライン

    // subject.txtのキャッシュが無かったら modified をリセット
    std::string path_subject = CACHE::path_board_root_fast( url_boardbase() ) + m_subjecttxt;
    if( CACHE::file_exists( path_subject ) != CACHE::EXIST_FILE ) set_date_modified( std::string() );

    JDLIB::LOADERDATA data;    
    create_loaderdata( data );
    if( ! start_load( data ) ){
        send_update_board();
        clear();
    }
}



//
// ロード用データ作成
//
void BoardBase::create_loaderdata( JDLIB::LOADERDATA& data )
{
    data.url = url_subject();
    data.agent = get_agent();
    data.host_proxy = get_proxy_host();
    data.port_proxy = get_proxy_port();
    data.basicauth_proxy = get_proxy_basicauth();
    data.size_buf = CONFIG::get_loader_bufsize_board();
    data.timeout = CONFIG::get_loader_timeout();
    data.basicauth = get_basicauth();
    data.modified = get_date_modified();
    data.cookie_for_request = cookie_for_request();
}


//
// ローダよりsubject.txt受信
//
void BoardBase::receive_data( const char* data, size_t size )
{
    if( ! size ) return;

    if( m_rawdata.capacity() < SIZE_OF_RAWDATA ) {
        m_rawdata.reserve( SIZE_OF_RAWDATA );
    }
    m_rawdata.append( data, size );

    if( m_read_url_boardbase ) return; // url_boardbase をロードして移転が起きたかチェック中


    //
    // 改行ごとに区切ってUTF8に文字コード変換して解析
    //

    if( m_rawdata_left.capacity() < SIZE_OF_RAWDATA ) {
        m_rawdata_left.reserve( SIZE_OF_RAWDATA );
    }
    if( ! m_iconv ) m_iconv = std::make_unique<JDLIB::Iconv>( "UTF-8", m_charset );

    m_rawdata_left.append( data, size );

    std::size_t byte_in = m_rawdata_left.rfind( '\n' );
    if( byte_in != std::string::npos ) {
        byte_in += 1; // 改行まで含める

        int byte_out;
        const char* rawdata_utf8 = m_iconv->convert( &*m_rawdata_left.begin(), byte_in, byte_out );

        parse_subject( rawdata_utf8 );

        // 残りを先頭に移動
        m_rawdata_left.erase( 0, byte_in );

#ifdef _DEBUG
        std::cout << "BoardBase::receive_data rawdata.size = " << m_rawdata.size() << " size = " << size
                  << " byte_in = " << byte_in << " byte_out = " << byte_out
                  << " rawdata_left.size = " << m_rawdata_left.size() << std::endl;
#endif
    }
}


//
// ロード完了
//
void BoardBase::receive_finish()
{
    // 別スレッドでローカルルールとSETTING.TXT のダウンロード開始
    if( m_is_online ) download_rule_setting();

    m_list_subject.clear();
    m_list_abone_thread_remove.clear();
    m_status &= ~STATUS_UPDATED;

    bool read_from_cache = false;
    const std::string path_subject = CACHE::path_board_root_fast( url_boardbase() ) + m_subjecttxt;
    const std::string path_oldsubject = CACHE::path_board_root_fast( url_boardbase() ) + "old-" + m_subjecttxt;

#ifdef _DEBUG
    std::cout << "----------------------------------\nBoardBase::receive_finish code = " << get_str_code() << std::endl;
    std::cout << "rawdata.size = " << m_rawdata.size() << std::endl;
#endif

    ///////////////////////////////////////////////////////
    //
    // url_boardbase をロードして移転が起きたかチェック
    if( m_read_url_boardbase ){

#ifdef _DEBUG
        std::cout << "move check\n";
#endif
        set_date_modified( std::string() );
        send_update_board();

        // Locationヘッダーで移転先を指定された場合
        if( get_code() == HTTP_MOVED_PERM && ! location().empty() ) {

            // location() は url_boardbase() の移転先 (start_checkking_if_board_moved() を参照)
            if( DBTREE::move_board( url_boardbase(), location() ) ) {
                // 再読み込み
                const std::string str_tab = "false";
                CORE::core_set_command( "open_board", url_boardbase(), str_tab );
            }
        }
        // HTMLの埋め込みスクリプトで移動を指示された場合
        else if( !m_rawdata.empty() && get_code() == HTTP_OK
                && m_rawdata.find( "window.location.href" ) != std::string::npos ) {

#ifdef _DEBUG
            std::cout << m_rawdata << std::endl;
#endif
            JDLIB::Regex regex;
            const size_t offset = 0;
            const bool icase = false;
            const bool newline = true;
            const bool usemigemo = false;
            const bool wchar = false;

            std::string query = ".*window.location.href=\"([^\"]*)\".*";
            if( regex.exec( query, m_rawdata, offset, icase, newline, usemigemo, wchar ) ){

                std::string new_url = regex.str( 1 );
                if( new_url.rfind( "//", 0 ) == 0 ){
                    std::string tmp_url = url_boardbase();
                    size_t pos = tmp_url.find("://");
                    if( pos != std::string::npos )
                        new_url.insert( 0, tmp_url.substr( 0, pos + 1 ) );
                }
                int ret = Gtk::RESPONSE_YES;

                if( CONFIG::get_show_movediag() ){

                    const std::string msg = "「" + get_name() + "」は\n\n" + new_url + " に移転しました。\n\nデータベースを更新しますか？";

                    SKELETON::MsgCheckDiag mdiag( nullptr,
                                                  msg,
                                                  "今後表示しない(常に更新)(_D)",
                                                  Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_YES_NO );

                    mdiag.set_title( "更新確認" );
                    ret = mdiag.run();
                    if( ret == Gtk::RESPONSE_YES && mdiag.get_chkbutton().get_active() ) CONFIG::set_show_movediag( false );
                }

                if( ret == Gtk::RESPONSE_YES ){

                    if( DBTREE::move_board( url_boardbase(), new_url ) ){

                        // 再読み込み
                        const std::string str_tab = "false";
                        CORE::core_set_command( "open_board", url_boardbase(), str_tab );
                    }
                }
            }
        }
        else{
            SKELETON::MsgDiag mdiag( nullptr, "移転しました\n\n板一覧を更新しますか？", false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_YES_NO );
            mdiag.set_default_response( Gtk::RESPONSE_YES );
            if( mdiag.run() == Gtk::RESPONSE_YES ) CORE::core_set_command( "reload_bbsmenu" );
        }

        clear();
        return;
    }
    // 移転チェック終わり
    // 
    ///////////////////////////////////////////////////////


    // サーバーからtimeoutなどのエラーが返った or 移転
    if( get_code() != HTTP_ERR // HTTP_ERR はローダでの内部のエラー
        && get_code() != HTTP_OK
        && get_code() != HTTP_NOT_MODIFIED ){

        //
        // リダイレクト(302)の場合は移転確認
        //
        // ちなみにdatの読み込みでリダイレクト(302)が返ってきたときは、移転かdat落ちか判断出来ないので注意
        // NodeTree2ch::receive_finish()も参照せよ
        //
        if( get_code() == HTTP_REDIRECT || get_code() == HTTP_MOVED_PERM ){

            set_date_modified( std::string() );

            if( start_checkking_if_board_moved() ) return;

            send_update_board();
            clear();
            return;
        }

        // サーバからエラーが返ったらキャッシュからデータをロード
        read_from_cache = true;
    }

    // ローダがエラーを返した 又は not modified 又はオフラインならキャッシュからデータをロード
    if( get_code() == HTTP_ERR
        || get_code() == HTTP_NOT_MODIFIED
        || ! m_is_online ) read_from_cache = true;

    // キャッシュから読み込み
    if( read_from_cache ){

#ifdef _DEBUG
        std::cout << "read from cache " << path_subject << std::endl;
#endif
        m_rawdata.clear();
        m_rawdata_left.clear();

        std::vector<char> rawdata( SIZE_OF_RAWDATA );
        const std::size_t lng = CACHE::load_rawdata( path_subject, rawdata.data(), SIZE_OF_RAWDATA );
        receive_data( rawdata.data(), lng );
    }

#ifdef _DEBUG
    std::cout << "size = " << m_list_artinfo.size() << " rawdata.size = " << m_rawdata.size()
              << " left = " << m_rawdata_left.size() << std::endl;
#endif

    // データが無い
    if( ! m_list_artinfo.size() ){

        set_date_modified( std::string() );

        // 移転した可能性があるので url_boardbase をロードして解析
        if( m_is_online && get_code() == HTTP_OK ){
            if( start_checkking_if_board_moved() ) return;
        }

        send_update_board();
        clear();
        return;
    }

    //////////////////////////////
    // データベース更新
    // subject.txtを解析して現行スレだけsubjectリスト(m_list_subject)に加える

    // キャッシュにあるレスをデータベースに登録
    append_all_article_in_cache();

    // 一度全てのarticleをdat落ち状態にして subject.txt に
    // 含まれているものだけ regist_article()の中で通常状態にする
    for( ArticleBase* article : m_hash_article ) {

        if( read_from_cache && ! article->is_924()
                && article->get_since_time() > m_last_access_time ){
            // キャッシュから読み込む場合にsubject.txtよりも新しいスレは残す
            article->read_info();
            if( ! is_abone_thread( article ) ) m_list_subject.push_back( article );
        }
        else{
            int status = article->get_status();
            status &= ~STATUS_NORMAL;
            status |= STATUS_OLD;
            article->set_status( status );
        }
    }

    m_is_booting = false;

    regist_article( m_is_online );

    // list_subject 更新
    if( ! m_list_subject.empty() ){

#ifdef _DEBUG
        std::cout << "list_subject was updated\n";
#endif

        if( m_is_online ){

            // 既読スレに更新があったかチェック
            std::vector< ArticleBase* >::iterator it_art;
            for( it_art = m_list_subject.begin(); it_art != m_list_subject.end(); ++it_art ){

                if( ( *it_art )->is_cached() && ( *it_art )->get_number() > ( *it_art )->get_number_load() ){

                    m_status |= STATUS_UPDATED;
                    break;
                }
            }

            show_updateicon( false );
        }

        // DAT落ちなどでsubject.txtに無いスレもsubjectリストに加える
        if( CONFIG::get_show_oldarticle() || m_show_oldlog ){

            for( ArticleBase* article : m_hash_article ) {

                if( article->is_cached()
                    && ( article->get_status() & STATUS_OLD )
                    ){


                    // サブジェクトやロード数などの情報が無いのでスレのinfoファイルから
                    // 取得する必要がある
                    // TODO : 数が多いとboardビューを開くまで時間がかかるのをなんとかする
#ifdef _DEBUG
                    std::cout << "read article_info : " << article->get_url() << std::endl;
#endif
                    article->read_info();

                    if( ! is_abone_thread( article ) ) m_list_subject.push_back( article );
                }
            }
        }

        // オンライン、かつcodeが200か304なら最終アクセス時刻を更新
        if( m_is_online && ( get_code() == HTTP_OK || get_code() == HTTP_NOT_MODIFIED ) ){

            m_last_access_time = time( nullptr );

#ifdef _DEBUG
            std::cout << "access time " << m_last_access_time << std::endl;
#endif
        }

        // オンライン、かつcodeが200なら情報を更新・保存して subject.txt をキャッシュに保存
        if( m_is_online && get_code() == HTTP_OK ){

            m_last_access_time = time( nullptr );

#ifdef _DEBUG
            std::cout << "save info and subject.txt\n";
            std::cout << "rename " << path_subject << " " << path_oldsubject << std::endl;
            std::cout << "save " << path_subject << std::endl;    
#endif

            save_info();

            // subject.txtをキャッシュ
            if( CACHE::mkdir_boardroot( url_boardbase() ) ){

                // 古いファイルをrename
                if( CACHE::file_exists( path_subject ) == CACHE::EXIST_FILE ){
                    if( rename( path_subject.c_str(), path_oldsubject.c_str() ) != 0 ){
                        MISC::ERRMSG( "rename failed " + path_subject );
                    }
                }

                // subject.txt セーブ
                CACHE::save_rawdata( path_subject, m_rawdata );
            }
        }

        m_list_subject_created = true;

    } //  if( ! m_list_subject.empty() )

    // list_subject が更新されなかった
    else{

#ifdef _DEBUG
        std::cout << "list_subject was NOT updated\n";
#endif

        set_date_modified( std::string() );
    }

    // コアにデータベース更新を知らせる
    send_update_board();

    clear();
}


//
// url_boardbase をロードして移転したかどうか解析開始
//
// 移転するとコード200で
//
// <script language="javascript">window.location.href="http://hoge.2ch.net/hoge/"</script>
//
// の様なHTML本文が送られて来るので移転先が分かる
//
bool BoardBase::start_checkking_if_board_moved()
{
#ifdef _DEBUG
    std::cout << "BoardBase::start_checkking_if_board_moved " << url_boardbase() << std::endl;
#endif

    m_rawdata.clear();

    JDLIB::LOADERDATA data;
    data.init_for_data();
    data.url = url_boardbase();
    data.cookie_for_request = cookie_for_request();

    if( start_load( data ) ){
        m_read_url_boardbase = true;
        return true;
    }

    return false;
}


//
// キャッシュのディレクトリ内にあるスレのファイル名のリストを取得
//
std::list<std::string> BoardBase::get_filelist_in_cache() const
{
    std::list<std::string> list_out;
    if( empty() ) return list_out;

    const std::string path_board_root = CACHE::path_board_root_fast( url_boardbase() );

    std::list<std::string> list_file = CACHE::get_filelist( path_board_root );
    if( list_file.empty() ) return list_out;

    for( std::string& file : list_file ) {
        if( is_valid( file ) ) list_out.emplace_back( std::move( file ) );
    }

    return list_out;
}


//
// キャッシュのディレクトリ内にあるスレのファイル名を取得してDBにすべてを登録
//
// 全てのスレのinfoファイルを読むと遅くなるのでこの段階では読まない(DBへの登録のみ)
//
// boardビューに一覧表示するためBoardBaseの派生クラスのparse_subject()を呼び出したり、
// BoardBase::get_article_fromURL()で参照されたときに初めてスレのinfoファイルを読み込む
//
void BoardBase::append_all_article_in_cache()
{
    if( m_append_articles ) return;
    m_append_articles = true;

    const std::string datbase = url_datbase();

#ifdef _DEBUG
    std::cout << "BoardBase::append_all_article_in_cache url = " << datbase << std::endl;
#endif
    
    std::list< std::string >list_file = get_filelist_in_cache();

    std::list< std::string >::iterator it = list_file.begin();
    for(; it != list_file.end(); ++it ){

#ifdef _DEBUG
        std::cout << "append id = " << ( *it ) << std::endl;
#endif

        // キャッシュあり( cached = true ) 指定でDBに登録
        // キャッシュに無いスレはsubject.txtを読み込んだ時に
        // 派生クラスのparse_subject()で登録する。
        append_article( datbase, ( *it ),
                        true  // キャッシュあり
            );
    }
}



//
// 配下の全articlebaseクラスのあぼーん状態の更新
//
void BoardBase::update_abone_all_article()
{
    for( ArticleBase* a : m_hash_article ) a->update_abone();
}




//
// スレあぼーん判定
//
bool BoardBase::is_abone_thread( ArticleBase* article )
{
    if( ! article ) return false;
    if( article->empty() ) return false;

    const int check_number = article->get_number_load() ? 0: ( m_abone_number_thread ? m_abone_number_thread : get_abone_number_global() );
    const int check_hour = article->get_number_load() ? 0: ( m_abone_hour_thread ? m_abone_hour_thread : CONFIG::get_abone_hour_thread() );
    const bool check_thread = ! m_list_abone_thread.empty();
    const bool check_word = ! m_list_abone_word_thread.empty();
    const bool check_regex = ! m_list_abone_regex_thread.empty();
    const bool check_word_global = ! CONFIG::get_list_abone_word_thread().empty();
    const bool check_regex_global = ! CONFIG::get_list_abone_regex_thread().empty();

    if( !check_number && !check_hour && !check_thread && !check_word && !check_regex && !check_word_global && !check_regex_global ) return false;

    JDLIB::Regex regex;
    const size_t offset = 0;
    const bool icase = CONFIG::get_abone_icase();
    const bool newline = true;
    const bool usemigemo = false;
    const bool wchar = CONFIG::get_abone_wchar();

    // レスの数であぼーん
    if( check_number ) if( article->get_number() >= check_number ) return true;

    // スレ立てからの時間であぼーん
    if( check_hour ) if( article->get_hour() >= check_hour ) return true;
    
    // スレあぼーん
    if( check_thread ){
        std::list< std::string >::iterator it = m_list_abone_thread.begin();
        for( ; it != m_list_abone_thread.end(); ++it ){
            if( MISC::remove_space( article->get_subject() ) == MISC::remove_space(*it) ){

                // 対象スレがDat落ちした場合はあぼーんしなかったスレ名をリストから消去する
                // remove_old_abone_thread() も参照
                m_list_abone_thread_remove.push_back( *it );
                return true;
            }
        }
    }

    // ローカル NG word
    if( check_word ){
        std::list< std::string >::iterator it = m_list_abone_word_thread.begin();
        for( ; it != m_list_abone_word_thread.end(); ++it ){
            if( article->get_subject().find( *it ) != std::string::npos ) return true;
        }
    }

    // ローカル NG regex
    if( check_regex ){
        std::list< std::string >::iterator it = m_list_abone_regex_thread.begin();
        for( ; it != m_list_abone_regex_thread.end(); ++it ){
            if( regex.exec( *it, article->get_subject(), offset, icase, newline, usemigemo, wchar ) ) return true;
        }
    }

    // 全体 NG word
    if( check_word_global ){
        std::list< std::string >::iterator it = CONFIG::get_list_abone_word_thread().begin();
        for( ; it != CONFIG::get_list_abone_word_thread().end(); ++it ){
            if( article->get_subject().find( *it ) != std::string::npos ) return true;
        }
    }

    // 全体 NG regex
    if( check_regex_global ){
        std::list< std::string >::iterator it = CONFIG::get_list_abone_regex_thread().begin();
        for( ; it != CONFIG::get_list_abone_regex_thread().end(); ++it ){
            if( regex.exec( *it, article->get_subject(), offset, icase, newline, usemigemo, wchar ) ) return true;
        }
    }

    return false;
}


//
// subject.txtのロード後にdat落ちしたスレッドをスレあぼーんのリストから取り除く
//
// is_abone_thread() も参照せよ
//
void BoardBase::remove_old_abone_thread()
{
    if( m_cancel_remove_abone_thread ) return;
    if( CONFIG::get_remove_old_abone_thread() == 2 ) return;
    if( m_list_abone_thread.empty() ) return;
    if( m_list_abone_thread.size() == m_list_abone_thread_remove.size() ) return;

#ifdef _DEBUG
    std::cout << "BoardBase::remove_old_abone_thread\n";
    std::list< std::string >::const_iterator it1 = m_list_abone_thread.begin();
    for( ; it1 != m_list_abone_thread.end(); ++it1 ) std::cout << ( *it1 ) << std::endl;
    std::cout << "->\n";
    std::list< std::string >::const_iterator it2 = m_list_abone_thread_remove.begin();
    for( ; it2 != m_list_abone_thread_remove.end(); ++it2 ) std::cout << ( *it2 ) << std::endl;
#endif

    if( CONFIG::get_remove_old_abone_thread() == 0 ){

        SKELETON::MsgCheckDiag mdiag( nullptr,
                                      "NGスレタイトルに登録したスレがdat落ちしました。\n\nNGスレタイトルから除外しますか？\n後で板のプロパティから削除する事も可能です。",
                                      "今後表示しない(_D)",
                                      Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_NONE );

        mdiag.add_default_button( g_dgettext( GTK_DOMAIN, "_No" ), Gtk::RESPONSE_NO );
        mdiag.add_button( g_dgettext( GTK_DOMAIN, "_Yes" ), Gtk::RESPONSE_YES );

        const int ret = mdiag.run();

        // 一度いいえを選択したらあとは再起動するまでダイアログを表示しない
        if( ret != Gtk::RESPONSE_YES ) m_cancel_remove_abone_thread = true; 

        if( mdiag.get_chkbutton().get_active() ){

            if( ret == Gtk::RESPONSE_YES ) CONFIG::set_remove_old_abone_thread( 1 );
            else CONFIG::set_remove_old_abone_thread( 2 );
        }

        if( ret != Gtk::RESPONSE_YES ) return;
    }

    std::list< std::string >::iterator it = m_list_abone_thread.begin();
    for( ; it != m_list_abone_thread.end(); ++it ){

        if( std::find( m_list_abone_thread_remove.begin(), m_list_abone_thread_remove.end(), *it )
            == m_list_abone_thread_remove.end() ){

            m_list_abone_thread.erase( it );
            it = m_list_abone_thread.begin();
            continue;
        }
    }
}


//
// スレあぼーん情報を更新した時に対応するスレ一覧の表示を更新する
//
// CONFIG::set_abone_number_thread() などでグローバル設定をした後などに呼び出す
//
// redraw : スレ一覧の表示更新を行う
//
void BoardBase::update_abone_thread( const bool redraw )
{
#ifdef _DEBUG
    std::cout << "BoardBase::update_abone_thread\n";
#endif

    // まだスレ一覧にスレを表示していないBoardBaseクラスは更新しない
    if( ! m_list_subject_created ) return;

    // オフラインに切り替えてからキャッシュにあるsubject.txtを再読み込みする
    const bool online = SESSION::is_online();
    SESSION::set_online( false );

    download_subject( ( redraw ? url_boardbase() : std::string() ), false );

    SESSION::set_online( online );
}



//
// 板レベルでのあぼーん状態のリセット(情報セットとスレビューの表示更新を同時におこなう)
//
void BoardBase::reset_abone_board( const std::list< std::string >& ids,
                                   const std::list< std::string >& names,
                                   const std::list< std::string >& words,
                                   const std::list< std::string >& regexs )
{
    if( empty() ) return;

    // 前後の空白と空白行を除く

    m_list_abone_id = MISC::remove_space_from_list( ids );
    m_list_abone_id = MISC::remove_nullline_from_list( m_list_abone_id );

    m_list_abone_name = MISC::remove_space_from_list( names );
    m_list_abone_name = MISC::remove_nullline_from_list( m_list_abone_name );

    m_list_abone_word = MISC::remove_space_from_list( words );
    m_list_abone_word = MISC::remove_nullline_from_list( m_list_abone_word );

    m_list_abone_regex = MISC::remove_space_from_list( regexs );
    m_list_abone_regex = MISC::remove_nullline_from_list( m_list_abone_regex );

    update_abone_all_article();
    CORE::core_set_command( "relayout_all_article" );
}


// 板レベルでのあぼ〜ん状態更新(reset_abone()と違って各項目ごと個別におこなう。スレビューの表示更新も同時におこなう)
void BoardBase::add_abone_id_board( const std::string& id )
{
    if( empty() ) return;
    if( id.empty() ) return;

    std::string id_tmp = id.substr( strlen( PROTO_ID ) );

    m_list_abone_id.push_back( id_tmp );

    update_abone_all_article();
    CORE::core_set_command( "relayout_all_article" );
}

void BoardBase::add_abone_name_board( const std::string& name )
{
    if( empty() ) return;
    if( name.empty() ) return;

    m_list_abone_name.push_back( name );

    update_abone_all_article();
    CORE::core_set_command( "relayout_all_article" );
}

void BoardBase::add_abone_word_board( const std::string& word )
{
    if( empty() ) return;
    if( word.empty() ) return;

    m_list_abone_word.push_back( word );

    update_abone_all_article();
    CORE::core_set_command( "relayout_all_article" );
}


//
// スレあぼーん状態のリセット
//
// redraw : スレ一覧の表示更新を行う
//
void BoardBase::reset_abone_thread( const std::list< std::string >& threads,
                                    const std::list< std::string >& words,
                                    const std::list< std::string >& regexs,
                                    const int number,
                                    const int hour,
                                    const bool redraw
    )
{
    if( empty() ) return;

#ifdef _DEBUG
    std::cout << "BoardBase::reset_abone_thread\n";
#endif

    // 前後の空白と空白行を除く

    m_list_abone_thread = MISC::remove_space_from_list( threads );
    m_list_abone_thread = MISC::remove_nullline_from_list( m_list_abone_thread );

    m_list_abone_word_thread = MISC::remove_space_from_list( words );
    m_list_abone_word_thread = MISC::remove_nullline_from_list( m_list_abone_word_thread );

    m_list_abone_regex_thread = MISC::remove_space_from_list( regexs );
    m_list_abone_regex_thread = MISC::remove_nullline_from_list( m_list_abone_regex_thread );

    m_abone_number_thread = number;
    m_abone_hour_thread = hour;

    update_abone_thread( redraw );
}


//
// 読み込み用ローカルプロキシ設定
//
void BoardBase::set_local_proxy( const std::string& proxy )
{
    m_local_proxy = proxy;
    m_local_proxy_basicauth = std::string();
    if( proxy.empty() ) return;

    // basic認証
    JDLIB::Regex regex;
    const size_t offset = 0;
    const bool icase = false;
    const bool newline = true;
    const bool usemigemo = false;
    const bool wchar = false;

    if( regex.exec( "([^/]+:[^/]+@)(.+)$" , proxy, offset, icase, newline, usemigemo, wchar ) )
    {
        m_local_proxy_basicauth = regex.str( 1 ).substr( 0, regex.length( 1 ) - 1 );
        m_local_proxy = regex.str( 2 );
    }
}


//
// 書き込み用ローカルプロキシ設定
//
void BoardBase::set_local_proxy_w( const std::string& proxy )
{
    m_local_proxy_w = proxy;
    m_local_proxy_basicauth_w = std::string();
    if( proxy.empty() ) return;

    // basic認証
    JDLIB::Regex regex;
    const size_t offset = 0;
    const bool icase = false;
    const bool newline = true;
    const bool usemigemo = false;
    const bool wchar = false;

    if( regex.exec( "([^/]+:[^/]+@)(.+)$" , proxy, offset, icase, newline, usemigemo, wchar ) )
    {
        m_local_proxy_basicauth_w = regex.str( 1 ).substr( 0, regex.length( 1 ) - 1 );
        m_local_proxy_w = regex.str( 2 );
    }
}


//
// キャッシュ内のログ検索
//
// ArticleBase のアドレスをリスト(list_article)にセットして返す
// query が空の時はキャッシュにあるログを全てヒットさせる
// bm がtrueの時、しおりが付いている(スレ一覧でしおりを付けた or レスに一つでもしおりが付いている)スレのみを対象に検索する
//
void BoardBase::search_cache( std::vector< DBTREE::ArticleBase* >& list_article,
                              const std::string& query,
                              const bool mode_or, // 今のところ無視
                              const bool bm,
                              const bool stop // 呼出元のスレッドで true にセットすると検索を停止する
    )
{
#ifdef _DEBUG
    std::cout << "BoardBase::search_cache " << url_boardbase() << std::endl;
#endif

    if( empty() ) return;

    // キャッシュにあるレスをデータベースに登録
    append_all_article_in_cache();
    if( m_hash_article.size() == 0 ) return;

    const bool append_all = query.empty();
    const std::string query_local = MISC::Iconv( query, get_charset(), "UTF-8" );
    const std::list< std::string > list_query = MISC::split_line( query_local );

    const std::string path_board_root = CACHE::path_board_root_fast( url_boardbase() );

    for( ArticleBase* article : m_hash_article ) {

        if( stop ) break;

        if( ! article->is_cached() ) continue;

        // しおりがついているスレだけ追加
        if( bm ){
            article->read_info();
            if( ! article->is_bookmarked_thread() && ! article->get_num_bookmark() ) continue;
        }

        // 全て追加
        if( append_all ){

            article->read_info();

#ifdef _DEBUG
            std::cout << "append " << article->get_subject() << " bm = " << article->get_num_bookmark() << std::endl;
#endif
            list_article.push_back( article );
            continue;
        }

        const std::string path = path_board_root + article->get_id();
        std::string rawdata;

        if( CACHE::load_rawdata( path, rawdata ) > 0 ){

            bool apnd = true;

#ifdef _DEBUG
            std::cout << "load " << path << " size = " << rawdata.size() << " byte" << std::endl;
#endif

            std::list< std::string >::const_iterator it_query = list_query.begin();
            for( ; it_query != list_query.end(); ++it_query ){

                // 今の所 AND だけ対応
                if( rawdata.find( *it_query ) == std::string::npos ){
                    apnd = false;
                    break;
                }
            }

            if( apnd ){

                article->read_info();

#ifdef _DEBUG
                std::cout << "found word in " << url_readcgi( article->get_url(), 0, 0 ) << std::endl
                          << article->get_subject() << std::endl;
#endif
                list_article.push_back( article );
            }
        }
    }
}


// datファイルのインポート
// 成功したらdat型のurlを返す
std::string BoardBase::import_dat( const std::string& filename )
{
    if( empty() ) return std::string();
    if( CACHE::file_exists( filename ) != CACHE::EXIST_FILE ){
        SKELETON::MsgDiag mdiag( nullptr, "datファイルが存在しません" );
        mdiag.run();
        return std::string();
    }

    const std::string id = MISC::get_filename( filename );
    if( id.empty() || ! is_valid( id ) ){
        SKELETON::MsgDiag mdiag( nullptr, "ファイル名が正しくありません" );
        mdiag.run();
        return std::string();
    }

    const std::string file_to = CACHE::path_board_root_fast( url_boardbase() ) + id;

#ifdef _DEBUG
    std::cout << "BoardBase::import_dat cp " << filename
              << " " << file_to << std::endl;
#endif

    const std::string datbase = url_datbase();
    ArticleBase* art = get_article_create( datbase, id );
    if( ! art->is_cached() ){

        CACHE::jdcopy( filename, file_to );
        if( CACHE::file_exists( file_to ) == CACHE::EXIST_FILE ){
            art->set_cached( true );
            art->read_info();
        }
        else{
            SKELETON::MsgDiag mdiag( nullptr, "datファイルのコピーに失敗しました" );
            mdiag.run();
            return std::string();
        }
    }

    return art->get_url();
}


//
// board.info( jd用 ) を読む
// BoardBase::read_info() も参照すること
//
void BoardBase::read_board_info()
{
    if( empty() ) return;
    
    std::string path_info = CACHE::path_jdboard_info( url_boardbase() );

#ifdef _DEBUG
    std::cout << "BoardBase::read_board_info " << path_info << std::endl;
#endif    

    JDLIB::ConfLoader cf( path_info, std::string() );

    std::string modified = cf.get_option_str( "modified", "" );
    set_date_modified( modified );

    m_modified_localrule = cf.get_option_str( "modified_localrule", std::string() );
    m_modified_setting = cf.get_option_str( "modified_setting", std::string() );

    m_view_sort_column = cf.get_option_int( "view_sort_column", -1, -1, COL_VISIBLE_END-1 );
    m_view_sort_mode = cf.get_option_int( "view_sort_mode", SORTMODE_ASCEND, 0, SORTMODE_NUM -1 );
    m_view_sort_pre_column = cf.get_option_int( "view_sort_pre_column", -1, -1, COL_VISIBLE_END-1 );
    m_view_sort_pre_mode = cf.get_option_int( "view_sort_pre_mode", SORTMODE_ASCEND, 0, SORTMODE_NUM -1 );

    m_check_noname = cf.get_option_bool( "check_noname", false );

    m_show_oldlog = cf.get_option_bool( "show_oldlog", false );

    std::string str_tmp;

    // あぼーん id は再起動ごとにリセット
//    str_tmp = cf.get_option( "aboneid", std::string() );
//    if( ! str_tmp.empty() ) m_list_abone_id = MISC::strtolist( str_tmp );

    // あぼーん name
    str_tmp = cf.get_option_str( "abonename", "" );
    if( ! str_tmp.empty() ) m_list_abone_name = MISC::strtolist( str_tmp );

    // あぼーん word
    str_tmp = cf.get_option_str( "aboneword", "" );
    if( ! str_tmp.empty() ) m_list_abone_word = MISC::strtolist( str_tmp );

    // あぼーん regex
    str_tmp = cf.get_option_str( "aboneregex", "" );
    if( ! str_tmp.empty() ) m_list_abone_regex = MISC::strtolist( str_tmp );

    // スレ あぼーん
    str_tmp = cf.get_option_str( "abonethread", "" );
    if( ! str_tmp.empty() ) m_list_abone_thread = MISC::strtolist( str_tmp );

    // スレ あぼーん word
    str_tmp = cf.get_option_str( "abonewordthread", "" );
    if( ! str_tmp.empty() ) m_list_abone_word_thread = MISC::strtolist( str_tmp );

    // スレ あぼーん regex
    str_tmp = cf.get_option_str( "aboneregexthread", "" );
    if( ! str_tmp.empty() ) m_list_abone_regex_thread = MISC::strtolist( str_tmp );

    // レス数であぼーん
    m_abone_number_thread = cf.get_option_int( "abonenumberthread", 0, 0, 9999 );

    // スレ立てからの経過時間であぼーん
    m_abone_hour_thread = cf.get_option_int( "abonehourthread", 0, 0, 9999 );

    // ローカルプロキシ
    m_mode_local_proxy = cf.get_option_int( "mode_local_proxy", PROXY_GLOBAL, 0, PROXY_NUM -1 );
    str_tmp = cf.get_option_str( "local_proxy", "" );
    set_local_proxy( str_tmp );
    m_local_proxy_port = cf.get_option_int( "local_proxy_port", 8080, 1, 65535 );
 
    m_mode_local_proxy_w = cf.get_option_int( "mode_local_proxy_w", PROXY_GLOBAL, 0, PROXY_NUM -1 );
    str_tmp = cf.get_option_str( "local_proxy_w", "" );
    set_local_proxy_w( str_tmp );
    m_local_proxy_port_w = cf.get_option_int( "local_proxy_port_w", 8080, 1, 65535 );

    // 書き込み時のデフォルトの名前とメアド
    m_write_name = cf.get_option_str( "write_name", "" );
    m_write_mail = cf.get_option_str( "write_mail", "" );

    if( ! m_write_name.empty()
        && ( m_write_name == CONFIG::get_write_name()
          || ( m_write_name == JD_NAME_BLANK && CONFIG::get_write_name().empty() ) )
        ){
#ifdef _DEBUG
        std::cout << "reset name = " << m_write_name << std::endl;
#endif
        m_write_name = std::string();
    }
    if( ! m_write_mail.empty()
        && ( m_write_mail == CONFIG::get_write_mail()
          || ( m_write_mail == JD_MAIL_BLANK && CONFIG::get_write_mail().empty() ) )
        ){
#ifdef _DEBUG
        std::cout << "reset mail = " << m_write_mail << std::endl;
#endif
        m_write_mail = std::string();
    }

    // samba24
    m_samba_sec = cf.get_option_int( "samba_sec", 0, 0, 65535 );

    // 実況の秒数
    m_live_sec = cf.get_option_int( "live_sec", 0, 0, 65535 );

    // 最終アクセス時刻
    m_last_access_time = cf.get_option_int( "last_access_time", 0, 0, 2147483647 );

    // ステータス
    m_status = cf.get_option_int( "status", STATUS_UNKNOWN, 0, 8192 );

    // 最大レス数
    m_number_max_res = cf.get_option_int( "max_res", get_default_number_max_res(), 0, CONFIG::get_max_resnumber() );

#ifdef _DEBUG
    std::cout << "modified = " << get_date_modified() << std::endl;
#endif
}


//
// 情報保存
//
void BoardBase::save_info()
{
    if( empty() ) return;

    if( ! CACHE::mkdir_boardroot( url_boardbase() ) ) return;

    save_jdboard_info();
    save_summary();
    save_board_info();
}
    


//
// board.info( jd用 ) セーブ
//
void BoardBase::save_jdboard_info()
{
    std::string path_info = CACHE::path_jdboard_info( url_boardbase() );

#ifdef _DEBUG
    std::cout << "BoardBase::save_jdboard_info file = " << path_info << std::endl;
#endif

    // あぼーん情報
//    std::string str_abone_id = MISC::listtostr( m_list_abone_id );
    std::string str_abone_name = MISC::listtostr( m_list_abone_name );
    std::string str_abone_word = MISC::listtostr( m_list_abone_word );
    std::string str_abone_regex = MISC::listtostr( m_list_abone_regex );

    // スレあぼーん情報
    std::string str_abone_thread = MISC::listtostr( m_list_abone_thread );
    std::string str_abone_word_thread = MISC::listtostr( m_list_abone_word_thread );
    std::string str_abone_regex_thread = MISC::listtostr( m_list_abone_regex_thread );

    // ローカルプロキシ
    std::string local_proxy;
    if( m_local_proxy_basicauth.empty() ) local_proxy = m_local_proxy;
    else local_proxy = m_local_proxy_basicauth + "@" + m_local_proxy;

    std::string local_proxy_w;
    if( m_local_proxy_basicauth_w.empty() ) local_proxy_w = m_local_proxy_w;
    else local_proxy_w = m_local_proxy_basicauth_w + "@" + m_local_proxy_w;
    
    std::ostringstream sstr;
    sstr << "modified = " << get_date_modified() << std::endl
         << "modified_localrule = " << m_modified_localrule << std::endl
         << "modified_setting = " << m_modified_setting << std::endl
         << "view_sort_column = " << m_view_sort_column << std::endl
         << "view_sort_mode = " << m_view_sort_mode << std::endl
         << "view_sort_pre_column = " << m_view_sort_pre_column << std::endl
         << "view_sort_pre_mode = " << m_view_sort_pre_mode << std::endl
         << "check_noname = " << m_check_noname << std::endl
         << "show_oldlog = " << m_show_oldlog << std::endl

    // IDは再起動ごとにリセット
//         << "aboneid = " << str_abone_id << std::endl
         << "abonename = " << str_abone_name << std::endl
         << "aboneword = " << str_abone_word << std::endl
         << "aboneregex = " << str_abone_regex << std::endl

         << "abonethread = " << str_abone_thread << std::endl
         << "abonewordthread = " << str_abone_word_thread << std::endl
         << "aboneregexthread = " << str_abone_regex_thread << std::endl
         << "abonenumberthread = " << m_abone_number_thread << std::endl
         << "abonehourthread = " << m_abone_hour_thread << std::endl
         << "mode_local_proxy = " << m_mode_local_proxy << std::endl

         << "local_proxy = " << local_proxy << std::endl
         << "local_proxy_port = " << m_local_proxy_port << std::endl
         << "mode_local_proxy_w = " << m_mode_local_proxy_w << std::endl
         << "local_proxy_w = " << local_proxy_w << std::endl
         << "local_proxy_port_w = " << m_local_proxy_port_w << std::endl
         << "write_name = " << m_write_name << std::endl
         << "write_mail = " << m_write_mail << std::endl
         << "samba_sec = " << m_samba_sec << std::endl
         << "live_sec = " << m_live_sec << std::endl
         << "last_access_time = " << m_last_access_time << std::endl
         << "status = " << m_status << std::endl
         << "max_res = " << m_number_max_res << std::endl
    ;

    CACHE::save_rawdata( path_info, sstr.str() );
}



//
// article-summary( navi2ch互換 )保存
//
// navi2chとの互換性のために保存しているだけで article-summary の情報は使わない
//
void BoardBase::save_summary()
{
    std::string path_summary = CACHE::path_article_summary( url_boardbase() );

#ifdef _DEBUG
    std::cout << "BoardBase::save_summary file = " << path_summary << std::endl;
#endif

    int count = 0;
    std::ostringstream sstr_out;
    sstr_out << "(";

    std::vector< ArticleBase* >::iterator it;
    for( it = m_list_subject.begin(); it != m_list_subject.end(); ++it ){

        ArticleBase* art = *( it );
        if( art->is_cached()
            && ( art->get_status() & STATUS_NORMAL ) ){
            if( count ) sstr_out << " ";
            ++count;

            // key
            sstr_out << "(\"" << art->get_key() << "\"";

            // 読んだ位置
            sstr_out << " :seen " << art->get_number_seen();

            // access-time
            sstr_out << " :access-time (" << art->get_access_time_str() << "))";
        }
    }

    sstr_out << ")";

#ifdef _DEBUG
//    std::cout << sstr_out.str() << std::endl;
#endif

    if( count ) CACHE::save_rawdata( path_summary, sstr_out.str() );
}



//
// board.info( navi2ch 互換 ) セーブ
//
// navi2chとの互換性のために保存しているだけで情報は使わない
// 
void BoardBase::save_board_info()
{
    std::string path_info = CACHE::path_board_info( url_boardbase() );

#ifdef _DEBUG
    std::cout << "BoardBase::save_board_info file = " << path_info << std::endl;
#endif
    
    // time だけ更新する(他の情報は使わない)
    std::string bookmark = "nil";
    std::string hide = "nil";
    std::string time = "(time . \"" + get_date_modified() + "\")";
    std::string logo = "nil";

    // board.info 読み込み
    if( CACHE::file_exists( path_info ) == CACHE::EXIST_FILE ){

        std::string str_info;
        CACHE::load_rawdata( path_info, str_info );
#ifdef _DEBUG
        std::cout << "str_info " << str_info << std::endl;
#endif

        std::list< std::string > lists = MISC::get_elisp_lists( str_info );
        std::list< std::string >::iterator it = lists.begin();
        for( ; it != lists.end(); ++it ){
            if( ( *it ).find( "bookmark" ) != std::string::npos ) bookmark = *it;
            if( ( *it ).find( "hide" ) != std::string::npos ) hide = *it;
            if( ( *it ).find( "logo" ) != std::string::npos ) logo = *it;
        }
    }

    std::string str_out = "(" + bookmark + " " + hide + " " + time + " " + logo + ")";

#ifdef _DEBUG
    std::cout << "info = " << str_out << std::endl;
#endif

    CACHE::save_rawdata( path_info, str_out );
}


//
// 配下の全articlebaseクラスの情報保存
//
void BoardBase::save_articleinfo_all()
{
    for( ArticleBase* a : m_hash_article ) a->save_info( false );
}


// 更新可能状態にしてお気に入りやスレ一覧のタブのアイコンに更新マークを表示
// update == true の時に表示。falseなら戻す
void BoardBase::show_updateicon( const bool update )
{
#ifdef _DEBUG
    std::cout << "BoardBase::show_updateicon url = " << url_boardbase()
              << " update = " << update << " status = " << ( m_status & STATUS_UPDATE ) << std::endl;
#endif

    if( update ){

        if( ! ( m_status & STATUS_UPDATE ) ){

#ifdef _DEBUG
            std::cout << "toggle_icon on\n";
#endif

            m_status |= STATUS_UPDATE;

            // スレ一覧のタブのアイコン表示を更新
            CORE::core_set_command( "toggle_board_icon", url_boardbase() );

            // サイドバーのアイコン表示を更新
            CORE::core_set_command( "toggle_sidebar_boardicon", url_datbase() );

            save_info();
        }
    }
    else{

        if( m_status & STATUS_UPDATE ){

#ifdef _DEBUG
            std::cout << "toggle_icon off\n";
#endif

            m_status &= ~STATUS_UPDATE;

            // サイドバーのアイコン表示を戻す
            // スレ一覧のタブのアイコンはBoardViewがロード終了時に自動的に戻す
            CORE::core_set_command( "toggle_sidebar_boardicon", url_boardbase() );

            save_info();
        }
    }
}


// 板の更新チェック時に、更新チェックを行うスレのアドレスのリスト
// キャッシュが存在し、かつdat落ちしていないで新着数が0のスレを速度の順でソートして返す
std::list< std::string > BoardBase::get_check_update_articles()
{
    std::list< std::string > list_url;

    if( empty() ) return list_url;
    if( is_loading() ) return list_url;
    if( m_status & STATUS_UPDATE ) return list_url;
    if( ! m_list_subject_created ) {
        download_subject( std::string(), true );
        return list_url;
    }

#ifdef _DEBUG
    std::cout << "BoardBase::get_check_update_articles url = " << url_boardbase() << std::endl;
#endif

    std::list< int > list_speed;

    for( ArticleBase* article : m_hash_article ) {

        if( article->is_cached()
            && article->get_number()
            && ! ( article->get_status() & STATUS_UPDATE )
            && ! ( article->get_status() & STATUS_OLD )
            && article->get_number() == article->get_number_load()
            ){

            const std::string& url = article->get_url();
            const int speed = article->get_speed();

#ifdef _DEBUG
            std::cout << "added " << url
                      << " number = " << article->get_number()
                      << " load = " << article->get_number_load()
                      << " speed = " << speed
                      << " " << article->get_subject() << std::endl;
#endif

            // 挿入ソート
            std::list< std::string >::iterator it_url = list_url.begin();
            std::list< int >::iterator it_speed = list_speed.begin();
            for( ; it_url != list_url.end(); ++it_url, ++it_speed ) if( ( *it_speed ) < speed ) break;
            list_url.insert( it_url, url );
            list_speed.insert( it_speed, speed );
        }
    }

#ifdef _DEBUG
    std::cout << "result of insert sorting\n";
    std::list< std::string >::const_iterator it_url = list_url.begin();
    std::list< int >::const_iterator it_speed = list_speed.begin();
    for( ; it_url != list_url.end(); ++it_url, ++it_speed ) std::cout << ( *it_speed ) << " / " << ( *it_url ) << std::endl;
#endif 

    return list_url;
}
