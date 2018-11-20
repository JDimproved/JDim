// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "articleadmin.h"
#include "articleview.h"
#include "drawareamain.h"

#include "skeleton/msgdiag.h"

#include "message/messageadmin.h"

#include "dbtree/interface.h"
#include "dbtree/articlebase.h"

#include "jdlib/miscutil.h"
#include "jdlib/misctime.h"

#include "config/globalconf.h"

#include "sound/soundmanager.h"

#include "control/controlid.h"

#include "history/historymanager.h"

#include "command.h"
#include "global.h"
#include "httpcode.h"
#include "session.h"

#include <sstream>


using namespace ARTICLE;


enum
{
    CANCEL_RELOAD = 800,  // msec  連続リロード防止用カウンタ
    LIVE_SEC_PLUS = 5, // 実況で更新失敗/成功ごとに増減する更新間隔(秒)
    LIVE_MAX_RELOAD = 5  // 実況でこの回数連続でリロードに失敗したら実況停止
};


// メインビュー

ArticleViewMain::ArticleViewMain( const std::string& url )
    : ArticleViewBase( url, url ),
      m_gotonum_reserve_to( 0 ),
      m_gotonum_reserve_from( 0 ),
      m_gotonum_seen( 0 ),
      m_playsound( false ),
      m_reload_reserve( false ),
      m_cancel_reload_counter( 0 )
{
#ifdef _DEBUG
    std::cout << "ArticleViewMain::ArticleViewMain " << get_url() << " url_article = " << url_article() << std::endl;
#endif

    // オートリロード可
    set_enable_autoreload( true );

    // 実況可能
    set_enable_live( true );

    // 実況したままスレを閉じたらJD終了後にスレを削除する
    // キャンセルするにはもう一度スレを開いて実況しないで閉じる
    // ArticleViewBase::set_command()も参照
    SESSION::remove_delete_list( url_article() );

    setup_view();
}



ArticleViewMain::~ArticleViewMain()
{
#ifdef _DEBUG    
    std::cout << "ArticleViewMain::~ArticleViewMain : " << get_url() << " url_article = " << url_article() << std::endl;
#endif

    save_session();

    // 閉じたタブ履歴更新
    HISTORY::append_history( URL_HISTCLOSEVIEW,
                             DBTREE::url_dat( get_url() ),
                             DBTREE::article_subject( get_url() ), TYPE_THREAD );

    CORE::core_set_command( "close_message" ,url_article() );

    if( get_live() ) live_stop();
}


void ArticleViewMain::save_session()
{
    const int seen = drawarea()->get_seen_current();
        
#ifdef _DEBUG    
    std::cout << "set seen to " << seen << std::endl;
#endif

    if( seen >= 1 ) get_article()->set_number_seen( seen );
}


//
// クロック入力
//
// virtual
void ArticleViewMain::clock_in()
{
    ArticleViewBase::clock_in();

    // 実況モードでリロード
    if( get_live() && ! is_loading() && inc_autoreload_counter() ) exec_reload();

    // 更新チェック中にshow_view()が呼び出された場合は
    // チェックが終わってから改めてロードする
    if( m_reload_reserve ){

        if( ! get_article()->is_checking_update() ){

            m_reload_reserve = false;
            show_view();
        }
    }
}


//
// クロック入力
// clock_in_always()は常に呼び出されるので重い処理を含めてはいけない
//
// virtual
void ArticleViewMain::clock_in_always()
{
    ArticleViewBase::clock_in_always();

    if( m_cancel_reload_counter ) --m_cancel_reload_counter;
}


//
// num 番にジャンプ
//
// ローディング中ならジャンプ予約をしてロード後に update_finish() の中で改めて goto_num() を呼び出す
//
void ArticleViewMain::goto_num( const int num_to, const int num_from )
{
#ifdef _DEBUG
    std::cout << "ArticleViewMain::goto_num num = " << num_to << " num_from " << num_from
              << " gotonum_seen = " << m_gotonum_seen << " gotonum_reserve = " << m_gotonum_reserve_to << std::endl;
#endif

    m_gotonum_seen = 0; // m_gotonum_reserve_to を優先させる
    m_gotonum_reserve_to = num_to;
    m_gotonum_reserve_from = num_from;

    if( get_article()->get_number_load() < num_to  && is_loading() ){
#ifdef _DEBUG
        std::cout << "reserve\n";
#endif
        return;
    }

#ifdef _DEBUG
    std::cout << "jump\n";
#endif

    ArticleViewBase::goto_num( num_to, num_from );
}


// ロード中
const bool ArticleViewMain::is_loading() const
{
    return get_article()->is_loading();
}


// 更新した
const bool ArticleViewMain::is_updated()
{

#ifdef _DEBUG
    std::cout << "ArticleViewMain::is_updated " << url_article() << " " << ( get_article()->get_status() & STATUS_UPDATED ) << std::endl;
#endif

    return ( get_article()->get_status() & STATUS_UPDATED );
}


// 更新チェックして更新可能か
const bool ArticleViewMain::is_check_update()
{
#ifdef _DEBUG
    std::cout << "ArticleViewMain::is_check_update " << url_article() << " " << ( get_article()->get_status() & STATUS_UPDATE ) << std::endl;
#endif

    return ( get_article()->get_status() & STATUS_UPDATE );
}

// 古いデータか
const bool ArticleViewMain::is_old()
{
    return ( get_article()->get_status() & STATUS_OLD );
}

// 壊れているか
const bool ArticleViewMain::is_broken()
{
    return ( get_article()->get_status() & STATUS_BROKEN );
}

//
// 再読み込み実行
//
// virtual
void ArticleViewMain::exec_reload()
{
#ifdef _DEBUG
    std::cout << "ArticleViewMain::exec_reload\n";
#endif

    // オフライン
    if( ! SESSION::is_online() ){
        SKELETON::MsgDiag mdiag( get_parent_win(), "オフラインです" );
        mdiag.run();

        if( get_live() ) ARTICLE::get_admin()->set_command( "live_stop", get_url() );

        return;
    }

    show_view();
}



//
//  キャッシュ表示 & 差分ロード開始
//
void ArticleViewMain::show_view()
{
    // 更新チェック中の場合はチェックが終わってからclock_in()でロードする
    if( get_article()->is_checking_update() ){
        m_reload_reserve = true;
        return;
    }

    if( is_loading() ) return;
    if( m_cancel_reload_counter ){
#ifdef _DEBUG
        std::cout << "cancel reload\n";
#endif
        // オートリロードのカウンタを0にする
        reset_autoreload_counter();
        // タブのアイコン状態を更新
        ARTICLE::get_admin()->set_command( "toggle_icon", get_url() );

        return;
    }

    // キャッシュを削除してからスレを再読み込み
    if( SESSION::is_online() && get_reget() ){

        int jump_to = drawarea()->get_seen_current();
        if( ! jump_to ) jump_to = get_article()->get_number_seen();

#ifdef _DEBUG
        std::cout << "ArticleViewMain::show_view reget url_article = " << url_article()
                  << " jump_to = " << jump_to << std::endl;
#endif

        set_reget( false );
        CORE::core_set_command( "delete_article", url_article(), "reget", MISC::itostr( jump_to ) );        
        return;
    }

    m_gotonum_reserve_to = 0;
    m_gotonum_reserve_from = 0;
    m_gotonum_seen = 0;
    m_set_history = false;
    m_show_instdialog = false;
    m_playsound = false;
    m_show_closedialog = false;

    // オートリロードのカウンタを0にする
    reset_autoreload_counter();

#ifdef _DEBUG
    std::cout << "ArticleViewMain::show_view " << url_article() << std::endl;
#endif

    if( get_url().empty() ){
        set_status( "invalid URL" );
        ARTICLE::get_admin()->set_command( "set_status", get_url(), get_status() );
        return;
    }

    // 負荷を減らすため update_view() や update_finish() が呼び出されるまで描画不可にしておく
    drawarea()->set_enable_draw( false );

    // articleクラスがまだキャッシュにあるdatを解析していないときに
    // drawarea()->append_res()を呼ぶと、 nodetree が作られて update_finish() が
    // コールバックされるので、 キャッシュをまだ読み込んでない時は show_view() の中で
    // update_finish()を呼ばないようにする。動作をまとめると次のようになる。
    //
    // オフライン　かつ
    //   キャッシュを読み込んでいない場合  -> articleでnodetreeが作られた時に update_finish がコールバックされる
    //   キャッシュを読み込んでいる場合    -> show_viewから直接  update_finish を呼ぶ
    //
    // オンライン　かつ
    //   キャッシュを読み込んでいない場合  -> articleでnodetreeが作られた時に update_finish がコールバックされる
    // 　　　　　　　　　　　　　　　　　　　　ロード終了時にもupdate_finish がコールバックされる
    //   キャッシュを読み込んでいる場合    -> show_viewから直接  update_finish を呼ぶ
    //  　　　　　　　　　　　　　　　　　　　　ロード終了時にもupdate_finish がコールバックされる

    const bool call_update_finish = get_article()->is_cache_read();

    // キャッシュに含まれているレスを表示
    const int from_num = drawarea()->max_number() + 1;
    const int to_num = get_article()->get_number_load();
    if( from_num <= to_num ){

        drawarea()->append_res( from_num, to_num );

        // update_finish()を呼び出したときに以前見ていたところにジャンプ
        m_gotonum_seen = get_article()->get_number_seen();
    }

    // セパレータを最後に移動
    drawarea()->set_separator_new( to_num + 1 );

    // update_finish() を呼んでキャッシュの分を描画
    if( call_update_finish ){

#ifdef _DEBUG
        std::cout << "call_update_finish\n";
#endif

        // update_finish()後に一番最後や新着にジャンプしないように設定を一時的に解除する
        const bool jump_bottom = CONFIG::get_jump_after_reload();
        const bool jump_new = CONFIG::get_jump_new_after_reload();
        CONFIG::set_jump_after_reload( false );
        CONFIG::set_jump_new_after_reload( false );

        // 一時的に実況モード解除
        const bool live = get_live();
        set_live( false );

        if( ! SESSION::is_online() ) m_set_history = true;

        update_finish();

        CONFIG::set_jump_after_reload( jump_bottom );
        CONFIG::set_jump_new_after_reload( jump_new );
        set_live( live );
    }
    else{

        // キャッシュにログが無く、かつオフラインで開くとラベルが表示されないので
        // ラベルとタブのアイコン状態を更新しておく
        if( ! SESSION::is_online() ) update_finish();
    }

    m_set_history = true;

    // オフラインならダウンロードを開始しない
    if( ! SESSION::is_online() ) return;

    // 板一覧との切り替え方法説明ダイアログ表示
    if( CONFIG::get_instruct_tglart() && SESSION::get_mode_pane() == SESSION::MODE_2PANE ){
        m_show_instdialog = true;
    }

    m_show_closedialog = true;

    clear_highlight();
    if( ! get_live() && SESSION::is_online() ) m_playsound = true;

    // 差分 download 開始
    const bool check_update = false;
    get_article()->download_dat( check_update );
    if( is_loading() ){
#ifdef _DEBUG
        std::cout << "loading start\n";
#endif
        set_status( "loading..." );
        ARTICLE::get_admin()->set_command( "set_status", get_url(), get_status(), ( get_live() ? "force" : "" ) );

        // タブのアイコン状態を更新
        ARTICLE::get_admin()->set_command( "toggle_icon", get_url() );

        // スレ一覧などを素早くクリックした時などに2回リロードされるのを防ぐ
        m_cancel_reload_counter = CANCEL_RELOAD / TIMER_TIMEOUT;
    }
}



//
// ロード中にノード構造が変わったら呼ばれる
//
void ArticleViewMain::update_view()
{
    const int code = DBTREE::article_code( url_article() );
    const int num_from = drawarea()->max_number() + 1;
    const int num_to = get_article()->get_number_load();

#ifdef _DEBUG
    std::cout << "ArticleViewMain::update_view : from " << num_from << " to " << num_to
              << " play = " << m_playsound << " code = " << code << std::endl;
#endif

    // 音を鳴らす
    if( m_playsound ){

        if( num_to >= num_from ){ 

            // 新着
            if( num_from == 1 ) SOUND::play( SOUND::SOUND_NEW );

            // 更新
            else SOUND::play( SOUND::SOUND_RES );

            m_playsound = false;
        }

        else{

            // 更新無し
            if( code == HTTP_NOT_MODIFIED ){
                SOUND::play( SOUND::SOUND_NO );
                m_playsound = false;
            }

            // エラー
            else if( code != HTTP_INIT ){
                SOUND::play( SOUND::SOUND_ERR );
                m_playsound = false;
            }
        }
    }

    if( num_from > num_to ) return;

#ifdef _DEBUG
    std::cout << "append " << num_from << " to " << num_to << std::endl;
#endif

    drawarea()->append_res( num_from, num_to );

    drawarea()->set_enable_draw( true );
    drawarea()->redraw_view();
}



//
// ロードが終わったときに呼ばれる
//
void ArticleViewMain::update_finish()
{
    // スレラベルセット
    std::string str_tablabel;
    if( is_broken() ) str_tablabel = "[ 壊れています ]  ";
    else if( is_old() ) str_tablabel = "[ DAT落ち ]  ";

    if( get_label().empty() || ! str_tablabel.empty() ) set_label( str_tablabel + DBTREE::article_subject( url_article() ) );
    ARTICLE::get_admin()->set_command( "redraw_toolbar" );

    // タブのラベルセット
    std::string str_label = DBTREE::article_subject( url_article() );
    if( str_label.empty() ) str_label = "???";
    ARTICLE::get_admin()->set_command( "set_tablabel", get_url(), str_label ); 

    // タブのアイコン状態を更新
    ARTICLE::get_admin()->set_command( "toggle_icon", get_url() );

#ifdef _DEBUG
    const int code = DBTREE::article_code( url_article() );
    std::cout << "ArticleViewMain::update_finish " << str_label << " code = " << code << std::endl;;
#endif

    // 新着セパレータを消す
    const int number_load = DBTREE::article_number_load( url_article() );
    const int number_new = DBTREE::article_number_new( url_article() );
    if( ! number_new ) drawarea()->hide_separator_new();

    // ステータス更新 (実況中はフォーカスされてなくても表示)
    std::string force;
    if( get_live() ) force = "force";
    create_status_message();
    ARTICLE::get_admin()->set_command( "set_status", get_url(), get_status(), force );
    ARTICLE::get_admin()->set_command( "set_status_color", get_url(), get_color(), force );

    // タイトルセット
    set_title( DBTREE::article_subject( url_article() ) );
    ARTICLE::get_admin()->set_command( "set_title", get_url(), get_title() );

    drawarea()->set_enable_draw( true );

    // ロード中に goto_num() が明示的に呼び出された場合はgoto_num()を呼びつづける
    if( m_gotonum_reserve_to ){
#ifdef _DEBUG
        std::cout << "reserve\n";
#endif
        goto_num( m_gotonum_reserve_to, m_gotonum_reserve_from );
    }

    // 前回見ていた所にジャンプ
    else if( m_gotonum_seen && number_load >= m_gotonum_seen ){
#ifdef _DEBUG
        std::cout << "goto_seen\n";
#endif
        ArticleViewBase::goto_num( m_gotonum_seen, 0 );
        m_gotonum_seen = 0;
    }

    // ロード後に末尾ジャンプ
    else if( ! get_live() && CONFIG::get_jump_after_reload() && number_new ){
#ifdef _DEBUG
        std::cout << "jump_after_reload\n";
#endif
        goto_bottom();
    }

    // ロード後に新着へジャンプ
    else if( ! get_live() && CONFIG::get_jump_new_after_reload() && number_new ){
#ifdef _DEBUG
        std::cout << "jump_new_after_reload\n";
#endif
        goto_new();
    }

    // 全体再描画
    else{
#ifdef _DEBUG
        std::cout << "redraw\n";
#endif
        drawarea()->redraw_view();
    }

    // 実況モードで新着がない場合はリロード間隔を空ける
    if( get_live() ){

        const int live_sec = DBTREE::board_get_live_sec( get_url() );

        // 新着無し
        if( ! number_new ){

            set_autoreload_sec( get_autoreload_sec() + LIVE_SEC_PLUS );

            // 何回かリロードに失敗したら実況モード停止
            if( get_autoreload_sec() >= live_sec + LIVE_MAX_RELOAD * LIVE_SEC_PLUS ){
                ARTICLE::get_admin()->set_command( "live_stop", get_url() );
            }

            // DAT 落ちしていたら停止
            if( is_old() ) ARTICLE::get_admin()->set_command( "live_stop", get_url() );
        }

        // 新着あり
        else{

            set_autoreload_sec( MAX( live_sec, get_autoreload_sec() - LIVE_SEC_PLUS ) );

            // messageビューが出ているときはフォーカスを移す
            if( ! MESSAGE::get_admin()->empty() ){
                // 実況モードかつポップアップが表示されている状態で、 "switch_message" コマンドを発行すると、
                // 埋め込みメッセージモードだと、 MessageAdmin に対して "focus_current_view" されるので、
                // ArticleView に focus_out イベントが発生して、ポップアップが消えてしまう
                // ポップアップが表示されているときは、フォーカスを移さない
                if( ! is_popup_shown() ){
                    CORE::core_set_command( "switch_message" );
                }
            }
        }

        drawarea()->update_live_speed( get_autoreload_sec() );
    }

    // 履歴に登録
    if( m_set_history ) HISTORY::append_history( URL_HISTTHREADVIEW,
                                                 DBTREE::url_dat( get_url() ),
                                                 DBTREE::article_subject( get_url() ), TYPE_THREAD );

    if( m_show_instdialog ) show_instruct_diag();

    if( m_show_closedialog && !number_load && is_old() ){

        SKELETON::MsgDiag mdiag( get_parent_win(), "DAT落ちしたためスレッドを取得できませんでした。\n\nタブを閉じますか？",
                                 false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_YES_NO );
        mdiag.set_default_response( Gtk::RESPONSE_YES );
        if( mdiag.run() == Gtk::RESPONSE_YES ) close_view();
    }
}


//
// ステータスに表示する文字列作成
//
void ArticleViewMain::create_status_message()
{
#ifdef _DEBUG
    std::cout << "ArticleViewMain::create_status_message\n";
#endif

    const int number_load = DBTREE::article_number_load( url_article() );
    const int number_new = DBTREE::article_number_new( url_article() );

    std::ostringstream ss_tmp;
    ss_tmp << DBTREE::article_str_code( url_article() )
           << " [ 全 " << number_load
           << " / 新着 " << number_new;

    const time_t wtime = DBTREE::article_write_time( url_article() );
    if( wtime ) ss_tmp << " / 最終書込 "
                       << ( MISC::timettostr( wtime, MISC::TIME_WEEK ) + " ( " +  MISC::timettostr( wtime, MISC::TIME_PASSED ) +  " )" );

    std::string str_stat;
    if( is_old() ) str_stat = "[ DAT落ち 又は 移転しました ] ";
    if( is_check_update() ) str_stat += "[ 更新可能です ] ";
    if( is_broken() ) str_stat += "[ 壊れています ] ";

    if( ! DBTREE::article_ext_err( url_article() ).empty() ) str_stat += "[ " + DBTREE::article_ext_err( url_article() ) + " ] ";

    ss_tmp << " / 速度 " << DBTREE::article_get_speed( url_article() )
           << " / " << DBTREE::article_lng_dat( url_article() )/1024 << " K ] "
           << str_stat;

    set_status( ss_tmp.str() );
}


//
// 板一覧との切り替え方法説明ダイアログ表示
//
void ArticleViewMain::show_instruct_diag()
{
    m_show_instdialog = false;

    SKELETON::MsgCheckDiag mdiag( get_parent_win(), 
                                  "スレビューからスレ一覧表示に戻る方法として\n\n(1) マウスジェスチャを使う\n(マウス右ボタンを押しながら左または下にドラッグして右ボタンを離す)\n\n(2) マウスの5ボタンを押す\n\n(3) Alt+x か h か ← を押す\n\n(4) ツールバーのスレ一覧アイコンを押す\n\n(5) 表示メニューからスレ一覧を選ぶ\n\nなどがあります。詳しくはオンラインマニュアルを参照してください。"
                                  , "今後表示しない(_D)"
        );
    mdiag.set_title( "ヒント" );
    mdiag.run();

    if( mdiag.get_chkbutton().get_active() ) CONFIG::set_instruct_tglart( false );
}



//
// 画面を消してレイアウトやりなおし & 再描画
//
void ArticleViewMain::relayout()
{
#ifdef _DEBUG
    std::cout << "ArticleViewMain::relayout " << DBTREE::article_subject( url_article() ) << std::endl;;
#endif

    hide_popup( true );

    int seen = drawarea()->get_seen_current();
    int num_reserve = drawarea()->get_goto_num_reserve();
    int separator_new = drawarea()->get_separator_new();

    drawarea()->clear_screen();
    drawarea()->set_separator_new( separator_new );
    drawarea()->append_res( 1, get_article()->get_number_load() );
    if( num_reserve ) drawarea()->goto_num( num_reserve );
    else if( seen ) drawarea()->goto_num( seen );
    drawarea()->redraw_view();

    // ステータス更新
    create_status_message();
    ARTICLE::get_admin()->set_command( "set_status", get_url(), get_status() );
}



//
// 実況開始
//
// virtual
void ArticleViewMain::live_start()
{
    if( get_live() ) return;

    // オフライン
    if( ! SESSION::is_online() ){
        SKELETON::MsgDiag mdiag( get_parent_win(), "オフラインです" );
        mdiag.run();
        return;
    }

#ifdef _DEBUG
    std::cout << "ArticleViewMain::live_start\n";
#endif

    const int live_sec = DBTREE::board_get_live_sec( get_url() );

    set_live( true );
    ARTICLE::get_admin()->set_command_immediately( "start_autoreload", get_url(), "on", MISC::itostr( live_sec ) );
    set_autoreload_counter( live_sec * 1000/TIMER_TIMEOUT );
    drawarea()->live_start();
    drawarea()->update_live_speed( live_sec );

    goto_bottom();
}


//
// 実況停止
//
// virtual
void ArticleViewMain::live_stop()
{
    if( ! get_live() ) return;

#ifdef _DEBUG
    std::cout << "ArticleViewMain::live_stop\n";
#endif

    set_live( false );
    ARTICLE::get_admin()->set_command_immediately( "stop_autoreload", get_url() );
    drawarea()->live_stop();

    set_status( "実況停止" );
    ARTICLE::get_admin()->set_command( "set_status", get_url(), get_status(), "force" );
}
