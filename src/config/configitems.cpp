// ライセンス: GPL2

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

//#define _DEBUG
#include "jddebug.h"

#include "configitems.h"
#include "defaultconf.h"

#include "jdlib/confloader.h"
#include "jdlib/miscutil.h"
#include "jdlib/miscgtk.h"

#include "colorid.h"
#include "fontid.h"
#include "global.h"
#include "cache.h"
#include "browsers.h"

#include <set>

using namespace CONFIG;

//
// デフォルトフォント取得
//

#define IS_DEFAULT_FONT( name ) do{ if( set_fonts.find( name ) != set_fonts.end() ) return name; } while(0)

std::string get_default_font()
{
    std::set< std::string > set_fonts = MISC::get_font_families();

    IS_DEFAULT_FONT( "IPA モナー Pゴシック" );
    IS_DEFAULT_FONT( "IPAMonaPGothic" );
    IS_DEFAULT_FONT( "Mona" );
    IS_DEFAULT_FONT( "VL Pゴシック" );
    IS_DEFAULT_FONT( "VL PGothic" );
    IS_DEFAULT_FONT( "さざなみゴシック" );
    IS_DEFAULT_FONT( "Sazanami Gothic" );
    IS_DEFAULT_FONT( "Sans" );

    return std::string();
}


////////////////////////////////////////


ConfigItems::ConfigItems()
{
    str_color.resize( COLOR_NUM );
    fontname.resize( FONT_NUM );
}

ConfigItems::~ConfigItems()
{}

// 設定読み込み
const bool ConfigItems::load()
{
    std::string path_conf = CACHE::path_conf();

    // 新設定ファイルが無かったら旧ファイルから読み込む
    if( CACHE::file_exists( path_conf ) != CACHE::EXIST_FILE ){

        if( CACHE::file_exists( CACHE::path_conf_old() ) == CACHE::EXIST_FILE ) path_conf = CACHE::path_conf_old();
    }

    JDLIB::ConfLoader cf( path_conf, std::string() );

#ifdef _DEBUG
    std::cout << "ConfigItems::load" << std::endl
              << "conffile = " << path_conf << " empty = " << cf.empty() << std::endl;
#endif

    // 前回開いたviewを復元するか
    restore_board = cf.get_option( "restore_board", false );
    restore_article = cf.get_option( "restore_article", false );
    restore_image = cf.get_option( "restore_image", false );

    std::string defaultfont = get_default_font();

    // フォント
    fontname[ FONT_MAIN ] = cf.get_option( "fontname_main", defaultfont + " " + std::string( CONF_FONTSIZE_THREAD ) );

    // ポップアップのフォント
    fontname[ FONT_POPUP ] = cf.get_option( "fontname_popup", defaultfont + " " + std::string( CONF_FONTSIZE_POPUP ) );

    // スレ一覧のフォント
    fontname[ FONT_BBS ] = cf.get_option( "fontname_bbs", defaultfont + " " + std::string(CONF_FONTSIZE_TREE ) );

    // 板一覧のフォント
    fontname[ FONT_BOARD ] = cf.get_option( "fontname_board", fontname[ FONT_BBS ] );

    // 書き込みウィンドウのフォント
    fontname[ FONT_MESSAGE ] = cf.get_option( "fontname_message", fontname[ FONT_MAIN ] );

    // レスを参照するときに前に付ける文字
    ref_prefix = cf.get_option( "ref_prefix", ">" );

    // ref_prefix の後のスペースの数
    // JDLIB::ConfLoader の中で MISC::remove_space() が呼ばれて空白が消えるので別設定とした
    ref_prefix_space = cf.get_option( "ref_prefix_space", 1 );
    for( int i = 0; i < ref_prefix_space; ++i ) ref_prefix += " ";

    // キャッシュのルートディレクトリ(旧バージョンとの互換のため残している)
    path_cacheroot = cf.get_option( "path_cacheroot", "~/.jd/" );

    // 読み込み用プロクシとポート番号
    use_proxy_for2ch = cf.get_option( "use_proxy_for2ch", 0 );
    proxy_for2ch = cf.get_option( "proxy_for2ch", "" );
    proxy_port_for2ch = cf.get_option( "proxy_port_for2ch", 8080 );

    // 書き込み用プロクシとポート番号
    use_proxy_for2ch_w = cf.get_option( "use_proxy_for2ch_w", 0 );
    proxy_for2ch_w = cf.get_option( "proxy_for2ch_w", "" );
    proxy_port_for2ch_w = cf.get_option( "proxy_port_for2ch_w", 8080 );

    // 2chの外にアクセスするときのプロクシとポート番号
    use_proxy_for_data = cf.get_option( "use_proxy_for_data", 0 );
    proxy_for_data = cf.get_option( "proxy_for_data", "" );
    proxy_port_for_data = cf.get_option( "proxy_port_for_data", 8080 );

    // 2ch にアクセスするときのエージェント名
    agent_for2ch = cf.get_option( "agent_for2ch", "Monazilla/1.00 JD" );

    // 2ch外にアクセスするときのエージェント名
    agent_for_data = cf.get_option( "agent_for_data", "Mozilla/5.0 (Windows; U; Windows NT 5.0; ja; rv:1.8) Gecko/20051111 Firefox/1.5" );

    // 2ch にログインするときのX-2ch-UA
    x_2ch_ua = cf.get_option( "x_2ch_ua", "Navigator for 2ch 1.7.5" );

    // ローダのバッファサイズ
    loader_bufsize = cf.get_option( "loader_bufsize", 32 );

    // ローダのタイムアウト値
    loader_timeout = cf.get_option( "loader_timeout", 10 );
    loader_timeout_post = cf.get_option( "loader_timeout_post", 30 ); // ポスト
    loader_timeout_img = cf.get_option( "loader_timeout_img", 30 ); // 画像

    // ipv6使用
    use_ipv6 = cf.get_option( "use_ipv6", CONF_USE_IPV6 );

    // ブラウザ設定ダイアログのコンボボックスの番号
    browsercombo_id = cf.get_option( "brownsercombo_id", CONF_BROWSER_NO );

    // リンクをクリックしたときに実行するコマンド
    command_openurl = cf.get_option( "command_openurl", CORE::get_browser_name( CONF_BROWSER_NO ) );

    // レス番号の上にマウスオーバーしたときに参照ポップアップ表示する
    refpopup_by_mo = cf.get_option( "refpopup_by_mo", false );

    // 名前の上にマウスオーバーしたときにポップアップ表示する
    namepopup_by_mo = cf.get_option( "namepopup_by_mo", false );

    // IDの上にマウスオーバーしたときにIDをポップアップ表示する
    idpopup_by_mo = cf.get_option( "idpopup_by_mo", false );

    // 画像ポップアップサイズ
    imgpopup_width = cf.get_option( "imgpopup_width", CONF_IMGPOPUP_WIDTH );
    imgpopup_height = cf.get_option( "imgpopup_height", CONF_IMGPOPUP_HEIGHT );

    // 画像ビューを使用する
    use_image_view = cf.get_option( "use_image_view", CONF_USE_IMAGE_VIEW );

    // インライン画像を表示する
    use_inline_image = cf.get_option( "use_inline_image", CONF_INLINE_IMG );

    // 画像にモザイクをかける
    use_mosaic = cf.get_option( "use_mosaic", CONF_USE_MOSAIC );

    // 画像をデフォルトでウィンドウサイズに合わせる
    zoom_to_fit = cf.get_option( "zoom_to_fit", CONF_ZOOM_TO_FIT );

    // 画像キャッシュ削除の日数
    del_img_day = cf.get_option( "del_img_day", CONF_DEL_IMG_DAY );

    // ダウンロードする画像の最大サイズ(Mbyte)
    max_img_size = cf.get_option( "max_img_size", CONF_MAX_IMG_SIZE );

    // JD ホームページのアドレス
    url_jdhp = cf.get_option( "url_jdhp", CONF_JDHP );

    // 2chの認証サーバ
    url_login2ch = cf.get_option( "url_login2ch", CONF_LOGIN2CH );

    // bbsmenu.htmlのURL
    url_bbsmenu = cf.get_option( "url_bbsmenu", CONF_BBSMENU );

    // bbsmenu.html内にあるリンクは全て板とみなす
    use_link_as_board = cf.get_option( "use_link_as_board", CONF_LINK_AS_BOARD );


    /////////
    // 色

    // 文字色
    str_color[ COLOR_CHAR ] = cf.get_option( "cl_char", CONF_COLOR_CHAR );

    // 名前欄の文字色
    str_color[ COLOR_CHAR_NAME ] = cf.get_option( "cl_char_name", CONF_COLOR_CHAR_NAME );

    // トリップ等の名前欄の文字色
    str_color[ COLOR_CHAR_NAME_B ] = cf.get_option( "cl_char_name_b", CONF_COLOR_CHAR_NAME_B );

    // ageの時のメール欄の文字色
    str_color[ COLOR_CHAR_AGE ] = cf.get_option( "cl_char_age", CONF_COLOR_CHAR_AGE );

    // 選択範囲の文字色
    str_color[ COLOR_CHAR_SELECTION ] = cf.get_option( "cl_char_selection", CONF_COLOR_CHAR_SELECTION );

    // ハイライトの文字色
    str_color[ COLOR_CHAR_HIGHLIGHT ] = cf.get_option( "cl_char_highlight", CONF_COLOR_CHAR_HIGHLIGHT );

    // ブックマークの文字色
    str_color[ COLOR_CHAR_BOOKMARK ] = cf.get_option( "cl_char_bookmark", CONF_COLOR_CHAR_BOOKMARK );

    // リンク(通常)の文字色
    str_color[ COLOR_CHAR_LINK ] = cf.get_option( "cl_char_link", CONF_COLOR_CHAR_LINK );

    // リンク(複数)の文字色
    str_color[ COLOR_CHAR_LINK_LOW ] = cf.get_option( "cl_char_link_low", CONF_COLOR_CHAR_LINK_LOW );

    // リンク(多数)の文字色
    str_color[ COLOR_CHAR_LINK_HIGH ] = cf.get_option( "cl_char_link_high", CONF_COLOR_CHAR_LINK_HIGH );

    // メッセージビューの文字色
    str_color[ COLOR_CHAR_MESSAGE ] = cf.get_option( "cl_char_message", CONF_COLOR_CHAR_MESSAGE );

    // メッセージビュー(選択範囲)の文字色
    str_color[ COLOR_CHAR_MESSAGE_SELECTION ] = cf.get_option( "cl_char_message_selection", CONF_COLOR_CHAR_MESSAGE_SELECTION );

    // 画像(キャッシュ無)の色
    str_color[ COLOR_IMG_NOCACHE ] = cf.get_option( "cl_img_nocache", "#a5a52a2a2a2a" );

    // 画像(キャッシュ有)の色
    str_color[ COLOR_IMG_CACHED ] = cf.get_option( "cl_img_cached", "#00008b8b8b8b" );

    // 画像(ロード中)の色
    str_color[ COLOR_IMG_LOADING ] = cf.get_option( "cl_img_loading", "#ffff8c8c0000" );

    // 画像(エラー)の色
    str_color[ COLOR_IMG_ERR ] = cf.get_option( "cl_img_err", str_color[ COLOR_CHAR_AGE ] );


    // スレ背景色
    str_color[ COLOR_BACK ] = cf.get_option( "cl_back", CONF_COLOR_BACK );

    // ポップアップの背景色
    str_color[ COLOR_BACK_POPUP ] = cf.get_option( "cl_back_popup", CONF_COLOR_BACK_POPUP );

    // 選択範囲の背景色
    str_color[ COLOR_BACK_SELECTION ] = cf.get_option( "cl_back_selection", CONF_COLOR_BACK_SELECTION );

    // ハイライトの背景色
    str_color[ COLOR_BACK_HIGHLIGHT ] = cf.get_option( "cl_back_highlight", CONF_COLOR_HL );

    // ハイライトの背景色(ツリー用)
    str_color[ COLOR_BACK_HIGHLIGHT_TREE ] = cf.get_option( "cl_back_highlight_tree", CONF_COLOR_HL_TREE );

    // メッセージビューの背景色
    str_color[ COLOR_BACK_MESSAGE ] = cf.get_option( "cl_back_message", CONF_COLOR_BACK_MESSAGE );

    // メッセージビューの選択色
    str_color[ COLOR_BACK_MESSAGE_SELECTION ] = cf.get_option( "cl_back_message_selection", CONF_COLOR_BACK_MESSAGE_SELECTION );

    // 新着セパレータ
    str_color[ COLOR_SEPARATOR_NEW ] = cf.get_option( "cl_sepa_new", CONF_COLOR_SEPARATOR_NEW );

    // 板一覧の文字
    str_color[ COLOR_CHAR_BBS ] = cf.get_option( "cl_chr_bbs", CONF_COLOR_CHAR_BBS );

    // スレ一覧の文字
    str_color[ COLOR_CHAR_BOARD ] = cf.get_option( "cl_chr_board", CONF_COLOR_CHAR_BOARD );

    // 板一覧の背景色
    str_color[ COLOR_BACK_BBS ] = cf.get_option( "cl_back_bbs", CONF_COLOR_BACK_BBS );

    // 板一覧の背景色(偶数行)
    str_color[ COLOR_BACK_BBS_EVEN ] = cf.get_option( "cl_back_bbs_even", CONF_COLOR_BACK_BBS_EVEN );

    // スレ一覧の背景色
    str_color[ COLOR_BACK_BOARD ] = cf.get_option( "cl_back_board", CONF_COLOR_BACK_BOARD );

    // スレ一覧の背景色(偶数行)
    str_color[ COLOR_BACK_BOARD_EVEN ] = cf.get_option( "cl_back_board_even", CONF_COLOR_BACK_BOARD_EVEN );

    /////////////////////////


    // ツリービューでgtkrcの設定を使用するか
    use_tree_gtkrc = cf.get_option( "use_tree_gtkrc", false );

    // ツリービューの行間スペース
    tree_ypad = cf.get_option( "tree_ypad", CONF_TREE_YPAD );

    // boardビューで古いスレも表示
    show_oldarticle = cf.get_option( "show_oldarticle", false );

    // スレ一覧で指定した値(時間)よりも後に立てられたスレを新着とみなす
    newthread_hour = cf.get_option( "newthread_hour", CONF_NEWTHREAD_HOUR );

    // ツリービューのスクロール量(行数)
    tree_scroll_size = cf.get_option( "tree_scroll_size", CONF_TREE_SCROLL_SIZE );

    // スレビューのスクロール量
    scroll_size = cf.get_option( "scroll_size", CONF_SCROLL_SIZE );

    // スレビューのスクロール量(キー上下)
    key_scroll_size = cf.get_option( "key_scroll_size", CONF_KEY_SCROLL_SIZE );

    // 板一覧でカテゴリを常にひとつだけ開く
    open_one_category = cf.get_option( "open_one_category", false );

    // 書き込み時に書き込み確認ダイアログを出すかどうか
    always_write_ok = cf.get_option( "always_write_ok", false );

    // 書き込みログを保存
    save_postlog = cf.get_option( "save_postlog", false );

    // 「書き込み中」のダイアログを表示しない
    hide_writing_dialog = cf.get_option( "hide_writing_dialog", false );

    // ポップアップとカーソルの間のマージン
    margin_popup = cf.get_option( "margin_popup", CONF_MARGIN_POPUP );

    // 画像ポップアップとカーソルの間のマージン
    margin_imgpopup = cf.get_option( "margin_imgpopup", CONF_MARGIN_IMGPOPUP );

    // マウスジェスチャの判定開始半径
    mouse_radius = cf.get_option( "mouse_radius", CONF_MOUSE_RADIUS );

    // 履歴の保持数
    history_size = cf.get_option( "history_size", CONF_HISTORY );

    // AA履歴の保持数
    aahistory_size = cf.get_option( "aahistory_size", CONF_AAHISTORY );

    // 0以上なら多重ポップアップの説明を表示する
    instruct_popup = cf.get_option( "instruct_popup", CONF_INSTRUCT_POPUP );    

    // スレビューを開いたときにスレ一覧との切り替え方法を説明する
    instruct_tglart = cf.get_option( "instruct_tglart", true );
    instruct_tglart_end = false;

    // 画像ビューを開いたときにスレビューとの切り替え方法を説明する
    instruct_tglimg = cf.get_option( "instruct_tglimg", true );
    instruct_tglimg_end = false;
    
    // スレ表示の行間調整
    adjust_underline_pos = cf.get_option( "adjust_underline_pos", 1.0 );
    adjust_line_space = cf.get_option( "adjust_line_space", 1.0 );

    // リンク下線を表示
    draw_underline = cf.get_option( "draw_underline", true );

    // スレビューで文字幅の近似を厳密にする
    strict_char_width = cf.get_option( "strict_char_width", false );

    // datのパース時にURL判定を甘くする(^なども含める)
    loose_url = cf.get_option( "loose_url", CONF_LOOSE_URL );

    // ユーザーコマンドで選択できない項目を非表示にする
    hide_usrcmd = cf.get_option( "hide_usrcmd", false );

    // 指定した数よりもユーザーコマンドが多い場合はサブメニュー化する
    max_show_usrcmd = cf.get_option( "max_show_usrcmd", 3 );

    // スレビューで再読み込みボタンを押したときに全タブを更新する
    reload_allthreads = cf.get_option( "reload_allthreads", CONF_RELOAD_ALLTHREAD );

    // タブに表示する文字列の最小値
    tab_min_str = cf.get_option( "tab_min_str", 4 );

    // タブにアイコンを表示するか
    show_tab_icon = cf.get_option( "show_tab_icon", true );

    std::list< std::string > list_tmp;
    std::list< std::string >::iterator it_tmp;
    std::string str_tmp;

    // スレ あぼーん word
    str_tmp = cf.get_option( "abonewordthread", "" );
    if( ! str_tmp.empty() ) list_abone_word_thread = MISC::strtolist( str_tmp );

    // スレ あぼーん regex
    str_tmp = cf.get_option( "aboneregexthread", "" );
    if( ! str_tmp.empty() ) list_abone_regex_thread = MISC::strtolist( str_tmp );

    // あぼーん name
    str_tmp = cf.get_option( "abonename", "" );
    if( ! str_tmp.empty() ) list_abone_name = MISC::strtolist( str_tmp );

    // あぼーん word
    str_tmp = cf.get_option( "aboneword", "" );
    if( ! str_tmp.empty() ) list_abone_word = MISC::strtolist( str_tmp );

    // あぼーん regex
    str_tmp = cf.get_option( "aboneregex", "" );
    if( ! str_tmp.empty() ) list_abone_regex = MISC::strtolist( str_tmp );

    // デフォルトで透明、連鎖あぼーんをするか
    abone_transparent = cf.get_option( "abone_transparent", CONF_ABONE_TRANSPARENT );
    abone_chain = cf.get_option( "abone_chain", CONF_ABONE_CHAIN );

    // 右ペーンが空の時にサイドバーを閉じるか
    expand_sidebar = cf.get_option( "expand_sidebar", CONF_EXPAND_SIDEBAR );

#ifdef HAVE_MIGEMO_H
    // migemo-dictの場所
    migemodict_path = cf.get_option( "migemodict_path", CONF_MIGEMO_PATH );
#endif

    return ! cf.empty();
}


// 保存
void ConfigItems::save()
{
    save_impl( CACHE::path_conf() );
    if( CACHE::file_exists( CACHE::path_conf_old() ) == CACHE::EXIST_FILE ) save_impl( CACHE::path_conf_old() );
}


void ConfigItems::save_impl( const std::string& path )
{
#ifdef _DEBUG
    std::cout << "ConfigItems::save_impl path = " << path << std::endl;
#endif

    JDLIB::ConfLoader cf( path, std::string() );

    cf.update( "restore_board", restore_board );
    cf.update( "restore_article", restore_article );
    cf.update( "restore_image", restore_image );
    cf.update( "url_jdhp", url_jdhp );
    cf.update( "url_login2ch", url_login2ch );
    cf.update( "url_bbsmenu", url_bbsmenu );
    cf.update( "use_link_as_board", use_link_as_board );

    cf.update( "fontname_main", fontname[ FONT_MAIN ] );
    cf.update( "fontname_popup", fontname[ FONT_POPUP ] );
    cf.update( "fontname_bbs", fontname[ FONT_BBS ] );
    cf.update( "fontname_board", fontname[ FONT_BOARD ] );
    cf.update( "fontname_message", fontname[ FONT_MESSAGE ] );

    cf.update( "ref_prefix", ref_prefix );
    cf.update( "ref_prefix_space", ref_prefix_space );

    cf.update( "path_cacheroot", path_cacheroot );

    cf.update( "agent_for2ch", agent_for2ch );

    cf.update( "use_proxy_for2ch", use_proxy_for2ch );
    cf.update( "proxy_for2ch", proxy_for2ch );
    cf.update( "proxy_port_for2ch", proxy_port_for2ch );

    cf.update( "use_proxy_for2ch_w", use_proxy_for2ch_w );
    cf.update( "proxy_for2ch_w", proxy_for2ch_w );
    cf.update( "proxy_port_for2ch_w", proxy_port_for2ch_w );

    cf.update( "agent_for_data", agent_for_data );

    cf.update( "use_proxy_for_data", use_proxy_for_data );
    cf.update( "proxy_for_data", proxy_for_data );
    cf.update( "proxy_port_for_data", proxy_port_for_data );

    cf.update( "x_2ch_ua", x_2ch_ua );

    cf.update( "loader_bufsize", loader_bufsize );
    cf.update( "loader_timeout", loader_timeout );
    cf.update( "loader_timeout_post", loader_timeout_post );
    cf.update( "loader_timeout_img", loader_timeout_img );

    cf.update( "use_ipv6", use_ipv6 );

    cf.update( "command_openurl", command_openurl );
    cf.update( "brownsercombo_id", browsercombo_id );

    cf.update( "refpopup_by_mo", refpopup_by_mo );
    cf.update( "namepopup_by_mo", namepopup_by_mo );
    cf.update( "idpopup_by_mo", idpopup_by_mo );

    cf.update( "imgpopup_width", imgpopup_width );
    cf.update( "imgpopup_height", imgpopup_height );
    cf.update( "use_image_view", use_image_view );
    cf.update( "use_inline_image", use_inline_image );
    cf.update( "use_mosaic", use_mosaic );
    cf.update( "zoom_to_fit", zoom_to_fit );
    cf.update( "del_img_day", del_img_day );
    cf.update( "max_img_size", max_img_size );

    cf.update( "cl_char", str_color[ COLOR_CHAR ] );
    cf.update( "cl_char_name", str_color[ COLOR_CHAR_NAME ] );
    cf.update( "cl_char_name_b", str_color[ COLOR_CHAR_NAME_B ] );
    cf.update( "cl_char_age", str_color[ COLOR_CHAR_AGE ] );
    cf.update( "cl_char_selection", str_color[ COLOR_CHAR_SELECTION ] );
    cf.update( "cl_char_highlight", str_color[ COLOR_CHAR_HIGHLIGHT ] );
    cf.update( "cl_char_bookmark", str_color[ COLOR_CHAR_BOOKMARK ] );
    cf.update( "cl_char_link", str_color[ COLOR_CHAR_LINK ] );
    cf.update( "cl_char_link_low", str_color[ COLOR_CHAR_LINK_LOW ] );
    cf.update( "cl_char_link_high", str_color[ COLOR_CHAR_LINK_HIGH ] );
    cf.update( "cl_char_message", str_color[ COLOR_CHAR_MESSAGE ] );
    cf.update( "cl_char_message_selection", str_color[ COLOR_CHAR_MESSAGE_SELECTION ] );
    cf.update( "cl_img_nocache", str_color[ COLOR_IMG_NOCACHE ] );
    cf.update( "cl_img_cached", str_color[ COLOR_IMG_CACHED ] );
    cf.update( "cl_img_loading", str_color[ COLOR_IMG_LOADING ] );
    cf.update( "cl_img_err", str_color[ COLOR_IMG_ERR ] );
    cf.update( "cl_back", str_color[ COLOR_BACK ] );
    cf.update( "cl_back_popup", str_color[ COLOR_BACK_POPUP ] );
    cf.update( "cl_back_selection", str_color[ COLOR_BACK_SELECTION ] );
    cf.update( "cl_back_highlight",str_color[ COLOR_BACK_HIGHLIGHT ]  );
    cf.update( "cl_back_highlight_tree",str_color[ COLOR_BACK_HIGHLIGHT_TREE ]  );
    cf.update( "cl_back_message", str_color[ COLOR_BACK_MESSAGE ] );
    cf.update( "cl_back_message_selection", str_color[ COLOR_BACK_MESSAGE_SELECTION ] );
    cf.update( "cl_sepa_new", str_color[ COLOR_SEPARATOR_NEW ] );
    cf.update( "cl_chr_bbs", str_color[ COLOR_CHAR_BBS ] );
    cf.update( "cl_chr_board", str_color[ COLOR_CHAR_BOARD ] );
    cf.update( "cl_back_bbs", str_color[ COLOR_BACK_BBS ] );
    cf.update( "cl_back_bbs_even", str_color[ COLOR_BACK_BBS_EVEN ] );
    cf.update( "cl_back_board", str_color[ COLOR_BACK_BOARD ] );
    cf.update( "cl_back_board_even", str_color[ COLOR_BACK_BOARD_EVEN ] );

    cf.update( "use_tree_gtkrc", use_tree_gtkrc );

    cf.update( "tree_ypad", tree_ypad );

    cf.update( "show_oldarticle", show_oldarticle );
    cf.update( "newthread_hour", newthread_hour );

    cf.update( "tree_scroll_size", tree_scroll_size );
    cf.update( "scroll_size", scroll_size );
    cf.update( "key_scroll_size", key_scroll_size );
    cf.update( "open_one_category", open_one_category );
    cf.update( "always_write_ok", always_write_ok );
    cf.update( "save_postlog", save_postlog );
    cf.update( "hide_writing_dialog", hide_writing_dialog );
    cf.update( "margin_popup", margin_popup );
    cf.update( "margin_imgpopup", margin_imgpopup );
    cf.update( "mouse_radius", mouse_radius );
    cf.update( "history_size", history_size );
    cf.update( "aahistory_size", aahistory_size );
    cf.update( "instruct_popup", instruct_popup );
    cf.update( "instruct_tglart", instruct_tglart );
    cf.update( "instruct_tglimg", instruct_tglimg );

    cf.update( "adjust_underline_pos", adjust_underline_pos );
    cf.update( "adjust_line_space", adjust_line_space );

    cf.update( "draw_underline", draw_underline );
    cf.update( "strict_char_width", strict_char_width );
    cf.update( "loose_url", loose_url );

    cf.update( "hide_usrcmd", hide_usrcmd );
    cf.update( "max_show_usrcmd", max_show_usrcmd );
    cf.update( "reload_allthreads", reload_allthreads );

    cf.update( "tab_min_str", tab_min_str );

    cf.update( "show_tab_icon", show_tab_icon );

    // スレあぼーん情報
    std::string str_abone_word_thread = MISC::listtostr( list_abone_word_thread );
    std::string str_abone_regex_thread = MISC::listtostr( list_abone_regex_thread );

    cf.update( "abonewordthread", str_abone_word_thread );
    cf.update( "aboneregexthread", str_abone_regex_thread );

    // あぼーん情報
    std::string str_abone_name = MISC::listtostr( list_abone_name );
    std::string str_abone_word = MISC::listtostr( list_abone_word );
    std::string str_abone_regex = MISC::listtostr( list_abone_regex );

    cf.update( "abonename", str_abone_name );
    cf.update( "aboneword", str_abone_word );
    cf.update( "aboneregex", str_abone_regex );

    cf.update( "abone_transparent", abone_transparent );
    cf.update( "abone_chain", abone_chain );

    cf.update( "expand_sidebar", expand_sidebar );

#ifdef HAVE_MIGEMO_H
    cf.update( "migemodict_path", migemodict_path );
#endif

    cf.save();
}
