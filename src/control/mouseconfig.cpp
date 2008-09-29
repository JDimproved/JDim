// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "mouseconfig.h"
#include "mousekeyitem.h"
#include "controlutil.h"

#include "jdlib/miscutil.h"
#include "jdlib/miscmsg.h"
#include "jdlib/confloader.h"

#include "cache.h"

CONTROL::MouseConfig* instance_mouseconfig = NULL;


CONTROL::MouseConfig* CONTROL::get_mouseconfig()
{
    if( ! instance_mouseconfig ) instance_mouseconfig = new CONTROL::MouseConfig();

    return instance_mouseconfig;
}


void CONTROL::delete_mouseconfig()
{
    if( instance_mouseconfig ) delete instance_mouseconfig;
    instance_mouseconfig = NULL;
}


//////////////////////////////////////////////////////////

using namespace CONTROL;


MouseConfig::MouseConfig()
    : MouseKeyConf()
{}



MouseConfig::~MouseConfig()
{}


//
// 設定ファイル読み込み
//
void MouseConfig::load_conf()
{
    std::string str_motions;
    JDLIB::ConfLoader cf( CACHE::path_mouseconf(), std::string() );

    // 共通
    SETMOTION( "Right", "6" );
    SETMOTION( "Left", "4" );
    SETMOTION( "TabRight", "86" );
    SETMOTION( "TabLeft", "84" );
    SETMOTION( "ToggleArticle", "2" );
    SETMOTION( "Home", "68" );
    SETMOTION( "End", "62" );
    SETMOTION( "Quit", "26" );
    SETMOTION( "Reload", "82" );
    SETMOTION( "StopLoading", "8" );
    SETMOTION( "NewArticle", "24" );
    SETMOTION( "WriteMessage", "24" );

    // 全お気に入り更新チェック
    SETMOTION( "CheckUpdateRoot", "48" );
    SETMOTION( "CheckUpdateOpenRoot", "42" );

    // ARTICLE
    SETMOTION( "GotoNew", "626" );

    // IMAGE
    SETMOTION( "CancelMosaicButton", "28" );
}



// ひとつの操作をデータベースに登録
void MouseConfig::set_one_motion_impl( const int id, const int mode, const std::string& name, const std::string& str_motion )
{
    if( name.empty() || str_motion.empty() ) return;

#ifdef _DEBUG
    std::cout << "MouseConfig::set_one_motion_impl " << name << std::endl;
    std::cout << "motion = " << str_motion << std::endl;
#endif

#ifdef _DEBUG
    std::cout << CONTROL::get_label( id  ) << std::endl;
#endif

    const bool ctrl = false;
    const bool shift = false;
    const bool alt = false;
    const guint motion = atoi( str_motion.c_str() );
    if( !motion ) return;
    const bool dblclick = false;
    const bool trpclick = false;

    int id_check = check_conflict( mode, motion, ctrl, shift, alt, dblclick, trpclick );
    if( id_check != CONTROL::None ){
        MISC::ERRMSG( "mouse config : ID " + str_motion + " は既に使われています。" );
        return;
    }

    vec_items().push_back( MouseKeyItem( id, mode, name, str_motion, motion, ctrl, shift, alt, dblclick, trpclick ) );
}


// 操作文字列取得
const std::string MouseConfig::get_str_motions( const int id_ )
{
    int id = id_;

    // (注) この行が無いと画像ビューのコンテキストメニューにマウスジェスチャが表示されない
    if( id == CONTROL::CancelMosaic ) id = CONTROL::CancelMosaicButton;

    std::string motions = MouseKeyConf::get_str_motions( id );
    motions = MISC::replace_str( motions, "8", "↑" );
    motions = MISC::replace_str( motions, "6", "→" );
    motions = MISC::replace_str( motions, "4", "←" );
    motions = MISC::replace_str( motions, "2", "↓" );

    return motions;
}
