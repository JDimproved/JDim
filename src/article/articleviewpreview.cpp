// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "articleadmin.h"
#include "articleviewpreview.h"
#include "drawareamain.h"

#include "message/messageadmin.h"

#include "dbtree/articlebase.h"

#include "control/controlid.h"

#include "command.h"

using namespace ARTICLE;


ArticleViewPreview::ArticleViewPreview( const std::string& url )
    : ArticleViewBase( url, url )
{
    m_url_messageview = url;

#ifdef _DEBUG
    std::cout << "ArticleViewPreview::ArticleViewPreview " << get_url() << std::endl;
#endif

    set_writeable( false );

    setup_view();

    // コントロールモード設定
    get_control().clear_mode();
    get_control().add_mode( CONTROL::MODE_MESSAGE );
    get_control().add_mode( CONTROL::MODE_ARTICLE );
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
bool ArticleViewPreview::operate_view( const int control )
{
    if( control == CONTROL::None ) return false;

    // スクロール系操作
    if( drawarea()->set_scroll( control ) ) return true;

    switch( control ){

            // コピー
        case CONTROL::Copy:
            slot_copy_selection_str();
            break;

            // 全て選択
        case CONTROL::SelectAll:
            slot_select_all();
            break;

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
        case CONTROL::TabLeftUpdated:
            MESSAGE::get_admin()->set_command( "tab_left" );
            break;

        case CONTROL::TabRight:
        case CONTROL::TabRightUpdated:
            MESSAGE::get_admin()->set_command( "tab_right" );
        break;

        case CONTROL::FocusWrite:
            MESSAGE::get_admin()->set_command( "focus_button_write" );
        break;

        default:
            return false;
    }

    return true;
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
