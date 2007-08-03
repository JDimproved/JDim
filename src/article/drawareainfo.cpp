// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "drawareainfo.h"

#include "fontid.h"

using namespace ARTICLE;


DrawAreaInfo::DrawAreaInfo( const std::string& url )
    : DrawAreaBase( url )
{
#ifdef _DEBUG
    std::cout << "DrawAreaInfo::DrawAreaInfo url = " << url << std::endl;
#endif

    // フォント設定
    set_fontid( FONT_ENTRY_DEFAULT );

    // 文字色
    set_colorid_text( COLOR_CHAR_ENTRY_DEFAULT );

    // 背景色
    set_colorid_back( COLOR_BACK_ENTRY_DEFAULT );

    setup( false, true );
}
