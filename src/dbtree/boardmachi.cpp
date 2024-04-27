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

#include <algorithm>
#include <cstring>
#include <sstream>

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
    set_encoding( Encoding::sjis );
    set_default_encoding( Encoding::sjis );
}



//
// url がこの板のものかどうか
//
bool BoardMachi::equal( const std::string& url ) const
{
    if( url.rfind( get_root(), 0 ) == 0 ){

        if( url.find( get_path_board() + "/" ) != std::string::npos ) return true;
        if( url.find( "BBS=" + get_id() ) != std::string::npos ) return true;
    }

    return false;
}



//
// キャッシュのファイル名が正しいか
//
bool BoardMachi::is_valid( const std::string& filename ) const
{
    if( filename.length() != 10 ) return false;

    return std::all_of( filename.cbegin(), filename.cend(),
                        []( char c ) { return '0' <= c && c <= '9'; } );
}




//
// 新しくArticleBaseクラスを追加してそのポインタを返す
//
// cached : HDD にキャッシュがあるならtrue
//
ArticleBase* BoardMachi::append_article( const std::string& datbase, const std::string& id, const bool cached )
{
    if( empty() ) return get_article_null();

    ArticleBase* article = insert( std::make_unique<DBTREE::ArticleMachi>( datbase, id, cached, get_encoding() ) );

    if( ! article ) return get_article_null();
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
    else {
        // offlaw.cgi v2 のdat URLを解析する
        // NOTE: read.cgi のdat URLはお気に入りやスレ履歴の保存に使われているため、
        // url_dat() の戻り値を offlaw.cgi v2 の形式に変更すると互換性が壊れる。
        // 1. offlaw.cgi v2 のURL解析を実装しておき互換性の問題が小さくなる将来まで待つ。 <- いまここ
        // 2. url_dat() の戻り値を offlaw.cgi v2 の形式に変更する。
        // 3. read.cgi のdat URLを解析する処理は後方互換性のため残す。
        const std::string datpath2 = MISC::replace_str( "/bbs/offlaw.cgi/2/" + get_id() + "/", "?", "\\?" );
        const std::string query_dat2 = "^ *https?://.+" + datpath2 + "([0-9]+)(?:/|/l([0-9]+)|/([0-9]+)(-)?([0-9]+)?)?";
        if( regex.exec( query_dat2, url, offset, icase, newline, usemigemo, wchar ) ) {

            id = regex.str( 1 );

            if( regex.length( 2 ) > 0 ) { // l50 など
                num_from = 1;
                num_to = std::atoi( regex.str( 2 ).c_str() );
            }
            else {
                num_from = std::atoi( regex.str( 3 ).c_str() );
                num_to = std::atoi( regex.str( 5 ).c_str() );
            }

            if( num_from != 0 ) {
                num_from = (std::max)( 1, num_from );

                // 12- みたいな場合はとりあえず大きい数字を入れとく
                if( regex.length( 4 ) > 0 && num_to == 0 ) num_to = CONFIG::get_max_resnumber() + 1;
            }

            // -15 みたいな場合
            else if( num_to != 0 ) {
                num_from = 1;
            }

            num_to = (std::max)( num_from, num_to );
            num_str = MISC::get_filename( url );
        }
        else return std::string();
    }

    return url_datbase() + id;
}


//
// read.cgi のURLのパスを返す
//
// (例) "/bbs/read.cgi?BBS=board&KEY=" (最初に '/' がつく)
//
std::string BoardMachi::url_datpath() const
{
    if( empty() ) return std::string();

    return "/bbs/read.cgi?BBS=" + get_id() + "&KEY=";
}


std::string BoardMachi::create_newarticle_message( const std::string& subject, const std::string& name,
                                                   const std::string& mail, const std::string& msg, const bool )
{
    if( subject.empty() ) return std::string();
    if( msg.empty() ) return std::string();

    // まちではテスト出来ないので新スレを立てない
    return std::string();
}


//
// 新スレ作成時のbbscgi(write.cgi) のURL
//
std::string BoardMachi::url_bbscgi_new() const
{
    return std::string();
}


//
// 新スレ作成時のsubbbscgi のURL
//
std::string BoardMachi::url_subbbscgi_new() const
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
        lng_subject = static_cast<int>( pos - str_subject );

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
        artinfo.id = MISC::utf8_trim( artinfo.id );

        artinfo.subject.assign( str_subject, lng_subject );

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

    for( const ARTICLE_INFO& artinfo : get_list_artinfo() ) {

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
