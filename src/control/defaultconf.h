// ライセンス: GPL2
//
// 設定のデフォルト値
//

#ifndef _DEFAULTMOUSEKEYCONF_H
#define _DEFAULTMOUSEKEYCONF_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

namespace CONTROL
{

// ショートカットキー

#define KEYCONF_Up  "k Up KP_Up"
#define KEYCONF_Down  "j Down KP_Down"

#define KEYCONF_Right  "l Right KP_Right"
#define KEYCONF_Left  "h Left KP_left"

#define KEYCONF_TabRight  "Ctrl+Page_Down Ctrl+Tab Ctrl+Left_Tab Ctrl+l Ctrl+Right"
#define KEYCONF_TabLeft  "Ctrl+Page_Up Ctrl+Shift+Tab Ctrl+Shift+Left_Tab Ctrl+h Ctrl+Left"
#define KEYCONF_TabRightUpdated  "Ctrl+Shift+Page_Down Ctrl+L Ctrl+Shift+Right ]"
#define KEYCONF_TabLeftUpdated  "Ctrl+Shift+Page_Up Ctrl+H Ctrl+Shift+Left ["
#define KEYCONF_TabRightUpdatable  ""
#define KEYCONF_TabLeftUpdatable  ""

#define KEYCONF_TabNum1 "Alt+1"
#define KEYCONF_TabNum2 "Alt+2"
#define KEYCONF_TabNum3 "Alt+3"
#define KEYCONF_TabNum4 "Alt+4"
#define KEYCONF_TabNum5 "Alt+5"
#define KEYCONF_TabNum6 "Alt+6"
#define KEYCONF_TabNum7 "Alt+7"
#define KEYCONF_TabNum8 "Alt+8"
#define KEYCONF_TabNum9 "Alt+9"
#define KEYCONF_TabLast ""

#define KEYCONF_RestoreLastTab "Ctrl+T"

#define KEYCONF_PreBookMark  "Ctrl+F2"
#define KEYCONF_NextBookMark  "F2"

#define KEYCONF_PrevView  "Alt+Left"
#define KEYCONF_NextView  "Alt+Right"

#define KEYCONF_ToggleArticle  "Alt+x"

#define KEYCONF_ShowPopupMenu  "Shift+F10 Ctrl+m Menu"

#define KEYCONF_ShowMenuBar  "F8"
#define KEYCONF_ShowToolBarMain  ""
#define KEYCONF_ShowSideBar  "F9"

#define KEYCONF_PageUp  "Page_Up KP_Prior"
#define KEYCONF_PageDown  "Page_Down KP_Next"

#define KEYCONF_PrevDir  "{"
#define KEYCONF_NextDir  "}"

#define KEYCONF_Home  "Home g < KP_Home"
#define KEYCONF_End  "End G > KP_End"

#define KEYCONF_Back  "BackSpace"

#define KEYCONF_Undo  "Ctrl+/ Ctrl+z"
#define KEYCONF_Redo  "Ctrl+Z"

#define KEYCONF_Quit  "Ctrl+w q"
#define KEYCONF_Save  "Ctrl+s"
#define KEYCONF_Delete  "Delete KP_Delete"
#define KEYCONF_Reload  "F5 s"
#define KEYCONF_ReloadArticle  "Shift+F5 S"
#define KEYCONF_StopLoading  "Escape"
#define KEYCONF_OpenURL "Ctrl+o"
#define KEYCONF_Copy  "Ctrl+c"
#define KEYCONF_SelectAll  "Ctrl+a"
#define KEYCONF_AppendFavorite  "Ctrl+d"
#define KEYCONF_PreferenceView  "Ctrl+P"

#define KEYCONF_Search  "Ctrl+f / KP_Divide"
#define KEYCONF_SearchInvert  "?"
#define KEYCONF_SearchNext  "Enter F3 Ctrl+g"
#define KEYCONF_SearchPrev  "Shift+Enter Ctrl+F3 Ctrl+G N"
#define KEYCONF_SearchTitle "Ctrl+t"
#define KEYCONF_DrawOutAnd  "Ctrl+Enter"

#define KEYCONF_CheckUpdateRoot ""
#define KEYCONF_CheckUpdateOpenRoot ""

#define KEYCONF_FullScreen "F11"

// BBSLIST
#define KEYCONF_OpenBoard  "Space"
#define KEYCONF_OpenBoardTab  "Ctrl+Space"

// BOARD
#define KEYCONF_OpenArticle  "Space"
#define KEYCONF_OpenArticleTab  "Ctrl+Space"
#define KEYCONF_NewArticle  "w"
#define KEYCONF_SearchCache  "Ctrl+Enter"

#define KEYCONF_ScrollRightBoard  "L Shift+Right"
#define KEYCONF_ScrollLeftBoard  "H Shift+Left"

#define KEYCONF_SortColumnNoDefault  ""

// ARTICLE
#define KEYCONF_UpMid  "u"
#define KEYCONF_UpFast  "b Page_Up KP_Prior"

#define KEYCONF_DownMid  "d"
#define KEYCONF_DownFast  "Page_Down Space KP_Next"

#define KEYCONF_PrevRes  "p"
#define KEYCONF_NextRes  "n"

#define KEYCONF_PrePost  "Ctrl+Shift+F2"
#define KEYCONF_NextPost  "Shift+F2"

#define KEYCONF_GotoNew  "F4"
#define KEYCONF_WriteMessage  "w Alt+w"

#define KEYCONF_LiveStartStop  "F6"

#define KEYCONF_SearchNextArticle "Ctrl+Space"
#define KEYCONF_SearchWeb "Ctrl+k"
#define KEYCONF_SearchCacheLocal "Ctrl+Enter"
#define KEYCONF_SearchCacheAll ""

#define KEYCONF_ShowSelectImage  "Ctrl+I"
#define KEYCONF_DeleteSelectImage  ""
#define KEYCONF_AboneSelectImage  ""
#define KEYCONF_AboneSelectionRes  ""

// IMAGE
#define KEYCONF_CancelMosaic  "c"
#define KEYCONF_ZoomFitImage  "x"
#define KEYCONF_ZoomInImage  "+ KP_Add"  // (注意) ver.2.0.2 以前は + は Plus だった
#define KEYCONF_ZoomOutImage  "- KP_Subtract"
#define KEYCONF_OrgSizeImage  "z"

#define KEYCONF_ScrollUpImage  "K k Shift+Up Up"
#define KEYCONF_ScrollDownImage  "J j Shift+Down Down"
#define KEYCONF_ScrollLeftImage  "H Shift+Left"
#define KEYCONF_ScrollRightImage  "L Shift+Right"

// MESSAGE
#define KEYCONF_CancelWrite  "Alt+q"
#define KEYCONF_ExecWrite  "Alt+w" 

#define KEYCONF_FocusWrite  "Tab" 
#define KEYCONF_ToggleSage  "Alt+s" 

// EDIT
#define KEYCONF_HomeEdit  ""
#define KEYCONF_EndEdit  ""

#define KEYCONF_UpEdit  ""
#define KEYCONF_DownEdit  ""
#define KEYCONF_RightEdit  ""
#define KEYCONF_LeftEdit  ""

#define KEYCONF_DeleteEdit  ""
#define KEYCONF_BackspEdit  ""
#define KEYCONF_UndoEdit  "Ctrl+/ Ctrl+z"
#define KEYCONF_EnterEdit  ""

#define KEYCONF_InputAA  "Alt+a"

// JD globals
#define KEYCONF_JDExit  "Ctrl+q"
#define KEYCONF_JDHelp  "F1"

//////////////////////////////////

// マウスジェスチャ

//      8
//      ↑
//  4 ←  → 6
//      ↓
//      2 
//
// ( 例 ) ↑→↓← = 8624

// 共通
#define MOUSECONF_Right "6"
#define MOUSECONF_Left "4"

#define MOUSECONF_TabRight "86"
#define MOUSECONF_TabLeft "84"
#define MOUSECONF_TabRightUpdated ""
#define MOUSECONF_TabLeftUpdated ""
#define MOUSECONF_TabRightUpdatable ""
#define MOUSECONF_TabLeftUpdatable ""

#define MOUSECONF_CloseAllTabs ""
#define MOUSECONF_CloseOtherTabs ""

#define MOUSECONF_RestoreLastTab "64"

#define MOUSECONF_CheckUpdateTabs ""

#define MOUSECONF_ToggleArticle "2"
#define MOUSECONF_ShowSideBar  ""
#define MOUSECONF_ShowMenuBar ""
#define MOUSECONF_ShowToolBarMain ""
#define MOUSECONF_Home "68"
#define MOUSECONF_End "62"
#define MOUSECONF_Quit "26"
#define MOUSECONF_Reload "82"
#define MOUSECONF_Delete "262"
#define MOUSECONF_StopLoading "8"
#define MOUSECONF_AppendFavorite  ""

#define MOUSECONF_NewArticle "24"
#define MOUSECONF_WriteMessage "24"

#define MOUSECONF_SearchTitle ""

#define MOUSECONF_CheckUpdateRoot "48"
#define MOUSECONF_CheckUpdateOpenRoot "42"

#define MOUSECONF_QuitJD ""
#define MOUSECONF_MaximizeMainWin ""
#define MOUSECONF_IconifyMainWin ""

// ARTICLE
#define MOUSECONF_GotoNew "626"
#define MOUSECONF_SearchNextArticle "28"
#define MOUSECONF_SearchWeb ""
#define MOUSECONF_LiveStartStop ""

// IMAGE
#define MOUSECONF_CancelMosaicButton "28"


//////////////////////////////////

// ボタン割り当て

// 共通
#define BUTTONCONF_ClickButton "Left"
#define BUTTONCONF_DblClickButton "DblLeft"
#define BUTTONCONF_TrpClickButton "TrpLeft"
#define BUTTONCONF_CloseTabButton "Mid"
#define BUTTONCONF_ReloadTabButton "DblLeft"
#define BUTTONCONF_AutoScrollButton "Mid"
#define BUTTONCONF_GestureButton "Right"
#define BUTTONCONF_PopupmenuButton "Right"
#define BUTTONCONF_DragStartButton "Left"
#define BUTTONCONF_TreeRowSelectionButton "Mid"
#define BUTTONCONF_Reload "Button4"
#define BUTTONCONF_ToggleArticle "Button5"

#define BUTTONCONF_Right ""
#define BUTTONCONF_Left ""

// BBSLIST用ボタン設定
#define BUTTONCONF_OpenBoardButton "Left"
#define BUTTONCONF_OpenBoardTabButton "Mid"

// BOARD用ボタン設定
#define BUTTONCONF_OpenArticleButton "Left"
#define BUTTONCONF_OpenArticleTabButton "Mid"

#define BUTTONCONF_ScrollRightBoard "Tilt_Right"
#define BUTTONCONF_ScrollLeftBoard "Tilt_Left"

// ARTICLE用ボタン設定
#define BUTTONCONF_PopupWarpButton ""

#define BUTTONCONF_ReferResButton "Right"
#define BUTTONCONF_BmResButton "Mid"
#define BUTTONCONF_PostedMarkButton "Ctrl+Left"
#define BUTTONCONF_PopupmenuResButton "Left"

#define BUTTONCONF_DrawoutAncButton "Mid"
#define BUTTONCONF_PopupmenuAncButton "Left Right"
#define BUTTONCONF_JumpAncButton ""

#define BUTTONCONF_PopupIDButton "Right"
#define BUTTONCONF_DrawoutIDButton "Mid"
#define BUTTONCONF_PopupmenuIDButton "Left"

#define BUTTONCONF_OpenImageButton "Left"
#define BUTTONCONF_OpenBackImageButton "Mid Ctrl+Left"
#define BUTTONCONF_PopupmenuImageButton "Right"

#define BUTTONCONF_OpenBeButton "Left Mid"
#define BUTTONCONF_PopupmenuBeButton "Right"

// IMAGE ICON用ボタン設定
#define BUTTONCONF_CloseImageTabButton "Mid"

// IMAGE用ボタン設定
#define BUTTONCONF_CloseImageButton "Mid"
#define BUTTONCONF_ScrollImageButton "Left"
#define BUTTONCONF_CancelMosaicButton ""
#define BUTTONCONF_SaveImageButton ""
#define BUTTONCONF_ResizeImageButton "Left"

}

#endif
