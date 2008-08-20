// ライセンス: GPL2

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
            if( (*it)->get_id() == id && (*it)->get_save() ){
                name = (*it)->get_name();
                if( !motion.empty() ) motion += " ";
                motion += (*it)->get_str_motion();
            }
        }

        if( !name.empty() ){
#ifdef _DEBUG
            std::cout << name << " = " << motion << std::endl;
#endif
            cf.update( name, motion );
        }
    }

    cf.save();
}



// 操作からID取得
const int MouseKeyConf::get_id( const int mode,
                          const guint motion, const bool ctrl, const bool shift, const bool alt,
                          const bool dblclick, const bool trpclick )
{
    int id = CONTROL::None;;
    std::vector< MouseKeyItem* >::iterator it = m_vec_items.begin();
    for( ; it != m_vec_items.end(); ++it ){

        id = (*it)->is_activated( mode, motion, ctrl, shift, alt, dblclick, trpclick );
        if( id != CONTROL::None ) break;
    }

#ifdef _DEBUG
    std::cout << "MouseKeyConf::get_id mode = " << mode << " id = " << id << " motion = " << motion << std::endl;
#endif

    return id;
}


// ID から操作を取得
// (注意) リストの一番上にあるものを出力
const bool MouseKeyConf::get_motion( const int id, guint& motion, bool& ctrl, bool& shift, bool& alt, bool& dblclick, bool& trpclick )
{
    std::vector< MouseKeyItem* >::iterator it = m_vec_items.begin();
    for( ; it != m_vec_items.end(); ++it ){

        if( (*it)->get_id() == id ){
            motion = (*it)->get_motion();
            ctrl = (*it)->get_ctrl();
            shift = (*it)->get_shift();
            alt = (*it)->get_alt();
            dblclick = (*it)->get_dblclick();
            trpclick = (*it)->get_trpclick();
            return true;
        }
    }

    return false;
}




// ID が割り当てられているかチェック
const bool MouseKeyConf::alloted( const int id,
                            const guint motion, const bool ctrl, const bool shift, const bool alt,
                            const bool dblclick, const bool trpclick )
{
    std::vector< MouseKeyItem* >::iterator it = m_vec_items.begin();
    for( ; it != m_vec_items.end(); ++it ){
        if( (*it)->equal( motion, ctrl, shift, alt, dblclick, trpclick ) == id ) return true;
    }

    return false;
}



// モーションが重複していないかチェック
const int MouseKeyConf::check_conflict( const int mode,
                                  const guint motion, const bool ctrl, const bool shift, const bool alt,
                                  const bool dblclick, const bool trpclick )
{
    int id = CONTROL::None;;
    std::vector< MouseKeyItem* >::iterator it = m_vec_items.begin();
    for( ; it != m_vec_items.end(); ++it ){

        id = (*it)->is_activated( mode, motion, ctrl, shift, alt, dblclick, trpclick );
        if( id != CONTROL::None ) break;
    }

    return id;
}




// IDからモードを取得
const int MouseKeyConf::get_mode( const int& id )
{
    if( id < CONTROL::COMMONMOTION_END ) return CONTROL::MODE_COMMON;
    if( id < CONTROL::BBSLISTMOTION_END ) return CONTROL::MODE_BBSLIST;
    if( id < CONTROL::BOARDMOTION_END ) return CONTROL::MODE_BOARD;
    if( id < CONTROL::ARTICLEMOTION_END ) return CONTROL::MODE_ARTICLE;
    if( id < CONTROL::IMAGEICONMOTION_END ) return CONTROL::MODE_IMAGEICON;
    if( id < CONTROL::IMAGEVIEWMOTION_END ) return CONTROL::MODE_IMAGEVIEW;
    if( id < CONTROL::MESSAGEMOTION_END ) return CONTROL::MODE_MESSAGE;
    if( id < CONTROL::EDITMOTION_END ) return CONTROL::MODE_EDIT;

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


// 指定したIDのアイテムを全て削除
// 削除したら true を返す
const bool MouseKeyConf::remove_items( int id )
{
    bool ret = false;

    std::vector< MouseKeyItem* >::iterator it = m_vec_items.begin();
    for( ; it != m_vec_items.end(); ++it ){

        MouseKeyItem* item = (*it);
        if( item->get_id() == id ){
            m_vec_items.erase( it );
            delete item;
            remove_items( id );
            ret = true;
            break;
        }
    }

    if( ret ) remove_items( id );

    return ret;
}
