// ライセンス: GPL2

// コントロールID

#ifndef _CONTROLID_H
#define _CONTROLID_H

namespace CONTROL
{
    // コントロールモード
    enum
    {
        MODE_COMMON,
        MODE_BBSLIST,
        MODE_BOARD,
        MODE_ARTICLE,
        MODE_IMAGE,
        MODE_MESSAGE,
        MODE_EDIT,

        MODE_ERROR
    };

    // 動作
    //
    // 綱目を増やしたら controllabel.h も修正すること
    //
    enum
    {
        // 共通
        COMMONMOTION = 0,

        Up,
        Down,

        Right,
        Left,

        TabRight,
        TabLeft,

        PreBookMark,
        NextBookMark,
        
        ToggleArticle,

        ShowPopupMenu,

        ShowMenuBar,
        ShowSideBar,

        PageUp,
        PageDown,

        Home,
        End,

        Back,

        Quit,
        Save,
        Delete,
        Reload,
        StopLoading,
        Cancel = StopLoading,
        Copy,
        AppendFavorite,
        Property,
    
        Search,
        CloseSearchBar,
        HiLightOff,
        SearchInvert,
        SearchNext,
        SearchPrev,
        DrawOutAnd,
        DrawOutOr,

        ClickButton, // 以下、マウスボタン専用の設定
        DblClickButton,
        CloseTabButton,
        ReloadTabButton,
        AutoScrollButton,
        GestureButton,
        PopupmenuButton,
        DragStartButton,
        TreeRowSelectionButton,

        ScrollUp,
        ScrollDown,
        ScrollLeft,
        ScrollRight,

        COMMONMOTION_END,

        // BBSLIST系
        BBSLISTMOTION,

        OpenBoard,
        OpenBoardTab,

        OpenBoardButton, // 以下、マウスボタン専用の設定
        OpenBoardTabButton,

        BBSLISTMOTION_END,

        // BOARD系
        BOARDMOTION,

        OpenArticle,
        OpenArticleTab,
        NewArticle,
        SearchCache,

        OpenArticleButton, // 以下、マウスボタン専用の設定
        OpenArticleTabButton,

        BOARDMOTION_END,

        // ARTICLE系
        ARTICLEMOTION,

        UpMid,
        UpFast,

        DownMid,
        DownFast,

        GotoNew,
        OpenParentBoard,
        WriteMessage,

        PopupWarpButton, // 以下、マウスボタン専用の設定
        ReferResButton, 
        BmResButton,
        PopupmenuResButton,

        DrawoutAncButton,
        PopupmenuAncButton,

        PopupIDButton,
        DrawoutIDButton,
        PopupmenuIDButton,

        OpenImageButton,
        OpenBackImageButton,
        PopupmenuImageButton,

        OpenBeButton,
        PopupmenuBeButton,

        ARTICLEMOTION_END,

        // IMAGE 系
        IMAGEMOTION,

        CancelMosaic,
        ZoomFitImage,
        ZoomInImage,
        ZoomOutImage,
        OrgSizeImage,

        IMAGEMOTION_END,

        // MESSAGE 系
        MESSAGEMOTION,

        CancelWrite,
        ExecWrite,

        FocusWrite,

        MESSAGEMOTION_END,

        // EDIT 系
        EDITMOTION,

        HomeEdit,
        EndEdit,

        UpEdit,
        DownEdit,
        RightEdit,
        LeftEdit,

        DeleteEdit,
        UndoEdit,

        InputAA,

        EDITMOTION_END,

        // その他
        CancelMG,
        None,

        CONTROL_END
    };
}

#endif
