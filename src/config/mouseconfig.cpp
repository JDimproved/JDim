// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "mouseconfig.h"
#include "mousekeyitem.h"

#include "cache.h"
#include "controlutil.h"

#include "jdlib/miscutil.h"
#include "jdlib/miscmsg.h"
#include "jdlib/confloader.h"


CONFIG::MouseConfig* instance_mouseconfig = NULL;


CONFIG::MouseConfig* CONFIG::get_mouseconfig()
{
    if( ! instance_mouseconfig ) instance_mouseconfig = new CONFIG::MouseConfig();

    return instance_mouseconfig;
}


void CONFIG::delete_mouseconfig()
{
    if( instance_mouseconfig ) delete instance_mouseconfig;
    instance_mouseconfig = NULL;
}


//////////////////////////////////////////////////////////

using namespace CONFIG;


MouseConfig::MouseConfig()
    : MouseKeyConf()
{
    load_conf();
}



MouseConfig::~MouseConfig()
{
    save_conf( CACHE::path_mouseconf() );
}


//
// 設定ファイル読み込み
//
void MouseConfig::load_conf()
{
    std::string str_motion;
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

    // この行を入れないと画像ビューのコンテキストメニューにマウスジェスチャが表示されない
    // ただし設定ファイルには保存しない。MouseConfig::set_one_motion()も参照すること
    set_motion( "CancelMosaic", str_motion ); 
}



// ひとつの操作をデータベースに登録
void MouseConfig::set_one_motion( const std::string& name, const std::string& str_motion )
{
    if( name.empty() || str_motion.empty() ) return;

    int id = CONTROL::get_id( name );
    if( id == CONTROL::None ) return;

    int mode = get_mode( id );
    if( mode == CONTROL::MODE_ERROR ) return;

    bool ctrl = false;
    bool shift = false;
    bool alt = false;
    guint motion = atoi( str_motion.c_str() );
    if( !motion ) return;

    int id_check = check_conflict( mode, motion, ctrl, shift, alt, false );
    if( id_check != CONTROL::None ){
        MISC::ERRMSG( "mouse config : ID " + str_motion + " は既に使われています。" );
        return;
    }

    bool dblclick = false;
    bool save = true;

    // "CancelMosaic"は設定ファイルに保存しない
    if( name == "CancelMosaic" ) save = false;

    MouseKeyItem* item = new MouseKeyItem( id, mode, name, str_motion, motion, ctrl, shift, alt, dblclick, save );
    vec_items().push_back( item );
}


// 操作文字列取得
const std::string MouseConfig::get_str_motion( int id )
{
    std::string str_motion = MouseKeyConf::get_str_motion( id );
    str_motion = MISC::replace_str( str_motion, "8", "↑" );
    str_motion = MISC::replace_str( str_motion, "6", "→" );
    str_motion = MISC::replace_str( str_motion, "4", "←" );
    str_motion = MISC::replace_str( str_motion, "2", "↓" );

    return str_motion;
}
