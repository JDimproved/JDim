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

#define DEFALUT_CONFIG \
    "#\n" \
    "# UrlReplace設定ファイル (urlreplace.conf)\n" \
    "#\n" \
    "# 設定ファイルの書式:\n" \
    "#   正規表現<タブ>変換後URL<タブ>リファラURL<タブ>制御文字\n" \
    "#\n" \
    "# 設定例:\n" \
    "#   http://www\\.foobar\\.com/(view/|img\\.php\\?id=)([0-9]+)	http://www.foobar.com/view/$2	$0	$IMAGE\n" \
    "#\n" \
    "# 詳細な書式はマニュアルを参照してください。\n" \
    "# この機能を無効にする場合は、このファイルの内容を空にして保存してください。\n" \
    "#\n" \
    "http://www\\.youtube\\.com/watch\\?(|[^#]+&)v=([^&#/]+)	http://img.youtube.com/vi/$2/0.jpg\n" \
    "http://youtu\\.be/([^#&=/]+)	http://img.youtube.com/vi/$1/0.jpg\n" \
    "http://img\\.youtube\\.com/vi/[^/]+/0.jpg	$0		$THUMBNAIL\n" \
    "\n"


Urlreplace_Manager::Urlreplace_Manager()
{
    std::string conf;
    const std::string path = CACHE::path_urlreplace();
    if( CACHE::load_rawdata( path, conf ) ){
        conf2list( conf );
    } else {
        // 読み込みエラー、または空ファイル
        if( CACHE::file_exists( path ) == CACHE::EXIST_ERROR ){
            // ファイルが存在しないとき、デフォルト設定ファイルを作成する
            conf = DEFALUT_CONFIG;
            if( CACHE::save_rawdata( path, conf ) ){
                conf2list( conf );
            }
        }
    }
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
        if( (*it).length() <= 0 ) continue;
        if( (*it)[0] == '#' ) continue; // コメント行

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
        item.referer.clear();
         if( (++str) != line.end() ) item.referer = *str;
        // 4: コントロール
        item.imgctrl = IMGCTRL_NONE;
        item.match_break = false;
        if( (++str) != line.end() ){
            ctrl = *str;
            int imgctrl = IMGCTRL_INIT;

            // 拡張子がない場合でも画像として扱う
            if( ctrl.find( "$IMAGE" ) != std::string::npos ){
                imgctrl += IMGCTRL_FORCEIMAGE;
            }
            // 拡張子があっても画像として扱わない
            else if( ctrl.find( "$BROWSER" ) != std::string::npos ){
                imgctrl += IMGCTRL_FORCEBROWSER; // $IMAGEを優先
            }
            // 拡張子の偽装をチェックしない
            if( ctrl.find( "$GENUINE" ) != std::string::npos ){
                imgctrl += IMGCTRL_GENUINE;
            }
            // サムネイル画像
            if( ctrl.find( "$THUMBNAIL" ) != std::string::npos ){
                imgctrl += IMGCTRL_THUMBNAIL;
            }

            if( imgctrl != IMGCTRL_INIT ) item.imgctrl = imgctrl;

            // 正規表現に一致したら以降の判定を行わない
            item.match_break = ( ctrl.find( "$BREAK" ) != std::string::npos );
        }

        if( ! item.match.empty() && ! item.replace.empty() ) m_list_cmd.push_back( item );
    }
}



//
// URLを任意の正規表現で変換する
//
bool Urlreplace_Manager::exec( std::string &url )
{
    if( m_list_cmd.empty() ) return false;

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
        if( regex.exec( (*it).match, url,
                offset, icase, newline, usemigemo, wchar ) ){
            matched = true;

            // 置換URLの変換
            url = (*it).replace;
            replace( regex, url );

            // URLが空になったか、以降の判定を行わない
            if( url.empty() || (*it).match_break ) break;
        }
    }
    return matched;
}


//
// URLからリファラを求める
//
bool Urlreplace_Manager::referer( const std::string &url, std::string &referer )
{
    if( m_list_cmd.empty() ) return false;

    referer = url;

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

            // URLが空になったか、以降の判定を行わない
            if( referer.empty() || (*it).match_break ) break;
        }
    }
    return matched;
}


//
// URLの画像コントロールを取得する (URLキャッシュ)
//
int Urlreplace_Manager::get_imgctrl( const std::string &url )
{
    if( m_list_cmd.empty() ) return IMGCTRL_NONE;

    // 取得済みのURLか
    std::map< std::string, int >::iterator it = m_map_imgctrl.find( url );
    if( it != m_map_imgctrl.end() ) return ( *it ).second;

    // 新たに取得してキャッシュする
    int imgctrl = get_imgctrl_impl( url );
    m_map_imgctrl.insert( make_pair( url, imgctrl ) );

    return imgctrl;
}

//
// URLの画像コントロールを取得する
//
int Urlreplace_Manager::get_imgctrl_impl( const std::string &url )
{
    int imgctrl = IMGCTRL_NONE;

    JDLIB::Regex regex;
    const size_t offset = 0;
    const bool icase = false;
    const bool newline = true;
    const bool usemigemo = false;
    const bool wchar = false;

    // いずれかの正規表現に一致するか
    std::list< UrlreplaceItem >::iterator it = m_list_cmd.begin();
    for( ; it != m_list_cmd.end(); ++it ){
        if( regex.exec( (*it).match, url,
                offset, icase, newline, usemigemo, wchar ) ){

            // 画像コントロールを取得
            imgctrl = (*it).imgctrl;

            // 以降の判定を行わない
            if( (*it).match_break ) break;
        }
    }
    return imgctrl;
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

