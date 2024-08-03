// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "boardjbbs.h"
#include "articlejbbs.h"
#include "articlehash.h"
#include "ruleloader.h"
#include "settingloader.h"

#include "jdlib/miscutil.h"
#include "jdlib/miscmsg.h"
#include "jdlib/loaderdata.h"

#include "config/globalconf.h"

#include "global.h"
#include "httpcode.h"

#include <algorithm>
#include <cstring>
#include <sstream>

using namespace DBTREE;


BoardJBBS::BoardJBBS( const std::string& root, const std::string& path_board, const std::string& name )
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
    set_encoding( Encoding::eucjp );
    set_default_encoding( Encoding::eucjp );
}


BoardJBBS::~BoardJBBS() noexcept
{
    if( m_settingloader ) {
        m_settingloader->terminate_load();
    }
    if( m_ruleloader ) {
        m_ruleloader->terminate_load();
    }
}


//
// キャッシュのファイル名が正しいか
//
bool BoardJBBS::is_valid( const std::string& filename ) const
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
ArticleBase* BoardJBBS::append_article( const std::string& datbase, const std::string& id, const bool cached )
{
    if( empty() ) return get_article_null();

    ArticleBase* article = insert( std::make_unique<DBTREE::ArticleJBBS>( datbase, id, cached, get_encoding() ) );

    if( ! article ) return get_article_null();
    return article;
}



//
// rawmode のURLのパスを返す
//
// (例) "/bbs/rawmode.cgi/board/"  (最初と最後に '/' がつく)
//
std::string BoardJBBS::url_datpath() const
{
    if( empty() ) return std::string();

    return "/bbs/rawmode.cgi" + get_path_board() + "/";
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
std::string BoardJBBS::create_newarticle_message( const std::string& subject, const std::string& name,
                                                  const std::string& mail, const std::string& msg, bool utf8_post )
{
    if( subject.empty() ) return std::string();
    if( msg.empty() ) return std::string();

    // DIR と BBS を分離する( ID = DIR/BBS )
    std::string boardid = get_id();
    int i = boardid.find( '/' );
    std::string dir = boardid.substr( 0, i );
    std::string bbs = boardid.substr( i + 1 );

    const Encoding enc{ utf8_post ? Encoding::utf8 : get_encoding() };

    std::stringstream ss_post;
    ss_post.clear();
    ss_post << "SUBJECT="  << MISC::url_encode_plus( subject, enc )
            << "&submit="  << MISC::url_encode_plus( "新規書き込み", enc )
            << "&NAME="    << MISC::url_encode_plus( name, enc )
            << "&MAIL="    << MISC::url_encode_plus( mail, enc )
            << "&MESSAGE=" << MISC::url_encode_plus( msg, enc )
            << "&DIR="     << dir
            << "&BBS="     << bbs
            << "&TIME="    << get_time_modified();

#ifdef _DEBUG
    std::cout << "BoardJBBS::create_newarticle_message " << ss_post.str() << std::endl;
#endif

    return ss_post.str();
}


//
// 新スレ作成時のbbscgi(write.cgi) のURL
//
// (例) "http://jbbs.shitaraba.net/bbs/write.cgi/computer/123/new/"
//
//
std::string BoardJBBS::url_bbscgi_new() const
{
    return url_bbscgibase() + get_id() + "/new/";
}


//
// 新スレ作成時のsubbbscgi のURL
//
// (例) "http://jbbs.shitaraba.net/bbs/write.cgi/computer/123/new/"
//
std::string BoardJBBS::url_subbbscgi_new() const
{
    return url_subbbscgibase() + get_id() + "/new/";
}


std::string BoardJBBS::localrule() const
{
    if( m_ruleloader ){
        if( m_ruleloader->is_loading() ) return "ロード中です";
        else if( m_ruleloader->get_code() == HTTP_OK
                 || m_ruleloader->get_code() == HTTP_REDIRECT
                 || m_ruleloader->get_code() == HTTP_MOVED_PERM
                 || m_ruleloader->get_code() == HTTP_PERMANENT_REDIRECT )
        {
            const std::string& data = m_ruleloader->get_data();
            return data.empty() ? "ローカルルールはありません" : data;
        }
        return "ロードに失敗しました : " + m_ruleloader->get_str_code();
    }

    return BoardBase::localrule();
}


std::string BoardJBBS::settingtxt() const
{
    if( m_settingloader ){
        if( m_settingloader->is_loading() ) return "ロード中です";
        else if( m_settingloader->get_code() == HTTP_OK
                 || m_settingloader->get_code() == HTTP_REDIRECT
                 || m_settingloader->get_code() == HTTP_MOVED_PERM
                 || m_settingloader->get_code() == HTTP_PERMANENT_REDIRECT )
        {
            const std::string& data = m_settingloader->get_data();
            return data.empty() ? "SETTING.TXTはありません" : data;
        }
        return "ロードに失敗しました : " + m_settingloader->get_str_code();
    }

    return BoardBase::settingtxt();
}


std::string BoardJBBS::default_noname() const
{
    if( m_settingloader && m_settingloader->get_code() == HTTP_OK ) {
        return m_settingloader->default_noname();
    }
    return BoardBase::default_noname();
}


//
// SETTING.TXT のURL
//
// (例) "http://jbbs.shitaraba.net/bbs/api/setting.cgi/computer/123/"
//
std::string BoardJBBS::url_settingtxt() const
{
    return get_root() + "/bbs/api/setting.cgi/" + get_id() + "/";
}


//
// ローカルルールとSETTING.TXTをキャッシュから読み込む
//
// BoardBase::read_info()で呼び出す
//
void BoardJBBS::load_rule_setting()
{
    if( ! m_ruleloader ) m_ruleloader = std::make_unique<RuleLoader>( url_boardbase() );
    m_ruleloader->load_text( Encoding::sjis );

    if( ! m_settingloader ) m_settingloader = std::make_unique<SettingLoader>( url_boardbase() );
    m_settingloader->load_text( get_encoding() );
}


//
// SETTING.TXTをサーバからダウンロード
// ローカルルールはスレ本文とエンコーディングが異なる
//
// 読み込むタイミングはsubject.txtを読み終わった直後( BoardBase::receive_finish() )
//
void BoardJBBS::download_rule_setting()
{
    if( ! m_ruleloader ) m_ruleloader = std::make_unique<RuleLoader>( url_boardbase() );
    m_ruleloader->download_text( Encoding::sjis );

    if( ! m_settingloader ) m_settingloader = std::make_unique<SettingLoader>( url_boardbase() );
    m_settingloader->download_text( get_encoding() );
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


void BoardJBBS::regist_article( const bool is_online )
{
    if( ! get_list_artinfo().size() ) return;

#ifdef _DEBUG
    std::cout << "BoardJBBS::regist_article size = " << get_list_artinfo().size() << std::endl;
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
            // JBBSは最初と最後の行が同じになる仕様があるので最後の行を除く
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
