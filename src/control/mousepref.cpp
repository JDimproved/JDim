// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "mousepref.h"
#include "controlid.h"
#include "controlutil.h"


using namespace CONTROL;


//
// マウスジェスチャ入力をラベルに表示するダイアログ
//
MouseInputDiag::MouseInputDiag( Gtk::Window* parent, const std::string& url, const int id )
    : CONTROL::InputDiag( parent, url, id, "マウスジェスチャ", INPUTDIAG_MODE_MOUSE )
{}


///////////////////////////////


//
// 個別のマウスジェスチャ設定ダイアログ
//
MouseDiag::MouseDiag( Gtk::Window* parent, const std::string& url, const int id, const std::string& str_motions )
    : CONTROL::MouseKeyDiag( parent, url, id, "マウスジェスチャ", str_motions )
{}


InputDiag* MouseDiag::create_inputdiag()
{
    return new MouseInputDiag( this, "", get_id() );
}


const std::string MouseDiag::get_default_motions( const int id )
{
    return CONTROL::get_default_mousemotions( id );
}


const std::vector< int > MouseDiag::check_conflict( const int mode, const std::string& str_motion )
{
    return CONTROL::check_mouse_conflict( mode, str_motion );
}


///////////////////////////////////////////////


//
// マウスジェスチャ設定ダイアログ
//
MousePref::MousePref( Gtk::Window* parent, const std::string& url )
    : MouseKeyPref( parent, url, "マウスジェスチャ" )
{

    // マウスジェスチャのバックアップを取る
    // キャンセルを押したら戻す
    CONTROL::bkup_mouseconfig();

    append_comment_row( "■ "+ CONTROL::get_mode_label( CONTROL::MODE_COMMON ) );
    append_row( CONTROL::Right );
    append_row( CONTROL::Left );
    append_row( CONTROL::TabRight );
    append_row( CONTROL::TabLeft );
    append_row( CONTROL::ToggleArticle );
    append_row( CONTROL::Home );
    append_row( CONTROL::End );
    append_row( CONTROL::Quit );
    append_row( CONTROL::Reload );
    append_row( CONTROL::StopLoading );

    append_comment_row( "" );
    append_comment_row( "■ "+ CONTROL::get_mode_label( CONTROL::MODE_BBSLIST ) );
    append_row( CONTROL::CheckUpdateRoot );
    append_row( CONTROL::CheckUpdateOpenRoot );

    append_comment_row( "" );
    append_comment_row( "■ "+ CONTROL::get_mode_label( CONTROL::MODE_BOARD ) );
    append_row( CONTROL::NewArticle );

    append_comment_row( "" );
    append_comment_row( "■ "+ CONTROL::get_mode_label( CONTROL::MODE_ARTICLE ) );
    append_row( CONTROL::GotoNew );
    append_row( CONTROL::WriteMessage );

    append_comment_row( "" );
    append_comment_row( "■ "+ CONTROL::get_mode_label( CONTROL::MODE_IMAGEVIEW ) );
    append_row( CONTROL::CancelMosaicButton );
}


MouseKeyDiag* MousePref::create_setting_diag( const int id, const std::string& str_motions )
{
    return new MouseDiag( this, "", id, str_motions );
}


const std::string MousePref::get_str_motions( const int id )
{
    return CONTROL::get_str_mousemotions( id );
}


const std::string MousePref::get_default_motions( const int id )
{
    return CONTROL::get_default_mousemotions( id );
}


void MousePref::set_motions( const std::string& name, const std::string& str_motions )
{
    CONTROL::set_mousemotions( name, str_motions );
}


const bool MousePref::remove_motions( const int id )
{
    return CONTROL::remove_mousemotions( id );
}


//
// キャンセルボタンを押した
//
void MousePref::slot_cancel_clicked()
{
#ifdef _DEBUG
    std::cout << "MousePref::slot_cancel_clicked\n";
#endif

    // 設定を戻す
    CONTROL::restore_mouseconfig();
}
