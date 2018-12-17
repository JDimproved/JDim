// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "boardlocal.h"
#include "articlelocal.h"
#include "articlehash.h"

#include "jdlib/miscutil.h"
#include "jdlib/jdregex.h"

#include "cache.h"

using namespace DBTREE;


BoardLocal::BoardLocal( const std::string& root, const std::string& path_board, const std::string& name )
    : Board2chCompati( root, path_board, name, std::string() )
{
#ifdef _DEBUG
    std::cout << "BoardLocal::BoardLocal\n";
#endif
}


BoardLocal::~BoardLocal() noexcept
{}


//
// url がこの板のものかどうか
//
bool BoardLocal::equal( const std::string& url )
{
    if( url.find( "file://" ) == 0 ) return true;

    return false;
}


// そのまま出力
std::string BoardLocal::url_dat( const std::string& url, int& num_from, int& num_to, std::string& num_str )
{
    num_from = 0;
    num_to = 0;
    num_str = std::string();

    return url_readcgi( url, num_from, num_to );
}


// そのまま出力
std::string BoardLocal::url_readcgi( const std::string& url, int num_from, int num_to )
{
    JDLIB::Regex regex;
    const size_t offset = 0;
    const bool icase = false;
    const bool newline = true;
    const bool usemigemo = false;
    const bool wchar = false;

    const std::string query = "^ *file://.*/[1234567890]+" + get_ext() + " *$";
    if( ! regex.exec( query , url, offset, icase, newline, usemigemo, wchar ) ) return std::string();

#ifdef _DEBUG
    std::cout << "BoardLocal::url_readcgi : url = " << url << std::endl;
#endif

    return url;
}


void BoardLocal::download_subject( const std::string& url_update_view, const bool )
{
    // ダウンロードを実行しない
    get_url_update_views().push_back( url_update_view );
    send_update_board();
}


ArticleBase* BoardLocal::append_article( const std::string& datbase, const std::string& id, const bool cached )
{
#ifdef _DEBUG
    std::cout << "BoardLocal::append_article datbase = " << datbase
              << ", id = " << id << std::endl;
#endif

    ArticleBase* article = new DBTREE::ArticleLocal( datbase, id );
    if( article ){

        get_hash_article()->push( article );

        // subject にも追加する
        get_list_subject().push_back( article );
    }
    else return get_article_null();
    
    return article;
}


// datファイルのインポート
std::string BoardLocal::import_dat( const std::string& filename )
{
    if( empty() ) return FALSE;
    if( CACHE::file_exists( filename ) != CACHE::EXIST_FILE ) return std::string();

    int num_from, num_to;
    std::string num_str;
    const std::string urldat = url_dat( filename, num_from, num_to, num_str );
    if( urldat.empty() ) return std::string();

    const std::string datbase = MISC::get_dir( urldat );
    const std::string id = urldat.substr( datbase.length() );
    if( ! id.empty() ) return std::string();

    ArticleBase* art = get_article( datbase, id );

    // データベースに無いのでインポート
    if( art->empty() ){

#ifdef _DEBUG
        std::cout << "BoardLocal::import_dat file = " << filename << std::endl;
#endif

        art = append_article( datbase, id,
                              true // キャッシュあり
            );
        assert( art );

        art->read_info();

        return filename;
    }

    return std::string();
}
