// ライセンス: GPL2

//#define _DEBUG
//#define _DEBUG_KEY
#include "jddebug.h"

#include "messageadmin.h"
#include "messageviewbase.h"
#include "post.h"

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
#include "cache.h"

#include <sstream>
#include <sys/time.h>


using namespace MESSAGE;

#define MAX_STR_ICONV 128*1024


enum{
    PAGE_MESSAGE = 0,
    PAGE_PREVIEW
};


MessageViewBase::MessageViewBase( const std::string& url )
    : SKELETON::View( url ),
      m_post( 0 ),
      m_preview( 0 ),
      m_button_write( ICON::WRITE ),
      m_button_cancel( Gtk::Stock::CLOSE ),
      m_button_undo( Gtk::Stock::UNDO ),
      m_entry_subject( false, " [ " + DBTREE::board_name( url ) + " ]  ", "" )
{
#ifdef _DEBUG
    std::cout << "MessageViewBase::MessageViewBase " << get_url() << std::endl;
#endif

    // コントロールモード設定
    get_control().set_mode( CONTROL::MODE_MESSAGE );

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


void MessageViewBase::clock_in()
{
    if( m_preview ) m_preview->clock_in();
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
// コマンド
//
bool MessageViewBase::set_command( const std::string& command, const std::string& arg )
{
    if( command == "empty" ) return get_message().empty();

    if( command == "loading" ){
        if( !m_post ) return false;
        return m_post->is_loading();
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
    m_entry_subject.set_text( DBTREE::article_subject( get_url() ) );

    m_button_write.signal_clicked().connect( sigc::mem_fun( *this, &MessageViewBase::slot_write_clicked ) );
    m_button_cancel.signal_clicked().connect( sigc::mem_fun( *this, &MessageViewBase::slot_cancel_clicked ) );
    m_button_undo.signal_clicked().connect( sigc::mem_fun( *this, &MessageViewBase::slot_undo_clicked ) );
    
    m_tooltip.set_tip( m_button_write, CONTROL::get_label_motion( CONTROL::ExecWrite ) );
    m_tooltip.set_tip( m_button_cancel, CONTROL::get_label_motion( CONTROL::CancelWrite ) );
    m_tooltip.set_tip( m_button_undo, CONTROL::get_label_motion( CONTROL::UndoEdit ) );

    m_toolbar.pack_start( m_button_write, Gtk::PACK_SHRINK );
    m_toolbar.pack_start( m_entry_subject, Gtk::PACK_EXPAND_WIDGET, 2 );
    m_toolbar.pack_start( m_button_undo, Gtk::PACK_SHRINK );
    m_toolbar.pack_end( m_button_cancel, Gtk::PACK_SHRINK );

    // 書き込みビュー
    m_label_name.set_alignment( Gtk::ALIGN_LEFT, Gtk::ALIGN_CENTER );
    m_label_mail.set_alignment( Gtk::ALIGN_LEFT, Gtk::ALIGN_CENTER );
    m_label_name.set_text( " 名前 " );
    m_label_mail.set_text( " メール " );

    m_check_fixname.set_label( "固定" );
    m_check_fixmail.set_label( "固定" );

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
        m_entry_mail.set_text( DBTREE::board_get_write_mail( get_url() ) );
    }
    else m_entry_mail.set_text( "sage" );

    m_hbox_name_mail.pack_start( m_label_name, Gtk::PACK_SHRINK );
    m_hbox_name_mail.pack_start( m_check_fixname, Gtk::PACK_SHRINK );
    m_hbox_name_mail.pack_start( m_entry_name );
    m_hbox_name_mail.pack_start( m_label_mail, Gtk::PACK_SHRINK );
    m_hbox_name_mail.pack_start( m_check_fixmail, Gtk::PACK_SHRINK );
    m_hbox_name_mail.pack_start( m_entry_mail );

#if GTKMMVER >= 260
    m_statbar.pack_start( m_label_stat, Gtk::PACK_SHRINK );
#endif

    m_msgview.pack_start( m_hbox_name_mail, Gtk::PACK_SHRINK );    
    m_msgview.pack_start( m_text_message );

    m_text_message.set_accepts_tab( false );
    m_text_message.sig_key_release().connect( sigc::mem_fun(*this, &MessageViewBase::slot_key_release ) );    
    m_text_message.get_buffer()->signal_changed().connect( sigc::mem_fun(*this, &MessageViewBase::show_status ) );

    // プレビュー
    m_preview = CORE::ViewFactory( CORE::VIEW_ARTICLEPREVIEW, get_url() );

    m_notebook.append_page( m_msgview, "メッセージ" );
    m_notebook.append_page( *m_preview, "プレビュー" );
    m_notebook.signal_switch_page().connect( sigc::mem_fun( *this, &MessageViewBase::slot_switch_page ) );

    pack_start( m_toolbar, Gtk::PACK_SHRINK );
    pack_start( m_notebook );
    pack_start( m_statbar, Gtk::PACK_SHRINK );

    // フォントセット
    init_font( CONFIG::get_fontname_message() );

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
    m_text_message.focus_view();
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
            slot_cancel_clicked();
            break;

            // 書き込み実行
        case CONTROL::ExecWrite:
            slot_write_clicked();
            break;

        case CONTROL::TabLeft:
            tab_left();
            break;

        case CONTROL::TabRight:
            tab_right();
            break;
    }
}



//
// 書き込むボタン押した
//
void MessageViewBase::slot_write_clicked()
{
/* 書き込み確認(やってみたらうざかったので様子見)

    Gtk::Menu* popupmenu = dynamic_cast< Gtk::Menu* >( m_ui_manager->get_widget( "/popup_menu_write" ) );
    if( popupmenu ) popupmenu->popup( 0, gtk_get_current_event_time() );      
*/

    if( m_post && m_post->is_loading() ){
        Gtk::MessageDialog mdiag( "書き込み中です" );
        mdiag.run();
        return;
    }

    // fusianasan チェック
    if( DBTREE::default_noname( get_url() ) == "fusianasan" ) DBTREE::board_set_check_noname( get_url(), true );

    // 名無し書き込みチェック
    if( DBTREE::board_check_noname( get_url() ) ){

        std::string name = get_entry_name().get_text();
        if( name.empty() ){
            Gtk::MessageDialog mdiag( "名前欄が空白です。fusianasan 書き込みになる可能性があります。" );
            mdiag.run();
            return;
        }
    }


    // 行数チェック
    if( m_max_line ){

        if( m_text_message.get_buffer()->get_line_count() > m_max_line ){
            Gtk::MessageDialog mdiag( "行数が多すぎます。" );
            mdiag.run();
            return;
        }
    }

    // バイト数チェック
    if( m_max_str ){

        if( m_lng_str_enc > m_max_str ){
            Gtk::MessageDialog mdiag( "文字数が多すぎます。" );
            mdiag.run();
            return;
        }
    }

    write();
}


//
// キャンセルボタンを押した
//
void MessageViewBase::slot_cancel_clicked()
{
    close_view();
}


//
// undoボタンを押した
//
void MessageViewBase::slot_undo_clicked()
{
    m_text_message.undo();
}


//
// テキストビューのキー操作
//
bool MessageViewBase::slot_key_release( GdkEventKey* event )
{
#ifdef _DEBUG_KEY
    guint key = event->keyval;
    bool ctrl = ( event->state ) & GDK_CONTROL_MASK;
    bool shift = ( event->state ) & GDK_SHIFT_MASK;
    bool alt = ( event->state ) & GDK_MOD1_MASK;

    std::cout << "MessageViewBase::slot_key_release"
              << " key = " << key
              << " ctrl = " << ctrl
              << " shift = " << shift
              << " alt = " << alt << std::endl;
#endif

    operate_view( SKELETON::View::get_control().key_press( event ) );

    return true;
}



//
// フォントの更新
//
void MessageViewBase::relayout()
{
    init_font( CONFIG::get_fontname_message() );
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
    int page = m_notebook.get_current_page();
    if( page == PAGE_MESSAGE ) return;

    m_notebook.set_current_page( PAGE_MESSAGE );
    focus_view();
}


//
// タブ右移動
//
void MessageViewBase::tab_right()
{
    int page = m_notebook.get_current_page();
    if( page == PAGE_PREVIEW ) return;

    m_notebook.set_current_page( PAGE_PREVIEW );
    m_preview->focus_view();
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

        Gtk::MessageDialog mdiag( "タイムアウトしました\n\n書き込み自体は成功している可能性があります。\nメッセージのバックアップをとってからスレを再読み込みして下さい。" );
        mdiag.run();
    }

    // 失敗
    else if( code != HTTP_CANCEL ){

        Gtk::MessageDialog mdiag( "書き込みに失敗しました\n\n" + m_post->errmsg(), false, Gtk::MESSAGE_ERROR  );
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

    // プレビュー表示
    if( m_preview && page == PAGE_PREVIEW ){

        // 編集ボタン無効化
        m_button_undo.set_sensitive( false );

        std::string msg = m_text_message.get_text();
        msg = MISC::replace_str( msg, "\"", "&quot;" );
        msg = MISC::replace_str( msg, "<", "&lt;" );
        msg = MISC::replace_str( msg, ">", "&gt;" );
        msg = MISC::replace_str( msg, "\n", " <br> " );
        
        std::stringstream ss;

        // 名前 + トリップ
        if( ! m_entry_name.get_text().empty() ){

            std::string name = m_entry_name.get_text();
            name = MISC::replace_str( name, "\"", "&quot;" );
            name = MISC::replace_str( name, "<", "&lt;" );
            name = MISC::replace_str( name, ">", "&gt;" );

            std::string trip;

            unsigned int i = name.find( "#" );
            if( i != std::string::npos ){

                trip = MISC::get_trip( name.substr( i+1 ), DBTREE::board_charset( get_url() ) );
                name = name.substr( 0, i );
            }

            ss << name;
            if( ! trip.empty() ) ss << " ◆" << trip;
        }
        else ss << DBTREE::default_noname( get_url() );

        std::string mail = m_entry_mail.get_text();
        mail = MISC::replace_str( mail, "<", "&lt;" );
        mail = MISC::replace_str( mail, ">", "&gt;" );

        ss << "<>" << mail  << "<>";

        struct timeval tv;
        struct timezone tz;
        gettimeofday( &tv, &tz );
        ss << MISC::timettostr( tv.tv_sec );

        ss << " ID:???" << "<>" << msg << "<>\n";

#ifdef _DEBUG
        std::cout << ss.str() << std::endl;
#endif

        m_preview->set_command( "append_dat", ss.str() );
    }

    // メッセージビュー
    else if( page == PAGE_MESSAGE ){

        // 編集ボタン有効化
        m_button_undo.set_sensitive( true );

        MESSAGE::get_admin()->set_command( "focus_view" );
    }
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

    ss << " ]";

#if GTKMMVER <= 240
    m_statbar.push( ss.str() );
#else
    m_label_stat.set_text( ss.str() );
#endif        
}



// 書き込みログ保存
void MessageViewBase::save_postlog()
{
    if( ! CONFIG::get_save_postlog() ) return;

    std::string subject = get_entry_subject().get_text();
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
