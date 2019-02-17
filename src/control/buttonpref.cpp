// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "buttonpref.h"
#include "controlid.h"
#include "controlutil.h"


using namespace CONTROL;


//
// ボタン入力をラベルに表示するダイアログ
//
ButtonInputDiag::ButtonInputDiag( Gtk::Window* parent, const std::string& url, const int id )
    : CONTROL::InputDiag( parent, url, id, "マウスボタン", INPUTDIAG_MODE_BUTTON )
{}


///////////////////////////////


//
// 個別のボタン設定ダイアログ
//
ButtonDiag::ButtonDiag( Gtk::Window* parent, const std::string& url, const int id, const std::string& str_motions )
    : CONTROL::MouseKeyDiag( parent, url, id, "マウスボタン", str_motions )
{}


InputDiag* ButtonDiag::create_inputdiag()
{
    return new ButtonInputDiag( this, "", get_id() );
}


std::string ButtonDiag::get_default_motions( const int id )
{
    return CONTROL::get_default_buttonmotions( id );
}


std::vector< int > ButtonDiag::check_conflict( const int mode, const std::string& str_motion )
{
    // 衝突判定をしない
    return {};
}


///////////////////////////////////////////////


//
// マウスボタン設定ダイアログ
//
ButtonPref::ButtonPref( Gtk::Window* parent, const std::string& url )
    : MouseKeyPref( parent, url, "マウスボタン" )
{

    // マウスボタンのバックアップを取る
    // キャンセルを押したら戻す
    CONTROL::bkup_buttonconfig();

    append_comment_row( "■ "+ CONTROL::get_mode_label( CONTROL::MODE_COMMON ) );
    append_row( CONTROL::ClickButton );
    append_row( CONTROL::DblClickButton );
    append_row( CONTROL::TrpClickButton );
    append_row( CONTROL::CloseTabButton );
    append_row( CONTROL::ReloadTabButton );
    append_row( CONTROL::AutoScrollButton );
    append_row( CONTROL::GestureButton );
    append_row( CONTROL::PopupmenuButton );
    append_row( CONTROL::DragStartButton );
    append_row( CONTROL::TreeRowSelectionButton );
    append_row( CONTROL::Reload );
    append_row( CONTROL::ToggleArticle );
    append_row( CONTROL::Right );
    append_row( CONTROL::Left );

    append_comment_row( "" );
    append_comment_row( "■ "+ CONTROL::get_mode_label( CONTROL::MODE_BBSLIST ) );
    append_row( CONTROL::OpenBoardButton );
    append_row( CONTROL::OpenBoardTabButton );

    append_comment_row( "" );
    append_comment_row( "■ "+ CONTROL::get_mode_label( CONTROL::MODE_BOARD ) );
    append_row( CONTROL::OpenArticleButton );
    append_row( CONTROL::OpenArticleTabButton );
    append_row( CONTROL::ScrollRightBoard );
    append_row( CONTROL::ScrollLeftBoard );

    append_comment_row( "" );
    append_comment_row( "■ "+ CONTROL::get_mode_label( CONTROL::MODE_ARTICLE ) );
    append_row( CONTROL::PopupWarpButton );
    append_row( CONTROL::ReferResButton );
    append_row( CONTROL::BmResButton );
    append_row( CONTROL::PopupmenuResButton );
    append_row( CONTROL::DrawoutAncButton );
    append_row( CONTROL::PopupmenuAncButton );
    append_row( CONTROL::JumpAncButton );
    append_row( CONTROL::PopupIDButton );
    append_row( CONTROL::DrawoutIDButton );
    append_row( CONTROL::PopupmenuIDButton );
    append_row( CONTROL::OpenImageButton );
    append_row( CONTROL::OpenBackImageButton );
    append_row( CONTROL::PopupmenuImageButton );
    append_row( CONTROL::OpenBeButton );
    append_row( CONTROL::PopupmenuBeButton );

    append_comment_row( "" );
    append_comment_row( "■ "+ CONTROL::get_mode_label( CONTROL::MODE_IMAGEVIEW ) );
    append_row( CONTROL::CloseImageTabButton );
    append_row( CONTROL::CloseImageButton );
    append_row( CONTROL::ScrollImageButton );
    append_row( CONTROL::CancelMosaicButton );
    append_row( CONTROL::SaveImageButton );
    append_row( CONTROL::ResizeImageButton );
}


MouseKeyDiag* ButtonPref::create_setting_diag( const int id, const std::string& str_motions )
{
    return new ButtonDiag( this, "", id, str_motions );
}


std::string ButtonPref::get_str_motions( const int id )
{
    return CONTROL::get_str_buttonmotions( id );
}


std::string ButtonPref::get_default_motions( const int id )
{
    return CONTROL::get_default_buttonmotions( id );
}


void ButtonPref::set_motions( const int id, const std::string& str_motions )
{
    CONTROL::set_buttonmotions( id, str_motions );
}


bool ButtonPref::remove_motions( const int id )
{
    return CONTROL::remove_buttonmotions( id );
}


//
// キャンセルボタンを押した
//
void ButtonPref::slot_cancel_clicked()
{
#ifdef _DEBUG
    std::cout << "ButtonPref::slot_cancel_clicked\n";
#endif

    // 設定を戻す
    CONTROL::restore_buttonconfig();
}
