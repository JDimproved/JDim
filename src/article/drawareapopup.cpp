// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "drawareapopup.h"

#include "config/globalconf.h"

#include "colorid.h"
#include "fontid.h"

using namespace ARTICLE;

// スクロールバーが付くとレイアウトがずれるのでクライアント領域の横幅をその分広げる
#define POPUP_RIGHT_MRG 40
#define POPUP_OFFSET_Y 1


// show_abone == true ならあぼーんされたスレも表示
DrawAreaPopup::DrawAreaPopup( const std::string& url, bool show_abone )
    : DrawAreaBase( url )
{
#ifdef _DEBUG
    std::cout << "DrawAreaPopup::DrawAreaPopup url = " << url << std::endl;
#endif

    // フォント設定
    set_fontid( FONT_POPUP );

    // 背景色
    set_colorid_back( COLOR_BACK_POPUP );

    setup( show_abone, false );
    set_draw_frame( true );
}


//
// レイアウト実行
//
void DrawAreaPopup::exec_layout()
{
    // まだクライアント領域のサイズが未取得のときはwrapなしで強制的にレイアウトする
    // (親ウィンドウにクライアントのサイズを知らせるため)
    bool nowrap = ( width_client() == 0 || height_client() == 0 );

#ifdef _DEBUG
    std::cout << "DrawAreaPopup::exec_layout() " << get_url() << " nowrap = " << nowrap << std::endl;
#endif

    if( exec_layout_impl( nowrap, POPUP_OFFSET_Y, POPUP_RIGHT_MRG ) ) draw_backscreen( true );
}


//
// drawarea がリサイズした
//
// ポップアップの場合は先頭に戻る
//
bool DrawAreaPopup::slot_configure_event( GdkEventConfigure* event )
{
    exec_layout();
    redraw_view();
    DrawAreaBase::goto_top();

    return true;
}
