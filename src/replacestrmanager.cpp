// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "replacestrmanager.h"
#include "cache.h"
#include "type.h"

#include "jdlib/miscmsg.h"
#include "jdlib/miscutil.h"

#include "xml/document.h"
#include "xml/tools.h"

#include <algorithm> // std::any_of(), std::find_if()
#include <bitset>
#include <cstdlib>
#include <iterator> // std::distance()


constexpr const char* kRootNodeNameReplaceStr = "replacestrlist";


static CORE::ReplaceStr_Manager* instance_replacestr_manager = nullptr;

CORE::ReplaceStr_Manager* CORE::get_replacestr_manager()
{
    if( ! instance_replacestr_manager ) instance_replacestr_manager = new ReplaceStr_Manager();
    assert( instance_replacestr_manager );

    return instance_replacestr_manager;
}


void CORE::delete_replacestr_manager()
{
    if( instance_replacestr_manager ) delete instance_replacestr_manager;
    instance_replacestr_manager = nullptr;
}

///////////////////////////////////////////////

using namespace CORE;


unsigned long ReplaceStrCondition::to_ulong() const
{
    // エンディアンの問題を回避するため std::bitset を使って変換する
    // データ互換性のため削除したフラグのビット位置を再利用しないこと
    std::bitset<31> out;
    out[0] = active;
    out[1] = icase;
    out[2] = regex;
    out[3] = wchar;
    out[4] = norm;
    return out.to_ulong();
}


ReplaceStrCondition ReplaceStrCondition::from_ulong( unsigned long condition ) noexcept
{
    const std::bitset<31> input( condition );
    ReplaceStrCondition out{};
    out.active = input[0];
    out.icase = input[1];
    out.regex = input[2];
    out.wchar = input[3];
    out.norm = input[4];
    return out;
}


ReplaceStr_Manager::ReplaceStr_Manager()
{
    std::string xml;
    if( CACHE::load_rawdata( CACHE::path_replacestr(), xml ) ) xml2list( xml );
    else {
        // デフォルトの設定を行う
        ReplaceStrCondition condition{};
        list_append( REPLACETARGET_MESSAGE, condition, "ダブルクリックすると編集出来ます",
                     "(この項目は削除して構いません)" );
        condition.regex = true;
        list_append( REPLACETARGET_MESSAGE, condition, "(?<!t)ps://([[:alnum:]]+)", "https://\\1" );
        list_append( REPLACETARGET_MESSAGE, condition,
                     "(?<!ps://)i\\.imgur\\.com/([[:alnum:]]{7})\\.(jpe?g|gif|png)",
                     "https://i.imgur.com/\\1.\\2" );
    }
}


//
// 置換処理を行うか状態を返す
// 文字参照のデコードを行う、または有効になってる置換条件があるときはtrueを返す
//
bool ReplaceStr_Manager::list_get_active( const int id ) const
{
    return m_chref[id] || std::any_of( m_list[id].begin(), m_list[id].end(),
                                       []( const ReplaceStrItem& item ) { return item.condition.active; } );
}


//
// リストをクリア
//
void ReplaceStr_Manager::list_clear( const int id )
{
    m_list[id].clear();
}


//
// アイテムをリストに追加
//
void ReplaceStr_Manager::list_append( const int id, ReplaceStrCondition condition,
                                      const std::string& pattern, const std::string& replace )
{
    if( id >= REPLACETARGET_MAX ) return;

    JDLIB::RegexPattern creg;
    if( condition.regex ) {
        constexpr bool newline = true;
        constexpr bool migemo = false;

        if( ! creg.set( pattern, condition.icase, newline, migemo,
                        condition.wchar ) ) {
            std::string msg = "invlid replacestr pattern: ";
            msg.append( creg.errstr() + ": " + pattern );
            MISC::ERRMSG( msg );
        }
    }

#ifdef _DEBUG
    std::cout << "ReplaceStr_Manager::list_append id=" << id << " conditnon=" << condition.to_ulong()
              << " pattern=" << pattern << " replace=" << replace << std::endl;
#endif

    m_list[id].push_back( ReplaceStrItem{ condition, pattern, replace, std::move( creg ) } );
}


//
// xml -> リスト
//
void ReplaceStr_Manager::xml2list( const std::string& xml )
{
    for( auto& item : m_list ) {
        item.clear();
    }
    if( xml.empty() ) return;

    const XML::Document document( xml );

    const XML::Dom* const root = document.get_root_element( kRootNodeNameReplaceStr );
    if( ! root ) return;

    for( const XML::Dom* dom : *root ) {

        if( dom->nodeType() != XML::NODE_TYPE_ELEMENT ) continue;

        const int type = XML::get_type( dom->nodeName() );
        if( type != TYPE_DIR ) continue;

        const std::size_t id = target_id( dom->getAttribute( "name" ) );
        if( id >= REPLACETARGET_MAX ) continue;

        m_chref[id] = dom->getAttribute( "chref" ) == "true";

        for( const XML::Dom* query : *dom ) {

            if( query->nodeType() != XML::NODE_TYPE_ELEMENT ) continue;

            const int element_type = XML::get_type( query->nodeName() );
            if( element_type != TYPE_REPLACESTR ) continue;

            const std::string cond_str = query->getAttribute( "condition" );
            // 解析に失敗した時は初期値(0)にする
            const unsigned long cond_num = std::strtoul( cond_str.c_str(), nullptr, 10 );
            list_append( id, ReplaceStrCondition::from_ulong( cond_num ),
                         query->getAttribute( "pattern" ),
                         query->getAttribute( "replace" ) );
        }
    }
}


//
// XML 保存
//
void ReplaceStr_Manager::save_xml()
{
    XML::Document document;
    XML::Dom* root = document.appendChild( XML::NODE_TYPE_ELEMENT, kRootNodeNameReplaceStr );
    if( ! root ) return;

    for( int i = 0; i < REPLACETARGET_MAX; ++i ) {

        XML::Dom* node = dom_append( root, i, m_chref[i] );

        for( const auto& item : m_list[i] ) {
            dom_append( node, item.condition, item.pattern, item.replace );
        }
    }

#ifdef _DEBUG
    std::cout << "ReplaceStr_Manager::save_xml" << std::endl;
    std::cout << document.get_xml() << std::endl;
#endif

    CACHE::save_rawdata( CACHE::path_replacestr(), document.get_xml() );
}


//
// 置換対象の要素名からid取得
//
int ReplaceStr_Manager::target_id( const std::string& name )
{
    auto it = std::find_if( kReplStrTargetNames.begin(), kReplStrTargetNames.end(),
                            [&name] ( const char* id ) { return name == id; } );
    return std::distance( kReplStrTargetNames.begin(), it );
}


//
// 置換対象idから要素名取得
//
std::string ReplaceStr_Manager::target_name( const int id )
{
    if( id >= REPLACETARGET_MAX ) return {};

    return kReplStrTargetNames[id];
}


//
// 実行
//
std::string ReplaceStr_Manager::replace( std::string_view str, const int id ) const
{
    if( id >= REPLACETARGET_MAX || ( m_list[id].empty() && ! m_chref[id] ) ) return std::string{ str };

    std::string buffer;

    if( m_chref[id] ) buffer = MISC::chref_decode( str, false );
    else buffer.assign( str );

#ifdef _DEBUG
    std::cout << "ReplaceStr_Manager::replace str=" << buffer << std::endl;
#endif

    // bufferに対してリストに登録された変換処理を実行していく
    for( const auto& item : m_list[id] ) {

        const ReplaceStrCondition condition = item.condition;

        if( ! condition.active || item.pattern.empty() ) continue;

        if( condition.regex ) {
            const int lng_buf = buffer.size();
            // 文字列が空の時は"^$"などにマッチするように改行を追加
            if( lng_buf == 0 ) buffer.push_back( '\n' );

            JDLIB::Regex regex;
            bool match = false;
            int offset = 0;
            std::string replaced;

            while( regex.match( item.creg, buffer, offset, match ) ) {
                match = true;

                const int p0 = regex.pos( 0 );
                if( p0 != offset ) replaced.append( buffer.substr( offset, p0 - offset ) );

                // \0 ... \9 の文字列を置換
                replaced.append( regex.replace( item.replace ) );

                offset = p0 + regex.length( 0 );
                if( offset >= lng_buf ) break;

                // 0文字にマッチするパターンは繰り返さない
                if( regex.length( 0 ) == 0 ) break;
            }

            if( match ) {
                if( lng_buf > offset ) {
                    replaced.append( buffer.substr( offset, lng_buf - offset ) );
                }
                buffer = std::move( replaced );
            }
        }

        // 正規表現を使わない置換処理
        else if( condition.icase ) buffer = MISC::replace_casestr( buffer, item.pattern, item.replace );
        else buffer = MISC::replace_str( buffer, item.pattern, item.replace );
    }

#ifdef _DEBUG
    std::cout << "ReplaceStr_Manager::replace replaced str=" << buffer << std::endl;
#endif

    return buffer;
}


//
// 置換対象のXMLノード作成
//
XML::Dom* ReplaceStr_Manager::dom_append( XML::Dom* node, const int id, const bool chref )
{
    XML::Dom* const dir = node->appendChild( XML::NODE_TYPE_ELEMENT, XML::get_name( TYPE_DIR ) );

    dir->setAttribute( "name", target_name( id ) );
    dir->setAttribute( "chref", chref ? "true" : "false" );

    return dir;
}


//
// 置換条件のXMLノード作成
//
XML::Dom* ReplaceStr_Manager::dom_append( XML::Dom* node, ReplaceStrCondition condition,
                                          const std::string& pattern, const std::string& replace )
{
    XML::Dom* const query = node->appendChild( XML::NODE_TYPE_ELEMENT, XML::get_name( TYPE_REPLACESTR ) );

    query->setAttribute( "condition", std::to_string( condition.to_ulong() ) );
    query->setAttribute( "pattern", pattern );
    query->setAttribute( "replace", replace );

    return query;
}
