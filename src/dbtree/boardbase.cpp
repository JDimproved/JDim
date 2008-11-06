// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "boardbase.h"
#include "articlebase.h"
#include "interface.h"

#include "skeleton/msgdiag.h"

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

#include <sstream>
#include <cstring>

enum
{
    SIZE_OF_RAWDATA = 2 * 1024 * 1024
};

using namespace DBTREE;


BoardBase::BoardBase( const std::string& root, const std::string& path_board, const std::string& name )
    : SKELETON::Loadable()
    , m_list_subject_created( false )
    , m_root( root )
    , m_path_board( path_board )
    , m_name( name )
    , m_samba_sec( 0 )
    , m_live_sec( 0 )
    , m_last_access_time( 0 )
    , m_number_max_res( 0 )
    , m_rawdata( 0 )
    , m_read_info( 0 )
    , m_append_articles( false )
    , m_article_null( 0 )
{
    clear();
    clear_load_data();

    memset( &m_write_time, 0, sizeof( struct timeval ) );

    // 板情報はクラスが作られた時点ではまだ読まない
    // BoardBase::read_info() の説明を見ること
}


//
// デストラクタで子ArticleBaseクラスを全部削除
//
BoardBase::~BoardBase()
{
#ifdef _DEBUG
    if( m_list_article.size() ) std::cout << "BoardBase::~BoardBase : " << url_boardbase() << std::endl;
#endif

    clear();

    std::list< ArticleBase* >::iterator it;
    for( it = m_list_article.begin(); it != m_list_article.end(); ++it ) delete ( *it );

    if( m_article_null ) delete m_article_null;
}


ArticleBase* BoardBase::get_article_null()
{
    if( ! m_article_null ) m_article_null = new DBTREE::ArticleBase( "", "", false );
    return m_article_null;
}


bool BoardBase::empty()
{
    return m_root.empty();
}


void BoardBase::set_check_noname( bool check )
{
    m_check_noname = check;
}


//
// url がこの板のものかどうか
//
bool BoardBase::equal( const std::string& url )
{
    if( url.find( get_root() ) == 0
        && url.find( get_path_board() + "/" ) != std::string::npos ) return true;

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


// user agent
const std::string& BoardBase::get_agent()
{
    return CONFIG::get_agent_for_data();
}


// プロキシ
const std::string BoardBase::get_proxy_host()
{
    int mode = get_mode_local_proxy();

    if( mode == DBTREE::PROXY_GLOBAL ){

        if( CONFIG::get_use_proxy_for_data() ) return CONFIG::get_proxy_for_data();
    }
    else if( mode == DBTREE::PROXY_LOCAL ) return get_local_proxy();

    return std::string();
}

const int BoardBase::get_proxy_port()
{
    int mode = get_mode_local_proxy();

    if( mode == DBTREE::PROXY_GLOBAL ) return CONFIG::get_proxy_port_for_data();
    else if( mode == DBTREE::PROXY_LOCAL ) return get_local_proxy_port();

    return 0;
}

// 書き込み用プロキシ
const std::string BoardBase::get_proxy_host_w()
{
    int mode = get_mode_local_proxy_w();

    if( mode == DBTREE::PROXY_GLOBAL ) return get_proxy_host();
    else if( mode == DBTREE::PROXY_LOCAL ) return get_local_proxy_w();

    return std::string();
}

const int BoardBase::get_proxy_port_w()
{
    int mode = get_mode_local_proxy_w();

    if( mode == DBTREE::PROXY_GLOBAL ) return get_proxy_port();
    else if( mode == DBTREE::PROXY_LOCAL ) return get_local_proxy_port_w();

    return 0;
}


// ローカルルール
const std::string BoardBase::localrule()
{
    return "利用できません";
}


// setting.txt
const std::string BoardBase::settingtxt()
{
    return "利用できません";
}

// デフォルトの名無し名
const std::string BoardBase::default_noname()
{
    return "???";
}


// 最大改行数/2
const int BoardBase::line_number()
{
    return 0;
}


// 最大書き込みバイト数
const int BoardBase::message_count()
{
    return 0;
}


//書き込み用クッキー
const std::string BoardBase::cookie_for_write()
{
    if( m_list_cookies_for_write.empty() ) return std::string();

    return *(m_list_cookies_for_write.begin() );
}


void BoardBase::clear()
{
    if( m_rawdata ) free( m_rawdata );
    m_rawdata = NULL;
    m_lng_rawdata = 0;

    m_get_article_url = std::string();
}



//
// 新しくArticleBaseクラスを追加してそのポインタを返す
//
ArticleBase* BoardBase::append_article( const std::string& id, bool cached )
{
    // ベースクラスでは何もしない
    return get_article_null();
}


//
// 書き込み時間更新
//
void BoardBase::update_writetime()
{
    struct timeval tv;
    struct timezone tz;
    if( gettimeofday( &tv, &tz ) == 0 ){

        m_write_time = tv;

#ifdef _DEBUG
        std::cout << "BoardBase::update_writetime : " << m_write_time.tv_sec << std::endl;
#endif
    }
}


//
// 経過時間(秒)
//
const time_t BoardBase::get_write_pass()
{
    time_t ret = 0;
    struct timeval tv;
    struct timezone tz;

    if( m_write_time.tv_sec && gettimeofday( &tv, &tz ) == 0 ) ret = MAX( 0, tv.tv_sec - m_write_time.tv_sec );

    return ret;
}


//
// 書き込み可能までの残り秒
//
time_t BoardBase::get_write_leftsec()
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
void BoardBase::clear_all_post_info()
{
    // キャッシュにあるレスをデータベースに登録
    append_all_article_in_cache();
    if( m_list_article.size() == 0 ) return;

    std::list< ArticleBase* >::iterator it;
    for( it = m_list_article.begin(); it != m_list_article.end(); ++it ) ( *it )->clear_post_info();
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

    // modified 時刻をリセット
    // 自動移転処理後に bbsmenu.html を読み込んだときに、bbsmenu.html の
    // アドレスが古いと二度と自動移転処理しなくなる
    set_date_modified( std::string() );

    // 配下の ArticleBase にも知らせてあげる
    std::list< ArticleBase* >::iterator it;
    for( it = m_list_article.begin(); it != m_list_article.end(); ++it ) ( *it )->update_datbase( url_datbase() );
}


//
// スレの urlをdat型のurlに変換
//
// url がスレッドのURLで無い時はempty()が返る
// もしurlが移転前の旧ホストのものだったら対応するarticlebaseクラスに旧ホスト名を知らせる
//
// (例) url =  "http://www.hoge2ch.net/test/read.cgi/hogeboard/12345/12-15"のとき、
//
// 戻り値 : "http://www.hoge2ch.net/hogeboard/dat/12345.dat",  num_from = 12, num_to = 15
//
const std::string BoardBase::url_dat( const std::string& url, int& num_from, int& num_to, std::string& num_str )
{
    if( empty() ) return std::string();

    JDLIB::Regex regex;
    std::string id; // スレッドのID

#ifdef _DEBUG
    std::cout << "BoardBase::url_dat : url = " << url << std::endl;
#endif
    
    num_from = num_to = 0;

    // dat 型
    const std::string datpath = MISC::replace_str( url_datpath(), "?", "\\?" );
    const std::string query_dat = "^ *(http://.+" + datpath  + ")([1234567890]+" + get_ext() + ") *$";

    // read.cgi型
    const std::string cgipath = MISC::replace_str( url_readcgipath(), "?", "\\?" );
    const std::string query_cgi = "^ *(http://.+" + cgipath + ")([1234567890]+)/?r?(l50)?([1234567890]+)?(-)?([1234567890]+)?.*$";

#ifdef _DEBUG
    std::cout << "query_dat = " << query_dat << std::endl;
    std::cout << "query_cgi = " << query_cgi << std::endl;
#endif

    if( regex.exec( query_dat , url ) ) id = regex.str( 2 );

    else if( regex.exec( query_cgi , url ) ){

        id = regex.str( 2 ) + get_ext(); 

        if( !regex.str( 3 ).empty() ){ // l50
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
            if( !regex.str( 5 ).empty() && !num_to ) num_to = MAX_RESNUMBER + 1;
        }

        // -15 みたいな場合
        else if( num_to != 0 ) num_from = 1;

        num_to = MAX( num_from, num_to );
        num_str = MISC::get_filename( url );
    }

    // どちらでもない(スレのURLでない)場合
    else{

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

#ifdef _DEBUG
    std::cout << "BoardBase::url_dat result : " << url << " ->\n";
    std::cout << "datbase = " << url_datbase() << " id = " << id << " from " << num_from << " to " << num_to
              << " num = " << num_str << std::endl;
#endif

    // もしurl(スレッドのURL)が移転前の旧URLのものだったら対応するarticlebaseクラスに旧ホスト名を教えてあげる
    // ( offlaw による dat落ちスレの読み込み時に使用する )
    if( m_root.find( MISC::get_hostname( url ) ) != 0 ){
#ifdef _DEBUG
        std::cout << "org_host : " << MISC::get_hostname( url ) << std::endl;
#endif
        get_article_create( id )->set_org_host( MISC::get_hostname( url ) );
    }

    return url_datbase() + id;
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
const std::string BoardBase::url_readcgi( const std::string& url, int num_from, int num_to )
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
const std::string BoardBase::url_subject()
{
    if( empty() ) return std::string();

    return url_boardbase() + m_subjecttxt;
}



//
//  ルートアドレス
//
// (例) "http://www.hoge2ch.net/"  (最後に '/' がつく)
//
const std::string BoardBase::url_root()
{
    if( empty() ) return std::string();

    return m_root + "/";
}


//
//  板のベースアドレス
//
// (例) "http://www.hoge2ch.net/hogeboard/"  (最後に '/' がつく)
//
const std::string BoardBase::url_boardbase()
{
    if( empty() ) return std::string();

    return m_root + m_path_board + "/";
}


//
// dat ファイルのURLのベースアドレスを返す
//
// (例) "http://www.hoge2ch.net/hogeboard/dat/" (最後に '/' がつく)
//
const std::string BoardBase::url_datbase()
{
    if( empty() ) return std::string();

    return m_root + url_datpath();
}


//
// dat ファイルのURLのパスを返す
//
// (例) "/hogeboard/dat/"  (最初と最後に '/' がつく)
//
const std::string BoardBase::url_datpath()
{
    if( empty() ) return std::string();

    return m_path_board + m_path_dat + "/";
}



//
// read.cgi のURLのベースアドレスを返す
//
// (例) "http://www.hoge2ch.net/test/read.cgi/hogeboard/"  (最後に '/' がつく)
//
const std::string BoardBase::url_readcgibase()
{
    if( empty() ) return std::string();

    return m_root + url_readcgipath();
}


//
// read.cgi のURLのパスを返す
//
// (例) "/test/read.cgi/hogeboard/"   (最初と最後に '/' がつく)
//
const std::string BoardBase::url_readcgipath()
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
const std::string BoardBase::url_bbscgibase()
{
    if( empty() ) return std::string();

    return m_root + m_path_bbscgi + "/";
}


//
// subbbscgi のURLのベースアドレス
//
// (例) "http://www.hoge2ch.net/test/subbbs.cgi/"  ( 最後に '/' がつく )
//
const std::string BoardBase::url_subbbscgibase()
{
    if( empty() ) return std::string();

    return m_root + m_path_subbbscgi + "/";
}



//
// article のIDから　m_list_article　から　article のポインタを検索して返すだけ
//
// 無ければNULLクラスを返す
//
ArticleBase* BoardBase::get_article( const std::string& id )
{
    if( id.empty() ) return get_article_null();

    // キャッシュにあるレスをデータベースに登録
    append_all_article_in_cache();

    // 線形リストなので遅い
    // TODO : ハッシュにする
    std::list< ArticleBase* >::iterator it;
    for( it = m_list_article.begin(); it != m_list_article.end(); ++it ){

        ArticleBase* art = *( it );
        if( art->get_id() == id ) return art;
    }

    return get_article_null();
}



//
// articleの IDを渡して m_list_article　から article のポインタを検索して返す
//
// ポインタがあった場合は情報ファイルを読み込む
// さらにデータベースにArticleBaseクラスが登録されてない場合はクラスを作成して登録する
//
ArticleBase* BoardBase::get_article_create( const std::string& id )
{
#ifdef _DEBUG
    std::cout << "BoardBase::get_article_create id = " << id << std::endl;
#endif

    ArticleBase* art = get_article( id );

    // get_article() の中で append_all_article_in_cache() を
    // 呼び出しているので、スレがキャッシュ内にある場合は !art->empty() になるのに注意
    if( ! art->empty() ){

#ifdef _DEBUG
        std::cout << "found id = " << art->get_id() << std::endl;
#endif
        // ポインタを返す前にスレッドの情報ファイルを読み込み
        art->read_info();
    }
    else{

        // データベースに無いので新規登録
        //
        // なおget_article()でキャッシュにある
        // スレは全てDBに登録されているので DBに無いということはキャッシュにも無いということ。
        // よって append_article()に  cached = false　をパラメータとして渡す

#ifdef _DEBUG        
        std::cout << "BoardBase::get_article_create : append_article id =  " << id << std::endl;
#endif
        art = append_article( id,
                              false // キャッシュ無し
            );
    }

    return art;
}




//
// article の URL を渡して　m_list_article　から　article のポインタを検索して返す
//
// さらにデータベースにArticleBaseクラスが登録されてない場合はクラスを作成して登録する
//
ArticleBase* BoardBase::get_article_fromURL( const std::string& url )
{
    if( empty() ) return get_article_null();

    // キャッシュ
    if( url == m_get_article_url ) return m_get_article;
    m_get_article_url = url;
    m_get_article = get_article_null();

#ifdef _DEBUG
    std::cout << "BoardBase::get_article_fromURL url = " << url << std::endl;
#endif

    // urlをdat型に変換してからID取得
    int num_from, num_to;
    std::string num_str;
    const std::string urldat = url_dat( url, num_from, num_to, num_str );

    // 板がDBに登録されてないので NULL クラスを返す
    if( urldat.empty() ){

#ifdef _DEBUG
        std::cout << "could not convert url to daturl\nreturn Article_Null\n";
#endif
        return m_get_article;
    }

    // id を抜き出して article クラスのポインタ取得
    std::string id = urldat.substr( url_datbase().length() );

#ifdef _DEBUG
    std::cout << "urldat = " << urldat << std::endl
              << "url_datbase = " << url_datbase() << std::endl
              << "id = " << id << std::endl;
    if( id.empty() ) std::cout << "return Article_Null\n";
#endif

    if( id.empty() ) return m_get_article;

    // get_article_create() 経由で ArticleBase::read_info() から get_article_fromURL()が
    // 再帰呼び出しされることもあるので m_get_article_url を空にしておく
    m_get_article_url = std::string(); 

    m_get_article = get_article_create( id );
    m_get_article_url = url;
    return m_get_article;
}



//
// subject.txt ダウンロード
//
void BoardBase::download_subject()
{
#ifdef _DEBUG
    std::cout << "BoardBase::download_subject " << url_subject() << std::endl;
#endif

    if( empty() ) return;
    if( is_loading() ) return;

    clear();
    m_rawdata = ( char* )malloc( SIZE_OF_RAWDATA );
    m_read_url_boardbase = false;
    m_load_rule_setting = false;

    // オフライン
    if( ! SESSION::is_online() ){

        set_str_code( "" );

        // ディスパッチャ経由でreceive_finish()を呼ぶ
        finish();
        return;
    }

    // オンライン

    m_load_rule_setting = true;

    // subject.txtのキャッシュが無かったら modified をリセット
    std::string path_subject = CACHE::path_board_root_fast( url_boardbase() ) + m_subjecttxt;
    if( CACHE::file_exists( path_subject ) != CACHE::EXIST_FILE ) set_date_modified( std::string() );

    JDLIB::LOADERDATA data;    
    create_loaderdata( data );
    if( ! start_load( data ) ){
        CORE::core_set_command( "update_board", url_subject() );
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
    data.size_buf = CONFIG::get_loader_bufsize();
    data.timeout = CONFIG::get_loader_timeout();
    data.basicauth = get_basicauth();
    data.modified = get_date_modified();
}


//
// ローダよりsubject.txt受信
//
void BoardBase::receive_data( const char* data, size_t size )
{
    memcpy( m_rawdata + m_lng_rawdata , data, size );
    m_lng_rawdata += size;
}


//
// ロード完了
//
void BoardBase::receive_finish()
{
    // 別スレッドでローカルルールとSETTING.TXT のダウンロード開始
    if( m_load_rule_setting ) download_rule_setting();

    m_list_subject.clear();

    bool read_from_cache = false;
    std::string path_subject = CACHE::path_board_root_fast( url_boardbase() ) + m_subjecttxt;
    std::string path_oldsubject = CACHE::path_board_root_fast( url_boardbase() ) + "old-" + m_subjecttxt;

#ifdef _DEBUG
    std::cout << "----------------------------------\nBoardBase::receive_finish code = " << get_str_code() << std::endl;
    std::cout << "size = " << m_lng_rawdata << std::endl;
#endif

    // url_boardbase をロードして移転が起きたかチェック
    if( m_read_url_boardbase ){

#ifdef _DEBUG
        std::cout << "move check\n";
#endif
        m_rawdata[ m_lng_rawdata ] = '\0';
        set_date_modified( std::string() );
        CORE::core_set_command( "update_board", url_subject() );

        if( get_code() == HTTP_OK && std::string( m_rawdata ).find( "window.location.href" ) != std::string::npos ){

#ifdef _DEBUG
            std::cout << m_rawdata << std::endl;
#endif
            JDLIB::Regex regex;
            std::string query = ".*window.location.href=\"([^\"]*)\".*";
            if( regex.exec( query, m_rawdata ) ){

                const std::string new_url = regex.str( 1 );
                int ret = Gtk::RESPONSE_YES;

                if( CONFIG::get_show_movediag() ){

                    const std::string msg = "「" + get_name() + "」は\n\n" + new_url + " に移転しました。\n\nデータベースを更新しますか？";

                    SKELETON::MsgCheckDiag mdiag( NULL,
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
                        CORE::core_set_command( "open_board", url_subject() );
                    }
                }
            }
        }
        else{
            SKELETON::MsgDiag mdiag( NULL, "移転しました\n\n板一覧を更新しますか？", false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_YES_NO );
            mdiag.set_default_response( Gtk::RESPONSE_YES );
            if( mdiag.run() == Gtk::RESPONSE_YES ) CORE::core_set_command( "reload_bbsmenu" );
        }

        clear();
        return;
    }

    // サーバーからtimeoutなどのエラーが返った or 移転
    if( get_code() != HTTP_ERR // HTTP_ERR はローダでの内部のエラー
        && get_code() != HTTP_OK
        && get_code() != HTTP_NOT_MODIFIED ){

        m_lng_rawdata = 0;

        // リダイレクトの場合は移転確認
        if( get_code() == HTTP_REDIRECT ){

            set_date_modified( std::string() );

            if( start_checkking_if_board_moved() ) return;

            CORE::core_set_command( "update_board", url_subject() );
            clear();
            return;
        }

        // サーバからエラーが返ったらキャッシュからデータをロード
        read_from_cache = true;
    }

    // ローダがエラーを返した or not modified or offline ならキャッシュからデータをロード
    if( get_code() == HTTP_ERR
        || get_code() == HTTP_NOT_MODIFIED
        || ! SESSION::is_online() ) read_from_cache = true;

    // キャッシュから読み込み
    if( read_from_cache ){

        m_lng_rawdata = CACHE::load_rawdata( path_subject, m_rawdata, SIZE_OF_RAWDATA );

#ifdef _DEBUG
        std::cout << "read from cache " << path_subject << std::endl;
#endif
    }

#ifdef _DEBUG
        std::cout << "size(final) = " << m_lng_rawdata << std::endl;
#endif

    // データが無い
    if( m_lng_rawdata == 0 ){

        set_date_modified( std::string() );

        // 移転した可能性があるので url_boardbase をロードして解析
        if( SESSION::is_online() && get_code() == HTTP_OK ){
            if( start_checkking_if_board_moved() ) return;
        }

        CORE::core_set_command( "update_board", url_subject() );
        clear();
        return;
    }

    // UTF-8に変換しておく
    JDLIB::Iconv* libiconv = new JDLIB::Iconv( m_charset, "UTF-8" );
    int byte_out;
    const char* rawdata_utf8 = libiconv->convert( m_rawdata , m_lng_rawdata,  byte_out );


    //////////////////////////////
    // データベース更新
    // subject.txtを解析して現行スレだけリスト(m_list_subject)に加える

    // キャッシュにあるレスをデータベースに登録
    append_all_article_in_cache();

    // 一度全てのarticleをdat落ち状態にして subject.txt に
    // 含まれているものだけ parse_subject()の中で通常状態にする
    std::list< ArticleBase* >::iterator it;
    for( it = m_list_article.begin(); it != m_list_article.end(); ++it ){

        int status = ( *it )->get_status();
        status &= ~STATUS_NORMAL;
        status |= STATUS_OLD;
        ( *it )->set_status( status );
    }

    // subject.txtをパースしながらデータベース更新
    parse_subject( rawdata_utf8 );

    // list_subject 更新
    if( ! m_list_subject.empty() ){

#ifdef _DEBUG
        std::cout << "list_subject was updated\n";
#endif

        // DAT落ちなどでsubject.txtに無いスレもリストに加える
        // サブジェクトやロード数などの情報が無いのでスレのinfoファイルから
        // 取得する必要がある
        if( CONFIG::get_show_oldarticle() ){

            for( it = m_list_article.begin(); it != m_list_article.end(); ++it ){

                if( ( *it )->is_cached()
                    && ( ( *it )->get_status() & STATUS_OLD )
                    ){

                    // info 読み込み
                    // TODO : 数が多いとboardビューを開くまで時間がかかるのをなんとかする
#ifdef _DEBUG
                    std::cout << "read article_info : " << ( *it )->get_url() << std::endl;
#endif                
                    ( *it )->read_info();

                    // read_info()で状態が変わる時があるのでDAT落ちにしてからリスト(m_list_subject)に加える
                    int status = ( *it )->get_status();

                    if( status & STATUS_NORMAL ){
                        status &= ~STATUS_NORMAL;
                        status |= STATUS_OLD;
                        ( *it )->set_status( status );
                        ( *it )->save_info( true );
                    }

                    if( ! is_abone_thread( *it ) ) m_list_subject.push_back( *it );
                }
            }
        }

        // オンライン、かつcodeが200か304なら最終アクセス時刻を更新
        if( SESSION::is_online() && ( get_code() == HTTP_OK || get_code() == HTTP_NOT_MODIFIED ) ){

            m_last_access_time = time( NULL );

#ifdef _DEBUG
            std::cout << "access time " << m_last_access_time << std::endl;
#endif
        }

        // オンライン、かつcodeが200なら情報を保存して subject.txt をキャッシュに保存
        if( SESSION::is_online() && get_code() == HTTP_OK ){

            m_last_access_time = time( NULL );

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
                CACHE::save_rawdata( path_subject, m_rawdata, m_lng_rawdata );
            }
        }

        m_list_subject_created = true;
    }

    // list_subject が更新されなかった
    else{

#ifdef _DEBUG
        std::cout << "list_subject was NOT updated\n";
#endif

        set_date_modified( std::string() );
    }

    // コアにデータベース更新を知らせる
    CORE::core_set_command( "update_board", url_subject() );

    delete libiconv;
    clear();
}


//
// url_boardbase をロードして移転したかどうか解析開始
//
// 移転した場合は window.location.href が含まれるテキストがサーバから送られてくる
//
bool BoardBase::start_checkking_if_board_moved()
{
#ifdef _DEBUG
    std::cout << "BoardBase::start_checkking_if_board_moved " << url_boardbase() << std::endl;
#endif

    JDLIB::LOADERDATA data;    
    data.url = url_boardbase();
    data.agent = CONFIG::get_agent_for_data();
    if( CONFIG::get_use_proxy_for_data() ) data.host_proxy = CONFIG::get_proxy_for_data();
    else data.host_proxy = std::string();
    data.port_proxy = CONFIG::get_proxy_port_for_data();
    data.size_buf = CONFIG::get_loader_bufsize();
    data.timeout = CONFIG::get_loader_timeout_img();

    if( start_load( data ) ){
        m_read_url_boardbase = true;
        return true;
    }

    return false;
}


//
// キャッシュのディレクトリ内にあるスレのファイル名のリストを取得
//
std::list< std::string > BoardBase::get_filelist_in_cache()
{
    std::list< std::string >list_out;
    if( empty() ) return list_out;

    std::list< std::string >list_file;
    std::string path_board_root = CACHE::path_board_root_fast( url_boardbase() );

    list_file = CACHE::get_filelist( path_board_root );
    if( ! list_file.size() ) return list_out;

    std::list< std::string >::iterator it = list_file.begin();
    for(; it != list_file.end(); ++it ){

        std::string& file = ( *it );
        if( is_valid( file ) ) list_out.push_back( file );
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

#ifdef _DEBUG
    std::cout << "BoardBase::append_all_article_in_cache\n";
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
        append_article( ( *it ),
                        true  // キャッシュあり
            );
    }
}



//
// 配下の全articlebaseクラスのあぼーん状態の更新
//
void BoardBase::update_abone_all_article()
{
    std::list< ArticleBase* >::iterator it = m_list_article.begin();
    for( ; it != m_list_article.end(); ++it ) ( *it )->update_abone();
}




//
// スレあぼーん判定
//
const bool BoardBase::is_abone_thread( ArticleBase* article )
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

    // レスの数であぼーん
    if( check_number ) if( article->get_number() >= check_number ) return true;

    // スレ立てからの時間であぼーん
    if( check_hour ) if( article->get_hour() >= check_hour ) return true;
    
    // スレあぼーん
    if( check_thread ){
        std::list< std::string >::iterator it = m_list_abone_thread.begin();
        for( ; it != m_list_abone_thread.end(); ++it ){
            if( article->get_subject() == (*it) ) return true;
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
            if( regex.exec( *it, article->get_subject() ) ) return true;
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
            if( regex.exec( *it, article->get_subject() ) ) return true;
        }
    }

    return false;
}


//
// スレあぼーん状態の更新
//
// force = true なら強制更新
//
void BoardBase::update_abone_thread()
{
    // Root::update_abone_all_board() であぼーん状態を全更新するときなど
    // まだスレ一覧にスレを表示していないBoardBaseクラスは更新しない
    if( ! m_list_subject_created ) return;

    // オフラインに切り替えてからキャッシュにあるsubject.txtを再読み込みする
    bool online = SESSION::is_online();
    SESSION::set_online( false );

    download_subject();

    SESSION::set_online( online );
}



//
// あぼーん状態のリセット(情報セットと状態更新を同時におこなう)
//
void BoardBase::reset_abone( std::list< std::string >& ids, std::list< std::string >& names, std::list< std::string >& words, std::list< std::string >& regexs )
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


// あぼ〜ん状態更新(reset_abone()と違って各項目ごと個別におこなう)
void BoardBase::add_abone_id( const std::string& id )
{
    if( empty() ) return;
    if( id.empty() ) return;

    std::string id_tmp = id.substr( strlen( PROTO_ID ) );

    m_list_abone_id.push_back( id_tmp );

    update_abone_all_article();
    CORE::core_set_command( "relayout_all_article" );
}

void BoardBase::add_abone_name( const std::string& name )
{
    if( empty() ) return;
    if( name.empty() ) return;

    m_list_abone_name.push_back( name );

    update_abone_all_article();
    CORE::core_set_command( "relayout_all_article" );
}

void BoardBase::add_abone_word( const std::string& word )
{
    if( empty() ) return;
    if( word.empty() ) return;

    m_list_abone_word.push_back( word );

    update_abone_all_article();
    CORE::core_set_command( "relayout_all_article" );
}


//
// あぼーん状態のリセット(情報セットと状態更新を同時におこなう)
//
void BoardBase::reset_abone_thread( std::list< std::string >& threads,
                                    std::list< std::string >& words, std::list< std::string >& regexs,
                                    const int number, const int hour )
{
    if( empty() ) return;

#ifdef _DEBUG
    std::cout << "BoardBase::reset_abone\n";
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

    update_abone_thread();
}


//
// キャッシュ内のログ検索
//
// datファイルのURL(read.cgi型)を返す
//
std::list< std::string > BoardBase::search_cache( const std::string& query,
                                                  bool mode_or, // 今のところ無視
                                                  bool& stop // 呼出元のスレッドで true にセットすると検索を停止する
    )
{
#ifdef _DEBUG
    std::cout << "BoardBase::search_cache " << query << std::endl;
#endif

    std::list< std::string >list_out;

    if( empty() ) return list_out;

    // キャッシュにあるレスをデータベースに登録
    append_all_article_in_cache();
    if( m_list_article.size() == 0 ) return list_out;

    std::string query_local = MISC::Iconv( query, "UTF-8", get_charset() );
    std::list< std::string > list_query = MISC::split_line( query_local );

    std::string path_board_root = CACHE::path_board_root_fast( url_boardbase() );

    std::list< ArticleBase* >::iterator it;
    for( it = m_list_article.begin(); it != m_list_article.end(); ++it ){

        ArticleBase* article = ( *it );
        if( ! article->is_cached() ) continue;

        std::string path = path_board_root + article->get_id();
        std::string rawdata;

        if( CACHE::load_rawdata( path, rawdata ) > 0 ){

            bool apnd = true;

#ifdef _DEBUG
            std::cout << "load " << path << " size = " << rawdata.size() << " byte" << std::endl;
#endif

            std::list< std::string >::iterator it_query = list_query.begin();
            for( ; it_query != list_query.end(); ++it_query ){

                if( rawdata.find( *it_query ) == std::string::npos ){
                    apnd = false;
                    break;
                }
            }

            if( apnd ){
#ifdef _DEBUG
                std::cout << "found word in " << url_readcgi( article->get_url(), 0, 0 ) << std::endl
                          << article->get_subject() << std::endl;
#endif
                std::string readcgi = url_readcgi( article->get_url(), 0, 0 );
                list_out.push_back( readcgi );
            }
        }

        if( stop ) break;
    }

    return list_out;
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

    std::string modified = cf.get_option( "modified", "" );
    set_date_modified( modified );

    m_modified_localrule = cf.get_option( "modified_localrule", "" );
    m_modified_setting = cf.get_option( "modified_setting", "" );

    m_view_sort_column = cf.get_option( "view_sort_column", -1 );
    m_view_sort_mode = cf.get_option( "view_sort_mode", 0 );
    m_view_sort_pre_column = cf.get_option( "view_sort_pre_column", -1 );
    m_view_sort_pre_mode = cf.get_option( "view_sort_pre_mode", 0 );

    m_check_noname = cf.get_option( "check_noname", false );

    std::string str_tmp;

    // あぼーん id は再起動ごとにリセット
//    str_tmp = cf.get_option( "aboneid", "" );
//    if( ! str_tmp.empty() ) m_list_abone_id = MISC::strtolist( str_tmp );

    // あぼーん name
    str_tmp = cf.get_option( "abonename", "" );
    if( ! str_tmp.empty() ) m_list_abone_name = MISC::strtolist( str_tmp );

    // あぼーん word
    str_tmp = cf.get_option( "aboneword", "" );
    if( ! str_tmp.empty() ) m_list_abone_word = MISC::strtolist( str_tmp );

    // あぼーん regex
    str_tmp = cf.get_option( "aboneregex", "" );
    if( ! str_tmp.empty() ) m_list_abone_regex = MISC::strtolist( str_tmp );

    // スレ あぼーん
    str_tmp = cf.get_option( "abonethread", "" );
    if( ! str_tmp.empty() ) m_list_abone_thread = MISC::strtolist( str_tmp );

    // スレ あぼーん word
    str_tmp = cf.get_option( "abonewordthread", "" );
    if( ! str_tmp.empty() ) m_list_abone_word_thread = MISC::strtolist( str_tmp );

    // スレ あぼーん regex
    str_tmp = cf.get_option( "aboneregexthread", "" );
    if( ! str_tmp.empty() ) m_list_abone_regex_thread = MISC::strtolist( str_tmp );

    // レス数であぼーん
    m_abone_number_thread = cf.get_option( "abonenumberthread", 0 );

    // スレ立てからの経過時間であぼーん
    m_abone_hour_thread = cf.get_option( "abonehourthread", 0 );

    // ローカルプロキシ
    m_mode_local_proxy = cf.get_option( "mode_local_proxy", 0 );
    m_local_proxy = cf.get_option( "local_proxy", "" );
    m_local_proxy_port = cf.get_option( "local_proxy_port", 8080 );
 
    m_mode_local_proxy_w = cf.get_option( "mode_local_proxy_w", 0 );
    m_local_proxy_w = cf.get_option( "local_proxy_w", "" );
    m_local_proxy_port_w = cf.get_option( "local_proxy_port_w", 8080 );

    // 書き込み時のデフォルトの名前とメアド
    m_write_name = cf.get_option( "write_name", "" );
    m_write_mail = cf.get_option( "write_mail", "" );

    // samba24
    m_samba_sec = cf.get_option( "samba_sec", 0 );

    // 実況の秒数
    m_live_sec = cf.get_option( "live_sec", 0 );

    // 最終アクセス時刻
    m_last_access_time = cf.get_option( "last_access_time", 0 );

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
    std::string str_abone_id = MISC::listtostr( m_list_abone_id );
    std::string str_abone_name = MISC::listtostr( m_list_abone_name );
    std::string str_abone_word = MISC::listtostr( m_list_abone_word );
    std::string str_abone_regex = MISC::listtostr( m_list_abone_regex );

    // スレあぼーん情報
    std::string str_abone_thread = MISC::listtostr( m_list_abone_thread );
    std::string str_abone_word_thread = MISC::listtostr( m_list_abone_word_thread );
    std::string str_abone_regex_thread = MISC::listtostr( m_list_abone_regex_thread );
    
    std::ostringstream sstr;
    sstr << "modified = " << get_date_modified() << std::endl
         << "modified_localrule = " << m_modified_localrule << std::endl
         << "modified_setting = " << m_modified_setting << std::endl
         << "view_sort_column = " << m_view_sort_column << std::endl
         << "view_sort_mode = " << m_view_sort_mode << std::endl
         << "view_sort_pre_column = " << m_view_sort_pre_column << std::endl
         << "view_sort_pre_mode = " << m_view_sort_pre_mode << std::endl
         << "check_noname = " << m_check_noname << std::endl

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

         << "local_proxy = " << m_local_proxy << std::endl
         << "local_proxy_port = " << m_local_proxy_port << std::endl
         << "mode_local_proxy_w = " << m_mode_local_proxy_w << std::endl
         << "local_proxy_w = " << m_local_proxy_w << std::endl
         << "local_proxy_port_w = " << m_local_proxy_port_w << std::endl
         << "write_name = " << m_write_name << std::endl
         << "write_mail = " << m_write_mail << std::endl
         << "samba_sec = " << m_samba_sec << std::endl
         << "live_sec = " << m_live_sec << std::endl
         << "last_access_time = " << m_last_access_time << std::endl
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

    std::list< ArticleBase* >::iterator it;
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
