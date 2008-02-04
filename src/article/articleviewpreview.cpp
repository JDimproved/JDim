// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "articleadmin.h"
#include "articleviewpreview.h"
#include "drawareamain.h"

#include "message/messageadmin.h"

#include "dbtree/articlebase.h"

#include "jdlib/misctime.h"

#include "controlid.h"
#include "command.h"

#include <sys/time.h>

using namespace ARTICLE;


ArticleViewPreview::ArticleViewPreview( const std::string& url )
    : ArticleViewBase( url )
{
    m_url_messageview = url;

    struct timeval tv;
    struct timezone tz;
    gettimeofday( &tv, &tz );
    set_url( url_article() + MISC::timevaltostr( tv ) + ARTICLE_SIGN + "_PREV_", false );

#ifdef _DEBUG
    std::cout << "ArticleViewPreview::ArticleViewPreview " << get_url() << std::endl;
#endif

    setup_view();

    // コントロールモード設定
    get_control().clear_mode();
    get_control().add_mode( CONTROL::MODE_MESSAGE );
}




ArticleViewPreview::~ArticleViewPreview()
{

#ifdef _DEBUG    
    std::cout << "ArticleViewPreview::~ArticleViewPreview : " << get_url() << std::endl;
#endif
}


//
// viewの操作
//
void ArticleViewPreview::operate_view( const int& control )
{
    switch( control ){
            
        // 閉じる
        case CONTROL::Quit:
        case CONTROL::CancelWrite:
            MESSAGE::get_admin()->set_command( "close_currentview" );
            break;

            // 書き込み実行
        case CONTROL::ExecWrite:
            MESSAGE::get_admin()->set_command( "toolbar_write", m_url_messageview );
            break;

        case CONTROL::TabLeft:
            MESSAGE::get_admin()->set_command( "tab_left" );
            break;

        case CONTROL::TabRight:
            MESSAGE::get_admin()->set_command( "tab_right" );
        break;

        case CONTROL::FocusWrite:
            MESSAGE::get_admin()->set_command( "focus_writebutton" );
        break;

        default:
            ArticleViewBase::operate_view( control );
            break;
    }
}


//
// drawarea のクリックイベント
//
// ArticleViewBase::slot_button_press()をオーパロードしてマウスジェスチャを無効にする
//
bool ArticleViewPreview::slot_button_press( std::string url, int res_number, GdkEventButton* event )
{
#ifdef _DEBUG
    std::cout << "ArticleViewPreview::slot_button_press url = " << get_url() << std::endl;
#endif

    MESSAGE::get_admin()->set_command( "switch_admin" );

    return true;
}
