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

//
// モード名
//
    char mode_label[][ MAX_CONTROL_LABEL ] ={
        "共通",
        "板一覧／お気に入り",
        "スレ一覧",
        "スレビュー",
        "画像ビュー",
        "画像ビュー",
        "書き込みビュー",
        "編集",
        "特殊キー (再起動が必要)"
    };


//
// control_label[ id ][ 0 ] を操作名 ( 例えば "Up" )
// control_label[ id ][ 1 ] をラベル ( 例えば "上移動" )
//
// と呼ぶことにする
//
    char control_label[][ 2 ][ MAX_CONTROL_LABEL ]={

        // 共通

        { "", "" },

        { "Up", "上移動" },
        { "Down", "下移動" },

        { "Right", "右移動" },
        { "Left", "左移動" },

        { "TabRight", "右のタブに移動" },
        { "TabLeft", "左のタブに移動" },
        { "TabRightUpdated", "右の更新済みタブに移動" },
        { "TabLeftUpdated", "左の更新済みタブに移動" },

        { "TabNum1", "タブ1に移動" },
        { "TabNum2", "タブ2に移動" },
        { "TabNum3", "タブ3に移動" },
        { "TabNum4", "タブ4に移動" },
        { "TabNum5", "タブ5に移動" },
        { "TabNum6", "タブ6に移動" },
        { "TabNum7", "タブ7に移動" },
        { "TabNum8", "タブ8に移動" },
        { "TabNum9", "タブ9に移動" },

        { "CloseAllTabs", "全てのタブを閉じる" },
        { "CloseOtherTabs", "他のタブを閉じる" },

        { "RestoreLastTab", "最後に閉じたタブを復元" },

        { "CheckUpdateTabs", "全ての更新されたタブを再読み込み" },

        { "PreBookMark", "前のしおりヘ移動" },
        { "NextBookMark", "次のしおりヘ移動" },

        { "PrevView", ITEM_NAME_BACK },
        { "NextView", ITEM_NAME_FORWARD },

        { "ToggleArticle", "スレ一覧とスレビュー切替" },

        { "ShowPopupMenu", "メニュー表示" },

        { "ShowMenuBar", "メニューバー表示" },
        { "ShowToolBarMain", "メインツールバー表示" },
        { "ShowSideBar", "サイドバー表示" },
        
        { "PageUp", "上のページに移動" },
        { "PageDown", "下のページに移動" },

        { "PrevDir", "前のディレクトリに移動" },
        { "NextDir", "次のディレクトリに移動" },

        { "Home", "先頭へ移動" },
        { "End", "最後へ移動" },

        { "Back", "戻る" },

        { "Undo", ITEM_NAME_UNDO },
        { "Redo", ITEM_NAME_REDO },

        { "Quit", ITEM_NAME_QUIT },
        { "Save", "名前を付けて保存..." },
        { "SaveDat", ITEM_NAME_SAVE_DAT "..." },
        { "Delete", ITEM_NAME_DELETE },
        { "Reload", ITEM_NAME_RELOAD },
        { "ReloadArticle", "元のスレを開く" },
        { "StopLoading", ITEM_NAME_STOPLOADING },
        { "OpenURL", "URLを開く..." },        
        { "Copy", ITEM_NAME_COPY },
        { "SelectAll", "全て選択" },
        { "AppendFavorite", ITEM_NAME_APPENDFAVORITE "..." },
        { "Lock", ITEM_NAME_LOCK },

        { "PreferenceView", ITEM_NAME_PREFERENCEVIEW "..." },
        { "PreferenceBoard", ITEM_NAME_PREF_BOARD "..." },
        { "PreferenceArticle", ITEM_NAME_PREF_THREAD "..." },
        { "PreferenceImage", ITEM_NAME_PREF_IMAGE "..." },

        { "Search", ITEM_NAME_SEARCH },
        { "CloseSearchBar", "検索バーを閉じる" },
        { "HiLightOff", ITEM_NAME_CLEAR_HIGHLIGHT },
        { "SearchInvert", "前方検索" },
        { "SearchNext", ITEM_NAME_SEARCH_NEXT },
        { "SearchPrev", ITEM_NAME_SEARCH_PREV },
        { "SearchTitle", "" }, //  CONTROL::get_keyconfig() で名前をセットする
        { "DrawOutAnd", "AND 抽出" },
        { "DrawOutOr", "OR 抽出" },

        { "CheckUpdateRoot", ITEM_NAME_CHECK_UPDATE_ROOT },
        { "CheckUpdateOpenRoot", ITEM_NAME_CHECK_UPDATE_OPEN_ROOT },

        { "QuitJD", "JDim終了" },
        { "MaximizeMainWin", "最大化 / 最大化解除" },
        { "IconifyMainWin", "最小化" },
        { "FullScreen", "全画面表示" },

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

        { "", "" },

        // BOARD

        { "", "" },

        { "OpenArticle", "スレを開く" },
        { "OpenArticleTab", ITEM_NAME_OPENARTICLETAB },
        { "NewArticle", ITEM_NAME_NEWARTICLE },
        { "SearchCache", "ログ検索" },

        { "ScrollLeftBoard", "左スクロール" },
        { "ScrollRightBoard", "右スクロール" },

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

        { "PrePost", "前の書き込みヘ移動" },
        { "NextPost", "次の書き込みヘ移動" },

        { "GotoNew", "新着へ移動" },
        { "OpenParentBoard", ITEM_NAME_OPENBOARD },
        { "WriteMessage", ITEM_NAME_WRITEMSG },

        { "LiveStartStop", ITEM_NAME_LIVE },

        { "SearchNextArticle", ITEM_NAME_NEXTARTICLE },
        { "SearchWeb", "" },  //  CONTROL::get_keyconfig() で名前をセットする
        { "SearchCacheLocal", "ログ検索(対象: 板)" },
        { "SearchCacheAll", "ログ検索(対象: 全ログ)" },

        { "ShowSelectImage", ITEM_NAME_SELECTIMG },
        { "DeleteSelectImage", "削除する" },
        { "AboneSelectImage", "あぼ〜んする" },
        { "AboneSelectionRes", ITEM_NAME_ABONE_SELECTION },

        { "PopupWarpButton", "クリックで多重ポップアップモードに移行" },
        { "ReferResButton", "参照レスポップアップ表示" },
        { "BmResButton", "ブックマーク" },
        { "PopupmenuResButton", "レス番号メニュー表示" },

        { "DrawoutAncButton", "レスの周辺を抽出" },
        { "PopupmenuAncButton", "アンカーをクリックでメニュー表示" },
        { "JumpAncButton", "アンカーをクリックでジャンプ" },

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

        { "ScrollUpImage", "上スクロール" },
        { "ScrollDownImage", "下スクロール" },
        { "ScrollLeftImage", "左スクロール" },
        { "ScrollRightImage", "右スクロール" },

        { "CloseImageTabButton", "タブを閉じる" },

        { "", "" },

        // IMAGE VIEW

        { "", "" },

        { "CloseImageButton", "画像を閉じる" },
        { "ScrollImageButton", "画像スクロール" },
        { "CancelMosaicButton", "モザイク解除" },
        { "SaveImageButton", "画像保存" },
        { "ResizeImageButton", "画像サイズ調整" },

        { "", "" },

        // MESSAGE

        { "", "" },

        { "CancelWrite", ITEM_NAME_QUIT },
        { "ExecWrite", ITEM_NAME_WRITEMSG },
        { "InsertText", ITEM_NAME_INSERTTEXT  },
        { "LockMessage", ITEM_NAME_LOCK_MESSAGE },
        { "Preview", ITEM_NAME_PREVIEW },

        { "FocusWrite", "書き込みボタンにフォーカスを移す" },
        { "ToggleSage", "sageのON/OFF切り替え" },

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
        { "BackspEdit", "BackSpace" },
        { "UndoEdit", ITEM_NAME_UNDO },
        { "EnterEdit", "改行" },

        { "InputAA", "アスキーアート入力" },

        { "", "" },

        // JD globals

        { "", "" },

        { "JDExit", "終了" },
        { "JDHelp", "オンラインマニュアル" },

        { "", "" },

        // その他

        { "", "" },
        { "", "" },

        { "", "" }
    };
}


#endif
