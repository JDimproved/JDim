// ライセンス: GPL2

//#define _DEBUG
//#define _DEBUG_KEY
#include "jddebug.h"
#include "gtkmmversion.h"

#include "messageadmin.h"
#include "messageviewbase.h"
#include "post.h"
#include "logmanager.h"

#include "skeleton/msgdiag.h"
#include "skeleton/label_entry.h"
#include "skeleton/editview.h"
#include "skeleton/detaildiag.h"

#include "jdlib/miscutil.h"
#include "jdlib/misctime.h"
#include "jdlib/misctrip.h"
#include "jdlib/jdiconv.h"
#include "jdlib/jdregex.h"

#include "dbtree/interface.h"

#include "config/globalconf.h"

#include "control/controlutil.h"
#include "control/controlid.h"

#include "httpcode.h"
#include "viewfactory.h"
#include "fontid.h"
#include "cache.h"
#include "session.h"
#include "colorid.h"
#include "global.h"
#include "compmanager.h"

#include <sstream>
#include <sys/time.h>
#include <cstring>


using namespace MESSAGE;

enum
{
    MAX_STR_ICONV = 128*1024,

    PASS_TIMEOUT = 500,
    PASS_MAXTIME = 120,
};


// ページ番号
enum
{
    PAGE_MESSAGE = 0,
    PAGE_PREVIEW
};


MessageViewBase::MessageViewBase( const std::string& url )
    : SKELETON::View( url ),
      m_post( 0 ),
      m_preview( 0 ),
      m_entry_name( CORE::COMP_NAME ),
      m_entry_mail( CORE::COMP_MAIL ),
      m_text_message( NULL ),
      m_enable_focus( true ),
      m_lng_str_enc( 0 ),
      m_counter( 0 ),
      m_text_changed( false ),
      m_over_lines( false ),
      m_over_lng( false )
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

    if( SESSION::get_close_mes() ) unlock();
    else lock();
}



MessageViewBase::~MessageViewBase()
{
#ifdef _DEBUG
    std::cout << "MessageViewBase::~MessageViewBase " << get_url() << std::endl
              << "lock = " << is_locked() << std::endl;
#endif

    // 名前、メール履歴保存
    std::string name = m_entry_name.get_text();
    std::string mail = m_entry_mail.get_text();
    CORE::get_completion_manager()->set_query( CORE::COMP_NAME, name );
    CORE::get_completion_manager()->set_query( CORE::COMP_MAIL, mail );

    if( m_preview ) delete m_preview;
    m_preview = NULL;

    if( m_post ){
        m_post->terminate_load();
        delete m_post;
    }
    m_post = NULL;

    if( m_iconv ) delete m_iconv;
    m_iconv = NULL;

    SESSION::set_close_mes( ! is_locked() );
}


SKELETON::Admin* MessageViewBase::get_admin()
{
    return MESSAGE::get_admin();
}


//
// 親ウィンドウを取得
//
Gtk::Window* MessageViewBase::get_parent_win()
{
    return MESSAGE::get_admin()->get_win();
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

    ++m_counter;
    if( m_counter % ( PASS_TIMEOUT / TIMER_TIMEOUT ) == 0 ){

        m_counter = 0;

        // 書き込みから時間があまり経っていなければステータス表示を更新
        if( time( NULL ) - DBTREE::article_write_time( get_url() ) < 60 * 60 ) show_status();

        // 書き込み規制経過時刻表示
        time_t left = DBTREE::board_write_leftsec( get_url() );
        if( left ){

            bool set_color = false;
            if( m_str_pass.empty() ) set_color = true;

            m_str_pass = "規制中 " + MISC::itostr( left ) + " 秒 ";

            std::string force;
            if( SESSION::focused_admin() == SESSION::FOCUS_MESSAGE ) force = "force";
            MESSAGE::get_admin()->set_command( "set_title", get_url(), m_str_pass + get_title(), force );
            MESSAGE::get_admin()->set_command( "set_status", get_url(), m_str_pass + get_status(), force );

            if( set_color ){
                MESSAGE::get_admin()->set_command( "redraw_toolbar" );
                MESSAGE::get_admin()->set_command( "set_status_color", get_url(), get_color(), force );
            }
        }
        else if( ! m_str_pass.empty() ){

            m_str_pass = std::string();
            std::string force;
            if( SESSION::focused_admin() == SESSION::FOCUS_MESSAGE ) force = "force";
            MESSAGE::get_admin()->set_command( "set_title", get_url(), get_title(), force );
            MESSAGE::get_admin()->set_command( "set_status", get_url(), get_status(), force );

            MESSAGE::get_admin()->set_command( "redraw_toolbar" );
            MESSAGE::get_admin()->set_command( "set_status_color", get_url(), get_color(), force );
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

    if( m_text_message ) m_text_message->modify_font( pfd );
}


//
// 色初期化
//
void MessageViewBase::init_color()
{
    if( m_text_message ){

        m_text_message->modify_text( Gtk::STATE_NORMAL, Gdk::Color( CONFIG::get_color( COLOR_CHAR_MESSAGE ) ) );
        m_text_message->modify_text( Gtk::STATE_SELECTED, Gdk::Color( CONFIG::get_color( COLOR_CHAR_MESSAGE_SELECTION ) ) );
        m_text_message->modify_base( Gtk::STATE_NORMAL, Gdk::Color( CONFIG::get_color( COLOR_BACK_MESSAGE ) ) );
        m_text_message->modify_base( Gtk::STATE_SELECTED, Gdk::Color( CONFIG::get_color( COLOR_BACK_MESSAGE_SELECTION ) ) );
    }
}


void MessageViewBase::set_message( const std::string& msg )
{
    if( m_text_message ) m_text_message->set_text( msg );
}


const Glib::ustring MessageViewBase::get_message()
{
    if( m_text_message ) return m_text_message->get_text();

    return Glib::ustring();
}


//
// ロード中
//
// virtual
bool MessageViewBase::is_loading() const
{
    if( ! m_post ) return false;

    return m_post->is_loading();
}


//
// コマンド
//
bool MessageViewBase::set_command( const std::string& command, const std::string& arg1, const std::string& arg2 )
{
    if( command == "empty" ) return get_message().empty();

    else if( command == "toggle_preview" ) toggle_preview();
    else if( command == "undo_text" && m_text_message ) m_text_message->undo();
    else if( command == "insert_draft" ) insert_draft();

    else if( command == "tab_left" ) tab_left();
    else if( command == "tab_right" ) tab_right();

    else if( command == "hide_popup" && m_preview ) m_preview->set_command( "hide_popup" );

    // メッセージをクリア
    else if( command == "clear_message" ){

        if( m_text_message ){
            m_text_message->set_text( std::string() );
            m_text_message->clear_undo();
        }

        if( m_notebook.get_current_page() != PAGE_MESSAGE ){
            m_enable_focus = false;
            m_notebook.set_current_page( PAGE_MESSAGE );
            m_enable_focus = true;
        }
    }

    // メッセージを追加
    else if( command == "add_message" )
    {
        if( ! arg1.empty() && m_text_message ) m_text_message->insert_str( arg1, true );
    }

    // メッセージ保存
    else if( command == "save_message" )
    {
        if( ! get_message().empty() ){

            std::string filename = "draft-" + MISC::get_filename( get_url() );
            if( filename.find( ".dat" ) != std::string::npos ) filename = MISC::replace_str( filename, ".dat", ".txt" );
            else filename += ".txt";
            std::string save_to = CACHE::open_save_diag( MESSAGE::get_admin()->get_win(), SESSION::get_dir_draft(), filename, CACHE::FILE_TYPE_TEXT );
            if( ! save_to.empty() ){

                SESSION::set_dir_draft( MISC::get_dir( save_to ) );

                if( CACHE::save_rawdata( save_to, get_message() ) != get_message().raw().length() ){
                    SKELETON::MsgDiag mdiag( get_parent_win(),
                                             "保存に失敗しました。\nハードディスクの容量やパーミッションなどを確認してください。" );
                    mdiag.run();
                }
                else return true;
            }
        }
    }

    // ビューの wrap 切り替え
    else if( command == "toggle_wrap" ) set_wrap();

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
// 名前欄に名前をセット
//
void MessageViewBase::set_name()
{
    // スレ別の名前
    if( DBTREE::write_fixname( get_url() ) ){
        m_check_fixname.set_active();
        m_entry_name.set_text( DBTREE::write_name( get_url() ) );
    }
    // スレ別の名前が設定されていなかったら板別の名前
    else if( ! DBTREE::board_get_write_name( get_url() ).empty() ){

        std::string tmpname = DBTREE::board_get_write_name( get_url() );

        // 空白セット
        if( tmpname == JD_NAME_BLANK ) m_entry_name.set_text( std::string() );
        else m_entry_name.set_text( tmpname );
    }
    // デフォルトをセット
    else m_entry_name.set_text( CONFIG::get_write_name() );
}


//
// メール欄にアドレスをセット
//
void MessageViewBase::set_mail()
{
    // スレ別のメール
    if( DBTREE::write_fixmail( get_url() ) ){
        m_check_fixmail.set_active();
        m_entry_mail.set_text( DBTREE::write_mail( get_url() ) );
    }
    // スレ別の名前が設定されていなかったら板別のメール
    else if( ! DBTREE::board_get_write_mail( get_url() ).empty() ){

        std::string tmpmail = DBTREE::board_get_write_mail( get_url() );

        // 空白セット
        if( tmpmail == JD_MAIL_BLANK ) m_entry_mail.set_text( std::string() );
        else m_entry_mail.set_text( tmpmail );
    }
    // デフォルトをセット
    else m_entry_mail.set_text( CONFIG::get_write_mail() );
}


//
// ツールバーなどのパック
//
void MessageViewBase::pack_widget()
{
    // 書き込みビュー
    m_label_name.set_alignment( Gtk::ALIGN_START, Gtk::ALIGN_CENTER );
    m_label_mail.set_alignment( Gtk::ALIGN_START, Gtk::ALIGN_CENTER );
    m_label_name.set_text( " 名前 " );
    m_label_mail.set_text( " メール " );

    m_check_fixname.set_label( "固定" );
    m_check_fixmail.set_label( "固定" );

    constexpr const char* name_tip = "チェックすると名前欄を保存して固定にする";
    constexpr const char* mail_tip = "チェックするとメール欄を保存して固定にする";
#if GTKMM_CHECK_VERSION(2,12,0)
    m_check_fixname.set_tooltip_text( name_tip );
    m_check_fixmail.set_tooltip_text( mail_tip );
#else
    m_tooltip.set_tip( m_check_fixname, name_tip );
    m_tooltip.set_tip( m_check_fixmail, mail_tip );
#endif

    set_name();
    set_mail();

    m_tool_name.add( m_label_name );
    m_tool_mail.add( m_label_mail );
    m_tool_fixname.add( m_check_fixname );
    m_tool_fixmail.add( m_check_fixmail );
    m_tool_entry_name.add( m_entry_name );
    m_tool_entry_mail.add( m_entry_mail );
    m_tool_entry_name.set_expand( true );
    m_tool_entry_mail.set_expand( true );

#if GTKMM_CHECK_VERSION(2,12,0)
    m_toolbar_name_mail.set_icon_size( Gtk::ICON_SIZE_MENU );
#endif
    m_toolbar_name_mail.set_toolbar_style( Gtk::TOOLBAR_ICONS );
    m_toolbar_name_mail.append( m_tool_name );
    m_toolbar_name_mail.append( m_tool_fixname );
    m_toolbar_name_mail.append( m_tool_entry_name );
    m_toolbar_name_mail.append( m_tool_mail );
    m_toolbar_name_mail.append( m_tool_fixmail );
    m_toolbar_name_mail.append( m_tool_entry_mail );

    m_msgview.pack_start( m_toolbar_name_mail, Gtk::PACK_SHRINK );

    if( ! m_text_message ){

        // 日本語のON/OFF状態を保存
        // Admin から EditView のインスタンスをもらう
        if( CONFIG::get_keep_im_status() ){

            m_text_message = MESSAGE::get_admin()->get_text_message();
            m_text_message->set_text( std::string() );
            m_text_message->clear_undo();
        }
        else m_text_message = Gtk::manage( new SKELETON::EditView() );
    }

    set_wrap();

    if( m_text_message->get_parent() ) m_text_message->reparent( m_msgview );
    else m_msgview.pack_start( *m_text_message );

    m_text_message->set_accepts_tab( false );
    m_text_message->sig_key_press().connect( sigc::mem_fun(*this, &MessageViewBase::slot_key_press ) );
    m_text_message->sig_button_press().connect( sigc::mem_fun(*this, &MessageViewBase::slot_button_press ) );
    m_text_message->get_buffer()->signal_changed().connect( sigc::mem_fun(*this, &MessageViewBase::slot_text_changed ) );

    // プレビュー
    m_preview = CORE::ViewFactory( CORE::VIEW_ARTICLEPREVIEW, get_url() );

    m_notebook.set_show_tabs( false );
    m_notebook.set_show_border( false );
    m_notebook.append_page( m_msgview, "メッセージ" );
    m_notebook.append_page( *m_preview, "プレビュー" );
    m_notebook.signal_switch_page().connect( sigc::mem_fun( *this, &MessageViewBase::slot_switch_page ) );
    m_notebook.set_current_page( PAGE_MESSAGE );

    pack_start( m_notebook );
    set_size_request( 1, 1 );

    // フォントセット
    init_font( CONFIG::get_fontname( FONT_MESSAGE ) );

    // 色セット
    init_color();

    show_status();
}


//
// テキストの折り返し
//
void MessageViewBase::set_wrap()
{
    if( ! m_text_message ) return;

    if( CONFIG::get_message_wrap() ) m_text_message->set_wrap_mode( Gtk::WRAP_CHAR );
    else m_text_message->set_wrap_mode( Gtk::WRAP_NONE );
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
//    std::cout << "MessageViewBase::focus_view page = " << m_notebook.get_current_page() << std::endl;
#endif

    if( m_notebook.get_current_page() == PAGE_MESSAGE && m_text_message ) m_text_message->focus_view();
    else if( m_preview && m_notebook.get_current_page() == PAGE_PREVIEW ) m_preview->focus_view();
}


//
// viewの操作
//
bool MessageViewBase::operate_view( const int control )
{
    if( control == CONTROL::None ) return false;

    switch( control ){

            // 書き込まずに閉じる
        case CONTROL::CancelWrite:
            close_view();
            break;

            // 書き込み実行
        case CONTROL::ExecWrite:
            MESSAGE::get_admin()->set_command( "toolbar_write", get_url() );
            break;

       case CONTROL::TabLeft:
        case CONTROL::TabLeftUpdated:
            MESSAGE::get_admin()->set_command( "tab_left" );
            break;

        case CONTROL::TabRight:
        case CONTROL::TabRightUpdated:
            MESSAGE::get_admin()->set_command( "tab_right" );
            break;

        case CONTROL::ToggleSage:
            if( m_entry_mail.get_text() == "sage" ) m_entry_mail.set_text( "" );
            else set_mail();
            break;

            // 書き込みボタンにフォーカスを移す
        case CONTROL::FocusWrite:
            MESSAGE::get_admin()->set_command( "focus_button_write" );
            break;

        default:
            return false;
    }

    return true;
}



//
// 書き込み実行
//
void MessageViewBase::write()
{
#ifdef _DEBUG
    std::cout << "MessageViewBase::write\n";
#endif

    time_t left = DBTREE::board_write_leftsec( get_url() );
    if( left
        && ! SESSION::loginbe() // BEログイン中はダイアログを表示しない
        ){
        SKELETON::MsgDiag mdiag( get_parent_win(), "書き込み規制中です ( 残り " + MISC::itostr( left ) + " 秒 )\n\nもう暫くお待ち下さい。規制秒数が短くなった場合は板のプロパティからリセットできます。" );
        mdiag.run();
        return;
    }

    if( m_post && m_post->is_loading() ){
        m_post->show_writingdiag( true );
        return;
    }

    // fusianasan チェック
    if( DBTREE::default_noname( get_url() ) == "fusianasan" ) DBTREE::board_set_check_noname( get_url(), true );

    // 名無し書き込みチェック
    if( DBTREE::board_check_noname( get_url() ) ){

        std::string name = get_entry_name().get_text();
        if( name.empty() ){
            SKELETON::MsgDiag mdiag( get_parent_win(), "名前欄が空白です。fusianasan 書き込みになる可能性があります。" );
            mdiag.run();
            return;
        }
    }


    // 行数チェック
    if( m_max_line ){

        if( m_text_message && m_text_message->get_buffer()->get_line_count() > m_max_line ){
            SKELETON::MsgDiag mdiag( get_parent_win(), "行数が多すぎます。" );
            mdiag.run();
            return;
        }
    }

    // バイト数チェック
    if( m_max_str ){

        if( m_lng_str_enc > m_max_str ){
            SKELETON::MsgDiag mdiag( get_parent_win(), "文字数が多すぎます。" );
            mdiag.run();
            return;
        }
    }

    const std::string msg = create_message();
    if( msg.empty() ) return;

    // 数値文字参照(&#????;)書き込み可能か
    if( DBTREE::get_unicode( get_url() ) == "change" ){

        JDLIB::Regex regex;
        const size_t offset = 0;
        const bool icase = false;
        const bool newline = true;
        const bool usemigemo = false;
        const bool wchar = false;
        if( regex.exec( "%26%23[0-9]*%3b", msg, offset, icase, newline, usemigemo, wchar ) ){

            SKELETON::MsgDiag mdiag( get_parent_win(),
                                     "ユニコード文字が含まれていますが、この板ではユニコード文字は文字化けします(BBS_UNICODE=change)。\n\n書き込みますか？",
                                     false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_YES_NO );
            if( mdiag.run() != Gtk::RESPONSE_YES ) return;
        }
    }

    write_impl( msg );
}


//
// 下書きファイル挿入
//
void MessageViewBase::insert_draft()
{
    const auto list_files = CACHE::open_load_diag( MESSAGE::get_admin()->get_win(),
                                                   SESSION::get_dir_draft(), CACHE::FILE_TYPE_TEXT, false );

    if( list_files.size() )
    {
        const std::string& open_path = list_files.front();
        std::string draft;

        SESSION::set_dir_draft( MISC::get_dir( open_path ) );
        CACHE::load_rawdata( open_path, draft );
        if( ! draft.empty() ) set_command( "add_message", draft );
    }
}


//
// プレビュー切り替え
//
void MessageViewBase::toggle_preview()
{
#ifdef _DEBUG
    std::cout << "MessageViewBase::toggle_preview page = " << m_notebook.get_current_page() << std::endl;
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

    return operate_view( SKELETON::View::get_control().key_press( event ) );
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
// 書き込み
//
void MessageViewBase::post_msg( const std::string& msg, bool new_article )
{
    push_logitem();

    if( m_post ){
        m_post->terminate_load();
        delete m_post;
    }
    m_post = new Post( this, get_url(),  msg, new_article );
    m_post->sig_fin().connect( sigc::mem_fun( *this, &MessageViewBase::post_fin ) );
    m_post->post_msg();

    // 名前、メール履歴保存
    std::string name = m_entry_name.get_text();
    std::string mail = m_entry_mail.get_text();
    CORE::get_completion_manager()->set_query( CORE::COMP_NAME, name );
    CORE::get_completion_manager()->set_query( CORE::COMP_MAIL, mail );
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
        || ( ( code == HTTP_MOVED_PERM || code == HTTP_REDIRECT ) && ! location.empty() ) // (まちBBSなどで)リダイレクトした場合
        ){
        save_postlog();

        if( m_text_message ){
            m_text_message->set_text( std::string() );
            m_text_message->clear_undo();
        }

        close_view();

        if( ! SESSION::is_live( get_url() ) ) reload();
    }

    // タイムアウト
    else if( code == HTTP_TIMEOUT ){

        SKELETON::MsgDiag mdiag( get_parent_win(),
                                 "タイムアウトしました\n\n書き込み自体は成功している可能性があります。\nメッセージのバックアップをとってからスレを再読み込みして下さい。" );
        mdiag.run();
    }

    // 失敗
    else if( code != HTTP_CANCEL ){

        SKELETON::DetailDiag ddiag( get_parent_win(), get_url(),
                          false,
                          "書き込みに失敗しました\n\n" + m_post->get_errmsg(), "概要",
                          m_post->get_return_html(), "詳細" );

        ddiag.set_title( "書き込みエラー" );
        ddiag.run();
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

    // プレビュー表示
    if( m_preview && page == PAGE_PREVIEW ){

        // ツールバー切り替え
        MESSAGE::get_admin()->set_command( "switch_toolbar_preview" );

        std::string new_subject = MESSAGE::get_admin()->get_new_subject();
        if( ! new_subject.empty() ) set_label( new_subject );

        // URLを除外してエスケープ
        const bool include_url = false;
        std::string msg;
        if( m_text_message ) msg = MISC::html_escape( m_text_message->get_text(), include_url );
        msg = MISC::replace_str( msg, "\n", " <br> " );

        std::stringstream ss;

        // 名前 + トリップ
        if( ! m_entry_name.get_text().empty() ){

            const std::string name_field = m_entry_name.get_text();

            const size_t trip_pos = name_field.find( "#", 0 );

            const std::string name = MISC::html_escape( name_field.substr( 0, trip_pos ) );

            std::string trip;
            if( trip_pos != std::string::npos )
            {
                trip = MISC::get_trip( name_field.substr( trip_pos + 1 ), DBTREE::board_charset( get_url() ) );
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
        ss << MISC::timettostr( tv.tv_sec, MISC::TIME_WEEK );

        ss << " ID:???" << "<>" << msg << "<>\n";

#ifdef _DEBUG
        std::cout << ss.str() << std::endl;
#endif

        m_preview->set_command( "clear_screen" );
        m_preview->set_command( "append_dat", ss.str() );
    }

    // メッセージビュー
    else if( page == PAGE_MESSAGE ){

        // ツールバー切り替え
        MESSAGE::get_admin()->set_command( "switch_toolbar_message" );
    }

    if( m_enable_focus ){
        MESSAGE::get_admin()->set_command( "switch_admin" );
        MESSAGE::get_admin()->set_command( "focus_current_view" );
    }
}


//
// 書き込み欄のテキストが更新された
//
void MessageViewBase::slot_text_changed()
{
    m_text_changed = true;
    show_status();
    m_text_changed = false;
}


//
// ステータス表示
//
void MessageViewBase::show_status()
{
    if( ! m_text_message ) return;

    const bool broken = is_broken();

    std::stringstream ss;

    int line_count = 0;
    if( m_text_message ){
        line_count = m_text_message->get_buffer()->get_line_count();
        ss << " [ 行数 " << line_count;
    }
    if( m_max_line ){
        ss << "/ " << m_max_line;
        if( m_max_line < line_count ) m_over_lines = true;
        else m_over_lines = false;
    }

    const std::string message = m_text_message->get_text();

    ss << "   /  文字数 ";

    if( ( int ) message.size() > m_lng_iconv )
    {
        ss << "過多";
    }
    else if( m_text_changed )
    {
        int byte_out;
        const char* msgc = message.c_str();
        std::string str_enc = m_iconv->convert( (char*)msgc, strlen( msgc ), byte_out );
        m_lng_str_enc = str_enc.length();

        // 特殊文字の文字数を計算
        m_lng_str_enc += MISC::count_chr( str_enc, '\n' ) * 5; // " <br> " = 6バイト
        m_lng_str_enc += MISC::count_chr( str_enc, '"' ) * 5; // &quot; = 6バイト
        m_lng_str_enc += MISC::count_chr( str_enc, '<' ) * 3; // &lt; = 4バイト
        m_lng_str_enc += MISC::count_chr( str_enc, '>' ) * 3; // &gt; = 4バイト

        ss << m_lng_str_enc;
    }
    else ss << m_lng_str_enc;

    if( m_max_str ){
        ss << "/ " << m_max_str;
        if( m_max_str < m_lng_str_enc ) m_over_lng = true;
        else m_over_lng = false;
    }

    if( DBTREE::get_unicode( get_url() ) == "pass" ) ss << " / unicode ○";
    else if( DBTREE::get_unicode( get_url() ) == "change" ) ss << " / unicode ×";

    const time_t wtime = DBTREE::article_write_time( get_url() );
    if( wtime ) ss << "  /  最終書込 "
                   << ( MISC::timettostr( wtime, MISC::TIME_WEEK ) + " ( " +  MISC::timettostr( wtime, MISC::TIME_PASSED ) +  " )" );

    ss << " ]";

    set_status( ss.str() );
    MESSAGE::get_admin()->set_command( "set_status", get_url(), m_str_pass + get_status() );

    if( broken != is_broken() ){
        MESSAGE::get_admin()->set_command( "redraw_toolbar" );
        MESSAGE::get_admin()->set_command( "set_status_color", get_url(), get_color() );
    }
}


//
// 自分の書き込みの判定用データの保存
//
// 実況中など post_fin() がコールされる前に自分のレスが表示されてしまう時があるので
//  m_post->post_msg() する前に情報を保存しておく
//
void MessageViewBase::push_logitem()
{
    if( ! m_text_message ) return;

    const bool newthread = ! ( MESSAGE::get_admin()->get_new_subject().empty() );
    const std::string msg = m_text_message->get_text();

    MESSAGE::get_log_manager()->push_logitem( get_url(), newthread, msg );
}


//
// 書き込みログ保存
//
void MessageViewBase::save_postlog()
{
    if( ! m_text_message ) return;

    std::string subject = MESSAGE::get_admin()->get_new_subject();
    if( subject.empty() ) subject = DBTREE::article_subject( get_url() );
    const std::string msg = m_text_message->get_text();
    const std::string name = get_entry_name().get_text();
    const std::string mail = get_entry_mail().get_text();

    MESSAGE::get_log_manager()->save( get_url(), subject, msg, name, mail );
}
