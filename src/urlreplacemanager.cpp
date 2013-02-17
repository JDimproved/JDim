// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "urlreplacemanager.h"
#include "cache.h"
#include "type.h"

#include "jdlib/miscutil.h"
#include "jdlib/miscmsg.h"

CORE::Urlreplace_Manager* instance_urlreplace_manager = NULL;

CORE::Urlreplace_Manager* CORE::get_urlreplace_manager()
{
    if( ! instance_urlreplace_manager ) instance_urlreplace_manager = new Urlreplace_Manager();
    assert( instance_urlreplace_manager );

    return instance_urlreplace_manager;
}


void CORE::delete_urlreplace_manager()
{
    if( instance_urlreplace_manager ) delete instance_urlreplace_manager;
    instance_urlreplace_manager = NULL;
}

///////////////////////////////////////////////

using namespace CORE;

Urlreplace_Manager::Urlreplace_Manager()
{
    std::string conf;
    if( CACHE::load_rawdata( CACHE::path_urlreplace(), conf ) ) conf2list( conf );
}


//
// conf -> リスト
//
void Urlreplace_Manager::conf2list( const std::string& conf )
{
    m_list_cmd.clear();
    if( conf.empty() ) return;

    std::list< std::string > lines = MISC::get_lines( conf );
    if( lines.size() == 0 ) return;

    std::list < std::string >::iterator it = lines.begin();
    for( ; it != lines.end(); ++it ){
        std::list< std::string > line = MISC::StringTokenizer( *it, '\t' );
        if( line.size() < 2 ) continue;

        UrlreplaceItem item;
        std::string ctrl;
        std::list < std::string >::iterator str = line.begin();
        // 1: 検索URL
        item.match = *str;
        // 2: 置換URL
        item.replace = *(++str);
        // 3: リファラURL
        if( (++str) != line.end() ) item.referer = *str;
        // 4: コントロール
        if( (++str) != line.end() ) ctrl = *str;

        // 拡張子がない場合でも画像として扱う
        item.force_image = ( ctrl.find( "$IMAGE" ) != std::string::npos );

        if( ! item.match.empty() && ! item.replace.empty() ) m_list_cmd.push_back( item );
    }
}



//
// URLを任意の正規表現で変換する
//
const std::string Urlreplace_Manager::exec( const std::string &url, bool &force_image )
{
    force_image = false;
    if( m_list_cmd.empty() ) return std::string();

    std::string link = url;

    JDLIB::Regex regex;
    const size_t offset = 0;
    const bool icase = false;
    const bool newline = true;
    const bool usemigemo = false;
    const bool wchar = false;

    // いずれかの正規表現に一致するか
    bool matched = false;
    std::list< UrlreplaceItem >::iterator it = m_list_cmd.begin();
    for( ; it != m_list_cmd.end(); ++it ){
        if( regex.exec( (*it).match, link,
                offset, icase, newline, usemigemo, wchar ) ){
            matched = true;

            // 置換URLの変換
            link = (*it).replace;
            replace( regex, link );

            // 拡張子がない場合でも画像として扱う
            force_image = (*it).force_image;
        }
    }
    if( ! matched ){
        return std::string(); // どの正規表現にも一致しなかった
    }

    return link;
}


//
// URLからリファラを求める
//
const std::string Urlreplace_Manager::referer( const std::string &url )
{
    if( m_list_cmd.empty() ) return std::string();

    std::string referer = url;

    JDLIB::Regex regex;
    const size_t offset = 0;
    const bool icase = false;
    const bool newline = true;
    const bool usemigemo = false;
    const bool wchar = false;

    // いずれかの正規表現に一致するか
    bool matched = false;
    std::list< UrlreplaceItem >::iterator it = m_list_cmd.begin();
    for( ; it != m_list_cmd.end(); ++it ){
        if( regex.exec( (*it).match, referer,
                offset, icase, newline, usemigemo, wchar ) ){
            matched = true;

            // リファラURLの変換
            referer = (*it).referer;
            replace( regex, referer );
        }
    }
    if( ! matched ){
        return std::string(); // どの正規表現にも一致しなかった
    }

    return referer;
}


//
// 置換文字列を変換
//   \0 ... \9 ( $0 ... $9 ) : 正規表現の部分一致
//
void Urlreplace_Manager::replace( JDLIB::Regex &regex, std::string &str )
{
    if( str.empty() ) return;
    
    char rep1[] = "\\0";
    char rep2[] = "$0";
    for( int i = 0; i < 9; i++ ){
        if( regex.pos( i ) == -1 ){
            break;
        }
        rep1[ 1 ] = '0' + i;
        if( str.find( rep1 ) != std::string::npos ){
            str = MISC::replace_str( str, rep1, regex.str( i ) );
        }
        rep2[ 1 ] = '0' + i;
        if( str.find( rep2 ) != std::string::npos ){
            str = MISC::replace_str( str, rep2, regex.str( i ) );
        }
    }
}

