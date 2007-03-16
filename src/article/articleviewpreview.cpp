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
    struct timeval tv;
    struct timezone tz;
    gettimeofday( &tv, &tz );
    set_url( url_article() + MISC::timevaltostr( tv ) + ARTICLE_SIGN + "_PREV_" );

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
// ウィジットのパッキング
//
// ArticleViewBase::pack_widget()をオーパロードしてツールバーをパックしない
//
void ArticleViewPreview::pack_widget()
{
    pack_start( *drawarea() );
}




//
// ビュー切り替え
//
void ArticleViewPreview::switch_view()
{
#ifdef _DEBUG
    std::cout << "ArticleViewPreview::switch_view\n";
#endif

    CORE::core_set_command( "switch_message" );
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
            MESSAGE::get_admin()->set_command( "exec_Write" );
            break;

        case CONTROL::TabLeft:
            MESSAGE::get_admin()->set_command( "tab_left" );
            break;

        case CONTROL::TabRight:
            MESSAGE::get_admin()->set_command( "tab_right" );
        break;

        case CONTROL::FocusWrite:
            MESSAGE::get_admin()->set_command( "focus_write" );
        break;

        default:
            ArticleViewBase::operate_view( control );
            break;
    }
}



//
// dat をappend
//
// ArticleViewBase::append_dat()をオーパロードしてappendする前に表示を消す
//
void ArticleViewPreview::append_dat( const std::string& dat, int num )
{
    drawarea()->clear_screen();
    ArticleViewBase::append_dat( dat, get_article()->get_number_load() +1 );
}
