// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "boardmachi.h"
#include "articlemachi.h"
#include "articlehash.h"

#include "jdlib/miscutil.h"
#include "jdlib/miscmsg.h"
#include "jdlib/loaderdata.h"
#include "jdlib/jdregex.h"

#include "config/globalconf.h"

#include "global.h"

#include <sstream>
#include <cstring>

using namespace DBTREE;


BoardMachi::BoardMachi( const std::string& root, const std::string& path_board, const std::string& name )
    : BoardBase( root, path_board, name )
{
    // dat のURLは特殊なので url_datpath()をオーバライドする
    set_path_dat( "" );
    set_path_readcgi( "/bbs/read.cgi" );
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

    size_t dig, n;
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
ArticleBase* BoardMachi::append_article( const std::string& datbase, const std::string& id, const bool cached )
{
    if( empty() ) return get_article_null();

    ArticleBase* article = new DBTREE::ArticleMachi( datbase, id, cached );
    if( article ){
        get_hash_article()->push( article );

        // 最大レス数セット
        article->set_number_max( get_number_max_res() );
    }
    else return get_article_null();

    return article;
}


//
// スレの url を dat型のurlに変換して出力
//
// (例) "http://hoge.machi.to/bbs/read.cgi?BBS=board&KEY=12345&START=12&END=15"" のとき
// 戻り値 : "http://hoge.machi.to/bbs/read.cgi?BBS=board&KEY=12345", num_from = 12, num_to = 15, num_str = 12-15
//
std::string BoardMachi::url_dat( const std::string& url, int& num_from, int& num_to, std::string& num_str )
{
    if( empty() ) return std::string();

    // read.cgi 型の判定
    std::string urldat = BoardBase::url_dat( url, num_from, num_to, num_str );
    if( ! urldat.empty() ) return urldat;

    // 旧形式(read.pl)型の場合
    if( url.find( "read.pl" ) != std::string::npos ){

        urldat = BoardBase::url_dat( MISC::replace_str( url, "read.pl", "read.cgi" ), num_from, num_to, num_str );
        if( ! urldat.empty() ) return urldat;
    }

#ifdef _DEBUG
    std::cout << "BoardMachi::url_dat : url = " << url << std::endl;
#endif
    
    JDLIB::Regex regex;
    const size_t offset = 0;
    const bool icase = false;
    const bool newline = true;
    const bool usemigemo = false;
    const bool wchar = false;

    std::string id; // スレッドのID

    num_from = num_to = 0;

    // dat 型
    // LAST, START, END を考慮する
    const std::string datpath = MISC::replace_str( url_datpath(), "?", "\\?" );
    const std::string query_dat = "^ *(https?://.+" + datpath  + ")([1234567890]+" + get_ext() + ")(&LAST=[1234567890]+)?(&START=([1234567890])+)?(&END=([1234567890])+)? *$";

#ifdef _DEBUG
    std::cout << "query_dat = " << query_dat << std::endl;
#endif

    if( regex.exec( query_dat , url, offset, icase, newline, usemigemo, wchar ) ){
        id = regex.str( 2 );

        if( regex.length( 5 ) ){
            num_from = atoi( regex.str( 5 ).c_str() );
            num_str = std::to_string( num_from );
        }
        if( regex.length( 7 ) ){
            num_to = atoi( regex.str( 7 ).c_str() );
            num_str += "-" + std::to_string( num_to );
        }

#ifdef _DEBUG
        std::cout << "id = " << id << std::endl
                  << "start = " << num_from << std::endl
                  << "end = " << num_to << std::endl
                  << "num = " << num_str << std::endl;
#endif
    }
    else return std::string();

    return url_datbase() + id;
}


//
// read.cgi のURLのパスを返す
//
// (例) "/bbs/read.cgi?BBS=board&KEY=" (最初に '/' がつく)
//
std::string BoardMachi::url_datpath()
{
    if( empty() ) return std::string();

    return "/bbs/read.cgi?BBS=" + get_id() + "&KEY=";
}


std::string BoardMachi::create_newarticle_message( const std::string& subject, const std::string& name,
                                                   const std::string& mail, const std::string& msg )
{
    if( subject.empty() ) return std::string();
    if( msg.empty() ) return std::string();

    // まちではテスト出来ないので新スレを立てない
    return std::string();
}


//
// 新スレ作成時のbbscgi(write.cgi) のURL
//
std::string BoardMachi::url_bbscgi_new()
{
    return std::string();
}


//
// 新スレ作成時のsubbbscgi のURL
//
std::string BoardMachi::url_subbbscgi_new()
{
    return std::string();
}



//
// subject.txt から Aarticle のリストにアイテムを追加・更新
//
void BoardMachi::parse_subject( const char* str_subject_txt )
{
#ifdef _DEBUG
    std::cout << "BoardMachi::parse_subject" << std::endl;
#endif 
   
    const char* pos = str_subject_txt;

    while( *pos != '\0' ){
        
        const char* str_id_dat;
        int lng_id_dat = 0;
        const char* str_subject;
        int lng_subject = 0;

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

        // レス数取得 (符号付き32bit整数より大きいと未定義)
        ++pos;
        std::string str_num;
        while( '0' <= *pos && *pos <= '9' ) str_num.push_back( *( pos++ ) );

        // 壊れてる
        if( str_num.empty() ){
            MISC::ERRMSG( "subject.txt is broken (res)" );
            break;
        }
        if( *pos == '\0' ) break;
        if( *pos == '\n' ) { ++pos; continue; }

        ++pos;

        // id, subject, number 取得
        ARTICLE_INFO artinfo;

        artinfo.id.assign( str_id_dat, lng_id_dat );
        artinfo.id = MISC::remove_space( artinfo.id );

        artinfo.subject.assign( str_subject, lng_subject );
        artinfo.subject = MISC::remove_space( artinfo.subject );
        artinfo.subject = MISC::replace_str( artinfo.subject, "&lt;", "<" );
        artinfo.subject = MISC::replace_str( artinfo.subject, "&gt;", ">" );

        const auto num = std::atoi( str_num.c_str() );
        artinfo.number = ( num < CONFIG::get_max_resnumber() ) ? num : CONFIG::get_max_resnumber();

        get_list_artinfo().push_back( artinfo );        

#ifdef _DEBUG
        std::cout << "pos = " << ( pos - str_subject_txt ) << " lng = " << lng_subject << " id = " << artinfo.id << " num = " << artinfo.number;
        std::cout << " : " << artinfo.subject << std::endl;
#endif
    }
}


void BoardMachi::regist_article( const bool is_online )
{
    if( ! get_list_artinfo().size() ) return;

#ifdef _DEBUG
    std::cout << "BoardMachii::regist_article size = " << get_list_artinfo().size() << std::endl;
#endif 

    ArticleBase* article_first = nullptr;

    const std::string datbase = url_datbase();

    for( unsigned int i = 0; i < get_list_artinfo().size(); ++i ){

        const ARTICLE_INFO& artinfo = get_list_artinfo()[ i ];

        // DBに登録されてるならarticle クラスの情報更新
        ArticleBase* article = get_article( datbase, artinfo.id );

        // DBにないなら新規に article クラスを追加
        //
        // なお BoardBase::receive_finish() のなかで append_all_article_in_cache() が既に呼び出されているため
        // DBに無いということはキャッシュにも無いということ。よって append_article()で  cached = false

        if( article->empty() ) article = append_article( datbase, artinfo.id, false );

        // スレ情報更新
        if( article ){

            // ステータスをDAT落ち状態から通常状態に変更
            int status = article->get_status();
            status |= STATUS_NORMAL;
            status &= ~STATUS_OLD;
            article->set_status( status );

            // 情報ファイル読み込み
            article->read_info();

            // 情報ファイルが無い場合もあるのでsubject.txtから取得したサブジェクト、レス数を指定しておく
            article->set_subject( artinfo.subject );
            article->set_number( artinfo.number, is_online );

            // boardビューに表示するリスト更新
            // Machiは最初と最後の行が同じになる仕様があるので最後の行を除く
            bool pushback = true;
            if( ! article_first ) article_first = article;
            else if( article == article_first ) pushback = false;

            if( pushback ){

                // 情報ファイル読み込み後にステータスが変わることがあるので、もう一度
                // ステータスをDAT落ち状態から通常状態に変更
                status = article->get_status();
                status |= STATUS_NORMAL;
                status &= ~STATUS_OLD;
                article->set_status( status );

                // boardビューに表示するリスト更新
                if( ! BoardBase::is_abone_thread( article ) ) get_list_subject().push_back( article );
            }
        }
    }
}
