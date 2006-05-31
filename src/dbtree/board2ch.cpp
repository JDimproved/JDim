// ライセンス: 最新のGPL

//#define _DEBUG
#include "jddebug.h"

#include "board2ch.h"
#include "article2ch.h"
#include "settingloader.h"

#include "config/globalconf.h"

#include "httpcode.h"

using namespace DBTREE;


Board2ch::Board2ch( const std::string& root, const std::string& path_board, const std::string& name )
    : Board2chCompati( root, path_board, name )
    , m_settingloader( 0 )
{
#ifdef _DEBUG
    std::cout << "Board2ch::Board2ch\n";
#endif
}


Board2ch::~Board2ch()
{
    if( m_settingloader ) delete m_settingloader;
    m_settingloader = NULL;
}


const std::string& Board2ch::get_agent()
{
    return CONFIG::get_agent_for2ch();
}


//
// ホスト別プロクシ
//
const std::string Board2ch::get_proxy_host()
{
    if( ! CONFIG::get_use_proxy_for2ch() ) return std::string();
    return CONFIG::get_proxy_for2ch();
}

const int Board2ch::get_proxy_port()
{
    return CONFIG::get_proxy_port_for2ch();
}


const std::string Board2ch::get_proxy_host_w()
{
    if( ! CONFIG::get_use_proxy_for2ch_w() ) return std::string();
    return CONFIG::get_proxy_for2ch_w();
}

const int Board2ch::get_proxy_port_w()
{
    return CONFIG::get_proxy_port_for2ch_w();
}


const std::string Board2ch::settingtxt()
{
    if( m_settingloader ){
        if( m_settingloader->is_loading() ) return "ロード中です";
        else if( m_settingloader->get_code() == HTTP_OK ) return m_settingloader->settingtxt();
        else return "ロードに失敗しました : " + m_settingloader->get_str_code();
    }

    return BoardBase::settingtxt();
}


const std::string Board2ch::default_noname()
{
    if( m_settingloader
        && m_settingloader->get_code() == HTTP_OK ) return m_settingloader->default_noname();

    return BoardBase::default_noname();
}


const int Board2ch::line_number()
{
    if( m_settingloader
        && m_settingloader->get_code() == HTTP_OK ) return m_settingloader->line_number();

    return BoardBase::line_number();
}


const int Board2ch::message_count()
{
    if( m_settingloader
        && m_settingloader->get_code() == HTTP_OK ) return m_settingloader->message_count();

    return BoardBase::message_count();
}    



//
// 新しくArticleBaseクラスを追加してそのポインタを返す
//
// cached : HDD にキャッシュがあるならtrue
//
ArticleBase* Board2ch::append_article( const std::string& id, bool cached )
{
    if( empty() ) return get_article_null();

    ArticleBase* article = new DBTREE::Article2ch( url_datbase(), id, cached );
    if( article ) get_list_article().push_back( article );
    else return get_article_null();

    return article;
}



//
// SETTING.TXTをキャッシュから読み込む
//
// BoardBase::read_info()で呼び出す
//
void Board2ch::load_setting()
{
#ifdef _DEBUG
    std::cout << "Board2ch::load_setting\n";
#endif

    if( ! m_settingloader ) m_settingloader = new SettingLoader( url_boardbase() );
    m_settingloader->load_setting();
}


//
// SETTING.TXTのサーバからダウンロード
//
// 読み込むタイミングはsubject.txtを読み終わった直後( BoardBase::receive_finish() )
//
void Board2ch::download_setting()
{
#ifdef _DEBUG
    std::cout << "Board2ch::download_setting\n";
#endif

    if( ! m_settingloader ) m_settingloader = new SettingLoader( url_boardbase() );
    m_settingloader->download_setting();
}
