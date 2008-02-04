// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "articleadmin.h"
#include "articleview.h"
#include "drawareamain.h"

#include "skeleton/msgdiag.h"

#include "dbtree/interface.h"
#include "dbtree/articlebase.h"

#include "jdlib/miscutil.h"

#include "config/globalconf.h"

#include "command.h"
#include "global.h"
#include "httpcode.h"
#include "session.h"
#include "controlid.h"

#include <sstream>


using namespace ARTICLE;


// メインビュー

ArticleViewMain::ArticleViewMain( const std::string& url )
    :  ArticleViewBase( url ), m_gotonum_reserve( 0 )
{
#ifdef _DEBUG
    std::cout << "ArticleViewMain::ArticleViewMain " << get_url() << " url_article = " << url_article() << std::endl;
#endif

    // オートリロード可
    set_enable_autoreload( true );

    setup_view();
}



ArticleViewMain::~ArticleViewMain()
{
#ifdef _DEBUG    
    std::cout << "ArticleViewMain::~ArticleViewMain : " << get_url() << " url_article = " << url_article() << std::endl;
#endif
    int seen = drawarea()->get_seen_current();
        
#ifdef _DEBUG    
    std::cout << "set seen to " << seen << std::endl;
#endif

    if( seen >= 1 ) get_article()->set_number_seen( seen );

    // 閉じたタブ履歴更新
    CORE::core_set_command( "set_history_close", url_article() );
}


//
// num 番にジャンプ
//
// ローディング中ならジャンプ予約をしてロード後に update_finish() の中で改めて goto_num() を呼び出す
//
void ArticleViewMain::goto_num( int num )
{
    if( get_article()->get_number_load() < num  && is_loading() ){

        m_gotonum_reserve = num;
        return;
    }

    m_gotonum_reserve = 0;
    ArticleViewBase::goto_num( num );
}


// ロード中
const bool ArticleViewMain::is_loading()
{
    return get_article()->is_loading();
}


// 更新した
const bool ArticleViewMain::is_updated()
{
    int code = DBTREE::article_code( url_article() );
    return ( code == HTTP_OK || code == HTTP_PARTIAL_CONTENT );
}


// 更新チェックして更新可能か
const bool ArticleViewMain::is_check_update()
{
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
// 再読み込み
//
void ArticleViewMain::reload()
{
    // オフライン
    if( ! SESSION::is_online() ){
        SKELETON::MsgDiag mdiag( NULL, "オフラインです" );
        mdiag.run();
        return;
    }

    // オートリロードのカウンタを0にする
    View::reset_autoreload_counter();

    show_view();

    // スレ履歴更新
    CORE::core_set_command( "set_history_article", url_article() );
}



//
//  キャッシュ表示 & 差分ロード開始
//
void ArticleViewMain::show_view()
{
    m_gotonum_reserve = 0;
    m_show_instdialog = false;

#ifdef _DEBUG
    std::cout << "ArticleViewBase::show_view\n";
#endif

    if( get_url().empty() ){
        set_status( "invalid URL" );
        ARTICLE::get_admin()->set_command( "set_status", get_url(), get_status() );
        return;
    }

    // もしarticleクラスがまだキャッシュにあるdatを解析していないときに
    // drawarea()->append_res()を呼ぶと update_finish() がコールバック
    // されて2回再描画することになるので、 show_view() の中で update_finish()を
    // 呼ばないようにする。動作をまとめると次のようになる。

    // オフライン　かつ
    //   キャッシュを読み込んでいない場合  -> articleでnodetreeが作られた時に update_finish がコールバックされる
    //   キャッシュを読み込んでいる場合    -> show_viewから直接  update_finish を呼ぶ
    //
    // オンライン　かつ
    //   キャッシュを読み込んでいない場合  -> articleでnodetreeが作られた時に update_finish がコールバックされる
    // 　　　　　　　　　　　　　　　　　　　　ロード終了時にもupdate_finish がコールバックされる
    //   キャッシュを読み込んでいる場合    -> show_viewから直接  update_finish を呼ぶ
    //  　　　　　　　　　　　　　　　　　　　　ロード終了時にもupdate_finish がコールバックされる

    bool call_update_finish = get_article()->is_cache_read();

    // キャッシュに含まれているレスを表示
    int from_num = drawarea()->max_number() + 1;
    int to_num = get_article()->get_number_load();
    if( from_num <= to_num ){

        drawarea()->append_res( from_num, to_num );

        // 以前見ていたところにジャンプ
        drawarea()->goto_num( get_article()->get_number_seen() );
    }

    // セパレータを最後に移動
    drawarea()->set_separator_new( to_num + 1 );

    // update_finish() を呼んでキャッシュの分を描画
    if( call_update_finish ){

        // update_finish()後に一番最後や新着にジャンプしないように設定を一時的に解除する
        const bool jump_bottom = CONFIG::get_jump_after_reload();
        const bool jump_new = CONFIG::get_jump_new_after_reload();
        CONFIG::set_jump_after_reload( false );
        CONFIG::set_jump_new_after_reload( false );

        update_finish();

        CONFIG::set_jump_after_reload( jump_bottom );
        CONFIG::set_jump_new_after_reload( jump_new );
    }
    else{

        // キャッシュにログが無く、かつオフラインで開くとラベルが表示されないので
        // ラベルとタブのアイコン状態を更新しておく
        if( ! SESSION::is_online() ) update_finish();
    }


    // オフラインならダウンロードを開始しない
    if( ! SESSION::is_online() ) return;

    // 板一覧との切り替え方法説明ダイアログ表示
    if( CONFIG::get_instruct_tglart() && SESSION::get_mode_pane() == SESSION::MODE_2PANE ){
        m_show_instdialog = true;
    }

    clear_highlight();

    // 差分 download 開始
    get_article()->download_dat( false );
    if( is_loading() ){

        set_status( "loading..." );
        ARTICLE::get_admin()->set_command( "set_status", get_url(), get_status() );

        // タブのアイコン状態を更新
        ARTICLE::get_admin()->set_command( "toggle_icon", get_url() );
    }
}



//
// ロード中にノード構造が変わったら呼ばれる
//
void ArticleViewMain::update_view()
{
    int num_from = drawarea()->max_number() + 1;
    int num_to = get_article()->get_number_load();

#ifdef _DEBUG
    std::cout << "ArticleViewMain::update_view : from " << num_from << " to " << num_to << std::endl;
#endif

    if( num_from > num_to ) return;

    drawarea()->append_res( num_from, num_to );
    drawarea()->redraw_view();
}



//
// ロードが終わったときに呼ばれる
//
void ArticleViewMain::update_finish()
{
    std::string str_stat;
    if( is_old() ) str_stat = "[ DAT落ち 又は 移転しました ] ";
    if( is_check_update() ) str_stat += "[ 更新可能です ] ";
    if( is_broken() ) str_stat += "[ 壊れています ] ";

    if( ! DBTREE::article_ext_err( url_article() ).empty() ) str_stat += "[ " + DBTREE::article_ext_err( url_article() ) + " ] ";

    // スレラベルセット
    std::string str_tablabel;
    if( is_broken() ) str_tablabel = "[ 壊れています ]  ";
    else if( is_old() ) str_tablabel = "[ DAT落ち ]  ";

    if( get_label().empty() || ! str_tablabel.empty() ) set_label( str_tablabel + DBTREE::article_subject( url_article() ) );
    ARTICLE::get_admin()->set_command( "update_toolbar_label" );

    // タブのラベルセット
    std::string str_label = DBTREE::article_subject( url_article() );
    if( str_label.empty() ) str_label = "???";
    ARTICLE::get_admin()->set_command( "set_tablabel", get_url(), str_label ); 

    // タブのアイコン状態を更新
    ARTICLE::get_admin()->set_command( "toggle_icon", get_url() );


#ifdef _DEBUG
    int code = DBTREE::article_code( url_article() );
    std::cout << "ArticleViewMain::update_finish " << str_label << " code = " << code << std::endl;;
#endif

    // 新着セパレータを消す
    int number_new = DBTREE::article_number_new( url_article() );
    if( ! number_new ) drawarea()->hide_separator_new();

    // ステータス表示
    std::ostringstream ss_tmp;
    ss_tmp << DBTREE::article_str_code( url_article() )
           << " [ 全 " << DBTREE::article_number_load( url_article() )
           << " / 新着 " << number_new;

    if( DBTREE::article_write_time( url_article() ) ) ss_tmp << " / 最終書込 " << DBTREE::article_write_date( url_article() );

    ss_tmp << " / 速度 " << DBTREE::article_get_speed( url_article() )
           << " / " << DBTREE::article_lng_dat( url_article() )/1024 << " k ] "
           << str_stat;

    set_status( ss_tmp.str() );
    ARTICLE::get_admin()->set_command( "set_status", get_url(), get_status() );

    // タイトルセット
    set_title( DBTREE::article_subject( url_article() ) );
    ARTICLE::get_admin()->set_command( "set_title", get_url(), get_title() );

    // 全体再描画
    drawarea()->redraw_view();

    if( CONFIG::get_jump_after_reload() ) goto_bottom();
    else if( number_new && CONFIG::get_jump_new_after_reload() && ! drawarea()->is_separator_on_screen() ) goto_new();
    else if( m_gotonum_reserve ) goto_num( m_gotonum_reserve );
    m_gotonum_reserve = 0;

    if( m_show_instdialog ) show_instruct_diag();
}


//
// 板一覧との切り替え方法説明ダイアログ表示
//
void ArticleViewMain::show_instruct_diag()
{
    const int mrg = 16;

    SKELETON::MsgDiag mdiag( NULL, 
        "スレビューからスレ一覧表示に戻る方法として\n\n(1) マウスジェスチャを使う\n(マウス右ボタンを押しながら左または下にドラッグして右ボタンを離す)\n\n(2) マウスの5ボタンを押す\n\n(3) Alt+x か h か ← を押す\n\n(4) ツールバーのスレ一覧アイコンを押す\n\n(5) 表示メニューからスレ一覧を選ぶ\n\nなどがあります。詳しくはオンラインマニュアルを参照してください。" );
    Gtk::HBox hbox;
    Gtk::CheckButton chkbutton( "今後表示しない(_D)", true );
    hbox.pack_start( chkbutton, Gtk::PACK_EXPAND_WIDGET, mrg );
    mdiag.get_vbox()->pack_start( hbox, Gtk::PACK_SHRINK );
    mdiag.set_title( "ヒント" );
    mdiag.show_all_children();
    mdiag.run();

    if( chkbutton.get_active() ) CONFIG::set_instruct_tglart( false );
    m_show_instdialog = false;
}



//
// 画面を消してレイアウトやりなおし & 再描画
//
void ArticleViewMain::relayout()
{
#ifdef _DEBUG
    std::cout << "ArticleViewMain::relayout\n";
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
}
