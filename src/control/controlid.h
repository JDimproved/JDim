// ライセンス: GPL2

// コントロールID

#ifndef _CONTROLID_H
#define _CONTROLID_H

namespace CONTROL
{
    // コントロールモード
    //
    // 項目を増やしたら controllabel.h も修正すること
    //
    enum
    {
        MODE_START = 0,

        MODE_COMMON = MODE_START,
        MODE_BBSLIST,
        MODE_BOARD,
        MODE_ARTICLE,
        MODE_IMAGEICON,
        MODE_IMAGEVIEW,
        MODE_MESSAGE,
        MODE_EDIT,
        MODE_JDGLOBALS,

        MODE_END = MODE_JDGLOBALS,

        MODE_ERROR
    };


    // 動作
    //
    // 項目を増やしたら controllabel.h も修正すること
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
        TabRightUpdated,
        TabLeftUpdated,
        TabRightUpdatable, ///< 右の更新可能なタブに移動
        TabLeftUpdatable,  ///< 左の更新可能なタブに移動

        TabNum1,
        TabNum2,
        TabNum3,
        TabNum4,
        TabNum5,
        TabNum6,
        TabNum7,
        TabNum8,
        TabNum9,
        TabLast,

        CloseAllTabs,
        CloseOtherTabs,

        RestoreLastTab,

        CheckUpdateTabs,

        PreBookMark,
        NextBookMark,

        PrevView,
        NextView,
        
        ToggleArticle,

        ShowPopupMenu,

        ShowMenuBar,
        ShowToolBarMain,
        ShowSideBar,

        PageUp,
        PageDown,

        PrevDir,
        NextDir,

        Home,
        End,

        Back,

        Undo,
        Redo,

        Quit,
        Save,
        SaveDat,            // alias: Save
        Delete,
        Reload,
        ReloadArticle,
        StopLoading,
        Cancel = StopLoading,
        OpenURL,
        Copy,
        SelectAll,
        AppendFavorite,
        Lock,

        PreferenceView,
        PreferenceBoard,    // alias: PreferenceView
        PreferenceArticle,  // alias: PreferenceView
        PreferenceImage,    // alias: PreferenceView

        Search,
        CloseSearchBar,
        HiLightOff,
        SearchInvert,
        SearchNext,
        SearchPrev,
        SearchTitle,
        DrawOutAnd,
        DrawOutOr,

        CheckUpdateRoot,
        CheckUpdateOpenRoot,

        QuitJD,
        MaximizeMainWin,
        IconifyMainWin,
        FullScreen,

        ClickButton, // 以下、マウスボタン専用の設定
        DblClickButton,
        TrpClickButton,
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

        BBSLISTMOTION_END,

        // BOARD系
        BOARDMOTION,

        OpenArticle,
        OpenArticleTab,
        NewArticle,
        SearchCache,

        ScrollLeftBoard,
        ScrollRightBoard,

        SortColumnMark,
        SortColumnID,
        SortColumnBoard,
        SortColumnSubject,
        SortColumnRes,
        SortColumnStrLoad,
        SortColumnStrNew,
        SortColumnSince,
        SortColumnWrite,
        SortColumnAccess,
        SortColumnSpeed,
        SortColumnDiff,

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

        PrePost,
        NextPost,

        GotoNew,
        OpenParentBoard,
        WriteMessage,

        LiveStartStop,

        SearchNextArticle,
        SearchWeb,
        SearchCacheLocal,
        SearchCacheAll,

        ShowSelectImage,
        DeleteSelectImage,
        AboneSelectImage,
        AboneSelectionRes,

        PopupWarpButton, // 以下、マウスボタン専用の設定
        ReferResButton, 
        BmResButton,
        PostedMarkButton,
        PopupmenuResButton,

        DrawoutAncButton,
        PopupmenuAncButton,
        JumpAncButton,

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
        SaveImageButton,
        ResizeImageButton,

        IMAGEVIEWMOTION_END,

        // MESSAGE 系
        MESSAGEMOTION,

        CancelWrite,
        ExecWrite,
        InsertText,
        LockMessage,
        Preview,

        FocusWrite,
        ToggleSage,

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
        EnterEdit,

        InputAA,

        EDITMOTION_END,

        // JD globals
        JDGLOBALS,

        JDExit,
        JDHelp,

        JDGLOBALS_END,

        // その他
        CancelMG,
        NoOperation,

        CONTROL_END
    };
}

#endif
