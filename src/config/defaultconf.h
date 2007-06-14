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
        CONF_USE_IPV6 = 1,          // ipv6使用
        CONF_BROWSER_NO = 2,        // browsers.h のラベル番号
        CONF_TREE_YPAD = 1,         // ツリービューの行間スペース
        CONF_NEWTHREAD_HOUR = 24,   // スレ一覧で指定した値(時間)よりも後に立てられたスレを新着とみなす
        CONF_TREE_SCROLL_SIZE = 4,  // ツリービューのスクロール量(行数)
        CONF_SCROLL_SIZE = 3,       // スレビューのスクロール量
        CONF_KEY_SCROLL_SIZE = 2,   // スレビューのスクロール量(キー上下)
        CONF_MARGIN_POPUP = 30,     // レスアンカーとポップアップの間のマージン
        CONF_MARGIN_IMGPOPUP = CONF_MARGIN_POPUP,  // レスアンカーと画像ポップアップの間のマージン
        CONF_MOUSE_RADIUS = 25,     // マウスジェスチャの判定開始半径
        CONF_HISTORY = 20,          // 履歴の保持数
        CONF_AAHISTORY = 7,         // AA履歴の保持数
        CONF_INSTRUCT_POPUP = 100,  // 0以上なら多重ポップアップの説明を表示する
        CONF_LOOSE_URL = 1,         // datのパース時にURL判定を甘くする(^なども含める)
        CONF_RELOAD_ALLTHREAD = 0,  // スレビューで再読み込みボタンを押したときに全タブを更新する
        CONF_IMGPOPUP_WIDTH = 320,  // 画像ポップアップ幅
        CONF_IMGPOPUP_HEIGHT = 240, // 画像ポップアップ高さ
        CONF_USE_IMAGE_VIEW = 1,    // 画像ビューを使用する
        CONF_INLINE_IMG = 0,        // インライン画像を表示する
        CONF_USE_MOSAIC = 1,        // 画像にモザイクをかける
        CONF_ZOOM_TO_FIT = 1,       // 画像をデフォルトでウィンドウサイズに合わせる
        CONF_DEL_IMG_DAY = 20,      // 画像キャッシュ削除の日数
        CONF_MAX_IMG_SIZE = 16,     // ダウンロードする画像の最大サイズ(Mbyte)
        CONF_LINK_AS_BOARD = 0,     // bbsmenu.html内にあるリンクは全て板とみなす
        CONF_ABONE_TRANSPARENT = 0, // デフォルトで透明あぼーんをする
        CONF_ABONE_CHAIN = 0,       // デフォルトで連鎖あぼーんをする
        CONF_EXPAND_SIDEBAR = 0       // 右ペーンが空の時にサイドバーを閉じる
    };

#define CONF_FONTSIZE_THREAD "12"
#define CONF_FONTSIZE_POPUP  "10"
#define CONF_FONTSIZE_TREE   "10"

// JD ホームページのアドレス
#define CONF_JDHP "http://jd4linux.sourceforge.jp/"

// 2chの認証サーバ
#define CONF_LOGIN2CH "https://2chv.tora3.net/futen.cgi"

// bbsmenu.htmlのURL
#define CONF_BBSMENU "http://menu.2ch.net/bbsmenu.html"

// 色
#define CONF_COLOR_CHAR    "#000000000000"     // スレの文字
#define CONF_COLOR_CHAR_NAME "#000064640000"   //名前欄の文字色
#define CONF_COLOR_CHAR_NAME_B "#000000008b8b" // トリップ等の名前欄の文字色
#define CONF_COLOR_CHAR_AGE "#fde800000000"    // ageの時のメール欄の文字色
#define CONF_COLOR_CHAR_SELECTION "#ffffffffffff"  // 選択範囲の文字色
#define CONF_COLOR_CHAR_HIGHLIGHT str_color[ COLOR_CHAR ] // ハイライトの文字色
#define CONF_COLOR_CHAR_BOOKMARK str_color[ COLOR_CHAR_AGE ] // ブックマークの文字色
#define CONF_COLOR_CHAR_LINK "#00000000ffff" //リンク(通常)の文字色
#define CONF_COLOR_CHAR_LINK_LOW "#ffff0000ffff" // リンク(複数)の文字色
#define CONF_COLOR_CHAR_LINK_HIGH str_color[ COLOR_CHAR_AGE ] // リンク(多数)の文字色
#define CONF_COLOR_CHAR_MESSAGE str_color[ COLOR_CHAR ] // メッセージビューの文字色
#define CONF_COLOR_CHAR_MESSAGE_SELECTION str_color[ COLOR_CHAR_SELECTION ] // メッセージビュー(選択範囲)の文字色

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
