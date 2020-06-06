// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "board2chcompati.h"
#include "article2chcompati.h"
#include "articlehash.h"
#include "settingloader.h"
#include "ruleloader.h"

#include "jdlib/miscutil.h"
#include "jdlib/miscmsg.h"
#include "jdlib/jdregex.h"

#include "config/globalconf.h"

#include "httpcode.h"
#include "global.h"

#include <sstream>
#include <cstring>

using namespace DBTREE;


#ifndef MAX
#define MAX( a, b ) ( a > b ? a : b )
#endif


#ifndef MIN
#define MIN( a, b ) ( a < b ? a : b )
#endif


Board2chCompati::Board2chCompati( const std::string& root, const std::string& path_board, const std::string& name,
    const std::string& basicauth)
    : BoardBase( root, path_board, name ),
      m_settingloader( nullptr ),
      m_ruleloader( nullptr )
{
    set_path_dat( "/dat" );
    set_path_readcgi( "/test/read.cgi" );
    set_path_bbscgi( "/test/bbs.cgi" );
    set_path_subbbscgi( "/test/subbbs.cgi" );
    set_subjecttxt( "subject.txt" );
    set_ext( ".dat" );
    set_id( path_board.substr( 1 ) ); // 先頭の '/' を除く
    set_charset( "MS932" );

    BoardBase::set_basicauth( basicauth );
}


Board2chCompati::~Board2chCompati()
{
    if( m_settingloader ){
        m_settingloader->terminate_load();
        delete m_settingloader;
    }
    m_settingloader = nullptr;

    if( m_ruleloader ){
        m_ruleloader->terminate_load();
        delete m_ruleloader;
    }
    m_ruleloader = nullptr;
}



//
// キャッシュのファイル名が正しいか
//
bool Board2chCompati::is_valid( const std::string& filename )
{
    if( filename.find( get_ext() ) == std::string::npos ) return false;
    if( filename.length() - filename.rfind( get_ext() ) != get_ext().length() ) return false;

    size_t dig, n;
    MISC::str_to_uint( filename.c_str(), dig, n );
    if( dig != n ) return false;
    if( dig == 0 ) return false;
        
    return true;
}


// 書き込み時に必要なキーワード( hana=mogera や suka=pontan など )を
// 確認画面のhtmlから解析する      
void Board2chCompati::analyze_keyword_for_write( const std::string& html )
{
    std::string keyword;

#ifdef _DEBUG
    std::cout << "Board2chCompati::analyze_keyword_for_write\n";
    std::cout << html << std::endl << "--------------------\n";
#endif

    JDLIB::Regex regex;
    size_t offset = 0;
    const bool icase = true; // 大文字小文字区別しない
    const bool newline = false;  // . に改行をマッチさせる
    const bool usemigemo = false;
    const bool wchar = false;

    for(;;){

        // <input type=hidden> のタグを解析して name と value を取得
        if( ! regex.exec( "<input +type=hidden +name=([^ ]*) +value=([^>]*)>", html, offset, icase, newline, usemigemo, wchar ) ) break;

        offset = html.find( regex.str( 0 ) );

        std::string name = MISC::remove_space( regex.str( 1 ) );
        if( name[ 0 ] == '\"' ) name = MISC::cut_str( name, "\"", "\"" );

        std::string value = MISC::remove_space( regex.str( 2 ) );
        if( value[ 0 ] == '\"' ) value = MISC::cut_str( value, "\"", "\"" );

#ifdef _DEBUG
        std::cout << "offset = " << offset << " "
                  << regex.str( 0 ) << std::endl
                  << "name = " << name << " value = " << value << std::endl;
#endif
        ++offset;

        // 除外する name の判定
        // 2ch の仕様が変わったら項目を追加すること
        const std::string lowname = MISC::tolower_str( name );
        if( lowname == "subject"
            || lowname == "from"
            || lowname == "mail"
            || lowname == "message"
            || lowname == "bbs"
            || lowname == "time"
            || lowname == "key" ) continue;

        // キーワード取得
        if( ! keyword.empty() ) keyword += "&";
        keyword += MISC::charset_url_encode( name, get_charset() ) + "=" + MISC::charset_url_encode( value, get_charset() );
    }

#ifdef _DEBUG
    std::cout << "keyword = " << keyword << std::endl;
#endif

    set_keyword_for_write( keyword );
}


// 新スレ作成時の書き込みメッセージ作成
std::string Board2chCompati::create_newarticle_message( const std::string& subject,
                                                       const std::string& name, const std::string& mail, const std::string& msg )
{
    if( subject.empty() ) return std::string();
    if( msg.empty() ) return std::string();

    std::stringstream ss_post;
    ss_post.clear();
    ss_post << "bbs="      << get_id()
            << "&subject=" << MISC::charset_url_encode( subject, get_charset() )
            << "&time="    << get_time_modified()
            << "&submit="  << MISC::charset_url_encode( "新規スレッド作成", get_charset() )
            << "&FROM="    << MISC::charset_url_encode( name, get_charset() )
            << "&mail="    << MISC::charset_url_encode( mail, get_charset() )
            << "&MESSAGE=" << MISC::charset_url_encode( msg, get_charset() );

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
std::string Board2chCompati::url_bbscgi_new()
{
    std::string cgibase = url_bbscgibase();
    return cgibase.substr( 0, cgibase.length() -1 ); // 最後の '/' を除く
}


//
// 新スレ作成時のsubbbscgi のURL
//
// (例) "http://www.hoge2ch.net/test/subbbs.cgi"
//
std::string Board2chCompati::url_subbbscgi_new()
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

    ArticleBase* article = new DBTREE::Article2chCompati( datbase, id, cached );
    if( article ){
        get_hash_article()->push( article );

        // 最大レス数セット
        article->set_number_max( get_number_max_res() );
    }
    else return get_article_null();
    
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
        bool exist_amp = false;
        ++pos;
        str_subject = pos;
        while( *pos != '\0' && *pos != '\n' ){
            if( *pos == '&' ) exist_amp = true;
            ++pos;
        }
        --pos;
        while( *pos != '(' && *pos != '\n' && pos != str_subject ) --pos;
        
        // 壊れてる
        if( *pos == '\n' || pos == str_subject ){
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

        if( str_subject[ lng_subject-1 ] == ' ' ){
            lng_subject--;  // 2chのsubject.txtは()の前に空白が一つ入る
        }
        artinfo.subject.assign( str_subject, lng_subject );

        if( exist_amp ){
            artinfo.subject = MISC::replace_str( artinfo.subject, "&lt;", "<" );
            artinfo.subject = MISC::replace_str( artinfo.subject, "&gt;", ">" );
        }

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

    for( unsigned int i = 0; i < get_list_artinfo().size(); ++i ){

        const ARTICLE_INFO& artinfo = get_list_artinfo()[ i ];
        
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


std::string Board2chCompati::localrule()
{
    if( m_ruleloader ){
        if( m_ruleloader->is_loading() ) return "ロード中です";
        else if( m_ruleloader->get_code() == HTTP_OK || m_ruleloader->get_code() == HTTP_REDIRECT || m_ruleloader->get_code() == HTTP_MOVED_PERM ){
            if( m_ruleloader->get_data().empty() ) return "ローカルルールはありません";
            else return m_ruleloader->get_data();
        }
        else return "ロードに失敗しました : " + m_ruleloader->get_str_code();
    }

    return BoardBase::localrule();
}


std::string Board2chCompati::settingtxt()
{
    if( m_settingloader ){
        if( m_settingloader->is_loading() ) return "ロード中です";
        else if( m_settingloader->get_code() == HTTP_OK || m_settingloader->get_code() == HTTP_REDIRECT || m_settingloader->get_code() == HTTP_MOVED_PERM ){
            if( m_settingloader->get_data().empty() ) return "SETTING.TXTはありません";
            else return m_settingloader->get_data();
        }
        else return "ロードに失敗しました : " + m_settingloader->get_str_code();
    }

    return BoardBase::settingtxt();
}


std::string Board2chCompati::default_noname()
{
    if( m_settingloader
        && m_settingloader->get_code() == HTTP_OK ) return m_settingloader->default_noname();

    return BoardBase::default_noname();
}


int Board2chCompati::line_number()
{
    if( m_settingloader
        && m_settingloader->get_code() == HTTP_OK ) return m_settingloader->line_number();

    return BoardBase::line_number();
}


int Board2chCompati::message_count()
{
    if( m_settingloader
        && m_settingloader->get_code() == HTTP_OK ) return m_settingloader->message_count();

    return BoardBase::message_count();
}    


std::string Board2chCompati::get_unicode()
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

    if( ! m_ruleloader ) m_ruleloader = new RuleLoader( url_boardbase() );
    m_ruleloader->load_text();

    if( ! m_settingloader ) m_settingloader = new SettingLoader( url_boardbase() );
    m_settingloader->load_text();
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

    if( ! m_ruleloader ) m_ruleloader = new RuleLoader( url_boardbase() );
    m_ruleloader->download_text();

    if( ! m_settingloader ) m_settingloader = new SettingLoader( url_boardbase() );
    m_settingloader->download_text();
}


//
// レス数であぼーん(グローバル)
//
int Board2chCompati::get_abone_number_global()
{
    return CONFIG::get_abone_number_thread();
}
