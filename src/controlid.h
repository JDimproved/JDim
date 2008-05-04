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
        MODE_IMAGEICON,
        MODE_IMAGEVIEW,
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

        PrevView,
        NextView,
        
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
        SelectAll,
        AppendFavorite,
        Property,
        Lock,
    
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

        COMMONMOTION_END,

        // BBSLIST系
        BBSLISTMOTION,

        OpenBoard,
        OpenBoardTab,

        OpenBoardButton, // 以下、マウスボタン専用の設定
        OpenBoardTabButton,

        CheckUpdateRoot,
        CheckUpdateOpenRoot,

        BBSLISTMOTION_END,

        // BOARD系
        BOARDMOTION,

        OpenArticle,
        OpenArticleTab,
        NewArticle,
        SearchCache,

        ScrollLeftBoard,
        ScrollRightBoard,

        OpenArticleButton, // 以下、マウスボタン専用の設定
        OpenArticleTabButton,

        BOARDMOTION_END,

        // ARTICLE系
        ARTICLEMOTION,

        UpMid,
        UpFast,

        DownMid,
        DownFast,

        PrevRes,
        NextRes,

        GotoNew,
        OpenParentBoard,
        WriteMessage,

        LiveStartStop,

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

        // IMAGE ICON 系
        IMAGEICONMOTION,

        CancelMosaic,
        ZoomFitImage,
        ZoomInImage,
        ZoomOutImage,
        OrgSizeImage,

        ScrollUpImage,
        ScrollDownImage,
        ScrollLeftImage,
        ScrollRightImage,

        CloseImageTabButton, // 以下、マウスボタン専用の設定

        IMAGEICONMOTION_END,

        // IMAGE VIEW 系
        IMAGEVIEWMOTION,

        CloseImageButton, // 以下、マウスボタン専用の設定
        ScrollImageButton,
        CancelMosaicButton,

        IMAGEVIEWMOTION_END,

        // MESSAGE 系
        MESSAGEMOTION,

        CancelWrite,
        ExecWrite,
        InsertText,
        LockMessage,
        Preview,

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
        BackspEdit,
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
