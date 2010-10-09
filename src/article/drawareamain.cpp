// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "drawareamain.h"

using namespace ARTICLE;


DrawAreaMain::DrawAreaMain( const std::string& url )
    : DrawAreaBase( url )
{
#ifdef _DEBUG
    std::cout << "DrawAreaMain::DrawAreaMain url = " << url << std::endl;
#endif

    const bool show_abone = false;
    const bool show_scrbar = true;
    const bool show_multispace = false;
    setup( show_abone, show_scrbar, show_multispace );
}
