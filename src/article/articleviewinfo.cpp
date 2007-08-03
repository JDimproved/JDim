// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "articleadmin.h"
#include "articleviewinfo.h"
#include "drawareainfo.h"

#include "jdlib/misctime.h"

#include "controlid.h"

#include <sys/time.h>

using namespace ARTICLE;


ArticleViewInfo::ArticleViewInfo( const std::string& url )
    : ArticleViewBase( url )
{
    struct timeval tv;
    struct timezone tz;
    gettimeofday( &tv, &tz );
    set_url( url_article() + MISC::timevaltostr( tv ) + ARTICLE_SIGN + "_PREV_" );

#ifdef _DEBUG
    std::cout << "ArticleViewInfo::ArticleViewInfo " << get_url() << std::endl;
#endif

    setup_view();
}



ArticleViewInfo::~ArticleViewInfo()
{

#ifdef _DEBUG    
    std::cout << "ArticleViewInfo::~ArticleViewInfo : " << get_url() << std::endl;
#endif
}


DrawAreaBase* ArticleViewInfo::create_drawarea()
{
    return Gtk::manage( new ARTICLE::DrawAreaInfo( url_article() ) );
}

//
// ウィジットのパッキング
//
// ArticleViewBase::pack_widget()をオーパロードしてツールバーをパックしない
//
void ArticleViewInfo::pack_widget()
{
    pack_start( *drawarea() );
}

