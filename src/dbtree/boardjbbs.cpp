// ライセンス: 最新のGPL

//#define _DEBUG
#include "jddebug.h"

#include "boardjbbs.h"
#include "articlejbbs.h"

#include "jdlib/miscutil.h"
#include "jdlib/miscmsg.h"
#include "jdlib/loaderdata.h"

#include <sstream>

using namespace DBTREE;


BoardJBBS::BoardJBBS( const std::string& root, const std::string& path_board, const std::string& name )
    : BoardBase( root, path_board, name )
{
    // dat のURLは特殊なので url_datpath()をオーバロードする
    set_path_dat( "" );

    set_path_readcgi( "/bbs/read.cgi" );
    set_path_bbscgi( "/bbs/write.cgi" );
    set_path_subbbscgi( "/bbs/write.cgi" );
    set_subjecttxt( "subject.txt" );
    set_ext( "" );
    set_id( path_board.substr( 1 ) ); // 先頭の '/' を除く  
    set_charset( "EUC-JP" );
}


//
// キャッシュのファイル名が正しいか
//
bool BoardJBBS::is_valid( const std::string& filename )
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
ArticleBase* BoardJBBS::append_article( const std::string& id, bool cached )
{
    if( empty() ) return get_article_null();

    ArticleBase* article = new DBTREE::ArticleJBBS( url_datbase(), id, cached );
    if( article ) get_list_article().push_back( article );
    else return get_article_null();

    return article;
}



//
// rawmode のURLのパスを返す
//
// (例) "/bbs/rawmode.cgi/board/"  (最初と最後に '/' がつく)
//
const std::string BoardJBBS::url_datpath()
{
    if( empty() ) return std::string();

    return "/bbs/rawmode.cgi" + get_path_board() + "/";
}



const std::string BoardJBBS::create_newarticle_message( const std::string& subject,
                                                        const std::string& name, const std::string& mail, const std::string& msg )
{
    if( subject.empty() ) return std::string();
    if( msg.empty() ) return std::string();

    // DIR と BBS を分離する( ID = DIR/BBS )
    std::string boardid = get_id();
    int i = boardid.find( "/" );
    std::string dir = boardid.substr( 0, i );
    std::string bbs = boardid.substr( i + 1 );

    std::stringstream ss_post;
    ss_post.clear();
    ss_post << "SUBJECT="  << MISC::charset_url_encode( subject, get_charset() )
            << "&submit="  << MISC::charset_url_encode( "新規書き込み", get_charset() )
            << "&NAME="    << MISC::charset_url_encode( name, get_charset() )
            << "&MAIL="    << MISC::charset_url_encode( mail, get_charset() )
            << "&MESSAGE=" << MISC::charset_url_encode( msg, get_charset() )
            << "&DIR="     << dir
            << "&BBS="     << bbs
            << "&TIME="    << time_modified();

#ifdef _DEBUG
    std::cout << "BoardJBBS::create_newarticle_message " << ss_post.str() << std::endl;
#endif

    return ss_post.str();
}


//
// 新スレ作成時のbbscgi(write.cgi) のURL
//
// (例) "http://jbbs.livedoor.jp/bbs/write.cgi/computer/123/new/"
//
//
const std::string BoardJBBS::url_bbscgi_new()
{
    return url_bbscgibase() + get_id() + "/new/";
}


//
// 新スレ作成時のsubbbscgi のURL
//
// (例) "http://jbbs.livedoor.jp/bbs/write.cgi/computer/123/new/"
//
const std::string BoardJBBS::url_subbbscgi_new()
{
    return url_subbbscgibase() + get_id() + "/new/";
}



//
// subject.txt から Aarticle のリストにアイテムを追加・更新
//
void BoardJBBS::parse_subject( const char* str_subject_txt )
{
#ifdef _DEBUG
    std::cout << "BoardJBBS::parse_subject\n";
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
        std::cout << id << " " << number << " " << subject << std::endl;
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
            // JBBSは最初と最後の行が同じになる仕様があるので最後の行を除く
            bool pushback = true;
            if( ! article_first ) article_first = article;
            else if( article == article_first ) pushback = false;

            if( pushback ){
                article->set_current( true );
                if( ! BoardBase::get_abone_thread( article ) ) get_list_subject().push_back( article );
            }
        }
    }
}
