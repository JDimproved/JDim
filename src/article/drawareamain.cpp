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

    setup( false, true );
}
