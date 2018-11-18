// ライセンス: GPL2

// アイコンテーマのファイル

#ifndef _ICONFILES_H
#define _ICONFILES_H

enum
{
    MAX_ICON_FILES = 32
};

namespace ICON
{
    char iconfiles[][ MAX_ICON_FILES ]={

        "jd16",
        "jd32",
        "jd48",
        "jd96",

        // サイドバーで使用するアイコン
        "dir",
        "image",
        "link",

        // サイドバーやタブで使用するアイコン
        "board",
        "board_update",
        "thread",
        "thread_update",
        "thread_old",

        // タブで使用するアイコン
        "board_updated",
        "thread_updated",
        "loading",
        "loading_stop",

        // スレ一覧で使用するアイコン
        "bkmark_update",
        "bkmark_broken_subject",
        "bkmark",
        "update",
        "newthread",
        "newthread_hour",
        "broken_subject",
        "check",
        "old",
        "info",

        // スレビューで使用するアイコン
        "bkmark_thread",
        "post",
        "post_refer",

        // その他
        "down",
        "transparent",

        ////////////////////////
        // ツールバーのアイコン

        // 共通
        "search_prev",  // 前検索
        "search_next",  // 次検索
        "stoploading",  // 読み込み中止
        "write",        // 書き込み / 新スレ作成
        "reload",       // 再読み込み
        "appendfavorite",  // お気に入りに追加
        "delete",          // 削除
        "quit",            // 閉じる
        "back",            // 前へ戻る
        "forward",         // 次へ進む
        "lock",            // タブをロックする
        "undo",            // 元に戻す(Undo)
        "redo",            // やり直し(Redo)

        // メイン
        "bbslistview",    // 板一覧
        "favoriteview",   // お気に入り
        "histview",       // スレ履歴
        "hist_boardview", // 板履歴
        "hist_closeview", // 最近閉じたスレ
        "hist_closeboardview", // 最近閉じた板
        "hist_closeimgview", // 最近閉じた画像
        "boardview",      // スレ一覧
        "articleview",    // スレビュー
        "imageview",      // 画像ビュー
        "go",             // 移動

        // サイドバー
        "check_update_root",      // サイドバー更新チェック
        "check_update_open_root", // サイドバー更新チェックして開く

        // スレビュー
        "search",  // 検索
        "live",    // 実況開始／停止

        // 検索バー
        "close_search",  // 検索バーを閉じる
        "clear_search",  // ハイライト解除
        "search_and",    // AND 抽出
        "search_or",     // OR 抽出

        // 書き込みビュー
        "preview",    // プレビュー表示
        "inserttext", // テキストファイル挿入

        ""
    };
}

#endif
