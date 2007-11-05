// ライセンス: GPL2
//
// 設定のデフォルト値
//

#ifndef _DEFAULTCONF_H
#define _DEFAULTCONF_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

namespace CONFIG
{
    enum{
        CONF_RESTORE_BOARD = 0,     // スレ一覧を復元
        CONF_RESTORE_ARTICLE = 0,   // スレを復元
        CONF_RESTORE_IMAGE = 0,     // 画像を復元
        CONF_REF_PREFIX_SPACE = 1, // 参照文字( CONF_REF_PREFIX ) の後のスペースの数
        CONF_USE_PROXY_FOR2CH = 0, // 2ch 読み込み用プロクシを使用するか
        CONF_PROXY_PORT_FOR2CH = 8080, // 2ch 読み込み用プロクシポート番号
        CONF_USE_PROXY_FOR2CH_W = 0, // 2ch 書き込み用プロクシを使用するか
        CONF_PROXY_PORT_FOR2CH_W = 8080, // 2ch 書き込み用プロクシポート番号
        CONF_USE_PROXY_FOR_DATA = 0, // 2ch 以外にアクセスするときにプロクシを使用するか
        CONF_PROXY_PORT_FOR_DATA = 8080, // 2ch 以外にアクセスするときのプロクシポート番号
        CONF_LOADER_BUFSIZE = 32,   // ローダのバッファサイズ
        CONF_LOADER_TIMEOUT = 10,   // ローダのタイムアウト値        
        CONF_LOADER_TIMEOUT_POST = 30, // ポストローダのタイムアウト値        
        CONF_LOADER_TIMEOUT_IMG = 30,  // 画像ローダのタイムアウト値
        CONF_LOADER_TIMEOUT_CHECKUPDATE = 10,  // 更新チェックのタイムアウト値
        CONF_USE_IPV6 = 1,          // ipv6使用
        CONF_BROWSER_NO = 2,        // browsers.h のラベル番号
        CONF_REFPOPUP_BY_MO = 0,    // レス番号の上にマウスオーバーしたときに参照ポップアップ表示する
        CONF_NAMEPOPUP_BY_MO = 0,   // 名前の上にマウスオーバーしたときにポップアップ表示する
        CONF_IDPOPUP_BY_MO = 0,     // IDの上にマウスオーバーしたときにIDをポップアップ表示する
        CONF_USE_TREE_GTKRC = 0,    // ツリービューでgtkrcの設定を使用するか
        CONF_TREE_YPAD = 1,         // ツリービューの行間スペース
        CONF_SHOW_OLDARTICLE = 0,   // スレ一覧で古いスレも表示
        CONF_NEWTHREAD_HOUR = 24,   // スレ一覧で指定した値(時間)よりも後に立てられたスレを新着とみなす
        CONF_INC_SEARCH_BOARD = 0,  // スレ一覧でインクリメント検索をする
        CONF_TREE_SCROLL_SIZE = 4,  // ツリービューのスクロール量(行数)
        CONF_SCROLL_SIZE = 3,       // スレビューのスクロール量
        CONF_KEY_SCROLL_SIZE = 2,   // スレビューのスクロール量(キー上下)
        CONF_OPEN_ONE_CATEGORY = 0, // 板一覧でカテゴリを常にひとつだけ開く
        CONF_ALWAYS_WRITE_OK = 0,   // 書き込み時に書き込み確認ダイアログを出さない
        CONF_SAVE_POSTLOG = 0,      //書き込みログを保存
        CONF_HIDE_WRITING_DIALOG = 0, // 「書き込み中」のダイアログを表示しない
        CONF_FOLD_MESSAGE = 0,      // 非アクティブ時に書き込みビューを折りたたむ
        CONF_MARGIN_POPUP = 30,     // レスアンカーとポップアップの間のマージン
        CONF_MARGIN_IMGPOPUP = CONF_MARGIN_POPUP,  // レスアンカーと画像ポップアップの間のマージン
        CONF_MOUSE_RADIUS = 25,     // マウスジェスチャの判定開始半径
        CONF_HISTORY = 20,          // 履歴の保持数
        CONF_AAHISTORY = 7,         // AA履歴の保持数
        CONF_INSTRUCT_POPUP = 100,  // 0以上なら多重ポップアップの説明を表示する
        CONF_INSTRUCT_TGLART = 1, // スレビューを開いたときにスレ一覧との切り替え方法を説明する
        CONF_INSTRUCT_TGLIMG = 1, // 画像ビューを開いたときにスレビューとの切り替え方法を説明する
        CONF_ADJUST_UNDERLINE_POS = 1, // 下線位置
        CONF_ADJUST_LINE_SPACE = 1,    // 行間スペース
        CONF_DRAW_UNDERLINE = 1,     // リンク下線を表示
        CONF_STRICT_CHAR_WIDTH = 0,  // スレビューで文字幅の近似を厳密にする
        CONF_CHECK_ID = 1,           // スレビューで発言数(ID)をカウントする
        CONF_NUM_REFERENCE_HIGH = 3,//レス参照で色を変える回数 (高) 
        CONF_NUM_REFERENCE_LOW = 1, //レス参照で色を変える回数 (低) 
        CONF_NUM_ID_HIGH = 4,       // 発言数で色を変える回数 (高)
        CONF_NUM_ID_LOW = 2,        // 発言数で色を変える回数 (低)
        CONF_LOOSE_URL = 1,         // datのパース時にURL判定を甘くする(^なども含める)
        CONF_HIDE_USRCMD = 0, // ユーザーコマンドで選択できない項目を非表示にする
        CONF_MAX_SHOW_USRCMD = 3, // 指定した数よりもユーザーコマンドが多い場合はサブメニュー化する
        CONF_RELOAD_ALLTHREAD = 0,  // スレビューで再読み込みボタンを押したときに全タブを更新する
        CONF_TAB_MIN_STR = 4, // タブに表示する文字列の最小値
        CONF_SHOW_TAB_ICON = 1, // タブにアイコンを表示するか
        CONF_IMGPOPUP_WIDTH = 320,  // 画像ポップアップ幅
        CONF_IMGPOPUP_HEIGHT = 240, // 画像ポップアップ高さ
        CONF_USE_IMAGE_POPUP = 1,    // 画像ポップアップを使用する
        CONF_USE_IMAGE_VIEW = 1,    // 画像ビューを使用する
        CONF_INLINE_IMG = 0,        // インライン画像を表示する
        CONF_USE_MOSAIC = 1,        // 画像にモザイクをかける
        CONF_ZOOM_TO_FIT = 1,       // 画像をデフォルトでウィンドウサイズに合わせる
        CONF_DEL_IMG_DAY = 20,      // 画像キャッシュ削除の日数
        CONF_DEL_IMGABONE_DAY = 20, // 画像あぼーん削除の日数
        CONF_MAX_IMG_SIZE = 16,     // ダウンロードする画像の最大サイズ(Mbyte)
        CONF_MAX_IMG_PIXEL = 20,     // 画像の最大サイズ(Mピクセル)
        CONF_LINK_AS_BOARD = 0,     // bbsmenu.html内にあるリンクは全て板とみなす
        CONF_ABONE_NUMBER_THREAD = 0, // スレあぼーん( レス数 )
        CONF_ABONE_HOUR_THREAD = 0,   // スレあぼーん( スレ立てからの経過時間 )
        CONF_ABONE_TRANSPARENT = 0, // デフォルトで透明あぼーんをする
        CONF_ABONE_CHAIN = 0,       // デフォルトで連鎖あぼーんをする
        CONF_EXPAND_SIDEBAR = 0       // 右ペーンが空の時にサイドバーを閉じる
    };

#define CONF_FONTSIZE_THREAD "12"
#define CONF_FONTSIZE_POPUP  "10"
#define CONF_FONTSIZE_TREE   "10"

// レスを参照するときに前に付ける文字
#define CONF_REF_PREFIX ">"

// キャッシュのルートディレクトリ(旧バージョンとの互換のため残している)
#define CONF_PATH_CACHEROOT "~/.jd/"

// 2ch にアクセスするときのエージェント名
#define AGENT_FOR2CH "Monazilla/1.00 JD"

// 2ch外にアクセスするときのエージェント名
#define AGENT_FOR_DATA "Mozilla/5.0 (Windows; U; Windows NT 5.0; ja; rv:1.8.1.5) Gecko/20070713 Firefox/2.0.0.5"

// 2ch にログインするときのX-2ch-UA
#define CONF_X_2CH_UA "Navigator for 2ch 1.7.5"

// JD ホームページのアドレス
#define CONF_JDHP "http://jd4linux.sourceforge.jp/"

// 2chの認証サーバ
#define CONF_LOGIN2CH "https://2chv.tora3.net/futen.cgi"

// bbsmenu.htmlのURL
#define CONF_BBSMENU "http://menu.2ch.net/bbsmenu.html"

// スレタイ検索用メニュータイトルアドレス
#define CONF_URL_SEARCH_MENU  "スレタイ検索(find2ch)"
#define CONF_URL_SEARCH_TITLE "http://find.2ch.net/?STR=$TEXTX&COUNT=50&TYPE=TITLE&BBS=ALL"

// スレタイ検索用正規表現
#define CONF_SEARCH_TITLE_REGEX "<a href=\"(http[^\"]*)\">(.+?)</a> \\(([0-9]{1,4})\\)"

// 色
#define CONF_COLOR_CHAR    "#000000000000"     // スレの文字
#define CONF_COLOR_CHAR_NAME "#000064640000"   //名前欄の文字色
#define CONF_COLOR_CHAR_NAME_B "#000000008b8b" // トリップ等の名前欄の文字色
#define CONF_COLOR_CHAR_NAME_NOMAIL "#000064640000"   //メール無し時の名前欄の文字色
#define CONF_COLOR_CHAR_AGE "#fde800000000"    // ageの時のメール欄の文字色
#define CONF_COLOR_CHAR_SELECTION "#ffffffffffff"  // 選択範囲の文字色
#define CONF_COLOR_CHAR_HIGHLIGHT str_color[ COLOR_CHAR ] // ハイライトの文字色
#define CONF_COLOR_CHAR_BOOKMARK str_color[ COLOR_CHAR_AGE ] // ブックマークの文字色
#define CONF_COLOR_CHAR_LINK "#00000000ffff" //リンク(通常)の文字色
#define CONF_COLOR_CHAR_LINK_LOW "#ffff0000ffff" // リンク(複数)の文字色
#define CONF_COLOR_CHAR_LINK_HIGH str_color[ COLOR_CHAR_AGE ] // リンク(多数)の文字色
#define CONF_COLOR_CHAR_MESSAGE str_color[ COLOR_CHAR ] // メッセージビューの文字色
#define CONF_COLOR_CHAR_MESSAGE_SELECTION str_color[ COLOR_CHAR_SELECTION ] // メッセージビュー(選択範囲)の文字色

#define CONF_COLOR_IMG_NOCACHE "#a5a52a2a2a2a" // 画像(キャッシュ無)の色
#define CONF_COLOR_IMG_CACHED  "#00008b8b8b8b" // 画像(キャッシュ有)の色
#define CONF_COLOR_IMG_LOADING "#ffff8c8c0000"  // 画像(ロード中)の色
#define CONF_COLOR_IMG_ERR str_color[ COLOR_CHAR_AGE ] // 画像(エラー)の色

#define CONF_COLOR_BACK           "#fde8fde8f618" // スレ背景色
#define CONF_COLOR_BACK_POPUP     str_color[ COLOR_BACK ] // ポップアップ背景色
#define CONF_COLOR_BACK_SELECTION str_color[ COLOR_CHAR_LINK ] // 選択範囲の背景色

#define CONF_COLOR_HL      "#ffffffff0000"  // ハイライト色
#define CONF_COLOR_HL_TREE str_color[ COLOR_BACK_HIGHLIGHT ] // ツリーのハイライト色

#define CONF_COLOR_BACK_MESSAGE str_color[ COLOR_BACK ] // メッセージビューの背景色
#define CONF_COLOR_BACK_MESSAGE_SELECTION str_color[ COLOR_BACK_SELECTION ] // メッセージビューの選択色

#define CONF_COLOR_SEPARATOR_NEW "#7d007d007d00" // セパレータ

#define CONF_COLOR_CHAR_BBS   str_color[ COLOR_CHAR ] // 板一覧の文字
#define CONF_COLOR_CHAR_BOARD str_color[ COLOR_CHAR ] // スレ一覧の文字

#define CONF_COLOR_BACK_BBS   str_color[ COLOR_BACK ] // 板一覧の背景色
#define CONF_COLOR_BACK_BBS_EVEN str_color[ COLOR_BACK_BBS ] // 板一覧の背景色(偶数行)
#define CONF_COLOR_BACK_BOARD str_color[ COLOR_BACK ] // スレ一覧の背景色
#define CONF_COLOR_BACK_BOARD_EVEN str_color[ COLOR_BACK_BOARD ] // スレ一覧の背景色(偶数行)

// migemo-dictの場所
#ifdef MIGEMODICT
#define CONF_MIGEMO_PATH MIGEMODICT
#else    
#define CONF_MIGEMO_PATH "/usr/share/migemo/utf-8/migemo-dict"
#endif

}

#endif
