// ライセンス: GPL2

// アイコンのID
//
// !!注意!!   項目を増やしたら iconfiles.h も修正すること
//

#ifndef _ICONID_H
#define _ICONID_H

namespace ICON
{
    enum
    {
        NONE = -1,

        JD16 = 0,
        JD32,
        JD48,
        JD96,

        // サイドバーで使用するアイコン
        DIR,
        IMAGE,
        LINK,

        // サイドバーやタブで使用するアイコン
        BOARD,
        BOARD_UPDATE,
        THREAD,
        THREAD_UPDATE,
        THREAD_OLD,

        // タブで使用するアイコン
        BOARD_UPDATED,
        THREAD_UPDATED,
        LOADING,
        LOADING_STOP,

        // スレ一覧で使用するアイコン
        BKMARK_UPDATE,
        BKMARK_BROKEN_SUBJECT,
        BKMARK,
        UPDATE,
        NEWTHREAD,
        NEWTHREAD_HOUR ,
        BROKEN_SUBJECT,
        CHECK,
        OLD,
        INFO,

        // スレビューで使用するアイコン
        BKMARK_THREAD,
        POST,
        POST_REFER,

        // その他
        DOWN,
        TRANSPARENT,

        ////////////////////////
        // ツールバーのアイコン

        // 共通
        SEARCH_PREV,  // 前検索
        SEARCH_NEXT,  // 次検索
        STOPLOADING,  // 読み込み中止
        WRITE,        // 書き込み / 新スレ作成
        RELOAD,       // 再読み込み
        APPENDFAVORITE,  // お気に入りに追加
        DELETE,          // 削除
        QUIT,            // 閉じる
        BACK,            // 前へ戻る
        FORWARD,         // 次へ進む
        LOCK,            // タブをロックする
        UNDO,            // 元に戻す(Undo)
        REDO,            // やり直し(Redo)

        // メイン
        BBSLISTVIEW,    // 板一覧
        FAVORITEVIEW,   // お気に入り
        HISTVIEW,       // スレ履歴
        HIST_BOARDVIEW, // 板履歴
        HIST_CLOSEVIEW, // 最近閉じたスレ
        HIST_CLOSEBOARDVIEW, // 最近閉じた板
        HIST_CLOSEIMGVIEW, // 最近閉じた画像
        BOARDVIEW,      // スレ一覧
        ARTICLEVIEW,    // スレビュー
        IMAGEVIEW,      // 画像ビュー
        GO,             // 移動

        // サイドバー
        CHECK_UPDATE_ROOT,      // サイドバー更新チェック
        CHECK_UPDATE_OPEN_ROOT, // サイドバー更新チェックして開く

        // スレビュー
        SEARCH,  // 検索
        LIVE,    // 実況開始／停止

        // 検索バー
        CLOSE_SEARCH,  // 検索バーを閉じる
        CLEAR_SEARCH,  // ハイライト解除
        SEARCH_AND,    // AND 抽出
        SEARCH_OR,     // OR 抽出

        // 書き込みビュー
        PREVIEW,    // プレビュー表示
        INSERTTEXT, // テキストファイル挿入

        ////////////////////////

        NUM_ICONS
    };
}

#endif
