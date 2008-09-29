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

#define KEYCONF_Up  "k Up KP_Up"
#define KEYCONF_Down  "j Down KP_Down"

#define KEYCONF_Right  "l Right KP_Right"
#define KEYCONF_Left  "h Left KP_left"

#define KEYCONF_TabRight  "Ctrl+Page_Down Ctrl+Tab Ctrl+Left_Tab Ctrl+l Ctrl+Right"
#define KEYCONF_TabLeft  "Ctrl+Page_Up Ctrl+Shift+Tab Ctrl+Shift+Left_Tab Ctrl+h Ctrl+Left"

#define KEYCONF_PreBookMark  "Ctrl+F2"
#define KEYCONF_NextBookMark  "F2"

#define KEYCONF_PrevView  "Alt+Left"
#define KEYCONF_NextView  "Alt+Right"

#define KEYCONF_ToggleArticle  "Alt+x"

#define KEYCONF_ShowPopupMenu  "Shift+F10 Ctrl+m Menu"

#define KEYCONF_ShowMenuBar  "F8"
#define KEYCONF_ShowSideBar  "F9"

#define KEYCONF_PageUp  "Page_Up KP_Prior"
#define KEYCONF_PageDown  "Page_Down KP_Next"

#define KEYCONF_Home  "Home g < KP_Home"
#define KEYCONF_End  "End G > KP_End"

#define KEYCONF_Back  "BackSpace"

#define KEYCONF_Quit  "Ctrl+w q"
#define KEYCONF_Save  "Ctrl+s"
#define KEYCONF_Delete  "Delete KP_Delete"
#define KEYCONF_Reload  "F5 s"
#define KEYCONF_StopLoading  "Escape"
#define KEYCONF_Copy  "Ctrl+c"
#define KEYCONF_SelectAll  "Ctrl+a"

#define KEYCONF_Search  "Ctrl+f / KP_Divide"
#define KEYCONF_SearchInvert  "?"
#define KEYCONF_SearchNext  "Enter F3 Ctrl+g n"
#define KEYCONF_SearchPrev  "Shift+Enter Ctrl+F3 Ctrl+G N"
#define KEYCONF_DrawOutAnd  "Ctrl+Enter"

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

// ARTICLE
#define KEYCONF_UpMid  "u"
#define KEYCONF_UpFast  "b Page_Up KP_Prior"

#define KEYCONF_DownMid  "d"
#define KEYCONF_DownFast  "Page_Down Space KP_Next"

#define KEYCONF_PrevRes  "p"
#define KEYCONF_NextRes  "n"

#define KEYCONF_GotoNew  "F4"
#define KEYCONF_WriteMessage  "w Alt+w"

#define KEYCONF_LiveStartStop  "F6"

// IMAGE
#define KEYCONF_CancelMosaic  "c"
#define KEYCONF_ZoomFitImage  "x"
#define KEYCONF_ZoomInImage  "+ KP_Add"
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

#define KEYCONF_InputAA  "Alt+a"

}

#endif
