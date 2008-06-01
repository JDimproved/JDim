// ライセンス: GPL2

// コントロールのラベル

#ifndef _CONTROLLABEL_H
#define _CONTROLLABEL_H

#include "controlid.h"
#include "global.h"

enum
{
    MAX_CONTROL_LABEL = 64
};

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

        { "PreBookMark", "前のしおりヘ移動" },
        { "NextBookMark", "次のしおりヘ移動" },

        { "PrevView", ITEM_NAME_PREVVIEW },
        { "NextView", ITEM_NAME_NEXTVIEW },

        { "ToggleArticle", "スレ一覧とスレビュー切替" },

        { "ShowPopupMenu", "メニュー表示" },

        { "ShowMenuBar", "メニューバー表示" },
        { "ShowSideBar", "サイドバー表示" },
        
        { "PageUp", "上のページに移動" },
        { "PageDown", "下のページに移動" },

        { "Home", "先頭へ移動" },
        { "End", "最後へ移動" },

        { "Back", "戻る" },

        { "Quit", ITEM_NAME_QUIT },
        { "Save", "名前を付けて保存..." },
        { "Delete", ITEM_NAME_DELETE },
        { "Reload", ITEM_NAME_RELOAD },
        { "StopLoading", ITEM_NAME_STOPLOADING },
        { "Copy", "コピー" },
        { "SelectAll", "全て選択" },
        { "AppendFavorite", ITEM_NAME_FAVORITE },
        { "Property", "プロパティ..." },
        { "Lock", ITEM_NAME_LOCK },

        { "Search", ITEM_NAME_SEARCH },
        { "CloseSearchBar", "検索バーを閉じる" },
        { "HiLightOff", "ハイライト解除" },
        { "SearchInvert", "前方検索" },
        { "SearchNext", ITEM_NAME_SEARCH_NEXT },
        { "SearchPrev", ITEM_NAME_SEARCH_PREV },
        { "DrawOutAnd", "AND 抽出" },
        { "DrawOutOr", "OR 抽出" },

        { "ClickButton", "クリック" },
        { "DblClickButton", "ダブルクリック" },
        { "TrpClickButton", "トリプルクリック" },
        { "CloseTabButton", "タブを閉じる" },
        { "ReloadTabButton", "タブを再読み込み" },
        { "AutoScrollButton", "オートスクロール" },
        { "GestureButton", "マウスジェスチャ" },
        { "PopupmenuButton", "ポップアップメニュー表示" },
        { "DragStartButton", "ドラッグ開始" },
        { "TreeRowSelectionButton", "行範囲選択" },

        { "", "" },

        // BBSLIST

        { "", "" },

        { "OpenBoard", "板を開く" },
        { "OpenBoardTab", "タブで板を開く" },

        { "OpenBoardButton", "板を開く" },
        { "OpenBoardTabButton", "タブで板を開く" },

        { "CheckUpdateRoot", "更新チェック" },
        { "CheckUpdateOpenRoot", "更新されたスレをタブで開く" },

        { "", "" },

        // BOARD

        { "", "" },

        { "OpenArticle", "スレを開く" },
        { "OpenArticleTab", "タブでスレを開く" },
        { "NewArticle", ITEM_NAME_NEWARTICLE },
        { "SearchCache", "ログ検索" },

        { "ScrollLeftBoard", "" }, // 左スクロール
        { "ScrollRightBoard", "" },// 右スクロール

        { "OpenArticleButton", "スレを開く" },
        { "OpenArticleTabButton", "タブでスレを開く" },

        { "", "" },

        // ARTICLE

        { "", "" },

        { "UpMid", "中速上移動" },
        { "UpFast", "高速上移動" },

        { "DownMid", "中速下移動" },
        { "DownFast", "高速下移動" },

        { "PrevRes", "前のレスへ移動" },
        { "NextRes", "次のレスへ移動" },

        { "GotoNew", "新着へ移動" },
        { "OpenParentBoard", ITEM_NAME_OPENBOARD },
        { "WriteMessage", ITEM_NAME_WRITEMSG },

        { "LiveStartStop", ITEM_NAME_LIVE },

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

        // IMAGE ICON

        { "", "" },

        { "CancelMosaic", "モザイク解除" },
        { "ZoomFitImage", "画面に画像サイズを合わせる" },
        { "ZoomInImage", "ズームイン" },
        { "ZoomOutImage", "ズームアウト" },
        { "OrgSizeImage", "元の画像サイズ" },

        { "ScrollUpImage", "" }, // 上スクロール
        { "ScrollDownImage", "" },// 下スクロール
        { "ScrollLeftImage", "" }, // 左スクロール
        { "ScrollRightImage", "" },// 右スクロール

        { "CloseImageTabButton", "タブを閉じる" },

        { "", "" },

        // IMAGE VIEW

        { "", "" },

        { "CloseImageButton", "画像を閉じる" },
        { "ScrollImageButton", "画像スクロール" },
        { "CancelMosaicButton", "モザイク解除" },

        { "", "" },

        // MESSAGE

        { "", "" },

        { "CancelWrite", ITEM_NAME_QUIT },
        { "ExecWrite", ITEM_NAME_WRITEMSG },
        { "InsertText", ITEM_NAME_INSERTTEXT  },
        { "LockMessage", ITEM_NAME_LOCK_MESSAGE },
        { "Preview", ITEM_NAME_PREVIEW },

        { "FocusWrite", "書き込みボタンにフォーカスを移す" },

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
        { "BackspEdit", "バックスペース" },
        { "UndoEdit", ITEM_NAME_UNDO },

        { "InputAA", "アスキーアート入力" },

        { "", "" },

        // その他

        { "", "" },
        { "", "" },

        { "", "" }        
    };
}


#endif
