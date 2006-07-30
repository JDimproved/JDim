// ライセンス: 最新のGPL

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

        PageUp,
        PageDown,

        Home,
        End,

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

        ReferResButton, // 以下、マウスボタン専用の設定
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

        MESSAGEMOTION_END,

        // その他
        CancelMG,
        None,

        CONTROL_END
    };
}

#endif
