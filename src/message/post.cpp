// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "messageadmin.h"
#include "post.h"

#include "skeleton/msgdiag.h"

#include "jdlib/loaderdata.h"
#include "jdlib/jdiconv.h"
#include "jdlib/miscmsg.h"
#include "jdlib/miscutil.h"
#include "jdlib/jdregex.h"

#include "dbtree/interface.h"

#include "config/globalconf.h"

#include "httpcode.h"


using namespace MESSAGE;

#define SIZE_OF_RAWDATA ( 2 * 1024 * 1024 )


Post::Post( Gtk::Widget* parent, const std::string& url, const std::string& msg, bool new_article )
    : SKELETON::Loadable(),
      m_parent( parent ),
      m_url( url ),
      m_msg( msg ),
      m_rawdata( 0 ),
      m_lng_rawdata( 0 ),
      m_count( 0 ),
      m_subbbs( 0 ),
      m_new_article( new_article ),
      m_writingdiag( 0 )
{
    clear();
}



Post::~Post()
{
#ifdef _DEBUG
    std::cout << "Post::~Post " << m_url << std::endl;
#endif

    clear();

    if( m_writingdiag ) delete m_writingdiag;
    m_writingdiag = NULL;
}


void Post::clear()
{
    if( m_rawdata ) free( m_rawdata );
    m_rawdata = NULL;

    if( m_writingdiag ) m_writingdiag->hide();
}


void Post::emit_sigfin()
{
    m_sig_fin.emit();

    clear();

    if( m_writingdiag ) delete m_writingdiag;
    m_writingdiag = NULL;
}


//
// ポスト実行
//
void Post::post_msg()
{
    if( is_loading() ) return;

    clear();
    m_rawdata = ( char* )malloc( SIZE_OF_RAWDATA );
    m_lng_rawdata = 0;

    // 書き込み中ダイアログ表示
    Gtk::Window* toplevel = dynamic_cast< Gtk::Window* >( m_parent->get_toplevel() );
    if( ! CONFIG::get_hide_writing_dialog() && toplevel ){

        if( ! m_writingdiag ) m_writingdiag = new SKELETON::MsgDiag( *toplevel, "書き込み中・・・", false, Gtk::MESSAGE_INFO, Gtk::BUTTONS_NONE, false );
        m_writingdiag->show();

        // gtkのバージョンによってはラベルが選択状態になっている場合があるので
        // 選択状態を解除する
        Gtk::Label *label = dynamic_cast< Gtk::Label* >( m_writingdiag->get_focus() );
        if( label ) label->set_selectable( false );
    }

    JDLIB::LOADERDATA data;

    // 通常書き込み
    if( ! m_new_article ){
        if( ! m_subbbs ) data.url = DBTREE::url_bbscgi( m_url );  // 1回目の投稿先
        else data.url = DBTREE::url_subbbscgi( m_url ); // 2回目の投稿先
    }

    // 新スレ作成
    else{
        if( ! m_subbbs ) data.url = DBTREE::url_bbscgi_new( m_url );  // 1回目の投稿先
        else data.url = DBTREE::url_subbbscgi_new( m_url ); // 2回目の投稿先
    }

    data.agent = DBTREE::get_agent( m_url );
    data.referer = DBTREE::url_boardbase( m_url );
    data.str_post = m_msg;
    data.host_proxy = DBTREE::get_proxy_host_w( m_url );
    data.port_proxy = DBTREE::get_proxy_port_w( m_url );
    data.size_buf = CONFIG::get_loader_bufsize();
    data.timeout = CONFIG::get_loader_timeout_post();
    data.cookie_for_write = DBTREE::board_cookie_for_write( m_url );
    data.basicauth = DBTREE::board_basicauth( m_url );

#ifdef _DEBUG
    std::cout << "Post::post_msg : " << std::endl
              << "url = " << data.url << std::endl
              << "agent = " << data.agent << std::endl
              << "referer = " << data.referer << std::endl
              << "cookie = " << data.cookie_for_write << std::endl
              << "proxy = " << data.host_proxy << ":" << data.port_proxy << std::endl
              << m_msg << std::endl;
#endif

    if( data.url.empty() ) return;

    start_load( data );
}



//
// ローダからデータを受け取る
//
void Post::receive_data( const char* data, size_t size )
{
    if( get_code() != HTTP_OK ) return;

    memcpy( m_rawdata + m_lng_rawdata , data, size );
    m_lng_rawdata += size;
    m_rawdata[ m_lng_rawdata ] = '\0';
}



//
// ローダがsendを終了したので戻り値解析
//
void Post::receive_finish()
{
#ifdef _DEBUG
    std::cout << "Post::receive_finish\n";
#endif

    std::string charset = DBTREE::board_charset( m_url );
    JDLIB::Iconv* libiconv = new JDLIB::Iconv( charset, "UTF-8" );
    int byte_out;
    const std::string str = libiconv->convert( m_rawdata, m_lng_rawdata, byte_out );
    delete libiconv;

#ifdef _DEBUG
    std::cout << "code = " << get_code() << std::endl;
    std::cout << str << std::endl;
#endif

    clear();

    ///////////////////
    
    // ポスト失敗
    if( get_code() != HTTP_OK
        && ! ( get_code() == HTTP_REDIRECT && ! location().empty() ) // リダイレクトは成功(かもしれない)
        ){

        m_errmsg = get_str_code();
        emit_sigfin();
        return;
    }

    // 以下、code == 200 or  302 で locationがセットされている(リダイレクト) の場合

    JDLIB::Regex regex;

    std::string title;
    std::string tag_2ch;
    std::string msg;
    std::string hana;
    std::string conf;

    bool ret;

    // タイトル
    regex.exec( ".*<title>([^<]*)</title>.*", str, 0, true, false );
    title = MISC::remove_space( regex.str( 1 ) );

    // エラー
    // 一番内側の<b>〜</b>を探す
    m_errmsg = std::string();
    if( regex.exec( "([^>]|[^b]>)*<b>(([^>]|[^b]>)*)</b>.*", str, 0, true, false ) ){

        m_errmsg = regex.str( 2 );

        // <a 〜を取り除く
        while( regex.exec( "(.*)<a +href *= *\"([^\"]*)\" *>(.*)</a>(.*)", m_errmsg ) ){
            m_errmsg = regex.str( 1 ) + " " + regex.str( 2 ) + " " + regex.str( 3 ) + regex.str( 4 );
        }

        // 改行
        m_errmsg= MISC::replace_str( m_errmsg, "<br>", "\n" );
    }

    // 2chタグ
    regex.exec( ".*2ch_X:([^\\-]*)\\-\\->.*", str, 0, false, false );
    tag_2ch = MISC::remove_space( regex.str( 1 ) );

    // 書き込み確認
    regex.exec( ".*<font size=\\+1 color=#FF0000>([^<]*)</font>.*", str );
    conf = MISC::remove_space( regex.str( 1 ) );

    // メッセージ本文

    // 2ch 型
    ret = regex.exec( ".*</ul>.*<b>(.*)</b>.*<form.*", str, 0, false, false );

    // 0ch 型
    if( ! ret ) ret = regex.exec( ".*</ul>.*<b>(.*)</b>.*<input.*", str, 0, false, false );

    msg = MISC::remove_space( regex.str( 1 ) );

    // 2ch の hana 値
    regex.exec( ".*<input +type=hidden +name=\"?hana\"? +value=\"?([^\"]*)\"?.*", str, 0, false, false );
    hana = MISC::remove_space( regex.str( 1 ) );

#ifdef _DEBUG
    std::cout << "TITLE: [" << title << "]\n";
    std::cout << "2ch_X: [" << tag_2ch << "]\n";
    std::cout << "CONF: [" << conf << "]\n";
    std::cout << "MSG: [" << msg << "]\n";
    std::cout << "ERR: [" << m_errmsg << "]\n";

    std::list< std::string > list_cookies = SKELETON::Loadable::cookies();
    std::list< std::string >::iterator it = list_cookies.begin();
    for( ; it != list_cookies.end(); ++it ) std::cout << "cookie : [" << (*it) << "]\n";

    std::cout << "hana: [" << hana << "]\n";
    std::cout << "location: [" << location() << "]\n";
#endif

    // 成功
    if( title.find( "書きこみました" ) != std::string::npos
        || tag_2ch.find( "true" ) != std::string::npos
        || ( get_code() == HTTP_REDIRECT && ! location().empty() )  // リダイレクトされた場合
        ){

#ifdef _DEBUG
        std::cout << "write ok" << std::endl;
#endif        

        DBTREE::article_update_writetime( m_url );
        emit_sigfin();
        return;
    }

    // クッキー確認
    else if( m_count < 1 && // 永久ループ防止
        ( title.find( "書き込み確認" ) != std::string::npos
          || tag_2ch.find( "cookie" ) != std::string::npos  ) ){

        clear();

        // 書き込み確認表示
        if( ! CONFIG::get_always_write_ok() ){

            std::string diagmsg = MISC::replace_str( msg, "<br>", "\n" );

            SKELETON::MsgDiag mdiag( MESSAGE::get_admin()->get_win(),
                                     diagmsg, false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_NONE, false );
            Gtk::CheckButton ckbt( "次回から表示しない(_D)", true );
            mdiag.get_vbox()->pack_start( ckbt, Gtk::PACK_SHRINK );
            mdiag.add_button( Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL );

            // OKボタンをデフォルトにする
            Gtk::Button okbt( Gtk::Stock::OK );
            mdiag.add_action_widget( okbt, Gtk::RESPONSE_OK );
            okbt.set_flags( Gtk::CAN_DEFAULT );
            okbt.grab_default();
            okbt.grab_focus();

            mdiag.show_all_children();

            int ret = mdiag.run();
            bool ckbt_active = ckbt.get_active();
            mdiag.hide();

            if( ret != Gtk::RESPONSE_OK ){

                set_code( HTTP_CANCEL );
                emit_sigfin();
                return;
            }
            CONFIG::set_always_write_ok( ckbt_active );
        }

        set_cookies_and_hana( SKELETON::Loadable::cookies(), hana );

        ++m_count; // 永久ループ防止
        post_msg();

        return;
    }

    // スレ立て時の書き込み確認
    else if( m_count < 1 // 永久ループ防止
             && ! m_subbbs && conf.find( "書き込み確認" ) != std::string::npos ){

        set_cookies_and_hana( SKELETON::Loadable::cookies(), hana );

        // subbbs.cgi にポスト先を変更してもう一回ポスト
        m_subbbs = true;
        ++m_count; // 永久ループ防止
        post_msg();
        return;
    }

    // その他エラー
    else{
        
#ifdef _DEBUG
        std::cout << "Error" << std::endl;
        std::cout << m_errmsg << std::endl;
#endif        

        MISC::ERRMSG( str );

        set_code( HTTP_ERR );
        emit_sigfin();
        return;
    }
}



//
// データベースにクッキーとhanaを登録
//
void Post::set_cookies_and_hana( const std::list< std::string >& cookies, const std::string& hana )
{
    if( ! cookies.empty() ) DBTREE::board_set_list_cookies_for_write( m_url, cookies );

    if( ! hana.empty() ){
        DBTREE::board_set_hana_for_write( m_url, hana );

        // 手抜き。後で直すこと
        if( m_msg.find( "hana=" ) == std::string::npos ) m_msg += "&hana=" + hana;
    }
}

