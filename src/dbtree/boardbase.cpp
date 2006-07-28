// ライセンス: 最新のGPL

//#define _DEBUG
#include "jddebug.h"

#include "root.h"
#include "boardbase.h"
#include "articlebase.h"

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

#define SIZE_OF_RAWDATA ( 2 * 1024 * 1024 )


using namespace DBTREE;


BoardBase::BoardBase( const std::string& root, const std::string& path_board, const std::string& name )
    : SKELETON::Loadable()
    , m_root( root )
    , m_path_board( path_board )
    , m_name( name )
    , m_rawdata( 0 )
    , m_read_info( 0 )
    , m_save_info( 0 )
    , m_article_null( 0 )
{
    clear();
    clear_load_data();
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


void BoardBase::set_view_sort_column( int column )
{
    m_save_info = true;
    m_view_sort_column = column;
}


void BoardBase::set_view_sort_ascend( bool ascend )
{
    m_save_info = true;
    m_view_sort_ascend = ascend;
}



void BoardBase::set_check_noname( bool check )
{
    m_save_info = true;
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


// user agent
const std::string& BoardBase::get_agent()
{
    return CONFIG::get_agent_for_data();
}


// プロキシ
const std::string BoardBase::get_proxy_host()
{
    if( ! CONFIG::get_use_proxy_for_data() ) return std::string();
    return CONFIG::get_proxy_for_data();
}

const int BoardBase::get_proxy_port()
{
    return CONFIG::get_proxy_port_for_data();
}

// 書き込み用プロキシ
const std::string BoardBase::get_proxy_host_w()
{
    return get_proxy_host();
}

const int BoardBase::get_proxy_port_w()
{
    return get_proxy_port();
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

        // キャッシュにあるレスをデータベースに登録
        append_all_article_in_cache();

        // キャッシュからSETTING.TXT のロード
        load_setting();
    }
}



//
// 移転などで板のルートを変更する
//
void BoardBase::update_root( const std::string& root )
{
    m_root = root;

    // 配下の ArticleBase にも知らせてあげる
    std::list< ArticleBase* >::iterator it;
    for( it = m_list_article.begin(); it != m_list_article.end(); ++it ) ( *it )->update_url( url_datbase() );
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
const std::string BoardBase::url_dat( const std::string& url, int& num_from, int& num_to )
{
    if( empty() ) return std::string();

    JDLIB::Regex regex;
    std::string id; // スレッドのID

#ifdef _DEBUG
    std::cout << "BoardBase::::url_dat : url = " << url << std::endl;
#endif
    
    num_from = num_to = 0;

    // dat 型
    std::string datpath = MISC::replace_str( url_datpath(), "?", "\\?" );
    std::string query_dat = "^ *(http://.+" + datpath  + ")([1234567890]+" + get_ext() + ") *$";

    // read.cgi型
    std::string cgipath = MISC::replace_str( url_readcgipath(), "?", "\\?" );
    std::string query_cgi = "^ *(http://.+" + cgipath + ")([1234567890]+)/?r?(l50)?([1234567890]+)?(-)?([1234567890]+)?.*$";

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
    }

    // どちらでもない(スレのURLでない)場合
    else return std::string();

#ifdef _DEBUG
    std::cout << "BoardBase::url_dat result : " << url << " ->\n";
    std::cout << "datbase = " << url_datbase() << " id = " << id << " from " << num_from << " to " << num_to << std::endl;
#endif

    // もしurl(スレッドのURL)が移転前の旧URLのものだったら対応するarticlebaseクラスに旧ホスト名を教えてあげる
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
// (例) "/hogeboreadcgipath(ard/dat/"  (最初と最後に '/' がつく)
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
ArticleBase* BoardBase::get_article( const std::string id )
{
    if( id.empty() ) return get_article_null();

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
// m_list_article　から article のポインタを検索して返す
//
// ポインタがあった場合は情報ファイルを読み込む
// さらにデータベースにArticleBaseクラスが登録されてない場合はクラスを作成して登録する
//
ArticleBase* BoardBase::get_article_create( const std::string id )
{
#ifdef _DEBUG
    std::cout << "BoardBase::get_article_create id = " << id << std::endl;
#endif

    ArticleBase* art = get_article( id );

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
        // なおRoot::get_board()　経由で呼ばれる BoardBase::read_info() でキャッシュにある
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
ArticleBase* BoardBase::get_article_fromURL( const std::string url )
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
    // 変換できないなら板がDBに登録されてないってことなので NULL クラスを返す
    int num_from, num_to;
    std::string urldat = url_dat( url, num_from, num_to );
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

    m_get_article = get_article_create( id );
    return m_get_article;
}



//
// subject.txt ダウンロード
//
void BoardBase::download_subject()
{
#ifdef _DEBUG
    std::cout << "BoardBase::download_subject() " << url_boardbase() << std::endl;
#endif

    if( empty() ) return;
    if( is_loading() ) return;

    clear();
    m_rawdata = ( char* )malloc( SIZE_OF_RAWDATA );

    // オフライン
    if( ! SESSION::is_online() ){

        set_str_code( "" );

        // ディスパッチャ経由でreceive_finish()を呼ぶ
        finish();
        return;
    }

    // オンライン

    // subject.txtのキャッシュが無かったら modified をリセット
    std::string path_subject = CACHE::path_board_root( url_boardbase() ) + m_subjecttxt;
    if( CACHE::is_file_exists( path_subject ) != CACHE::EXIST_FILE ) set_date_modified( std::string() );

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
    if( ! date_modified().empty() ) data.modified = date_modified();
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
    // 別スレッドでSETTING.TXT のダウンロード開始
    download_setting();

    bool read_from_cache = false;
    std::string path_subject = CACHE::path_board_root( url_boardbase() ) + m_subjecttxt;
    std::string path_oldsubject = CACHE::path_board_root( url_boardbase() ) + "old-" + m_subjecttxt;

#ifdef _DEBUG
    std::cout << "BoardBase::receive_finish code = " << get_str_code() << std::endl;
#endif

    // エラー or 移転
    if( get_code() != HTTP_ERR
        && get_code() != HTTP_OK
        && get_code() != HTTP_NOT_MODIFIED ){

        m_lng_rawdata = 0;

        // 移転した場合
        if( get_code() == HTTP_REDIRECT ){

            Gtk::MessageDialog mdiag( "移転しました\n\n板リストを更新しますか？", false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_OK_CANCEL );
            if( mdiag.run() == Gtk::RESPONSE_OK ) CORE::core_set_command( "reload_bbsmenu" );
        }

        // エラー
        else read_from_cache = true;
    }

    // not modified か offline ならキャッシュからデータをロード
    if( get_code() == HTTP_ERR
        || get_code() == HTTP_NOT_MODIFIED
        || ! SESSION::is_online() ) read_from_cache = true;

    // キャッシュから読み込み
    if( read_from_cache ){

        m_lng_rawdata = CACHE::load_rawdata( path_subject, m_rawdata, SIZE_OF_RAWDATA );

#ifdef _DEBUG
        std::cout << "BoardBase::receive_finish read cache " << path_subject << std::endl;
        std::cout << "size = " << m_lng_rawdata << std::endl;
#endif
    }

    if( m_lng_rawdata == 0 ){

        CORE::core_set_command( "update_board", url_subject() );
        clear();
        return;
    }

    // codeが200なら情報を保存して subject.txt をキャッシュに保存
    if( SESSION::is_online() && get_code() == HTTP_OK ){

#ifdef _DEBUG
        std::cout << "rename " << path_subject << " " << path_oldsubject << std::endl;
        std::cout << "save " << path_subject << std::endl;    
#endif

        m_save_info = true;
        save_info();

        // subject.txtをキャッシュ
        if( CACHE::mkdir_boardroot( url_boardbase() ) ){

            // 古いファイルをrename
            if( CACHE::is_file_exists( path_subject ) == CACHE::EXIST_FILE ){
                if( rename( path_subject.c_str(), path_oldsubject.c_str() ) != 0 ){
                    MISC::ERRMSG( "rename failed " + path_subject );
                }
            }

            // subject.txt セーブ
            CACHE::save_rawdata( path_subject, m_rawdata, m_lng_rawdata );
        }
    }

    // UTF-8に変換しておく
    JDLIB::Iconv* libiconv = new JDLIB::Iconv( m_charset );
    int byte_out;
    const char* rawdata_utf8 = libiconv->convert( m_rawdata , m_lng_rawdata,  byte_out );


    //////////////////////////////
    // データベース更新

    m_list_subject.clear();

    // subject.txtを解析して現行スレだけリストに加える
    std::list< ArticleBase* >::iterator it;
    for( it = m_list_article.begin(); it != m_list_article.end(); ++it ) ( *it )->set_current( false );

    // subject.txtをパースしながらデータベース更新
    parse_subject( rawdata_utf8 );

    // DAT落ちなどでsubject.txtに無いスレもリストに加える
    // サブジェクトやロード数などの情報が無いのでスレのinfoファイルから
    // 取得する必要がある
    if( CONFIG::get_show_oldarticle() ){
        for( it = m_list_article.begin(); it != m_list_article.end(); ++it ){
            if( ( *it )->is_cached() && ! ( *it )->is_current() ){

                // info 読み込み
                // TODO : 数が多いとboardビューを開くまで時間がかかるのをなんとかする
#ifdef _DEBUG
                std::cout << "read article_info << " << ( *it )->get_url() << std::endl;
#endif                
                ( *it )->read_info();
                if( ! get_abone_thread( *it ) ) m_list_subject.push_back( *it );
            }
        }
    }
    // コアにデータベース更新を知らせる
    CORE::core_set_command( "update_board", url_subject() );

    delete libiconv;
    clear();
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
#ifdef _DEBUG
    std::cout << "BoardBase::append_all_article_in_cache\n";
#endif
    
    std::list< std::string >list_file;
    std::string path_board_root = CACHE::path_board_root( url_boardbase() );
    list_file = CACHE::get_filelist( path_board_root );

    std::list< std::string >::iterator it = list_file.begin();
    for(; it != list_file.end(); ++it ){

        std::string& file = ( *it );
        if( is_valid( file ) ){

#ifdef _DEBUG
            std::cout << "append id = " << file << std::endl;
#endif
            // キャッシュあり( cached = false ) 指定でDBに登録
            // キャッシュに無いスレはsubject.txtを読み込んだ時に
            // 派生クラスのparse_subject()で登録する。
            append_article( file,
                            true 
                );
        }
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




// articleがあぼーんされているか
const bool BoardBase::get_abone_thread( ArticleBase* article )
{
    if( ! article ) return false;

    // wordあぼーん
    std::list< std::string >::iterator it = m_list_abone_word_thread.begin();
    for( ; it != m_list_abone_word_thread.end(); ++it ){
        if( ! ( *it ).empty() && article->get_subject().find( *it ) != std::string::npos ) return true;
    }

    return false;
}


//
// スレあぼーん状態の更新
//
void BoardBase::update_abone_thread()
{
    if( m_list_subject.empty() ) return;

    // オフラインに切り替えてからキャッシュにあるsubject.txtを再読み込みする
    bool online = SESSION::is_online();
    SESSION::set_online( false );

    download_subject();

    SESSION::set_online( online );
}



//
// あぼーん状態のリセット(情報セットと状態更新を同時におこなう)
//
void BoardBase::reset_abone_thread( std::list< std::string >& words, std::list< std::string >& regexs )
{
    if( empty() ) return;

#ifdef _DEBUG
    std::cout << "BoardBase::reset_abone\n";
#endif

    m_list_abone_word_thread = MISC::remove_nullline_from_list( words, false );
    m_list_abone_regex_thread = MISC::remove_nullline_from_list( regexs, false );

    update_abone_thread();

    m_save_info = true;
}




//
// board.info( jd用 ) を読む
// BoardBase::read_info() も参照すること
//
void BoardBase::read_board_info()
{
#ifdef _DEBUG
    std::cout << "BoardBase::read_board_info " << m_id << std::endl;
#endif    

    if( empty() ) return;
    
    std::string path_info = CACHE::path_jdboard_info( url_boardbase() );

    JDLIB::ConfLoader cf( path_info, std::string() );

    std::string modified = cf.get_option( "modified", "" );
    set_date_modified( modified );

    m_view_sort_column = cf.get_option( "view_sort_column", -1 );
    m_view_sort_ascend = cf.get_option( "view_sort_ascend", false );
    m_check_noname = cf.get_option( "check_noname", false );

    // スレ あぼーん word
    std::string str_tmp = cf.get_option( "abonewordthread", "" );
    if( ! str_tmp.empty() ) m_list_abone_word_thread = MISC::strtolist( str_tmp );

    // スレ あぼーん regex
    str_tmp = cf.get_option( "aboneregexthread", "" );
    if( ! str_tmp.empty() ) m_list_abone_regex_thread = MISC::strtolist( str_tmp );

#ifdef _DEBUG
    std::cout << "modified = " << date_modified() << std::endl;
#endif
}


//
// 強制情報保存
//
// board viewを閉じたとき(BoardView::~BoardView())などに呼び出す
//
void BoardBase::save_info_force()
{
    m_save_info = true;
    save_info();
}


//
// 情報保存
//
void BoardBase::save_info()
{
    if( empty() ) return;
    if( !m_save_info ) return;
    m_save_info = false;

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
    
    std::ostringstream sstr;
    sstr << "modified = " << date_modified() << std::endl
         << "view_sort_column = " << m_view_sort_column << std::endl
         << "view_sort_ascend = " << m_view_sort_ascend << std::endl
         << "check_noname = " << m_check_noname << std::endl;

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
        if( art->is_cached() && art->is_current() ){
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
    std::cout << sstr_out.str() << std::endl;
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
    std::string time = "(time . \"" + date_modified() + "\")";
    std::string logo = "nil";

    // board.info 読み込み
    if( CACHE::is_file_exists( path_info ) == CACHE::EXIST_FILE ){

        std::string str_info;
        CACHE::load_rawdata( path_info, str_info );
#ifdef _DEBUG
        std::cout << "str_info " << str_info << std::endl;
#endif

        std::list< std::string > lists = MISC::get_elisp_lists( str_info );
        std::list< std::string >::iterator it = lists.begin();
        bookmark = *( it++ );
        hide = *( it++ );
        ++it;
        logo = *( it++ );
    }

    std::string str_out = "(" + bookmark + " " + hide + " " + time + " " + logo + ")";

#ifdef _DEBUG
    std::cout << "info = " << str_out << std::endl;
#endif

    CACHE::save_rawdata( path_info, str_out );
}
