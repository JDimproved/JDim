// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "board2chcompati.h"
#include "article2chcompati.h"
#include "articlehash.h"
#include "settingloader.h"
#include "ruleloader.h"

#include "jdlib/jdregex.h"
#include "jdlib/misccharcode.h"
#include "jdlib/miscmsg.h"
#include "jdlib/miscutil.h"

#include "config/globalconf.h"

#include "httpcode.h"
#include "global.h"

#include <algorithm>
#include <cstring>
#include <iterator>
#include <sstream>

using namespace DBTREE;


#ifndef MAX
#define MAX( a, b ) ( a > b ? a : b )
#endif


#ifndef MIN
#define MIN( a, b ) ( a < b ? a : b )
#endif


Board2chCompati::Board2chCompati( const std::string& root, const std::string& path_board, const std::string& name,
                                  const std::string& basicauth )
    : BoardBase( root, path_board, name )
{
    set_path_dat( "/dat" );
    set_path_readcgi( "/test/read.cgi" );
    set_path_bbscgi( "/test/bbs.cgi" );
    set_path_subbbscgi( "/test/bbs.cgi" );
    set_subjecttxt( "subject.txt" );
    set_ext( ".dat" );
    set_id( path_board.substr( 1 ) ); // 先頭の '/' を除く
    set_encoding( Encoding::sjis );
    set_default_encoding( Encoding::sjis );

    BoardBase::set_basicauth( basicauth );
}


Board2chCompati::~Board2chCompati()
{
    if( m_settingloader ){
        m_settingloader->terminate_load();
    }
    if( m_ruleloader ){
        m_ruleloader->terminate_load();
    }
}



//
// キャッシュのファイル名が正しいか
//
bool Board2chCompati::is_valid( const std::string& filename ) const
{
    const std::string& ext = get_ext();
    if( filename.size() <= ext.size() ) return false;
    if( filename.compare( filename.size() - ext.size(), ext.size(), ext ) != 0 ) return false;

    return std::all_of( filename.cbegin(), std::prev( filename.cend(), ext.size() ),
                        []( char c ) { return '0' <= c && c <= '9'; } );
}


std::string Board2chCompati::analyze_keyword_impl( const std::string& html, bool full_parse )
{
    std::string keyword;

    // form要素から action属性(送信先URLのパス) を取得する
    const std::string path = MISC::parse_html_form_action( html );
    // action属性が見つかったら m_path_subbbscgi に設定する
    if( ! path.empty() ) {
#ifdef _DEBUG
        std::cout << "Board2chCompati::analyze_keyword_impl update subbbscgi path = " << path << std::endl;
#endif
        set_path_subbbscgi( path );
    }

    std::vector<MISC::FormDatum> data = MISC::parse_html_form_data( html );
    for( MISC::FormDatum& d : data ) {

        const std::string lowname = MISC::tolower_str( d.name );
        if( ! full_parse ) {
            // 除外する name の判定
            // 2ch の仕様が変わったら項目を追加すること
            if( lowname == "subject"
                || lowname == "from"
                || lowname == "mail"
                || lowname == "message"
                || lowname == "bbs"
                || lowname == "time"
                || lowname == "key"
                || lowname == "submit" ) continue;
        }
        if( lowname == "message" ) {
            // アンカー記号(>>)などがエスケープされる(&gt;&gt;)ため
            // 一度HTMLエスケープされた文字をデコードする
            d.value = MISC::html_unescape( d.value );
        }

        // キーワード取得
        if( ! keyword.empty() ) keyword.push_back( '&' );
        keyword.append( MISC::url_encode_plus( d.name, get_encoding() ) );
        keyword.push_back( '=' );
        keyword.append( MISC::url_encode_plus( d.value, get_encoding() ) );
    }
#ifdef _DEBUG
    std::cout << "Board2chCompati::analyze_keyword_impl form data = " << keyword << std::endl;
#endif

    return keyword;
}


// 書き込み時に必要なキーワード( hana=mogera や suka=pontan など )を
// 確認画面のhtmlから解析する
void Board2chCompati::analyze_keyword_for_write( const std::string& html )
{
#ifdef _DEBUG
    std::cout << "Board2chCompati::analyze_keyword_for_write\n";
    std::cout << html << std::endl << "--------------------\n";
#endif

    constexpr bool full_parse{ false };
    std::string keyword = analyze_keyword_impl( html, full_parse );
    set_keyword_for_write( keyword );
}


// スレ立て時に必要なキーワードをフロントページのhtmlから解析する
void Board2chCompati::analyze_keyword_for_newarticle( const std::string& html )
{
#ifdef _DEBUG
    std::cout << "Board2chCompati::analyze_keyword_for_newarticle\n";
    std::cout << html << std::endl << "--------------------\n";
#endif

    // XXX: スレ立てフォームはページの最後にあると決め打ちしている
    std::size_t i = html.rfind( "<form" );
    if( i == std::string::npos ) i = 0;

    constexpr bool full_parse{ false };
    std::string keyword = analyze_keyword_impl( html.substr( i ), full_parse );
    set_keyword_for_newarticle( keyword );
}


// 確認画面のHTMLから書き込み、スレ立て時に使うフォームデータを取得する
std::string Board2chCompati::parse_form_data( const std::string& html )
{
#ifdef _DEBUG
    std::cout << "Board2chCompati::parse_form_data\n";
    std::cout << html << std::endl << "--------------------\n";
#endif

    constexpr bool full_parse{ true };
    return analyze_keyword_impl( html, full_parse );
}


/** @brief 新スレ作成時の書き込みメッセージ作成
 *
 * @param[in] subject   スレタイトル
 * @param[in] name      名前、トリップ
 * @param[in] mail      メールアドレス、sage
 * @param[in] msg       書き込むメッセージ
 * @param[in] utf8_post trueならUTF-8のままURLエンコードする
 * @return URLエンコードしたフォームデータ (application/x-www-form-urlencoded)
 */
std::string Board2chCompati::create_newarticle_message( const std::string& subject, const std::string& name,
                                                        const std::string& mail, const std::string& msg,
                                                        const bool utf8_post )
{
    if( subject.empty() ) return std::string();
    if( msg.empty() ) return std::string();

    const Encoding enc{ utf8_post ? Encoding::utf8 : get_encoding() };

    std::stringstream ss_post;
    ss_post.clear();
    ss_post << "bbs="      << get_id()
            << "&subject=" << MISC::url_encode_plus( subject, enc )
            << "&time="    << get_time_modified()
            << "&submit="  << MISC::url_encode_plus( "新規スレッド作成", enc )
            << "&FROM="    << MISC::url_encode_plus( name, enc )
            << "&mail="    << MISC::url_encode_plus( mail, enc )
            << "&MESSAGE=" << MISC::url_encode_plus( msg, enc );

#ifdef _DEBUG
    std::cout << "Board2chCompati::create_newarticle_message " << ss_post.str() << std::endl;
#endif

    return ss_post.str();
}



//
// 新スレ作成時のbbscgi のURL
//
// (例) "http://www.hoge2ch.net/test/bbs.cgi"
//
//
std::string Board2chCompati::url_bbscgi_new() const
{
    std::string cgibase = url_bbscgibase();
    return cgibase.substr( 0, cgibase.length() -1 ); // 最後の '/' を除く
}


//
// 新スレ作成時のsubbbscgi のURL
//
// (例) "http://www.hoge2ch.net/test/subbbs.cgi"
//
std::string Board2chCompati::url_subbbscgi_new() const
{
    std::string cgibase = url_subbbscgibase();
    return cgibase.substr( 0, cgibase.length() -1 ); // 最後の '/' を除く
}



//
// 新しくArticleBaseクラスを追加してそのポインタを返す
//
// cached : HDD にキャッシュがあるならtrue
//
ArticleBase* Board2chCompati::append_article( const std::string& datbase, const std::string& id, const bool cached )
{
    if( empty() ) return get_article_null();

    ArticleBase* article = insert( std::make_unique<DBTREE::Article2chCompati>( datbase, id, cached, get_encoding() ) );

    if( ! article ) return get_article_null();
    return article;
}



//
// subject.txt から Aarticle のリストにアイテムを追加・更新
//
void Board2chCompati::parse_subject( const char* str_subject_txt )
{
#ifdef _DEBUG
    std::cout << "Board2chCompati::parse_subject\n";
#endif 

    const char* pos = str_subject_txt;

    while( *pos != '\0' ){
        
        const char* str_id_dat;
        int lng_id_dat = 0;
        const char* str_subject;
        int lng_subject = 0;

        while( *pos == ' ' ) ++pos;

        // datのID取得
        str_id_dat = pos;
        while( *pos != ' ' && *pos != '<' && *pos != '\0' && *pos != '\n' ) { ++pos; ++lng_id_dat; }
        
        // 壊れてる
        if( *pos == '\0' ){
            MISC::ERRMSG( "subject.txt is broken" );
            break;
        }
        if( *pos == '\n' ) { ++pos; continue; }

        while( *pos != '\0' && *pos != '<' ) ++pos;
        if( *pos != '\0' ) ++pos;
        if( *pos != '>' ){
            MISC::ERRMSG( "subject.txt is broken" );
            break;
        }

        // subject取得
        ++pos;
        str_subject = pos;
        while( *pos != '\0' && *pos != '\n' ) ++pos;
        --pos;
        while( *pos != '(' && *pos != '\n' && pos != str_subject ) --pos;
        
        // 壊れてる
        if( *pos == '\n' || pos == str_subject ){
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


void Board2chCompati::regist_article( const bool is_online )
{
    if( ! get_list_artinfo().size() ) return;

#ifdef _DEBUG
    std::cout << "Board2chCompati::regist_article size = " << get_list_artinfo().size() << std::endl;
#endif 

    const std::string datbase = url_datbase();

    for( const ARTICLE_INFO& artinfo : get_list_artinfo() ) {

        // DBに登録されてるならarticle クラスの情報更新
        ArticleBase* article = get_article( datbase, artinfo.id );

        // DBにないなら新規に article クラスをDBに登録
        //
        // なお BoardBase::receive_finish() のなかで append_all_article_in_cache() が既に呼び出されているため
        // DBに無いということはキャッシュに無いということ。よって append_article()の呼出に cached = false　を指定する
        if( article->empty() ) article = append_article( datbase, artinfo.id,
                                                         false // キャッシュ無し
            );

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


std::string Board2chCompati::localrule() const
{
    if( m_ruleloader ){
        if( m_ruleloader->is_loading() ) return "ロード中です";
        else if( m_ruleloader->get_code() == HTTP_OK
                || m_ruleloader->get_code() == HTTP_REDIRECT
                || m_ruleloader->get_code() == HTTP_MOVED_PERM
                || m_ruleloader->get_code() == HTTP_PERMANENT_REDIRECT ) {
            if( m_ruleloader->get_data().empty() ) return "ローカルルールはありません";
            else return m_ruleloader->get_data();
        }
        else return "ロードに失敗しました : " + m_ruleloader->get_str_code();
    }

    return BoardBase::localrule();
}


//
// SETTING.TXTのURL
//
// (例) "http://hoge.2ch.net/hogeboard/SETTING.TXT"
//
std::string Board2chCompati::url_settingtxt() const
{
    return url_boardbase() + DBTREE::kSettingTxt;
}


std::string Board2chCompati::settingtxt() const
{
    if( m_settingloader ){
        if( m_settingloader->is_loading() ) return "ロード中です";
        else if( m_settingloader->get_code() == HTTP_OK
                || m_settingloader->get_code() == HTTP_REDIRECT
                || m_settingloader->get_code() == HTTP_MOVED_PERM
                || m_settingloader->get_code() == HTTP_PERMANENT_REDIRECT ) {
            if( m_settingloader->get_data().empty() ) return "SETTING.TXTはありません";
            else return m_settingloader->get_data();
        }
        else return "ロードに失敗しました : " + m_settingloader->get_str_code();
    }

    return BoardBase::settingtxt();
}


std::string Board2chCompati::default_noname() const
{
    if( m_settingloader
        && m_settingloader->get_code() == HTTP_OK ) return m_settingloader->default_noname();

    return BoardBase::default_noname();
}


int Board2chCompati::line_number() const
{
    if( m_settingloader
        && m_settingloader->get_code() == HTTP_OK ) return m_settingloader->line_number();

    return BoardBase::line_number();
}


/**
 * @brief スレタイトルの最大バイト数
 */
int Board2chCompati::subject_count() const
{
    if( m_settingloader
        && m_settingloader->get_code() == HTTP_OK ) return m_settingloader->subject_count();

    return BoardBase::subject_count();
}


int Board2chCompati::message_count() const
{
    if( m_settingloader
        && m_settingloader->get_code() == HTTP_OK ) return m_settingloader->message_count();

    return BoardBase::message_count();
}    


std::string Board2chCompati::get_unicode() const
{
    if( m_settingloader
        && m_settingloader->get_code() == HTTP_OK ) return m_settingloader->get_unicode();

    return BoardBase::get_unicode();
}


//
// ローカルルールとSETTING.TXTをキャッシュから読み込む
//
// BoardBase::read_info()で呼び出す
//
void Board2chCompati::load_rule_setting()
{
#ifdef _DEBUG
    std::cout << "Board2chCompati::load_rule_setting\n";
#endif

    if( ! m_ruleloader ) m_ruleloader = std::make_unique<RuleLoader>( url_boardbase() );
    m_ruleloader->load_text( get_encoding() );

    if( ! m_settingloader ) m_settingloader = std::make_unique<SettingLoader>( url_boardbase() );
    m_settingloader->load_text( get_encoding() );
}


//
// ローカルルールとSETTING.TXTをサーバからダウンロード
//
// 読み込むタイミングはsubject.txtを読み終わった直後( BoardBase::receive_finish() )
//
void Board2chCompati::download_rule_setting()
{
#ifdef _DEBUG
    std::cout << "Board2chCompati::download_rule_setting\n";
#endif

    if( ! m_ruleloader ) m_ruleloader = std::make_unique<RuleLoader>( url_boardbase() );
    m_ruleloader->download_text( get_encoding() );

    if( ! m_settingloader ) m_settingloader = std::make_unique<SettingLoader>( url_boardbase() );
    m_settingloader->download_text( get_encoding() );
}


//
// レス数以下であぼーん(グローバル)
//
int Board2chCompati::get_abone_low_number_global() const
{
    return CONFIG::get_abone_low_number_thread();
}

//
// レス数以上であぼーん(グローバル)
//
int Board2chCompati::get_abone_high_number_global() const
{
    return CONFIG::get_abone_high_number_thread();
}
