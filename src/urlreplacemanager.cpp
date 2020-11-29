// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "urlreplacemanager.h"
#include "cache.h"
#include "type.h"

#include "jdlib/miscutil.h"
#include "jdlib/miscmsg.h"

#include <list>


CORE::Urlreplace_Manager* instance_urlreplace_manager = nullptr;

CORE::Urlreplace_Manager* CORE::get_urlreplace_manager()
{
    if( ! instance_urlreplace_manager ) instance_urlreplace_manager = new Urlreplace_Manager();
    assert( instance_urlreplace_manager );

    return instance_urlreplace_manager;
}


void CORE::delete_urlreplace_manager()
{
    if( instance_urlreplace_manager ) delete instance_urlreplace_manager;
    instance_urlreplace_manager = nullptr;
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
    "https?://www\\.youtube\\.com/watch\\?(|[^#]+&)v=([^&#/]+)	http://img.youtube.com/vi/$2/0.jpg\n" \
    "https?://youtu\\.be/([^#&=/]+)	http://img.youtube.com/vi/$1/0.jpg\n" \
    "https?://img\\.youtube\\.com/vi/[^/]+/0.jpg	$0		$THUMBNAIL\n" \
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

Urlreplace_Manager::~Urlreplace_Manager() noexcept = default;


//
// conf -> リスト
//
void Urlreplace_Manager::conf2list( const std::string& conf )
{
    m_list_cmd.clear();
    if( conf.empty() ) return;

    std::list< std::string > lines = MISC::get_lines( conf );
    if( lines.size() == 0 ) return;

    for( const std::string& conf_str : lines ) {
        if( conf_str.empty() ) continue;
        if( conf_str[0] == '#' ) continue; // コメント行

        std::list<std::string> tokens = MISC::StringTokenizer( conf_str, '\t' );
        if( tokens.size() < 2 ) continue;

        UrlreplaceItem item;
        std::list<std::string>::iterator str = tokens.begin();
        // 1: 検索URL
        constexpr bool icase = false;
        constexpr bool newline = true;
        constexpr bool migemo = false;
        constexpr bool wchar = false;
        if( (*str).empty() || ! item.creg.set( *str, icase, newline, migemo, wchar ) ) {
            continue;
        }
        // 2: 置換URL
        if( (*++str).empty() ) {
            continue;
        }
        item.replace = std::move( *str );
        std::string tgt_text = "$0";
        std::string rep_text = "\\0";
        for( int n = '0'; n <= '9'; ++n ) {
            tgt_text[1] = rep_text[1] = n;
            item.replace = MISC::replace_str( item.replace, tgt_text, rep_text );
        }
        // 3: リファラURL
        if( (++str) != tokens.end() ) item.referer = std::move( *str );
        // 4: コントロール
        item.imgctrl = IMGCTRL_NONE;
        item.match_break = false;
        if( (++str) != tokens.end() ) {
            const std::string& ctrl = *str;
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

        m_list_cmd.push_back( std::move( item ) );
    }
}



//
// URLを任意の正規表現で変換する
//
bool Urlreplace_Manager::exec( std::string &url )
{
    if( m_list_cmd.empty() ) return false;

    JDLIB::Regex regex;
    constexpr std::size_t offset = 0;

    // いずれかの正規表現に一致するか
    bool matched = false;
    for( const auto& cmd : m_list_cmd ) {
        if( regex.match( cmd.creg, url, offset ) ) {
            matched = true;

            // 置換URLの変換
            url = regex.replace( cmd.replace );

            // URLが空になったか、以降の判定を行わない
            if( url.empty() || cmd.match_break ) break;
        }
    }
    return matched;
}


//
// URLからリファラを求める
//
bool Urlreplace_Manager::referer( const std::string &url, std::string &refstr )
{
    if( m_list_cmd.empty() ) return false;

    refstr = url;

    JDLIB::Regex regex;
    constexpr std::size_t offset = 0;

    // いずれかの正規表現に一致するか
    bool matched = false;
    for( const auto& cmd : m_list_cmd ) {
        if( regex.match( cmd.creg, refstr, offset ) ) {
            matched = true;

            // リファラURLの変換
            refstr = regex.replace( cmd.referer );

            // URLが空になったか、以降の判定を行わない
            if( refstr.empty() || cmd.match_break ) break;
        }
    }
    return matched;
}


//
// URLの画像コントロールを取得する
//
int Urlreplace_Manager::get_imgctrl( const std::string &url )
{
    if( m_list_cmd.empty() ) return IMGCTRL_NONE;

    int imgctrl = IMGCTRL_NONE;

    JDLIB::Regex regex;
    constexpr std::size_t offset = 0;

    // いずれかの正規表現に一致するか
    for( const auto& cmd : m_list_cmd ) {
        if( regex.match( cmd.creg, url, offset ) ) {

            // 画像コントロールを取得
            imgctrl = cmd.imgctrl;

            // 以降の判定を行わない
            if( cmd.match_break ) break;
        }
    }
    return imgctrl;
}
