// ライセンス: 最新のGPL

//#define _DEBUG
#include "jddebug.h"

#include "drawareapopup.h"

#include "config/globalconf.h"

#include "global.h"

using namespace ARTICLE;

// スクロールバーが付くとレイアウトがずれるのでクライアント領域の横幅をその分広げる
#define POPUP_RIGHT_MRG 40
#define POPUP_OFFSET_Y 1


DrawAreaPopup::DrawAreaPopup( const std::string& url )
    : DrawAreaBase( url )
{
#ifdef _DEBUG
    std::cout << "DrawAreaPopup::DrawAreaPopup url = " << url << std::endl;
#endif

    setup( false, false );
}


// 背景色
const int* DrawAreaPopup::rgb_color_back()
{
    return  CONFIG::get_color_back_popup();
}


// フォント
const std::string& DrawAreaPopup::fontname()
{
    return CONFIG::get_fontname_popup();

}

// フォントモード
const int DrawAreaPopup::fontmode()
{
    return FONT_POPUP;
}



//
// レイアウト実行
//
void DrawAreaPopup::layout()
{
    // まだクライアント領域のサイズが未取得のときはwrapなしで強制的にレイアウトする
    // (親ウィンドウにクライアントのサイズを知らせるため)
    bool nowrap = ( width_client() == 0 || height_client() == 0 );

#ifdef _DEBUG
    std::cout << "DrawAreaPopup::layout() " << get_url() << " nowrap = " << nowrap << std::endl;
#endif

    layout_impl( nowrap, POPUP_OFFSET_Y, POPUP_RIGHT_MRG );
}



//
// バックスクリーンをDrawAreaにコピー
//
// ポップアップの場合は枠も描く
//
bool DrawAreaPopup::draw_drawarea()
{
    // 描画
    if( ! DrawAreaBase::draw_drawarea() ) return false;

    // フレーム描画
    draw_frame();

    return true;
}



//////////////////////////////////////////////


// あぼーんされたレスも表示する
DrawAreaPopupShowAbone::DrawAreaPopupShowAbone( const std::string& url )
    : DrawAreaPopup( url )
{
#ifdef _DEBUG
    std::cout << "DrawAreaPopupShowAbone::DrawAreaPopupShowAbone url = " << url << std::endl;
#endif

    setup( true, false );
}
