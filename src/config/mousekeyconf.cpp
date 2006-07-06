// ライセンス: 最新のGPL

//#define _DEBUG
#include "jddebug.h"

#include "mousekeyconf.h"
#include "mousekeyitem.h"

#include "jdlib/confloader.h"
#include "jdlib/miscutil.h"

#include "cache.h"
#include "controlutil.h"


using namespace CONFIG;


MouseKeyConf::MouseKeyConf()
{}



MouseKeyConf::~MouseKeyConf()
{
    std::vector< MouseKeyItem* >::iterator it = m_vec_items.begin();
    for( ; it != m_vec_items.end(); ++it ) delete (*it);
}



// 設定ファイル保存
void MouseKeyConf::save_conf( const std::string& savefile )
{
#ifdef _DEBUG
    std::cout << "MouseKeyConf::save_conf " << savefile << std::endl;
#endif

    JDLIB::ConfLoader cf( savefile, std::string() );

    int id = CONTROL::COMMONMOTION;
    for( ; id < CONTROL::CONTROL_END; ++id ){

        std::string name;
        std::string motion;
        std::vector< MouseKeyItem* >::iterator it = m_vec_items.begin();
        for( ; it != m_vec_items.end(); ++it ){
            if( (*it)->get_id() == id ){
                name = (*it)->get_name();
                if( !motion.empty() ) motion += " ";
                motion += (*it)->get_str_motion();
            }
        }

        if( !name.empty() && !motion.empty() ) cf.update( name, motion );
    }

    cf.save();
}



// 操作からID取得
int MouseKeyConf::get_id( const int& mode,
                          const guint& motion, const bool& ctrl, const bool& shift, const bool& alt, const bool& dblclick )
{
    int id = CONTROL::None;;
    std::vector< MouseKeyItem* >::iterator it = m_vec_items.begin();
    for( ; it != m_vec_items.end(); ++it ){

        id = (*it)->is_activated( mode, motion, ctrl, shift, alt, dblclick );
        if( id != CONTROL::None ) break;
    }

    // 共通モードにして再帰呼び出し
    if( id == CONTROL::None && mode != CONTROL::MODE_COMMON ) return get_id( CONTROL::MODE_COMMON, motion, ctrl, shift, alt, dblclick );

#ifdef _DEBUG
    std::cout << "MouseKeyConf::get_id mode = " << mode << " id = " << id << " motion = " << motion << std::endl;
#endif

    return id;
}


// ID から操作を取得
// (注意) リストの一番上にあるものを出力
bool MouseKeyConf::get_motion( const int& id, guint& motion, bool& ctrl, bool& shift, bool& alt, bool& dblclick )
{
    std::vector< MouseKeyItem* >::iterator it = m_vec_items.begin();
    for( ; it != m_vec_items.end(); ++it ){

        if( (*it)->get_id() == id ){
            motion = (*it)->get_motion();
            ctrl = (*it)->get_ctrl();
            shift = (*it)->get_shift();
            alt = (*it)->get_alt();
            dblclick = (*it)->get_dblclick();
            return true;
        }
    }

    return false;
}




// ID が割り当てられているかチェック
bool MouseKeyConf::alloted( const int& id,
                            const guint& motion, const bool& ctrl, const bool& shift, const bool& alt, const bool& dblclick )
{
    std::vector< MouseKeyItem* >::iterator it = m_vec_items.begin();
    for( ; it != m_vec_items.end(); ++it ){
        if( (*it)->equal( motion, ctrl, shift, alt, dblclick ) == id ) return true;
    }

    return false;
}



// モーションが重複していないかチェック
int MouseKeyConf::check_conflict( const int& mode,
                                  const guint& motion, const bool& ctrl, const bool& shift, const bool& alt, const bool& dblclick )
{
    int id = CONTROL::None;;
    std::vector< MouseKeyItem* >::iterator it = m_vec_items.begin();
    for( ; it != m_vec_items.end(); ++it ){

        id = (*it)->is_activated( mode, motion, ctrl, shift, alt, dblclick );
        if( id != CONTROL::None ) break;
    }

    return id;
}




// IDからモードを取得
int MouseKeyConf::get_mode( const int& id )
{
    if( id < CONTROL::COMMONMOTION_END ) return CONTROL::MODE_COMMON;
    if( id < CONTROL::BBSLISTMOTION_END ) return CONTROL::MODE_BBSLIST;
    if( id < CONTROL::BOARDMOTION_END ) return CONTROL::MODE_BOARD;
    if( id < CONTROL::ARTICLEMOTION_END ) return CONTROL::MODE_ARTICLE;
    if( id < CONTROL::IMAGEMOTION_END ) return CONTROL::MODE_IMAGE;
    if( id < CONTROL::MESSAGEMOTION_END ) return CONTROL::MODE_MESSAGE;

    return CONTROL::MODE_ERROR;
}


const std::string MouseKeyConf::get_str_motion( int id )
{
    std::string str_motion;

    std::vector< MouseKeyItem* >::iterator it = m_vec_items.begin();
    for( ; it != m_vec_items.end(); ++it ){

        if( (*it)->get_id() == id ){
            if( ! str_motion.empty() ) str_motion += " , ";
            str_motion += (*it)->get_str_motion();
        }
    }

    return str_motion;
}



// スペースで区切られた複数の操作をデータベースに登録
void MouseKeyConf::set_motion( const std::string& name, const std::string& str_motion )
{
    std::list< std::string > list_motion = MISC::StringTokenizer( str_motion, ' ' );
    std::list< std::string >::iterator it = list_motion.begin();
    for( ; it != list_motion.end(); ++it ) set_one_motion( name, (*it) );
}


// 指定したIDのアイテムを削除
void MouseKeyConf::remove_items( int id )
{
    std::vector< MouseKeyItem* >::iterator it = m_vec_items.begin();
    for( ; it != m_vec_items.end(); ++it ){

        MouseKeyItem* item = (*it);
        if( item->get_id() == id ){
            m_vec_items.erase( it );
            delete item;
            remove_items( id );
            return;
        }
    }
}
