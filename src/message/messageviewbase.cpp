// ライセンス: GPL2

//#define _DEBUG
//#define _DEBUG_KEY
#include "jddebug.h"

#include "messageadmin.h"
#include "messageviewbase.h"
#include "post.h"
#include "toolbar.h"

#include "skeleton/msgdiag.h"

#include "jdlib/miscutil.h"
#include "jdlib/misctime.h"
#include "jdlib/misctrip.h"
#include "jdlib/jdiconv.h"

#include "dbtree/interface.h"

#include "config/globalconf.h"

#include "icons/iconmanager.h"

#include "httpcode.h"
#include "command.h"
#include "viewfactory.h"
#include "controlutil.h"
#include "controlid.h"
#include "fontid.h"
#include "cache.h"
#include "session.h"
#include "colorid.h"
#include "global.h"

#include <sstream>
#include <sys/time.h>


using namespace MESSAGE;

#define MAX_STR_ICONV 128*1024

#define PASS_TIMEOUT 500
#define PASS_MAXTIME 120

enum{
    PAGE_MESSAGE = 0,
    PAGE_PREVIEW
};


MessageViewBase::MessageViewBase( const std::string& url )
    : SKELETON::View( url ),
      m_post( 0 ),
      m_preview( 0 ),
      m_enable_menuslot( true ),
      m_enable_focus( true ),
      m_counter( 0 )
{
#ifdef _DEBUG
    std::cout << "MessageViewBase::MessageViewBase " << get_url() << std::endl;
#endif

    // コントロールモード設定
    get_control().add_mode( CONTROL::MODE_MESSAGE );

    m_max_line = DBTREE::line_number( get_url() ) * 2;
    m_max_str = DBTREE::message_count( get_url() );

    m_iconv = new JDLIB::Iconv( "UTF-8", DBTREE::board_charset( get_url() ) );;

    m_lng_iconv = m_max_str * 3;
    if( ! m_lng_iconv ) m_lng_iconv = MAX_STR_ICONV;

    m_str_iconv = ( char* ) malloc( m_lng_iconv + 1024 );
}



MessageViewBase::~MessageViewBase()
{
#ifdef _DEBUG
    std::cout << "MessageViewBase::~MessageViewBase " << get_url() << std::endl;
#endif

    if( m_preview ) delete m_preview;
    m_preview = NULL;

    if( m_post ) delete m_post;
    m_post = NULL;

    if( m_iconv ) delete m_iconv;
    m_iconv = NULL;

    if( m_str_iconv ) free( m_str_iconv );
    m_str_iconv = NULL;
}


MessageToolBar* MessageViewBase::get_messagetoolbar()
{
    return dynamic_cast< MessageToolBar* >( get_toolbar() );
}


SKELETON::LabelEntry* MessageViewBase::get_entry_subject()
{
    if( ! get_messagetoolbar() ) return NULL;
    return &get_messagetoolbar()->m_entry_subject;
}


//
// コピー用URL( readcgi型 )
//
// メインウィンドウのURLバーなどに表示する)
//
const std::string MessageViewBase::url_for_copy()
{
    return DBTREE::url_readcgi( get_url(), 0, 0 );
}



void MessageViewBase::clock_in()
{
    if( m_preview ) m_preview->clock_in();

    // 経過時刻表示
    ++m_counter;
    if( m_counter % ( PASS_TIMEOUT / TIMER_TIMEOUT ) == 0 ){

        m_counter = 0;

        time_t left = DBTREE::board_write_leftsec( get_url() );
        if( left ){
            m_str_pass = "  ( 書込規制中 残り " + MISC::itostr( left ) + " 秒 )";
            MESSAGE::get_admin()->set_command( "set_status", get_url(), get_status() + m_str_pass );
        }
        else if( ! m_str_pass.empty() ){
            m_str_pass = std::string();
            MESSAGE::get_admin()->set_command( "set_status", get_url(), get_status() );
        }
    }
}


//
// セットアップ
//
void MessageViewBase::setup_view()
{
    pack_widget();
}



//
// フォント初期化
//
void MessageViewBase::init_font( const std::string& fontname )
{
    Pango::FontDescription pfd( fontname );
    pfd.set_weight( Pango::WEIGHT_NORMAL );

    m_entry_name.modify_font( pfd );
    m_entry_mail.modify_font( pfd );
    m_text_message.modify_font( pfd );
}


//
// 色初期化
//
void MessageViewBase::init_color()
{
    m_text_message.modify_text( Gtk::STATE_NORMAL, Gdk::Color( CONFIG::get_color( COLOR_CHAR_MESSAGE ) ) );
    m_text_message.modify_text( Gtk::STATE_SELECTED, Gdk::Color( CONFIG::get_color( COLOR_CHAR_MESSAGE_SELECTION ) ) );
    m_text_message.modify_base( Gtk::STATE_NORMAL, Gdk::Color( CONFIG::get_color( COLOR_BACK_MESSAGE ) ) );
    m_text_message.modify_base( Gtk::STATE_SELECTED, Gdk::Color( CONFIG::get_color( COLOR_BACK_MESSAGE_SELECTION ) ) );
}



//
// コマンド
//
bool MessageViewBase::set_command( const std::string& command, const std::string& arg )
{
    if( command == "empty" ) return get_message().empty();

    else if( command == "loading" ){
        if( !m_post ) return false;
        return m_post->is_loading();
    }

    else if( command == "exec_Write" ) slot_write_clicked();
    else if( command == "tab_left" ) tab_left();
    else if( command == "tab_right" ) tab_right();
    else if( command == "focus_write" ) focus_writebutton();

    // メッセージをクリア
    else if( command == "clear_message" ){

        m_text_message.set_text( std::string() );

        if( m_notebook.get_current_page() != PAGE_MESSAGE ){
            m_enable_focus = false;
            m_notebook.set_current_page( PAGE_MESSAGE );
            m_enable_focus = true;
        }
    }

    // メッセージを追加
    else if( command == "add_message" )
    {
        std::string caution;

        // 追加する文字列が制限値を超えていないかチェック
        if( m_max_str && int( arg.length() ) > m_max_str ) caution.append( "文字数" );
        if( m_max_line && MISC::count_str( arg, "\n" ) > m_max_line )
        {
            if( ! caution.empty() ) caution.append( "及び" );
            caution.append( "改行数" );
        }

        if( ! caution.empty() )
        {
            SKELETON::MsgDiag mdiag( MESSAGE::get_admin()->get_win(),
                                     caution.append( "がスレッドの制限値を超えています。\n\n追加しますか？" ),
                                     false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_OK_CANCEL );
            if( mdiag.run() != Gtk::RESPONSE_OK ) return false;
        }

        m_text_message.insert_str( arg, true );
    }

    // メッセージ保存
    else if ( "save_message" )
    {
        if( ! get_message().empty() ){

            std::string filename = "draft-" + MISC::get_filename( get_url() );
            if( filename.find( ".dat" ) != std::string::npos ) filename = MISC::replace_str( filename, ".dat", ".txt" );
            else filename += ".txt";
            std::string save_to = CACHE::open_save_diag( MESSAGE::get_admin()->get_win(), SESSION::get_dir_draft(), filename, CACHE::FILE_TYPE_TEXT );
            if( ! save_to.empty() ){

                SESSION::set_dir_draft( MISC::get_dir( save_to ) );

                if( CACHE::save_rawdata( save_to, get_message() ) != get_message().raw().length() ){
                    SKELETON::MsgDiag mdiag( MESSAGE::get_admin()->get_win(),
                                             "保存に失敗しました。\nハードディスクの容量やパーミッションなどを確認してください。" );
                    mdiag.run();
                }
            }
        }
    }

    return false;
}



//
// 名前やメールを保存
//
void MessageViewBase::save_name()
{
    bool check_fixname = m_check_fixname.get_active();
    bool check_fixmail = m_check_fixmail.get_active();

    if( check_fixname != DBTREE::write_fixname( get_url() ) ) DBTREE::set_write_fixname( get_url(), check_fixname );
    if( check_fixname ){
        std::string name = m_entry_name.get_text();
        if( name != DBTREE::write_name( get_url() ) ) DBTREE::set_write_name( get_url(), name );
    }

    if( check_fixmail != DBTREE::write_fixmail( get_url() ) ) DBTREE::set_write_fixmail( get_url(), check_fixmail );
    if( check_fixmail ){
        std::string mail = m_entry_mail.get_text();
        if( mail != DBTREE::write_mail( get_url() ) ) DBTREE::set_write_mail( get_url(), mail );
    }
}



//
// ツールバーなどのパック
//
void MessageViewBase::pack_widget()
{
    // ツールバー
    set_toolbar( Gtk::manage( new MessageToolBar( " [ " + DBTREE::board_name( get_url() ) + " ]  " ) ) );
    get_messagetoolbar()->m_entry_subject.set_text( DBTREE::article_subject( get_url() ) );
    get_messagetoolbar()->m_button_not_close.set_active( ! SESSION::get_close_mes() );

    get_messagetoolbar()->m_button_write.signal_clicked().connect( sigc::mem_fun( *this, &MessageViewBase::slot_write_clicked ) );
    get_messagetoolbar()->get_close_button().signal_clicked().connect( sigc::mem_fun( *this, &MessageViewBase::slot_close_clicked ) );
    get_messagetoolbar()->m_button_open.signal_clicked().connect( sigc::mem_fun( *this, &MessageViewBase::slot_draft_open ) );
    get_messagetoolbar()->m_button_undo.signal_clicked().connect( sigc::mem_fun( *this, &MessageViewBase::slot_undo_clicked ) );
    get_messagetoolbar()->m_button_not_close.signal_clicked().connect( sigc::mem_fun( *this, &MessageViewBase::slot_not_close_clicked ) );
    get_messagetoolbar()->m_button_preview.signal_clicked().connect( sigc::mem_fun( *this, &MessageViewBase::slot_preview_clicked ) );
    
    get_messagetoolbar()->show_toolbar();

    if( SESSION::get_close_mes() ) get_messagetoolbar()->get_close_button().set_sensitive( true );
    else get_messagetoolbar()->get_close_button().set_sensitive( false );

    // 書き込みビュー
    m_label_name.set_alignment( Gtk::ALIGN_LEFT, Gtk::ALIGN_CENTER );
    m_label_mail.set_alignment( Gtk::ALIGN_LEFT, Gtk::ALIGN_CENTER );
    m_label_name.set_text( " 名前 " );
    m_label_mail.set_text( " メール " );

    m_check_fixname.set_label( "固定" );
    m_check_fixmail.set_label( "固定" );

    m_tooltip.set_tip( m_check_fixname, "チェックすると名前欄を保存して固定にする" );
    m_tooltip.set_tip( m_check_fixmail, "チェックするとメール欄を保存して固定にする" );

    // 名前
    
    if( DBTREE::write_fixname( get_url() ) ){
        m_check_fixname.set_active();
        m_entry_name.set_text( DBTREE::write_name( get_url() ) );
    }
    else if( ! DBTREE::board_get_write_name( get_url() ).empty() ){
        m_entry_name.set_text( DBTREE::board_get_write_name( get_url() ) );
    }

    // メール
    if( DBTREE::write_fixmail( get_url() ) ){
        m_check_fixmail.set_active();
        m_entry_mail.set_text( DBTREE::write_mail( get_url() ) );
    }
    else if( ! DBTREE::board_get_write_mail( get_url() ).empty() ){

        std::string tmpmail = DBTREE::board_get_write_mail( get_url() );

        // 空白セット
        if( tmpmail == JD_MAIL_BLANK ) m_entry_mail.set_text( std::string() );
        else m_entry_mail.set_text( tmpmail );
    }
    else m_entry_mail.set_text( "sage" );

    m_hbox_name_mail.pack_start( m_label_name, Gtk::PACK_SHRINK );
    m_hbox_name_mail.pack_start( m_check_fixname, Gtk::PACK_SHRINK );
    m_hbox_name_mail.pack_start( m_entry_name );
    m_hbox_name_mail.pack_start( m_label_mail, Gtk::PACK_SHRINK );
    m_hbox_name_mail.pack_start( m_check_fixmail, Gtk::PACK_SHRINK );
    m_hbox_name_mail.pack_start( m_entry_mail );

    m_msgview.pack_start( m_hbox_name_mail, Gtk::PACK_SHRINK );    
    m_msgview.pack_start( m_text_message );

    m_text_message.set_accepts_tab( false );
    m_text_message.sig_key_press().connect( sigc::mem_fun(*this, &MessageViewBase::slot_key_press ) );    
    m_text_message.sig_button_press().connect( sigc::mem_fun(*this, &MessageViewBase::slot_button_press ) );
    m_text_message.get_buffer()->signal_changed().connect( sigc::mem_fun(*this, &MessageViewBase::show_status ) );

    // プレビュー
    m_preview = CORE::ViewFactory( CORE::VIEW_ARTICLEPREVIEW, get_url() );

    m_notebook.set_show_tabs( false );
    m_notebook.append_page( m_msgview, "メッセージ" );
    m_notebook.append_page( *m_preview, "プレビュー" );
    m_notebook.signal_switch_page().connect( sigc::mem_fun( *this, &MessageViewBase::slot_switch_page ) );
    m_notebook.set_current_page( PAGE_MESSAGE );

    pack_start( *get_messagetoolbar(), Gtk::PACK_SHRINK );
    pack_start( m_notebook );

    // フォントセット
    init_font( CONFIG::get_fontname( FONT_MESSAGE ) );

    // 色セット
    init_color();

    show_status();
}



//
// 再描画
//
void MessageViewBase::redraw_view()
{
    if( m_preview ) m_preview->redraw_view();
}



void MessageViewBase::focus_view()
{
#ifdef _DEBUG
    std::cout << "MessageViewBase::focus_view page = " << m_notebook.get_current_page() << std::endl;
#endif

    if( m_notebook.get_current_page() == PAGE_MESSAGE ) m_text_message.focus_view();
    else if( m_preview && m_notebook.get_current_page() == PAGE_PREVIEW ) m_preview->focus_view();
}


//
// viewの操作
//
void MessageViewBase::operate_view( const int& control )
{
    if( control == CONTROL::None ) return;

    switch( control ){
            
            // 書き込まずに閉じる
        case CONTROL::CancelWrite:
            close_view();
            break;

            // 書き込み実行
        case CONTROL::ExecWrite:
            MESSAGE::get_admin()->set_command( "exec_Write" );
            break;

        case CONTROL::TabLeft:
            MESSAGE::get_admin()->set_command( "tab_left" );
            break;

        case CONTROL::TabRight:
            MESSAGE::get_admin()->set_command( "tab_right" );
            break;

            // 書き込みボタンにフォーカスを移す
        case CONTROL::FocusWrite:
            focus_writebutton();
            break;
    }
}



//
// 書き込むボタン押した
//
void MessageViewBase::slot_write_clicked()
{
    time_t left = DBTREE::board_write_leftsec( get_url() );
    if( left ){
        SKELETON::MsgDiag mdiag( MESSAGE::get_admin()->get_win(), "書き込み規制中です ( 残り " + MISC::itostr( left ) + " 秒 )\n\nもう暫くお待ち下さい。規制秒数が短くなった場合は板のプロパティからリセットできます。" );
        mdiag.run();
        return;
    }

    if( m_post && m_post->is_loading() ){
        SKELETON::MsgDiag mdiag( MESSAGE::get_admin()->get_win(), "書き込み中です" );
        mdiag.run();
        return;
    }

    // fusianasan チェック
    if( DBTREE::default_noname( get_url() ) == "fusianasan" ) DBTREE::board_set_check_noname( get_url(), true );

    // 名無し書き込みチェック
    if( DBTREE::board_check_noname( get_url() ) ){

        std::string name = get_entry_name().get_text();
        if( name.empty() ){
            SKELETON::MsgDiag mdiag( MESSAGE::get_admin()->get_win(), "名前欄が空白です。fusianasan 書き込みになる可能性があります。" );
            mdiag.run();
            return;
        }
    }


    // 行数チェック
    if( m_max_line ){

        if( m_text_message.get_buffer()->get_line_count() > m_max_line ){
            SKELETON::MsgDiag mdiag( MESSAGE::get_admin()->get_win(), "行数が多すぎます。" );
            mdiag.run();
            return;
        }
    }

    // バイト数チェック
    if( m_max_str ){

        if( m_lng_str_enc > m_max_str ){
            SKELETON::MsgDiag mdiag( MESSAGE::get_admin()->get_win(), "文字数が多すぎます。" );
            mdiag.run();
            return;
        }
    }

    write();
}


//
// ファイル挿入
//
void MessageViewBase::slot_draft_open()
{
    std::string open_path = CACHE::open_load_diag( MESSAGE::get_admin()->get_win(), SESSION::get_dir_draft(), CACHE::FILE_TYPE_TEXT );

    if( ! open_path.empty() )
    {
        std::string draft;

        SESSION::set_dir_draft( MISC::get_dir( open_path ) );
        CACHE::load_rawdata( open_path, draft );
        if( ! draft.empty() ) set_command( "add_message", draft );
    }
}


//
// closeボタンを押した
//
void MessageViewBase::slot_close_clicked()
{
    if( !SESSION::get_close_mes() ){
        SKELETON::MsgDiag mdiag( MESSAGE::get_admin()->get_win(), "「ビューを閉じない」ボタンが押されています。" );
        mdiag.run();
    }
    else close_view();
}


//
// undoボタンを押した
//
void MessageViewBase::slot_undo_clicked()
{
    m_text_message.undo();
}


//
// ビューを閉じないボタンを押した
//
void MessageViewBase::slot_not_close_clicked()
{
    if( ! m_enable_menuslot ) return;
    if( ! get_messagetoolbar() ) return;

    SESSION::set_close_mes( ! SESSION::get_close_mes() );

    if( SESSION::get_close_mes() ) get_messagetoolbar()->get_close_button().set_sensitive( true );
    else get_messagetoolbar()->get_close_button().set_sensitive( false );
}


//
// プレビューボタンを押した
//
void MessageViewBase::slot_preview_clicked()
{
    if( ! m_enable_menuslot ) return;

#ifdef _DEBUG
    std::cout << "MessageViewBase::slot_preview_clicked page = " << m_notebook.get_current_page() << std::endl;
#endif

    if( m_notebook.get_current_page() == PAGE_MESSAGE ) tab_right();
    else tab_left();
}


//
// テキストビューでキーを押した
//
bool MessageViewBase::slot_key_press( GdkEventKey* event )
{
#ifdef _DEBUG_KEY
    guint key = event->keyval;
    bool ctrl = ( event->state ) & GDK_CONTROL_MASK;
    bool shift = ( event->state ) & GDK_SHIFT_MASK;
    bool alt = ( event->state ) & GDK_MOD1_MASK;

    std::cout << "MessageViewBase::slot_key_press"
              << " key = " << key
              << " ctrl = " << ctrl
              << " shift = " << shift
              << " alt = " << alt << std::endl;
#endif

    operate_view( SKELETON::View::get_control().key_press( event ) );

    return true;
}


//
// テキストビューでマウスボタン押した
//
bool MessageViewBase::slot_button_press( GdkEventButton* event )
{
#ifdef _DEBUG
    std::cout << "MessageViewBase::slot_button_press\n";
#endif

    MESSAGE::get_admin()->set_command( "switch_admin" );

    return true;
}



//
// フォントの更新
//
void MessageViewBase::relayout()
{
    init_font( CONFIG::get_fontname( FONT_MESSAGE ) );
    init_color();
}


//
// 閉じる
//
void MessageViewBase::close_view()
{
#ifdef _DEBUG
    std::cout << "MessageViewBase::close_view\n";
#endif

    MESSAGE::get_admin()->set_command( "close_currentview" );
}



//
// タブ左移動
//
void MessageViewBase::tab_left()
{
#ifdef _DEBUG
    std::cout << "MessageViewBase::tab_left\n";
#endif

    int page = m_notebook.get_current_page();
    if( page == PAGE_MESSAGE ) m_notebook.set_current_page( PAGE_PREVIEW );
    else m_notebook.set_current_page( PAGE_MESSAGE );

    focus_view();
}


//
// タブ右移動
//
void MessageViewBase::tab_right()
{
#ifdef _DEBUG
    std::cout << "MessageViewBase::tab_right\n";
#endif

    int page = m_notebook.get_current_page();
    if( page == PAGE_PREVIEW ) m_notebook.set_current_page( PAGE_MESSAGE );
    else m_notebook.set_current_page( PAGE_PREVIEW );

    m_preview->focus_view();
}



//
// 書き込みボタンをフォーカス
//
void MessageViewBase::focus_writebutton()
{
    if( ! get_messagetoolbar() ) return;
    get_messagetoolbar()->m_button_write.grab_focus();
}


//
// 書き込み
//
void MessageViewBase::post_msg( const std::string& msg, bool new_article )
{
    if( m_post ) delete m_post;
    m_post = new Post( this, get_url(),  msg, new_article );
    m_post->sig_fin().connect( sigc::mem_fun( *this, &MessageViewBase::post_fin ) );
    m_post->post_msg();
}



//
// 書き込みが終わったら呼ばれる
//
void MessageViewBase::post_fin()
{
    int code = m_post->get_code();
    std::string location = m_post->location();

#ifdef _DEBUG
    std::cout << "MessageViewBase::post_fin" << std::endl
              << "code = " << code << std::endl
              << "location = " << location << std::endl;
#endif

    // 成功
    if( code == HTTP_OK
        || ( code == HTTP_REDIRECT && ! location.empty() ) // (まちBBSなどで)リダイレクトした場合
        ){
        save_postlog();
        m_text_message.set_text( std::string() );

        close_view();

        reload();
    }

    // タイムアウト
    else if( code == HTTP_TIMEOUT ){

        SKELETON::MsgDiag mdiag( MESSAGE::get_admin()->get_win(),
                                 "タイムアウトしました\n\n書き込み自体は成功している可能性があります。\nメッセージのバックアップをとってからスレを再読み込みして下さい。" );
        mdiag.run();
    }

    // 失敗
    else if( code != HTTP_CANCEL ){

        SKELETON::MsgDiag mdiag( MESSAGE::get_admin()->get_win(),
                                 "書き込みに失敗しました\n\n" + m_post->errmsg(), false, Gtk::MESSAGE_ERROR  );
        mdiag.run();
    }
}


//
// タブのページが切り替わったら呼ばれるslot
//
void MessageViewBase::slot_switch_page( GtkNotebookPage*, guint page )
{
#ifdef _DEBUG
    std::cout << "MessageViewBase::slot_switch_page : " << get_url() << " page = " << page << std::endl;
#endif

    // toggle　アクションを activeにするとスロット関数が呼ばれるので処理しないようにする
    m_enable_menuslot = false;

    // プレビュー表示
    if( get_messagetoolbar() && m_preview && page == PAGE_PREVIEW ){

        // 各ボタンの状態更新
        get_messagetoolbar()->m_button_undo.set_sensitive( false );
        get_messagetoolbar()->m_button_open.set_sensitive( false );
        get_messagetoolbar()->m_button_preview.set_active( true );

        // URLを除外してエスケープ
        std::string msg = MISC::html_escape( m_text_message.get_text(), false );
        msg = MISC::replace_str( msg, "\n", " <br> " );
        
        std::stringstream ss;

        // 名前 + トリップ
        if( ! m_entry_name.get_text().empty() ){

            std::string name = MISC::html_escape( m_entry_name.get_text() );

            std::string trip;

	    std::string::size_type i = name.find( "#" );
            if( i != std::string::npos ){

                trip = MISC::get_trip( name.substr( i+1 ), DBTREE::board_charset( get_url() ) );
                name = name.substr( 0, i );
            }

            ss << name;
            if( ! trip.empty() ) ss << " ◆" << trip;
        }
        else ss << DBTREE::default_noname( get_url() );

        std::string mail = MISC::html_escape( m_entry_mail.get_text() );

        ss << "<>" << mail  << "<>";

        struct timeval tv;
        struct timezone tz;
        gettimeofday( &tv, &tz );
        ss << MISC::timettostr( tv.tv_sec );

        ss << " ID:???" << "<>" << msg << "<>\n";

#ifdef _DEBUG
        std::cout << ss.str() << std::endl;
#endif

        m_preview->set_command( "clear_screen" );
        m_preview->set_command( "append_dat", ss.str() );
    }

    // メッセージビュー
    else if( get_messagetoolbar() && page == PAGE_MESSAGE ){

        // 各ボタンの状態更新
        get_messagetoolbar()->m_button_undo.set_sensitive( true );
        get_messagetoolbar()->m_button_open.set_sensitive( true );
        get_messagetoolbar()->m_button_preview.set_active( false );
    }

    if( m_enable_focus ){
        MESSAGE::get_admin()->set_command( "switch_admin" );
        MESSAGE::get_admin()->set_command( "focus_current_view" );
    }

    m_enable_menuslot = true;
}



//
// ステータス表示
//
void MessageViewBase::show_status()
{
    std::stringstream ss;
    int byte_out;

    if( ( int ) m_text_message.get_text().size() > m_lng_iconv ) m_lng_str_enc = m_max_str;
    else{
        strcpy( m_str_iconv,  m_text_message.get_text().substr( 0, m_lng_iconv - 64 ).c_str() );
        std::string str_enc = m_iconv->convert( m_str_iconv, strlen( m_str_iconv ), byte_out );
        m_lng_str_enc = str_enc.length();
    }

    ss << " [ 行数 " << m_text_message.get_buffer()->get_line_count();
    if( m_max_line ) ss << "/ " << m_max_line;

    ss << "   /  文字数 " << m_lng_str_enc;
    if( m_max_str ) ss << "/ " << m_max_str;

    if( DBTREE::article_write_time( get_url() ) ) ss << "  /  最終書込 " << DBTREE::article_write_date( get_url() );

    ss << " ]";

    set_status( ss.str() );
    MESSAGE::get_admin()->set_command( "set_status", get_url(), get_status() + m_str_pass );
}



// 書き込みログ保存
void MessageViewBase::save_postlog()
{
    if( ! CONFIG::get_save_postlog() ) return;

    std::string subject;
    if( get_entry_subject() ) subject = get_entry_subject()->get_text();
    std::string msg = get_text_message().get_text();
    std::string name = get_entry_name().get_text();
    std::string mail = get_entry_mail().get_text();

    struct timeval tv;
    struct timezone tz;
    gettimeofday( &tv, &tz );
    std::string date = MISC::timettostr( tv.tv_sec );

    std::stringstream ss;
    ss << "---------------" << std::endl
       << get_url() << std::endl
       << "[ " << DBTREE::board_name( get_url() ) << " ] " << subject << std::endl
       << "名前：" << name << " [" << mail << "]：" << date << std::endl
       << msg << std::endl;

#ifdef _DEBUG
    std::cout << ss.str() << std::endl;
#endif 


    CACHE::save_rawdata( CACHE::path_postlog(), ss.str(), true );
}
