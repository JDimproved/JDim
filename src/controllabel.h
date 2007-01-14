// ライセンス: GPL2

// コントロールのラベル

#ifndef _CONTROLLABEL_H
#define _CONTROLLABEL_H

#include "controlid.h"

#define MAX_CONTROL_LABEL 64

namespace CONTROL
{
    char control_label[][ 2 ][ MAX_CONTROL_LABEL ]={

        // 共通

        { "", "" },

        { "Up", "上移動" },
        { "Down", "下移動" },

        { "Right", "右移動" },
        { "Left", "左移動" },

        { "TabRight", "右のタブに移動" },
        { "TabLeft", "左のタブに移動" },

        { "PreBookMark", "前のプックマークヘ移動" },
        { "NextBookMark", "次のプックマークヘ移動" },

        { "ToggleArticle", "スレ一覧とスレビュー切替" },

        { "ShowPopupMenu", "メニュー表示" },

        { "ShowSideBar", "サイドバー表示" },
        
        { "PageUp", "上のページに移動" },
        { "PageDown", "下のページに移動" },

        { "Home", "先頭へ移動" },
        { "End", "最後へ移動" },

        { "Back", "戻る" },

        { "Quit", "閉じる" },
        { "Save", "保存" },
        { "Delete", "削除" },
        { "Reload", "再読み込み" },
        { "StopLoading", "読み込み中止" },
        { "Copy", "コピー" },
        { "AppendFavorite", "お気に入りに追加" },
        { "Property", "プロパティ" },

        { "Search", "検索" },
        { "CloseSearchBar", "検索バーを閉じる" },
        { "HiLightOff", "ハイライト解除" },
        { "SearchInvert", "前方検索" },
        { "SearchNext", "次検索" },
        { "SearchPrev", "前検索" },
        { "DrawOutAnd", "AND 抽出" },
        { "DrawOutOr", "OR 抽出" },

        { "ClickButton", "クリック" },
        { "DblClickButton", "ダブルクリック" },
        { "CloseTabButton", "タブを閉じる" },
        { "ReloadTabButton", "タブを再読み込み" },
        { "AutoScrollButton", "オートスクロール" },
        { "GestureButton", "マウスジェスチャ" },
        { "PopupmenuButton", "ポップアップメニュー表示" },
        { "DragStartButton", "ドラッグ開始" },
        { "TreeRowSelectionButton", "行範囲選択" },

        
        { "ScrollUp", "" }, // 上スクロール
        { "ScrollDown", "" },// 下スクロール
        { "ScrollLeft", "" }, // 左スクロール
        { "ScrollRight", "" },// 右スクロール

        { "", "" },

        // BBSLIST

        { "", "" },

        { "OpenBoard", "板を開く" },
        { "OpenBoardTab", "タブで板を開く" },

        { "OpenBoardButton", "板を開く" },
        { "OpenBoardTabButton", "タブで板を開く" },

        { "", "" },

        // BOARD

        { "", "" },

        { "OpenArticle", "スレを開く" },
        { "OpenArticleTab", "タブでスレを開く" },
        { "NewArticle", "新スレ作成" },
        { "SearchCache", "ログ検索" },

        { "OpenArticleButton", "スレを開く" },
        { "OpenArticleTabButton", "タブでスレを開く" },

        { "", "" },

        // ARTICLE

        { "", "" },

        { "UpMid", "中速上移動" },
        { "UpFast", "高速上移動" },

        { "DownMid", "中速下移動" },
        { "DownFast", "高速下移動" },

        { "GotoNew", "新着へ移動" },
        { "OpenParentBoard", "板を開く" },
        { "WriteMessage", "書き込み" },

        { "PopupWarpButton", "クリックで多重ポップアップモードに移行" },
        { "ReferResButton", "参照レスポップアップ表示" },
        { "BmResButton", "ブックマーク" },
        { "PopupmenuResButton", "レス番号メニュー表示" },

        { "DrawoutAncButton", "レスの周辺を抽出" },
        { "PopupmenuAncButton", "アンカーメニュー表示" },

        { "PopupIDButton", "ID抽出ポップアップ表示" },
        { "DrawoutIDButton", "ID抽出" },
        { "PopupmenuIDButton", "IDメニュー表示" },

        { "OpenImageButton", "画像を開く" },
        { "OpenBackImageButton", "画像をバックで開く" },
        { "PopupmenuImageButton", "画像メニュー表示" },

        { "OpenBeButton", "ブラウザでBe表示" },
        { "PopupmenuBeButton", "Beメニュー表示" },

        { "", "" },

        // IMAGE

        { "", "" },

        { "CancelMosaic", "モザイク解除" },
        { "ZoomFitImage", "画面に画像サイズを会わせる" },
        { "ZoomInImage", "ズームイン" },
        { "ZoomOutImage", "ズームアウト" },
        { "OrgSizeImage", "元の画像サイズ" },

        { "", "" },

        // MESSAGE

        { "", "" },

        { "CancelWrite", "書き込みキャンセル" },
        { "ExecWrite", "書き込み実行" },

        { "", "" },

        // EDIT

        { "", "" },

        { "HomeEdit", "Home" },
        { "EndEdit", "End" },

        { "UpEdit", "カーソルを上へ移動" },
        { "DownEdit", "カーソルを下へ移動" },
        { "RightEdit", "カーソルを右へ移動" },
        { "LeftEdit", "カーソルを左へ移動" },

        { "DeleteEdit", "一文字削除" },
        { "UndoEdit", "元に戻す(Undo)" },

        { "", "" },

        // その他

        { "", "" },
        { "", "" },

        { "", "" }        
    };
}


#endif
