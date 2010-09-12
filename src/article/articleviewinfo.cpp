// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "articleadmin.h"
#include "articleviewinfo.h"
#include "drawareainfo.h"

using namespace ARTICLE;


ArticleViewInfo::ArticleViewInfo( const std::string& url )
    : ArticleViewBase( url, url )
{
#ifdef _DEBUG
    std::cout << "ArticleViewInfo::ArticleViewInfo " << get_url() << std::endl;
#endif

    set_writeable( false );

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
