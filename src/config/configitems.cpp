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
#include "jdlib/jdregex.h"
#include "skeleton/msgdiag.h"

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

    // 上の方が優先順位が高い
#ifdef _WIN32
    IS_DEFAULT_FONT( "MS PGothic" );
    IS_DEFAULT_FONT( "MS UI Gothic" );
#endif
    IS_DEFAULT_FONT( "IPA モナー Pゴシック" );
    IS_DEFAULT_FONT( "IPAMonaPGothic" );
    IS_DEFAULT_FONT( "Mona-VLGothic" );
    IS_DEFAULT_FONT( "Mona" );
    IS_DEFAULT_FONT( "Konatu" );
    IS_DEFAULT_FONT( "VL Pゴシック" );
    IS_DEFAULT_FONT( "VL PGothic" );
    IS_DEFAULT_FONT( "さざなみゴシック" );
    IS_DEFAULT_FONT( "Sazanami Gothic" );
    IS_DEFAULT_FONT( "Sans" );

    return std::string();
}


////////////////////////////////////////


ConfigItems::ConfigItems()
    : fontname( FONT_NUM )
    , str_color( COLOR_NUM )
{}

ConfigItems::~ConfigItems() noexcept = default;


// 設定読み込み
bool ConfigItems::load( const bool restore )
{
    // restoreモード
    const std::string path_conf = restore ? CACHE::path_conf_bkup() : CACHE::path_conf();

    JDLIB::ConfLoader cf( path_conf, std::string() );

#ifdef _DEBUG
    std::cout << "ConfigItems::load" << std::endl
              << "conffile = " << path_conf << " empty = " << cf.empty() << std::endl;
#endif

    std::string str_tmp;

    // 色
    set_colors( cf );

    // 前回開いたviewを復元するか
    restore_board = cf.get_option_bool( "restore_board", CONF_RESTORE_BOARD );
    restore_article = cf.get_option_bool( "restore_article", CONF_RESTORE_ARTICLE );
    restore_image = cf.get_option_bool( "restore_image", CONF_RESTORE_IMAGE );

    // 自前でウィンドウ配置を管理する
    manage_winpos = cf.get_option_bool( "manage_winpos", CONF_MANAGE_WINPOS );

    // フォント
    set_fonts( cf );

    // レスを参照するときに前に付ける文字
    ref_prefix = cf.get_option_str( "ref_prefix", CONF_REF_PREFIX, 16 );

    // 参照文字( ref_prefix ) の後のスペースの数
    // JDLIB::ConfLoader の中で MISC::remove_space() が呼ばれて空白が消えるので別設定とした
    ref_prefix_space = cf.get_option_int( "ref_prefix_space", CONF_REF_PREFIX_SPACE, 0, 16 );
    for( int i = 0; i < ref_prefix_space; ++i ) ref_prefix_space_str += " ";

    // レスにアスキーアートがあると判定する正規表現
    regex_res_aa = cf.get_option_str( "regex_res_aa", CONF_REGEX_RES_AA );

    // 読み込み用プロクシとポート番号
    use_proxy_for2ch = cf.get_option_bool( "use_proxy_for2ch", CONF_USE_PROXY_FOR2CH );
    send_cookie_to_proxy_for2ch = cf.get_option_bool( "send_cookie_to_proxy_for2ch", CONF_SEND_COOKIE_TO_PROXY_FOR2CH );
    str_tmp = cf.get_option_str( "proxy_for2ch", "" );
    set_proxy_for2ch( str_tmp );
    proxy_port_for2ch = cf.get_option_int( "proxy_port_for2ch", CONF_PROXY_PORT_FOR2CH, 1, 65535 );

    // 書き込み用プロクシとポート番号
    use_proxy_for2ch_w = cf.get_option_bool( "use_proxy_for2ch_w", CONF_USE_PROXY_FOR2CH_W );
    send_cookie_to_proxy_for2ch_w = cf.get_option_bool( "send_cookie_to_proxy_for2ch_w",
                                                        CONF_SEND_COOKIE_TO_PROXY_FOR2CH_W );
    str_tmp = cf.get_option_str( "proxy_for2ch_w", "" );
    set_proxy_for2ch_w( str_tmp );
    proxy_port_for2ch_w = cf.get_option_int( "proxy_port_for2ch_w", CONF_PROXY_PORT_FOR2CH_W, 1, 65535 );

    // 2chの外にアクセスするときのプロクシとポート番号
    use_proxy_for_data = cf.get_option_bool( "use_proxy_for_data", CONF_USE_PROXY_FOR_DATA );
    send_cookie_to_proxy_for_data = cf.get_option_bool( "send_cookie_to_proxy_for_data",
                                                        CONF_SEND_COOKIE_TO_PROXY_FOR_DATA );
    str_tmp = cf.get_option_str( "proxy_for_data", "" );
    set_proxy_for_data( str_tmp );
    proxy_port_for_data = cf.get_option_int( "proxy_port_for_data", CONF_PROXY_PORT_FOR_DATA, 1, 65535 );

    // 2ch にアクセスするときのエージェント名
    agent_for2ch = cf.get_option_str( "agent_for2ch", CONF_AGENT_FOR2CH );

    // 2ch外にアクセスするときのエージェント名
    agent_for_data = cf.get_option_str( "agent_for_data", CONF_AGENT_FOR_DATA );

    // 2ch にログインするときのX-2ch-UA
    x_2ch_ua = cf.get_option_str( "x_2ch_ua", CONF_X_2CH_UA );

    // ローダのバッファサイズ
    loader_bufsize = cf.get_option_int( "loader_bufsize", CONF_LOADER_BUFSIZE, 1, 4096 ); // 一般
    loader_bufsize_board = cf.get_option_int( "loader_bufsize_board", CONF_LOADER_BUFSIZE_BOARD, 1, 4096 ); // スレ一覧用

    // ローダのタイムアウト値
    loader_timeout = cf.get_option_int( "loader_timeout", CONF_LOADER_TIMEOUT, 1, 120 );
    loader_timeout_post = cf.get_option_int( "loader_timeout_post", CONF_LOADER_TIMEOUT_POST, 1, 120  ); // ポスト
    loader_timeout_img = cf.get_option_int( "loader_timeout_img", CONF_LOADER_TIMEOUT_IMG, 1, 120 ); // 画像
    loader_timeout_checkupdate = cf.get_option_int( "loader_timeout_checkupdate", CONF_LOADER_TIMEOUT_CHECKUPDATE, 1, 120 ); // 更新チェック

    // ipv6使用
    use_ipv6 = cf.get_option_bool( "use_ipv6", CONF_USE_IPV6 );

    // 同一ホストに対する最大コネクション数( 1 または 2 )
    connection_num = cf.get_option_int( "connection_num", CONF_CONNECTION_NUM, 1, 2 );

    // 2chのクッキーを保存する (互換性のため設定名は旧名称を使う)
    use_cookie_hap = cf.get_option_bool( "use_cookie_hap", CONF_USE_COOKIE_HAP );

    // 2chのクッキー (互換性のため設定名は旧名称を使う)
    cookie_hap = cf.get_option_str( "cookie_hap", CONF_COOKIE_HAP );
    cookie_hap_bbspink = cf.get_option_str( "cookie_hap_bbspink", CONF_COOKIE_HAP_BBSPINK );

    // ブラウザ設定ダイアログのコンボボックスの番号
    browsercombo_id = cf.get_option_int( "browsercombo_id", CONF_BROWSER_NO, 0, CORE::get_browser_number() -1 );

    // リンクをクリックしたときに実行するコマンド
    command_openurl = cf.get_option_str( "command_openurl", CORE::get_browser_name( CONF_BROWSER_NO ) );

    // レス番号の上にマウスオーバーしたときに参照ポップアップ表示する
    refpopup_by_mo = cf.get_option_bool( "refpopup_by_mo", CONF_REFPOPUP_BY_MO );

    // 名前の上にマウスオーバーしたときにポップアップ表示する
    namepopup_by_mo = cf.get_option_bool( "namepopup_by_mo", CONF_NAMEPOPUP_BY_MO );

    // IDの上にマウスオーバーしたときにIDをポップアップ表示する
    idpopup_by_mo = cf.get_option_bool( "idpopup_by_mo", CONF_IDPOPUP_BY_MO );

    // 画像のスムージングレベル(0-2, 2が最も高画質かつ低速)
    imgemb_interp = cf.get_option_int( "imgemb_interp", CONF_IMGEMB_INTERP, 0, 2 );
    imgmain_interp = cf.get_option_int( "imgmain_interp", CONF_IMGMAIN_INTERP, 0, 2 );
    imgpopup_interp = cf.get_option_int( "imgpopup_interp", CONF_IMGPOPUP_INTERP, 0, 2 );

    // 画像ポップアップサイズ
    imgpopup_width = cf.get_option_int( "imgpopup_width", CONF_IMGPOPUP_WIDTH, 16, 8192 );
    imgpopup_height = cf.get_option_int( "imgpopup_height", CONF_IMGPOPUP_HEIGHT, 16, 8192 );

    // 画像ポップアップを使用する
    use_image_popup = cf.get_option_bool( "use_image_popup", CONF_USE_IMAGE_POPUP );

    // 画像ビューを使用する
    use_image_view = cf.get_option_bool( "use_image_view", CONF_USE_IMAGE_VIEW );

    // インライン画像を表示する
    use_inline_image = cf.get_option_bool( "use_inline_image", CONF_INLINE_IMG );

    // ssspアイコン表示
    show_ssspicon = cf.get_option_bool( "show_ssspicon", CONF_SHOW_SSSPICON );

    // インライン画像の最大幅と高さ
    embimg_width = cf.get_option_int( "embimg_width", CONF_EMBIMG_WIDTH, 16, 8192 );
    embimg_height = cf.get_option_int( "embimg_height", CONF_EMBIMG_HEIGHT, 16, 8192 );

    // 埋め込み画像ビューを閉じたときにタブも閉じる
    hide_imagetab = cf.get_option_bool( "hide_imagetab", CONF_HIDE_IMAGETAB );

    // 画像ビューでdeleteを押したときに確認ダイアログを表示する
    show_delimgdiag = cf.get_option_bool( "show_delimgdiag", CONF_SHOW_DELIMGDIAG );

    // 画像にモザイクをかける
    use_mosaic = cf.get_option_bool( "use_mosaic", CONF_USE_MOSAIC );

    // モザイクの大きさ
    mosaic_size = cf.get_option_int( "mosaic_size", CONF_MOSAIC_SIZE, 1, 1024 );

    // 画像をデフォルトでウィンドウサイズに合わせる
    zoom_to_fit = cf.get_option_bool( "zoom_to_fit", CONF_ZOOM_TO_FIT );

    // 画像キャッシュ削除の日数
    del_img_day = cf.get_option_int( "del_img_day", CONF_DEL_IMG_DAY, 0, 65535 );

    // 画像あぼーん削除の日数
    del_imgabone_day = cf.get_option_int( "del_imgabone_day", CONF_DEL_IMGABONE_DAY, 0, 65535 );

    // ダウンロードする画像の最大ファイルサイズ(Mbyte)
    max_img_size = cf.get_option_int( "max_img_size", CONF_MAX_IMG_SIZE, 1, 1024 );

    // 画像の最大サイズ(Mピクセル)
    max_img_pixel = cf.get_option_int( "max_img_pixel", CONF_MAX_IMG_PIXEL, 1, 1024 );

    // 画像のメモリキャッシュ枚数
    imgcache_size = cf.get_option_int( "imgcache_size", CONF_IMGCACHE_SIZE, 0, 20 );

    // JD ホームページのアドレス
    url_jdhp = cf.get_option_str( "url_jdhp", CONF_URL_JDHP );

    // JDim ホームページのアドレス
    url_jdimhp = cf.get_option_str( "url_jdimhp", CONF_URL_JDIMHP );

    // 2chの認証サーバのアドレス
    url_login2ch = cf.get_option_str( "url_login2ch", CONF_LOGIN2CH );

    // BEの認証サーバのアドレス
    url_loginbe = cf.get_option_str( "url_loginbe", CONF_LOGINBE );

    // bbsmenu.htmlのURL
    url_bbsmenu = cf.get_option_str( "url_bbsmenu", CONF_URL_BBSMENU );

    // bbsmenu.html内にあるリンクは全て板とみなす
    use_link_as_board = cf.get_option_bool( "use_link_as_board", CONF_USE_LINK_AS_BOARD );

    // 板移転時に確認ダイアログを表示する
    show_movediag = cf.get_option_bool( "show_movediag", CONF_SHOW_MOVEDIAG );

    // スレタイ検索用メニュータイトルアドレス
    menu_search_title = cf.get_option_str( "menu_search_title", CONF_MENU_SEARCH_TITLE );
    url_search_title = cf.get_option_str( "url_search_title", CONF_URL_SEARCH_TITLE );

    // スレタイ検索用正規表現
    regex_search_title = cf.get_option_str( "regex_search_title", CONF_REGEX_SEARCH_TITLE );

    // web検索用メニュータイトルアドレス
    menu_search_web = cf.get_option_str( "menu_search_web", CONF_MENU_SEARCH_WEB );
    url_search_web = cf.get_option_str( "url_search_web", CONF_URL_SEARCH_WEB );

    // 書き込みビューでGTKテーマの設定を使用するか (GTK3版のみ)
    use_message_gtktheme = cf.get_option_bool( "use_message_gtktheme", CONF_USE_MESSAGE_GTKTHEME );

    // ツリービューでgtkrcの設定を使用するか
    use_tree_gtkrc = cf.get_option_bool( "use_tree_gtkrc", CONF_USE_TREE_GTKRC );

    // スレビューの選択色でgtkrcの設定を使用するか
    use_select_gtkrc = cf.get_option_bool( "use_select_gtkrc", CONF_USE_SELECT_GTKRC );

    // ツリービューの行間スペース
    tree_ypad = cf.get_option_int( "tree_ypad", CONF_TREE_YPAD, 0, 64 );

    // ツリービューにエクスパンダを表示
    tree_show_expanders = cf.get_option_bool( "tree_show_expanders", CONF_TREE_SHOW_EXPANDERS );

    // ツリービューのレベルインデント調整量(ピクセル)
    tree_level_indent = cf.get_option_int( "tree_level_indent", CONF_TREE_LEVEL_INDENT, -256, 256 );

    // カテゴリやディレクトリを開いたときにツリービューをスクロールする
    scroll_tree = cf.get_option_bool( "scroll_tree", CONF_SCROLL_TREE );

    // ツリービューの選択を表示中のビューと同期する
    select_item_sync = cf.get_option_int( "select_item_sync", CONF_SELECT_ITEM_SYNC, 0, 2 );

    // 各ビューと枠との間の余白
    view_margin = cf.get_option_int( "view_margin", CONF_VIEW_MARGIN, 0, 64 );

    // スクロールバーを左に配置
    left_scrbar = cf.get_option_bool( "left_scrbar", CONF_LEFT_SCRBAR );

    // スレ一覧で古いスレも表示
    show_oldarticle = cf.get_option_bool( "show_oldarticle", CONF_SHOW_OLDARTICLE );

    // スレ一覧で指定した値(時間)よりも後に立てられたスレを新着とみなす
    newthread_hour = cf.get_option_int( "newthread_hour", CONF_NEWTHREAD_HOUR, 1, 65535 );

    // スレ一覧でインクリメント検索をする
    inc_search_board = cf.get_option_bool( "inc_search_board", CONF_INC_SEARCH_BOARD );

    // スレ一覧でdeleteを押したときに確認ダイアログを表示する
    show_deldiag = cf.get_option_bool( "show_deldiag", CONF_SHOW_DELDIAG );

    // スレ一覧をロードする前にキャッシュにある一覧を表示
    show_cached_board = cf.get_option_bool( "show_cached_board", CONF_SHOW_CACHED_BOARD );

    // スレ一覧でお知らせスレ(924)のアイコンを表示する
    show_924 = cf.get_option_bool( "show_924", CONF_SHOW_924 );
 
    // ツリービューのスクロール量(行数)
    tree_scroll_size = cf.get_option_int( "tree_scroll_size", CONF_TREE_SCROLL_SIZE, 1, 64 );

    // スレビューのスクロール量
    scroll_size = cf.get_option_int( "scroll_size", CONF_SCROLL_SIZE, 1, 64 );

    // スレビューのスクロール量(キー上下)
    key_scroll_size = cf.get_option_int( "key_scroll_size", CONF_KEY_SCROLL_SIZE, 1, 64 );

    // スレビューの高速スクロール量(キー上下・ ページ高 - 行高 * key_fastscroll_size )
    key_fastscroll_size = cf.get_option_int( "key_fastscroll_size", CONF_KEY_FASTSCROLL_SIZE, 1, 64 );

    // スレビューでリロード後に一番下までスクロール
    jump_after_reload = cf.get_option_bool( "jump_after_reload", CONF_JUMP_AFTER_RELOAD );

    // スレビューでリロード後に新着までスクロール
    jump_new_after_reload = cf.get_option_bool( "jump_new_after_reload", CONF_JUMP_NEW_AFTER_RELOAD );

    // 実況モード
    live_mode = cf.get_option_int( "live_mode", LIVE_SCRMODE_VARIABLE, 0, LIVE_SCRMODE_NUM -1 );

    // 実況速度
    live_speed = cf.get_option_int( "live_speed", CONF_LIVE_SPEED, 0, 50 );

    // 実況のスクロールモードを切り替えるしきい値
    live_threshold = cf.get_option_int( "live_threshold", CONF_LIVE_THRESHOLD, 1, 1024 );

    // 板一覧でカテゴリを常にひとつだけ開く
    open_one_category = cf.get_option_bool( "open_one_category", CONF_OPEN_ONE_CATEGORY );

    // お気に入りでディレクトリを常にひとつだけ開く
    open_one_favorite = cf.get_option_bool( "open_one_favorite", CONF_OPEN_ONE_FAVORITE );

    // デフォルトの書き込み名
    write_name = cf.get_option_str( "write_name", CONF_WRITE_NAME );

    // デフォルトのメールアドレス
    write_mail = cf.get_option_str( "write_mail", CONF_WRITE_MAIL );

    // 書き込み時に書き込み確認ダイアログを出さない
    always_write_ok = cf.get_option_bool( "always_write_ok", CONF_ALWAYS_WRITE_OK );

    // 書き込みログを保存
    save_postlog = cf.get_option_bool( "save_postlog", CONF_SAVE_POSTLOG );

    // 書き込みログの最大サイズ
    maxsize_postlog = cf.get_option_int( "maxsize_postlog", CONF_MAXSIZE_POSTLOG, 1024, 1024 * 1024 );

    // 書き込み履歴を保存
    save_posthist = cf.get_option_bool( "save_posthist", CONF_SAVE_POSTHIST );

    // 「書き込み中」のダイアログを表示しない
    hide_writing_dialog = cf.get_option_bool( "hide_writing_dialog", CONF_HIDE_WRITING_DIALOG );

    // 編集中のメッセージの保存確認ダイアログを表示する
    show_savemsgdiag = cf.get_option_bool( "show_savemsgdiag", CONF_SHOW_SAVEMSGDIAG );

    // 書き込みビューでテキストを折り返す
    message_wrap = cf.get_option_bool( "message_wrap", CONF_MESSAGE_WRAP );

    // 非アクティブ時に書き込みビューを折りたたむ
    fold_message = cf.get_option_bool( "fold_message", CONF_FOLD_MESSAGE );

    // 非アクティブ時に画像ビューを折りたたむ
    fold_image = cf.get_option_bool( "fold_image", CONF_FOLD_IMAGE );

    // 書き込み欄の日本語のON/OFF状態を保存
    keep_im_status = cf.get_option_bool( "keep_im_status", CONF_KEEP_IM_STATUS );

    // ポップアップとカーソルの間のマージン
    margin_popup = cf.get_option_int( "margin_popup", CONF_MARGIN_POPUP, 0, 1024 );

    // 画像ポップアップとカーソルの間のマージン
    margin_imgpopup_x = cf.get_option_int( "margin_imgpopup_x", CONF_MARGIN_IMGPOPUP_X, 0, 1024 );
    margin_imgpopup = cf.get_option_int( "margin_imgpopup", CONF_MARGIN_IMGPOPUP, 0, 1024 );

    // ポップアップが消えるまでの時間(ミリ秒)
    hide_popup_msec = cf.get_option_int( "hide_popup_msec", CONF_HIDE_POPUP_MSEC, 0, 2000 );

    // マウスジェスチャを有効
    enable_mg = cf.get_option_bool( "enable_mg", CONF_ENABLE_MG );

    // マウスジェスチャの判定開始半径
    mouse_radius = cf.get_option_int( "mouse_radius", CONF_MOUSE_RADIUS, 4, 1024 );

    // 数字入力ジャンプの待ち時間(ミリ秒)
    numberjmp_msec = cf.get_option_int( "numberjmp_msec", CONF_NUMBERJMP_MSEC, 1, 5000 );

    // 履歴メニューの表示数
    history_size = cf.get_option_int( "history_size", CONF_HISTORY_SIZE, 0, 256 );

    // 履歴ビューの表示数
    historyview_size = cf.get_option_int( "historyview_size", CONF_HISTORYVIEW_SIZE, 0, 1024 );

    // AA履歴の保持数
    aahistory_size = cf.get_option_int( "aahistory_size", CONF_AAHISTORY, 0, 65535 );

    // 0以上なら多重ポップアップの説明を表示する
    instruct_popup = cf.get_option_int( "instruct_popup", CONF_INSTRUCT_POPUP, 0, CONF_INSTRUCT_POPUP );

    // スレビューを開いたときにスレ一覧との切り替え方法を説明する
    instruct_tglart = cf.get_option_bool( "instruct_tglart", CONF_INSTRUCT_TGLART );
    instruct_tglart_end = false;

    // 画像ビューを開いたときにスレビューとの切り替え方法を説明する
    instruct_tglimg = cf.get_option_bool( "instruct_tglimg", CONF_INSTRUCT_TGLIMG );
    instruct_tglimg_end = false;

    // スレビューでdeleteを押したときに確認ダイアログを表示する
    show_delartdiag = cf.get_option_bool( "show_delartdiag", CONF_SHOW_DELARTDIAG );

    // 下線位置
    adjust_underline_pos = cf.get_option_double( "adjust_underline_pos", ( double )CONF_ADJUST_UNDERLINE_POS, ( double )0, ( double )64 );

    // 行間スペース
    adjust_line_space = cf.get_option_double( "adjust_line_space", ( double )CONF_ADJUST_LINE_SPACE, ( double )0, ( double )64 );

    // リンク下線を表示
    draw_underline = cf.get_option_bool( "draw_underline", CONF_DRAW_UNDERLINE );

    // スレビューで文字幅の近似を厳密にする
    strict_char_width = cf.get_option_bool( "strict_char_width", CONF_STRICT_CHAR_WIDTH );

    // スレビューで発言数(ID)をカウントする
    check_id = cf.get_option_bool( "check_id", CONF_CHECK_ID );

    // レス参照で色を変える回数
    num_reference_high = cf.get_option_int( "num_reference_high", CONF_NUM_REFERENCE_HIGH, 1, 256 );
    num_reference_low = cf.get_option_int( "num_reference_low", CONF_NUM_REFERENCE_LOW, 1, 128 );

    // 発言数で色を変える回数
    num_id_high = cf.get_option_int( "num_id_high", CONF_NUM_ID_HIGH, 1, 256 );
    num_id_low = cf.get_option_int( "num_id_low", CONF_NUM_ID_LOW, 1, 128 );

    // datのパース時にURL判定を甘くする(^なども含める)
    loose_url = cf.get_option_bool( "loose_url", CONF_LOOSE_URL );

    // ユーザーコマンドで選択できない項目を非表示にする
    hide_usrcmd = cf.get_option_bool( "hide_usrcmd", CONF_HIDE_USRCMD );

    // スレビューで再読み込みボタンを押したときに全タブを更新する
    reload_allthreads = cf.get_option_bool( "reload_allthreads", CONF_RELOAD_ALLTHREAD );

    // タブに表示する文字列の最小値
    tab_min_str = cf.get_option_int( "tab_min_str", CONF_TAB_MIN_STR, 1, 256 );

    // タブにアイコンを表示するか
    show_tab_icon = cf.get_option_bool( "show_tab_icon", CONF_SHOW_TAB_ICON );

    // タブ上でマウスホイールを回転してタブを切り替える
    switchtab_wheel = cf.get_option_bool( "switchtab_wheel", CONF_SWITCHTAB_WHEEL );

    // 他のビューを開くときのタブの位置 ( 0: 一番右端 1:右隣 2:左隣 )
    newtab_pos = cf.get_option_int( "newtab_pos", CONF_NEWTAB_POS, 0, 2 );

    // ツリービューで選択したビューを開くときのタブの位置 ( 0: 一番右端 1:右隣 2:左隣 )
    opentab_pos = cf.get_option_int( "opentab_pos", CONF_OPENTAB_POS, 0, 2 );

    // 次スレ検索を開くときのタブの位置 ( 0: 次スレ検索タブ 1:新しいタブ 2:アクティブなタブを置き換え -1:2.8.5以前の動作 )
    boardnexttab_pos = cf.get_option_int( "boardnexttab_pos", CONF_BOARDNEXTTAB_POS, -1, 2 );

    // スレビューに書き込みマークを表示するか
    show_post_mark = cf.get_option_bool( "show_post_mark", CONF_SHOW_POST_MARK );

    // ボタンをフラットにするか
    flat_button = cf.get_option_bool( "flat_button", CONF_FLAT_BUTTON );

    // ツールバーの背景描画
    draw_toolbarback = cf.get_option_bool( "draw_toolbarback", CONF_DRAW_TOOLBARBACK );

    std::list< std::string > list_tmp;
    std::list< std::string >::iterator it_tmp;

    // スレ あぼーん word
    str_tmp = cf.get_option_str( "abonewordthread", "" );
    if( ! str_tmp.empty() ) list_abone_word_thread = MISC::strtolist( str_tmp );

    // スレ あぼーん regex
    str_tmp = cf.get_option_str( "aboneregexthread", "" );
    if( ! str_tmp.empty() ) list_abone_regex_thread = MISC::strtolist( str_tmp );

    // dat落ちしたスレをNGスレタイトルリストから取り除くか( 0: ダイアログ表示 1: 取り除く 2: 除かない )
    remove_old_abone_thread = cf.get_option_int( "remove_old_abone_thread", CONF_REMOVE_OLD_ABONE_THREAD, 0, 2 );

    // スレ あぼーん( レス数 )
    abone_number_thread = cf.get_option_int( "abone_number_thread", CONF_ABONE_NUMBER_THREAD, 0, 9999 );

    // スレ あぼーん( スレ立てからの経過時間 )
    abone_hour_thread = cf.get_option_int( "abone_hour_thread", CONF_ABONE_HOUR_THREAD, 0, 9999 );

    // あぼーん name
    str_tmp = cf.get_option_str( "abonename", "" );
    if( ! str_tmp.empty() ) list_abone_name = MISC::strtolist( str_tmp );

    // あぼーん word
    str_tmp = cf.get_option_str( "aboneword", "" );
    if( ! str_tmp.empty() ) list_abone_word = MISC::strtolist( str_tmp );

    // あぼーん regex
    str_tmp = cf.get_option_str( "aboneregex", "" );
    if( ! str_tmp.empty() ) list_abone_regex = MISC::strtolist( str_tmp );

    // デフォルトで透明、連鎖あぼーんをするか
    abone_transparent = cf.get_option_bool( "abone_transparent", CONF_ABONE_TRANSPARENT );
    abone_chain = cf.get_option_bool( "abone_chain", CONF_ABONE_CHAIN );

    // NG正規表現によるあぼーん時に大小と全半角文字の違いを無視
    abone_icase = cf.get_option_bool( "abone_icase", CONF_ABONE_ICASE );
    abone_wchar = cf.get_option_bool( "abone_wchar", CONF_ABONE_WCHAR );

    // 右ペーンが空の時にサイドバーを閉じるか
    expand_sidebar = cf.get_option_bool( "expand_sidebar", CONF_EXPAND_SIDEBAR );

    // 3ペーン時にスレ一覧やスレビューを最大化する
    expand_rpane = cf.get_option_bool( "expand_rpane", CONF_EXPAND_RPANE );

    // ペーンの境界をクリックしてサイドバーを開け閉めする
    open_sidebar_by_click = cf.get_option_bool( "open_sidebar_by_click", CONF_OPEN_SIDEBAR_BY_CLICK );

    // 次スレ検索の類似度のしきい値
    threshold_next = cf.get_option_int( "threshold_next", CONF_THRESHOLD_NEXT, 1, 10 );

    // 次スレを開いたときにお気に入りのアドレスと名前を自動更新
    replace_favorite_next = cf.get_option_int( "replace_favorite_next", CONF_REPLACE_FAVORITE_NEXT, 0, 2 );

    // お気に入りの自動更新をするかダイアログを出す
    show_diag_replace_favorite = cf.get_option_bool( "show_diag_replace_favorite", CONF_SHOW_DIAG_REPLACE_FAVORITE );

    // スレをお気に入りに追加したときにしおりをセットする
    bookmark_drop = cf.get_option_bool( "bookmark_drop", CONF_BOOKMARK_DROP );

    // お気に入りの更新チェック時に板の更新もチェックする
    check_update_board  = cf.get_option_bool( "check_update_board", CONF_CHECK_UPDATE_BOARD );

    // 起動時にお気に入りを自動でチェックする
    check_update_boot = cf.get_option_bool( "check_update_boot", CONF_CHECK_UPDATE_BOOT );

    // お気に入り登録時に重複項目を登録するか ( 0: 登録する 1: ダイアログ表示  2: 登録しない )
    check_favorite_dup = cf.get_option_int( "check_favorite_dup", CONF_CHECK_FAVORITE_DUP, 0, 2 );

    // お気に入り登録時に挿入先ダイアログを表示する ( 0 : 表示する 1: 表示せず先頭に追加 2: 表示せず最後に追加 )
    show_favorite_select_diag = cf.get_option_int( "show_favorite_select_diag", CONF_SHOW_FAVORITE_SELECT_DIAG, 0, 2 );

    // Ctrl+qでウィンドウを閉じない
    disable_close = cf.get_option_bool( "disable_close", CONF_DISABLE_CLOSE );

    // メニューバーを非表示にした時にダイアログを表示
    show_hide_menubar_diag = cf.get_option_bool( "show_hide_menubar_diag", CONF_SHOW_HIDE_MENUBAR_DIAG );

    // 状態変更時にメインステータスバーの色を変える
    change_stastatus_color = cf.get_option_bool( "change_stastatus_color", CONF_CHANGE_STASTATUS_COLOR );

    // まちBBSの取得に offlaw.cgi を使用する
    use_machi_offlaw = cf.get_option_bool( "use_machi_offlaw", CONF_USE_MACHI_OFFLAW );

    // 書き込み履歴のあるスレを削除する時にダイアログを表示
    show_del_written_thread_diag = cf.get_option_bool( "show_del_written_thread_diag", CONF_SHOW_DEL_WRITTEN_THREAD_DIAG );

    // スレを削除する時に画像キャッシュも削除する ( 0: ダイアログ表示 1: 削除 2: 削除しない )
    delete_img_in_thread = cf.get_option_int( "delete_img_in_thread", CONF_DELETE_IMG_IN_THREAD, 0, 2 );

    //最大表示可能レス数
    max_resnumber = cf.get_option_int( "max_resnumber", CONF_MAX_RESNUMBER, 1, std::numeric_limits< int >::max() - 1 );

    // FIFOの作成などにエラーがあったらダイアログを表示する
    show_diag_fifo_error = cf.get_option_bool( "show_diag_fifo_error", CONF_SHOW_DIAG_FIFO_ERROR );

    // 指定した分ごとにセッションを自動保存 (0: 保存しない)
    save_session = cf.get_option_int( "save_session", CONF_SAVE_SESSION, 0, (24*60) );

#ifdef HAVE_MIGEMO_H
    // migemo-dictの場所
    migemodict_path = cf.get_option_str( "migemodict_path", CONF_MIGEMO_PATH );
#endif

    m_loaded = true;

    // 設定値に壊れている物がある
    if( cf.is_broken() )
    {
        std::string msg;
        Gtk::MessageDialog *mdiag;

        // 非resotreモードでバックアップが存在する
        if( ! restore && CACHE::file_exists( CACHE::path_conf_bkup() ) == CACHE::EXIST_FILE )
        {
            msg = "設定ファイル (" + path_conf + ")に異常な値があります。全設定をバックアップから復元しますか？";
            mdiag = new Gtk::MessageDialog( msg, false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_YES_NO );
            int ret = mdiag->run();
            delete mdiag;
            if( ret != Gtk::RESPONSE_YES ) load( true ); // resotreモードで再帰
        }
        else
        {
            msg = "設定ファイル (" + path_conf + ")に異常な値があるため、一部をデフォルトに設定しました。";
            mdiag = new Gtk::MessageDialog( msg );
            mdiag->run();
            delete mdiag;
        }
    }
    // 正常ならバックアップ
    else if( CACHE::file_exists( CACHE::path_root() ) == CACHE::EXIST_DIR )
    {
        save_impl( CACHE::path_conf_bkup() );
    }

    return ! cf.empty();
}


// 保存
void ConfigItems::save()
{
    save_impl( CACHE::path_conf() );
}


void ConfigItems::save_impl( const std::string& path )
{
    if( ! m_loaded ) return;

#ifdef _DEBUG
    std::cout << "ConfigItems::save_impl path = " << path << std::endl;
#endif

    JDLIB::ConfLoader cf( path, std::string() );

    cf.update( "restore_board", restore_board );
    cf.update( "restore_article", restore_article );
    cf.update( "restore_image", restore_image );
    cf.update( "manage_winpos", manage_winpos );
    cf.update( "url_jdhp", url_jdhp );
    cf.update( "url_jdimhp", url_jdimhp );
    cf.update( "url_login2ch", url_login2ch );
    cf.update( "url_loginbe", url_loginbe );
    cf.update( "url_bbsmenu", url_bbsmenu );
    cf.update( "use_link_as_board", use_link_as_board );
    cf.update( "show_movediag", show_movediag );
    cf.update( "menu_search_title", menu_search_title );
    cf.update( "url_search_title", url_search_title );
    cf.update( "regex_search_title", regex_search_title );
    cf.update( "menu_search_web", menu_search_web );
    cf.update( "url_search_web", url_search_web );

    cf.update( "fontname_main", fontname[ FONT_MAIN ] );
    cf.update( "fontname_mail", fontname[ FONT_MAIL ] );
    cf.update( "fontname_popup", fontname[ FONT_POPUP ] );
    cf.update( "fontname_aa", fontname[ FONT_AA ] );
    cf.update( "fontname_bbs", fontname[ FONT_BBS ] );
    cf.update( "fontname_board", fontname[ FONT_BOARD ] );
    cf.update( "fontname_message", fontname[ FONT_MESSAGE ] );

    cf.update( "ref_prefix", ref_prefix );
    cf.update( "ref_prefix_space", ref_prefix_space );

    cf.update( "regex_res_aa", regex_res_aa );

    cf.update( "agent_for2ch", agent_for2ch );

    std::string tmp_proxy;
    if( proxy_basicauth_for2ch.empty() ) tmp_proxy = proxy_for2ch;
    else tmp_proxy = proxy_basicauth_for2ch + "@" + proxy_for2ch;
    cf.update( "use_proxy_for2ch", use_proxy_for2ch );
    cf.update( "send_cookie_to_proxy_for2ch", send_cookie_to_proxy_for2ch );
    cf.update( "proxy_for2ch", tmp_proxy );
    cf.update( "proxy_port_for2ch", proxy_port_for2ch );

    if( proxy_basicauth_for2ch_w.empty() ) tmp_proxy = proxy_for2ch_w;
    else tmp_proxy = proxy_basicauth_for2ch_w + "@" + proxy_for2ch_w;
    cf.update( "use_proxy_for2ch_w", use_proxy_for2ch_w );
    cf.update( "send_cookie_to_proxy_for2ch_w", send_cookie_to_proxy_for2ch_w );
    cf.update( "proxy_for2ch_w", tmp_proxy );
    cf.update( "proxy_port_for2ch_w", proxy_port_for2ch_w );

    cf.update( "agent_for_data", agent_for_data );

    if( proxy_basicauth_for_data.empty() ) tmp_proxy = proxy_for_data;
    else tmp_proxy = proxy_basicauth_for_data + "@" + proxy_for_data;
    cf.update( "use_proxy_for_data", use_proxy_for_data );
    cf.update( "send_cookie_to_proxy_for_data", send_cookie_to_proxy_for_data );
    cf.update( "proxy_for_data", tmp_proxy );
    cf.update( "proxy_port_for_data", proxy_port_for_data );

    cf.update( "x_2ch_ua", x_2ch_ua );

    cf.update( "loader_bufsize", loader_bufsize );
    cf.update( "loader_bufsize_board", loader_bufsize_board );

    cf.update( "loader_timeout", loader_timeout );
    cf.update( "loader_timeout_post", loader_timeout_post );
    cf.update( "loader_timeout_img", loader_timeout_img );
    cf.update( "loader_timeout_checkupdate", loader_timeout_checkupdate );

    cf.update( "use_ipv6", use_ipv6 );
    cf.update( "connection_num", connection_num );

    cf.update( "use_cookie_hap", use_cookie_hap );
    cf.update( "cookie_hap", cookie_hap );
    cf.update( "cookie_hap_bbspink", cookie_hap_bbspink );

    cf.update( "command_openurl", command_openurl );
    cf.update( "browsercombo_id", browsercombo_id );

    cf.update( "refpopup_by_mo", refpopup_by_mo );
    cf.update( "namepopup_by_mo", namepopup_by_mo );
    cf.update( "idpopup_by_mo", idpopup_by_mo );

    cf.update( "imgemb_interp", imgemb_interp );
    cf.update( "imgmain_interp", imgmain_interp );
    cf.update( "imgpopup_interp", imgpopup_interp );
    cf.update( "imgpopup_width", imgpopup_width );
    cf.update( "imgpopup_height", imgpopup_height );
    cf.update( "use_image_popup", use_image_popup );
    cf.update( "use_image_view", use_image_view );
    cf.update( "use_inline_image", use_inline_image );
    cf.update( "show_ssspicon", show_ssspicon );
    cf.update( "embimg_width", embimg_width );
    cf.update( "embimg_height", embimg_height );
    cf.update( "hide_imagetab", hide_imagetab );
    cf.update( "show_delimgdiag", show_delimgdiag );
    cf.update( "use_mosaic", use_mosaic );
    cf.update( "mosaic_size", mosaic_size );
    cf.update( "zoom_to_fit", zoom_to_fit );
    cf.update( "del_img_day", del_img_day );
    cf.update( "del_imgabone_day", del_imgabone_day );
    cf.update( "max_img_size", max_img_size );
    cf.update( "max_img_pixel", max_img_pixel );
    cf.update( "imgcache_size", imgcache_size );

    cf.update( "cl_char", str_color[ COLOR_CHAR ] );
    cf.update( "cl_char_name", str_color[ COLOR_CHAR_NAME ] );
    cf.update( "cl_char_name_b", str_color[ COLOR_CHAR_NAME_B ] );
    cf.update( "cl_char_name_nomail", str_color[ COLOR_CHAR_NAME_NOMAIL ] );
    cf.update( "cl_char_age", str_color[ COLOR_CHAR_AGE ] );
    cf.update( "cl_char_selection", str_color[ COLOR_CHAR_SELECTION ] );
    cf.update( "cl_char_highlight", str_color[ COLOR_CHAR_HIGHLIGHT ] );
    cf.update( "cl_char_link", str_color[ COLOR_CHAR_LINK ] );
    cf.update( "cl_char_link_id_low", str_color[ COLOR_CHAR_LINK_ID_LOW ] );
    cf.update( "cl_char_link_id_high", str_color[ COLOR_CHAR_LINK_ID_HIGH ] );
    cf.update( "cl_char_link_res", str_color[ COLOR_CHAR_LINK_RES ] );
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
    cf.update( "cl_frame", str_color[ COLOR_FRAME ] );
    cf.update( "cl_marker", str_color[ COLOR_MARKER ] );
    cf.update( "cl_chr_bbs", str_color[ COLOR_CHAR_BBS ] );
    cf.update( "cl_chr_bbs_com", str_color[ COLOR_CHAR_BBS_COMMENT ] );
    cf.update( "cl_chr_board", str_color[ COLOR_CHAR_BOARD ] );
    cf.update( "cl_back_bbs", str_color[ COLOR_BACK_BBS ] );
    cf.update( "cl_back_bbs_even", str_color[ COLOR_BACK_BBS_EVEN ] );
    cf.update( "cl_back_board", str_color[ COLOR_BACK_BOARD ] );
    cf.update( "cl_back_board_even", str_color[ COLOR_BACK_BOARD_EVEN ] );

    cf.update( "use_message_gtktheme", use_message_gtktheme );
    cf.update( "use_tree_gtkrc", use_tree_gtkrc );
    cf.update( "use_select_gtkrc", use_select_gtkrc );

    cf.update( "tree_ypad", tree_ypad );
    cf.update( "tree_show_expanders", tree_show_expanders );
    cf.update( "tree_level_indent", tree_level_indent );

    cf.update( "scroll_tree", scroll_tree );
    cf.update( "select_item_sync", select_item_sync );

    cf.update( "view_margin", view_margin );

    cf.update( "left_scrbar", left_scrbar );

    cf.update( "show_oldarticle", show_oldarticle );
    cf.update( "newthread_hour", newthread_hour );
    cf.update( "inc_search_board", inc_search_board );
    cf.update( "show_deldiag", show_deldiag );
    cf.update( "show_cached_board", show_cached_board );
    cf.update( "show_924", show_924 );

    cf.update( "tree_scroll_size", tree_scroll_size );
    cf.update( "scroll_size", scroll_size );
    cf.update( "key_scroll_size", key_scroll_size );
    cf.update( "key_fastscroll_size", key_fastscroll_size );
    cf.update( "jump_after_reload", jump_after_reload );
    cf.update( "jump_new_after_reload", jump_new_after_reload );

    cf.update( "live_mode", live_mode );
    cf.update( "live_speed", live_speed );
    cf.update( "live_threshold", live_threshold );

    cf.update( "open_one_category", open_one_category );
    cf.update( "open_one_favorite", open_one_favorite );

    cf.update( "write_mail", write_mail );
    cf.update( "write_name", write_name );
    cf.update( "always_write_ok", always_write_ok );
    cf.update( "save_postlog", save_postlog );
    cf.update( "maxsize_postlog", maxsize_postlog );
    cf.update( "save_posthist", save_posthist );
    cf.update( "hide_writing_dialog", hide_writing_dialog );
    cf.update( "show_savemsgdiag", show_savemsgdiag );
    cf.update( "message_wrap", message_wrap );
    cf.update( "fold_message", fold_message );
    cf.update( "fold_image", fold_image );
    cf.update( "keep_im_status", keep_im_status );
    cf.update( "margin_popup", margin_popup );
    cf.update( "margin_imgpopup_x", margin_imgpopup_x );
    cf.update( "margin_imgpopup", margin_imgpopup );
    cf.update( "hide_popup_msec", hide_popup_msec );
    cf.update( "enable_mg", enable_mg );
    cf.update( "mouse_radius", mouse_radius );
    cf.update( "numberjmp_msec", numberjmp_msec );
    cf.update( "history_size", history_size );
    cf.update( "historyview_size", historyview_size );
    cf.update( "aahistory_size", aahistory_size );
    cf.update( "instruct_popup", instruct_popup );
    cf.update( "instruct_tglart", instruct_tglart );
    cf.update( "instruct_tglimg", instruct_tglimg );
    cf.update( "show_delartdiag", show_delartdiag );

    cf.update( "adjust_underline_pos", adjust_underline_pos );
    cf.update( "adjust_line_space", adjust_line_space );

    cf.update( "draw_underline", draw_underline );
    cf.update( "strict_char_width", strict_char_width );
    cf.update( "check_id", check_id );

    cf.update( "num_reference_high", num_reference_high );
    cf.update( "num_reference_low", num_reference_low );
    cf.update( "num_id_high", num_id_high );
    cf.update( "num_id_low", num_id_low );

    cf.update( "loose_url", loose_url );

    cf.update( "hide_usrcmd", hide_usrcmd );
    cf.update( "reload_allthreads", reload_allthreads );

    cf.update( "tab_min_str", tab_min_str );

    cf.update( "show_tab_icon", show_tab_icon );

    cf.update( "switchtab_wheel", switchtab_wheel );

    cf.update( "newtab_pos", newtab_pos );
    cf.update( "opentab_pos", opentab_pos );

    cf.update( "boardnexttab_pos", boardnexttab_pos );

    cf.update( "show_post_mark", show_post_mark );

    cf.update( "flat_button", flat_button );

    cf.update( "draw_toolbarback", draw_toolbarback );

    // スレあぼーん情報
    std::string str_abone_word_thread = MISC::listtostr( list_abone_word_thread );
    std::string str_abone_regex_thread = MISC::listtostr( list_abone_regex_thread );

    cf.update( "abonewordthread", str_abone_word_thread );
    cf.update( "aboneregexthread", str_abone_regex_thread );

    cf.update( "remove_old_abone_thread", remove_old_abone_thread );

    cf.update( "abone_number_thread", abone_number_thread );
    cf.update( "abone_hour_thread", abone_hour_thread );

    // あぼーん情報
    std::string str_abone_name = MISC::listtostr( list_abone_name );
    std::string str_abone_word = MISC::listtostr( list_abone_word );
    std::string str_abone_regex = MISC::listtostr( list_abone_regex );

    cf.update( "abonename", str_abone_name );
    cf.update( "aboneword", str_abone_word );
    cf.update( "aboneregex", str_abone_regex );

    cf.update( "abone_transparent", abone_transparent );
    cf.update( "abone_chain", abone_chain );

    cf.update( "abone_icase", abone_icase );
    cf.update( "abone_wchar", abone_wchar );

    cf.update( "expand_sidebar", expand_sidebar );
    cf.update( "expand_rpane", expand_rpane );

    cf.update( "open_sidebar_by_click", open_sidebar_by_click );

    cf.update( "threshold_next", threshold_next );
    cf.update( "replace_favorite_next", replace_favorite_next );
    cf.update( "show_diag_replace_favorite", show_diag_replace_favorite );
    cf.update( "bookmark_drop", bookmark_drop );
    cf.update( "check_update_board", check_update_board );
    cf.update( "check_update_boot", check_update_boot );
    cf.update( "check_favorite_dup", check_favorite_dup );
    cf.update( "show_favorite_select_diag", show_favorite_select_diag );
    cf.update( "disable_close", disable_close );
    cf.update( "show_hide_menubar_diag", show_hide_menubar_diag );
    cf.update( "change_stastatus_color", change_stastatus_color );
    cf.update( "use_machi_offlaw", use_machi_offlaw );
    cf.update( "show_del_written_thread_diag", show_del_written_thread_diag );
    cf.update( "delete_img_in_thread", delete_img_in_thread );
    cf.update( "max_resnumber", max_resnumber );
    cf.update( "show_diag_fifo_error", show_diag_fifo_error );
    cf.update( "save_session", save_session );

#ifdef HAVE_MIGEMO_H
    cf.update( "migemodict_path", migemodict_path );
#endif

    cf.save();
}


//
// フォントのセット
//
void ConfigItems::set_fonts( JDLIB::ConfLoader& cf )
{
    const std::string defaultfont = get_default_font();

    // フォント
    fontname[ FONT_MAIN ] = cf.get_option_str( "fontname_main", defaultfont + " " + std::string( CONF_FONTSIZE_THREAD ) );
    fontname[ FONT_MAIL ] = cf.get_option_str( "fontname_mail", defaultfont + " " + std::string( CONF_FONTSIZE_MAIL ) );

    // ポップアップのフォント
    fontname[ FONT_POPUP ] = cf.get_option_str( "fontname_popup", defaultfont + " " + std::string( CONF_FONTSIZE_POPUP ) );

    // AA(スレビュー)のフォント
    fontname[ FONT_AA ] = cf.get_option_str( "fontname_aa", fontname[ FONT_MAIN ] );
    aafont_enabled = ( fontname[ FONT_MAIN ] != fontname[ FONT_AA ] );

    // スレ一覧のフォント
    fontname[ FONT_BBS ] = cf.get_option_str( "fontname_bbs", defaultfont + " " + std::string( CONF_FONTSIZE_TREE ) );

    // 板一覧のフォント
    fontname[ FONT_BOARD ] = cf.get_option_str( "fontname_board", fontname[ FONT_BBS ] );

    // 書き込みウィンドウのフォント
    fontname[ FONT_MESSAGE ] = cf.get_option_str( "fontname_message", fontname[ FONT_MAIN ] );

    // Gtk::Entryのデフォルトフォント
    fontname[ FONT_ENTRY_DEFAULT ] = MISC::get_entry_font();
}


//
// フォントのリセット
//
void ConfigItems::reset_fonts()
{
    // dummyのConfLoaderをset_fonts()に渡してデフォルト値をセットする
    JDLIB::ConfLoader cf( "", "dummy = dummy" );
    set_fonts( cf );
}


//
// 色のセット
//
void ConfigItems::set_colors( JDLIB::ConfLoader& cf )
{
    // 文字色 13 = "#FFFFFFFFFFFF" or "#%DDx%DDx%DDx"
    str_color[ COLOR_CHAR ] = cf.get_option_str( "cl_char", CONF_COLOR_CHAR, 13 );

    // 名前欄の文字色
    str_color[ COLOR_CHAR_NAME ] = cf.get_option_str( "cl_char_name", CONF_COLOR_CHAR_NAME, 13 );

    // トリップ等の名前欄の文字色
    str_color[ COLOR_CHAR_NAME_B ] = cf.get_option_str( "cl_char_name_b", CONF_COLOR_CHAR_NAME_B, 13 );

    // 名前無し時の名前欄の文字色
    str_color[ COLOR_CHAR_NAME_NOMAIL ] = cf.get_option_str( "cl_char_name_nomail", CONF_COLOR_CHAR_NAME_NOMAIL, 13 );

    // ageの時のメール欄の文字色
    str_color[ COLOR_CHAR_AGE ] = cf.get_option_str( "cl_char_age", CONF_COLOR_CHAR_AGE, 13 );

    // 選択範囲の文字色
    str_color[ COLOR_CHAR_SELECTION ] = cf.get_option_str( "cl_char_selection", CONF_COLOR_CHAR_SELECTION, 13 );

    // ハイライトの文字色
    str_color[ COLOR_CHAR_HIGHLIGHT ] = cf.get_option_str( "cl_char_highlight", CONF_COLOR_CHAR_HIGHLIGHT, 13 );

    // 通常のリンクの文字色
    str_color[ COLOR_CHAR_LINK ] = cf.get_option_str( "cl_char_link", CONF_COLOR_CHAR_LINK, 13 );

    // 複数発言したIDの文字色
    str_color[ COLOR_CHAR_LINK_ID_LOW ] = cf.get_option_str( "cl_char_link_id_low", CONF_COLOR_CHAR_LINK_ID_LOW, 13 );

    // 多く発言したIDの文字色
    str_color[ COLOR_CHAR_LINK_ID_HIGH ] = cf.get_option_str( "cl_char_link_id_high", CONF_COLOR_CHAR_LINK_ID_HIGH, 13 );

    // 参照されていないレス番号の文字色
    str_color[ COLOR_CHAR_LINK_RES ] = cf.get_option_str( "cl_char_link_res", CONF_COLOR_CHAR_LINK_RES, 13 );

    // 他のレスから参照されたレス番号の文字色
    str_color[ COLOR_CHAR_LINK_LOW ] = cf.get_option_str( "cl_char_link_low", CONF_COLOR_CHAR_LINK_LOW, 13 );

    // 参照された数が多いレス番号の文字色
    str_color[ COLOR_CHAR_LINK_HIGH ] = cf.get_option_str( "cl_char_link_high", CONF_COLOR_CHAR_LINK_HIGH, 13 );

    // メッセージビューの文字色
    str_color[ COLOR_CHAR_MESSAGE ] = cf.get_option_str( "cl_char_message", CONF_COLOR_CHAR_MESSAGE, 13 );

    // メッセージビュー(選択範囲)の文字色
    str_color[ COLOR_CHAR_MESSAGE_SELECTION ] = cf.get_option_str( "cl_char_message_selection", CONF_COLOR_CHAR_MESSAGE_SELECTION, 13 );

    // Gtk::Entryのデフォルトの文字色
    str_color[ COLOR_CHAR_ENTRY_DEFAULT ] = MISC::get_entry_color_text();

    // 画像(キャッシュ無)の色
    str_color[ COLOR_IMG_NOCACHE ] = cf.get_option_str( "cl_img_nocache", CONF_COLOR_IMG_NOCACHE, 13 );

    // 画像(キャッシュ有)の色
    str_color[ COLOR_IMG_CACHED ] = cf.get_option_str( "cl_img_cached", CONF_COLOR_IMG_CACHED, 13 );

    // 画像(ロード中)の色
    str_color[ COLOR_IMG_LOADING ] = cf.get_option_str( "cl_img_loading", CONF_COLOR_IMG_LOADING, 13 );

    // 画像(エラー)の色
    str_color[ COLOR_IMG_ERR ] = cf.get_option_str( "cl_img_err", CONF_COLOR_IMG_ERR, 13 );

    // スレ背景色
    str_color[ COLOR_BACK ] = cf.get_option_str( "cl_back", CONF_COLOR_BACK, 13 );

    // ポップアップの背景色
    str_color[ COLOR_BACK_POPUP ] = cf.get_option_str( "cl_back_popup", CONF_COLOR_BACK_POPUP, 13 );

    // 選択範囲の背景色
    str_color[ COLOR_BACK_SELECTION ] = cf.get_option_str( "cl_back_selection", CONF_COLOR_BACK_SELECTION, 13 );

    // ハイライトの背景色
    str_color[ COLOR_BACK_HIGHLIGHT ] = cf.get_option_str( "cl_back_highlight", CONF_COLOR_BACK_HIGHLIGHT, 13 );

    // ハイライトの背景色(ツリー用)
    str_color[ COLOR_BACK_HIGHLIGHT_TREE ] = cf.get_option_str( "cl_back_highlight_tree", CONF_COLOR_BACK_HIGHLIGHT_TREE, 13 );

    // メッセージビューの背景色
    str_color[ COLOR_BACK_MESSAGE ] = cf.get_option_str( "cl_back_message", CONF_COLOR_BACK_MESSAGE, 13 );

    // メッセージビューの選択色
    str_color[ COLOR_BACK_MESSAGE_SELECTION ] = cf.get_option_str( "cl_back_message_selection", CONF_COLOR_BACK_MESSAGE_SELECTION, 13 );

    // Gtk::Entryのデフォルトの背景色
    str_color[ COLOR_BACK_ENTRY_DEFAULT ] = MISC::get_entry_color_base();

    // 新着セパレータ
    str_color[ COLOR_SEPARATOR_NEW ] = cf.get_option_str( "cl_sepa_new", CONF_COLOR_SEPARATOR_NEW, 13 );

    // ポップアップフレーム色
    str_color[ COLOR_FRAME ] = cf.get_option_str( "cl_frame", CONF_COLOR_FRAME, 13 );

    // オートスクロールマーカー色
    str_color[ COLOR_MARKER ] = cf.get_option_str( "cl_marker", CONF_COLOR_MARKER, 13 );

    // 板一覧の文字
    str_color[ COLOR_CHAR_BBS ] = cf.get_option_str( "cl_chr_bbs", CONF_COLOR_CHAR_BBS, 13 );

    // 板一覧のコメント
    str_color[ COLOR_CHAR_BBS_COMMENT ] = cf.get_option_str( "cl_chr_bbs_com", CONF_COLOR_CHAR_BBS_COMMENT, 13 );

    // スレ一覧の文字
    str_color[ COLOR_CHAR_BOARD ] = cf.get_option_str( "cl_chr_board", CONF_COLOR_CHAR_BOARD, 13 );

    // 板一覧の背景色
    str_color[ COLOR_BACK_BBS ] = cf.get_option_str( "cl_back_bbs", CONF_COLOR_BACK_BBS, 13 );

    // 板一覧の背景色(偶数行)
    str_color[ COLOR_BACK_BBS_EVEN ] = cf.get_option_str( "cl_back_bbs_even", CONF_COLOR_BACK_BBS_EVEN, 13 );

    // スレ一覧の背景色
    str_color[ COLOR_BACK_BOARD ] = cf.get_option_str( "cl_back_board", CONF_COLOR_BACK_BOARD, 13 );

    // スレ一覧の背景色(偶数行)
    str_color[ COLOR_BACK_BOARD_EVEN ] = cf.get_option_str( "cl_back_board_even", CONF_COLOR_BACK_BOARD_EVEN, 13 );
}


//
// 色のリセット
//
void ConfigItems::reset_colors()
{
    // dummyのConfLoaderをset_colors()に渡してデフォルト値をセットする
    JDLIB::ConfLoader cf( "", "dummy = dummy" );
    set_colors( cf );
}


//
// プロクシ設定
//
void ConfigItems::set_proxy_for2ch( const std::string& proxy )
{
    proxy_for2ch = proxy;
    proxy_basicauth_for2ch = std::string();
    if( proxy.empty() ) return;

    // basic認証
    JDLIB::Regex regex;
    const size_t offset = 0;
    const bool icase = false;
    const bool newline = true;
    const bool usemigemo = false;
    const bool wchar = false;

    if( regex.exec( "([^/]+:[^/]+@)(.+)$" , proxy, offset, icase, newline, usemigemo, wchar ) )
    {
        proxy_basicauth_for2ch = regex.str( 1 ).substr( 0, regex.str( 1 ).length() - 1 );
        proxy_for2ch = regex.str( 2 );
    }
}

void ConfigItems::set_proxy_for2ch_w( const std::string& proxy )
{
    proxy_for2ch_w = proxy;
    proxy_basicauth_for2ch_w = std::string();
    if( proxy.empty() ) return;

    // basic認証
    JDLIB::Regex regex;
    const size_t offset = 0;
    const bool icase = false;
    const bool newline = true;
    const bool usemigemo = false;
    const bool wchar = false;

    if( regex.exec( "([^/]+:[^/]+@)(.+)$" , proxy, offset, icase, newline, usemigemo, wchar ) )
    {
        proxy_basicauth_for2ch_w = regex.str( 1 ).substr( 0, regex.str( 1 ).length() - 1 );
        proxy_for2ch_w = regex.str( 2 );
    }
}

void ConfigItems::set_proxy_for_data( const std::string& proxy )
{
    proxy_for_data = proxy;
    proxy_basicauth_for_data = std::string();
    if( proxy.empty() ) return;

    // basic認証
    JDLIB::Regex regex;
    const size_t offset = 0;
    const bool icase = false;
    const bool newline = true;
    const bool usemigemo = false;
    const bool wchar = false;

    if( regex.exec( "([^/]+:[^/]+@)(.+)$" , proxy, offset, icase, newline, usemigemo, wchar ) )
    {
        proxy_basicauth_for_data = regex.str( 1 ).substr( 0, regex.str( 1 ).length() - 1 );
        proxy_for_data = regex.str( 2 );
    }
}
