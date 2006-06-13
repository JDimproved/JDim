// ライセンス: 最新のGPL

//#define _DEBUG
#include "jddebug.h"

#include "boardmachi.h"
#include "articlemachi.h"

#include "jdlib/miscutil.h"
#include "jdlib/miscmsg.h"
#include "jdlib/loaderdata.h"

#include <sstream>

using namespace DBTREE;


BoardMachi::BoardMachi( const std::string& root, const std::string& path_board, const std::string& name )
    : BoardBase( root, path_board, name )
{
    // dat のURLは特殊なので url_datpath()をオーバロードする
    set_path_dat( "" );

    // readcgi(read.pl) のURLは特殊なので url_readcgibaseをオーバロードする
    set_path_readcgi( "" );

    set_path_bbscgi( "/bbs/write.cgi" );
    set_path_subbbscgi( "/bbs/write.cgi" );
    set_subjecttxt( "subject.txt" );
    set_ext( "" );
    set_id( path_board.substr( 1 ) ); // 先頭の '/' を除く  
    set_charset( "MS932" );
}



//
// url がこの板のものかどうか
//
bool  BoardMachi::equal( const std::string& url )
{
    if( url.find( get_root() ) == 0 ){

        if( url.find( get_path_board() + "/" ) != std::string::npos ) return true;
        if( url.find( "BBS=" + get_id() ) != std::string::npos ) return true;
    }

    return false;
}



//
// キャッシュのファイル名が正しいか
//
bool BoardMachi::is_valid( const std::string& filename )
{
    if( filename.length() != 10 ) return false;

    unsigned int dig, n;
    MISC::str_to_uint( filename.c_str(), dig, n );
    if( dig != n ) return false;
    if( dig != 10 ) return false;
        
    return true;
}




//
// 新しくArticleBaseクラスを追加してそのポインタを返す
//
// cached : HDD にキャッシュがあるならtrue
//
ArticleBase* BoardMachi::append_article( const std::string& id, bool cached )
{
    if( empty() ) return get_article_null();

    ArticleBase* article = new DBTREE::ArticleMachi( url_datbase(), id, cached );
    if( article ) get_list_article().push_back( article );
    else return get_article_null();

    return article;
}


//
// スレの url を read.cgi型のurlに変換
//
// url がスレッドのURLで無い時はempty()が返る
// num_from と num_to が 0 で無い時はスレ番号を付ける
//
// (例) "http://hoge.machi.to/bbs/read.pl?BBS=board&KEY=12345",  num_from = 12, num_to = 15 のとき
//
// 戻り値 : "http://hoge.machi.to/bbs/read.pl?BBS=board&KEY=12345&START=12&END=15"
//
const std::string BoardMachi::url_readcgi( const std::string& url, int num_from, int num_to )
{
    if( empty() ) return std::string();

#ifdef _DEBUG
    std::cout << "BoardMachi::url_readcgi : " << url  << " from " << num_from << " to " << num_to << std::endl;
#endif

    ArticleBase* article = get_article_fromURL( url );
    if( !article ) return std::string();

    std::string readcgi = url_readcgibase() + article->get_key();

#ifdef _DEBUG
    std::cout << "BoardMachi::url_readcgi : to " << readcgi << std::endl;
#endif

    if( num_from > 0 || num_to > 0 ){

        std::ostringstream ss;
        ss << readcgi;
        if( num_from > 0 ) ss << "&START="<< num_from << "&END=";
        if( num_to > num_from ) ss << num_to;
        else ss << num_from;

        return ss.str();
    }

    return readcgi;
}


//
// pread.pl のURLのパスを返す
//
// (例) "/bbs/read.pl?BBS=board&KEY=" (最初に '/' がつく)
//
const std::string BoardMachi::url_datpath()
{
    if( empty() ) return std::string();

    return "/bbs/read.pl?BBS=" + get_id() + "&KEY=";
}

//
// まちBBSはdat型とreadcgi型のURLが同じ
//
const std::string BoardMachi::url_readcgipath()
{
    return url_datpath();
}



const std::string BoardMachi::create_newarticle_message( const std::string& subject,
                                                        const std::string& name, const std::string& mail, const std::string& msg )
{
    if( subject.empty() ) return std::string();
    if( msg.empty() ) return std::string();

    // まちではテスト出来ないので新スレを立てない
    return std::string();
}


//
// 新スレ作成時のbbscgi(write.cgi) のURL
//
const std::string BoardMachi::url_bbscgi_new()
{
    return std::string();
}


//
// 新スレ作成時のsubbbscgi のURL
//
const std::string BoardMachi::url_subbbscgi_new()
{
    return std::string();
}



//
// subject.txt から Aarticle のリストにアイテムを追加・更新
//
void BoardMachi::parse_subject( const char* str_subject_txt )
{
#ifdef _DEBUG
    std::cout << "BoardMachi::parse_subject\n";
#endif 
   
    const char* pos = str_subject_txt;
    char str_tmp[ 1024 ];

    ArticleBase* article_first = NULL;

    while( *pos != '\0' ){
        
        const char* str_id_dat;
        int lng_id_dat = 0;
        const char* str_subject;
        int lng_subject = 0;
        char str_num[ 16 ];

        // datのID取得
        // ".cgi" は除く
        str_id_dat = pos;
        while( *pos != '.' && *pos != '\0' && *pos != '\n' ) { ++pos; ++lng_id_dat; }
        
        // 壊れてる
        if( *pos == '\0' ) break;
        if( *pos == '\n' ) { ++pos; continue; }

        // subject取得
        pos += 5;  // ' ".cgi,"
        str_subject = pos;
        while( *pos != '\0' && *pos != '\n' ) ++pos;
        --pos;
        while( *pos != '(' && *pos != '\n' && pos != str_subject_txt ) --pos;
        
        // 壊れてる
        if( *pos == '\n' || pos == str_subject_txt ){
            MISC::ERRMSG( "subject.txt is broken" );
            break;
        }
        lng_subject = ( int )( pos - str_subject );
        
        // レス数取得
        ++pos;
        int i = 0;
        while( *pos != ')' && *pos != '\0' && *pos != '\n' && i < 16 ) str_num[ i++ ] = *( pos++ );

        // 壊れてる
        if( *pos == '\0' ) break;
        if( *pos == '\n' ) { ++pos; continue; }

        str_num[ i ] = '\0';
        ++pos;

        // id, subject, number 取得
        memcpy( str_tmp, str_id_dat, lng_id_dat );
        str_tmp[ lng_id_dat ] = '\0';
        std::string id = MISC::remove_space( str_tmp );

        memcpy( str_tmp, str_subject, lng_subject );
        str_tmp[ lng_subject ] = '\0';
        std::string subject = MISC::remove_space( str_tmp );
        subject = MISC::replace_str( subject, "&lt;", "<" );
        subject = MISC::replace_str( subject, "&gt;", ">" );
        
        int number = atol( str_num );

#ifdef _DEBUG
//        std::cout << id << " " << number << " " << subject << std::endl;
#endif

        // DBに登録されてるならarticle クラスの情報更新
        ArticleBase* article = get_article( id );

        // DBにないなら新規に article クラスを追加
        //
        // なおRoot::get_board()　経由で void BoardBase::read_info()及び
        // BoardBase::read_all_article_info() が既に呼ばれているので
        // DBに無いということはキャッシュにも無いということ。よって append_article()で  cached = false

        if( article->empty() ) article = append_article( id, false );

        // スレ情報更新
        if( article ){

            article->read_info();
            article->set_subject( subject );
            article->set_number( number );

            // boardビューに表示するリスト更新
            // Machiは最初と最後の行が同じになる仕様があるので最後の行を除く
            bool pushback = true;
            if( ! article_first ) article_first = article;
            else if( article == article_first ) pushback = false;

            if( pushback ){
                article->set_current( true );
                get_list_subject().push_back( article );
            }
        }
    }
}
