// カラーID

#ifndef _COLOR_ID_H
#define _COLOR_ID_H

enum
{
    COLOR_NONE = 0,

    // スレビューで使用する色
    
    COLOR_FOR_THREAD = 0,

    COLOR_CHAR,             // スレビューなど基本の文字
    COLOR_CHAR_NAME,        // 名前欄
    COLOR_CHAR_NAME_B,      // トリップや fusianasan 等、<b>が含まれている名前欄
    COLOR_CHAR_NAME_NOMAIL, // メールが無いときの名前欄
    COLOR_CHAR_AGE,         // 非sageのメール欄
    COLOR_CHAR_SELECTION,   // 選択範囲の文字
    COLOR_CHAR_HIGHLIGHT,   // ハイライトの文字
    COLOR_CHAR_HIGHLIGHT_TREE, ///< @brief ツリー表示における検索結果や抽出項目のハイライト文字色
    COLOR_CHAR_LINK,        // 通常のリンクの文字色
    COLOR_CHAR_LINK_ID_LOW, // 複数発言したIDの文字色
    COLOR_CHAR_LINK_ID_HIGH,// 多く発言したIDの文字色
    COLOR_CHAR_LINK_RES,    // 参照されていないレス番号の文字色
    COLOR_CHAR_LINK_LOW,    // 他のレスから参照されたレス番号の文字色
    COLOR_CHAR_LINK_HIGH,   // 参照された数が多いレス番号の文字色
    COLOR_CHAR_MESSAGE,           // メッセージビューの文字
    COLOR_CHAR_MESSAGE_SELECTION, // メッセージビュー(選択範囲)の文字
    COLOR_CHAR_ENTRY_DEFAULT,      // Gtk::Entryのデフォルトの文字色

    COLOR_IMG_NOCACHE, // 画像のリンク(キャッシュ無)
    COLOR_IMG_CACHED,  // 画像のリンク(キャッシュ有)
    COLOR_IMG_LOADING, // 画像のリンク(ロード中)
    COLOR_IMG_ERR,     // 画像のリンク(エラー)

    COLOR_BACK,             // スレビューなど基本の背景
    COLOR_BACK_POPUP,       // ポップアップの背景
    COLOR_BACK_SELECTION,   // 選択範囲
    COLOR_BACK_HIGHLIGHT,   // ハイライト文字の背景色
    COLOR_BACK_HIGHLIGHT_TREE,  // ハイライト文字の背景色(ツリー用)
    COLOR_BACK_MESSAGE,            // メッセージビューの背景色
    COLOR_BACK_MESSAGE_SELECTION,  // メッセージビュー(選択範囲)の背景色
    COLOR_BACK_ENTRY_DEFAULT,      // Gtk::Entryのデフォルトの背景色

    COLOR_SEPARATOR_NEW,    // 新着セパレータ
    COLOR_FRAME,  // ポップアップフレーム色
    COLOR_MARKER, // オートスクロールのマーク色

    END_COLOR_FOR_THREAD,
    USRCOLOR_BASE = END_COLOR_FOR_THREAD, // cssで使用する色番号のベース

    // その他の色

    COLOR_CHAR_BBS, // 板一覧の文字
    COLOR_CHAR_BBS_COMMENT, // 板一覧のコメント
    COLOR_CHAR_BOARD, // スレ一覧の文字

    COLOR_BACK_BBS,         // 板一覧の背景
    COLOR_BACK_BBS_EVEN,    // 板一覧の背景(偶数行)
    COLOR_BACK_BOARD,       // スレ一覧の背景
    COLOR_BACK_BOARD_EVEN,  // スレ一覧の背景(偶数行)

    COLOR_NUM
};

#endif
