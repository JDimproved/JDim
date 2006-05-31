// ライセンス: 最新のGPL

//#define _DEBUG
#include "jddebug.h"

#include "articleviewpopup.h"
#include "drawareapopup.h"

#include "jdlib/misctime.h"

#include "global.h"

#include "config/globalconf.h"

#include <sys/time.h>

using namespace ARTICLE;


// show_abone == true ならあぼーんされたスレも表示
ArticleViewPopup::ArticleViewPopup( const std::string& url, bool show_abone )
    : ArticleViewBase( url ), m_show_abone( show_abone )
{
    struct timeval tv;
    struct timezone tz;
    gettimeofday( &tv, &tz );
    set_url( url_article() + MISC::timevaltostr( tv ) + "_POPUP_" );

#ifdef _DEBUG
    std::cout << "ArticleViewPopup::ArticleViewPupup " << get_url() << " show_abone " << m_show_abone << std::endl;
#endif

    setup_view();
}



ArticleViewPopup::~ArticleViewPopup()
{

#ifdef _DEBUG    
    std::cout << "ArticleViewPopup::~ArticleViewPopup : " << get_url() << std::endl;
#endif
}



//
// 多重ポップアップのヒント表示
//
void ArticleViewPopup::show_instruct_popup()
{
    if( CONFIG::get_instruct_popup() )
        append_html( "ヒント：マウスの右ボタンを押しながらポインタを移動すると多重ポップアップが可能" );
}


//
// drawareaの作成
//
DrawAreaBase* ArticleViewPopup::create_drawarea()
{
    if( m_show_abone ) return Gtk::manage( new ARTICLE::DrawAreaPopupShowAbone( url_article() ) );

    return Gtk::manage( new ARTICLE::DrawAreaPopup( url_article() ) );
}



//
// ウィジットのパッキング
//
// ArticleViewBase::pack_widget()をオーパロードしてツールバーをパックしない
//
void ArticleViewPopup::pack_widget()
{
    pack_start( *drawarea() );
    show_all_children();
}
