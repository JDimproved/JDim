// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "articleviewpopup.h"
#include "drawareapopup.h"

#include "global.h"

#include "config/globalconf.h"

using namespace ARTICLE;


// show_abone == true ならあぼーんされたスレも表示
ArticleViewPopup::ArticleViewPopup( const std::string& url, bool show_abone )
    : ArticleViewBase( url, url ), m_show_abone( show_abone )
{
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
        append_html( "ヒント：マウスの右ボタンを押しながらポインタを移動すると多重ポップアップが可能<br>"
                     "または設定メニューの「シングルクリックで多重ポップアップモードに移行する」<br>"
                     "をチェックしてからアンカーをクリックする（X11環境のみ）" );
}


//
// drawareaの作成
//
DrawAreaBase* ArticleViewPopup::create_drawarea()
{
    return Gtk::manage( new ARTICLE::DrawAreaPopup( url_article(), m_show_abone ) );
}
