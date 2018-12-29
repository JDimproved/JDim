// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "mousekeyconf.h"
#include "mousekeyitem.h"
#include "controlutil.h"

#include "jdlib/miscutil.h"


#include "cache.h"


using namespace CONTROL;


MouseKeyConf::MouseKeyConf()
{}



MouseKeyConf::~MouseKeyConf() noexcept
{}



// 設定ファイル保存
void MouseKeyConf::save_conf( const std::string& savefile )
{
#ifdef _DEBUG
    std::cout << "MouseKeyConf::save_conf " << savefile << std::endl;
#endif

    JDLIB::ConfLoader cf( savefile, std::string() );

    std::map< int, std::string >::iterator it_map = m_map_default_motions.begin();
    for( ; it_map != m_map_default_motions.end(); ++it_map ){

        const int id = ( *it_map ).first;
        const std::string name = CONTROL::get_name( id );
        std::string motions;
        std::vector< MouseKeyItem >::iterator it = m_vec_items.begin();
        for( ; it != m_vec_items.end(); ++it ){
            if( (*it).get_id() == id  ){
                if( !motions.empty() ) motions += " ";
                motions += (*it).get_str_motion();
            }
        }

#ifdef _DEBUG
        std::cout << name << " = " << motions << std::endl;
#endif
        cf.update( name, motions );
    }

    cf.save();
}



// 操作からID取得
const int MouseKeyConf::get_id( const int mode,
                          const guint motion, const bool ctrl, const bool shift, const bool alt,
                          const bool dblclick, const bool trpclick )
{
    int id = CONTROL::None;;
    std::vector< MouseKeyItem >::iterator it = m_vec_items.begin();
    for( ; it != m_vec_items.end(); ++it ){

        id = (*it).is_activated( mode, motion, ctrl, shift, alt, dblclick, trpclick );
        if( id != CONTROL::None ) break;
    }

#ifdef _DEBUG
    std::cout << "MouseKeyConf::get_id mode = " << mode << " id = " << id << " motion = " << motion
              << " ctrl = " << ctrl << " shift = " << shift << " alt = " << alt
              << " dblclick = " << dblclick << " trpclick = " << trpclick
              << std::endl;
#endif

    return id;
}


// ID から操作を取得
// (注意) リストの一番上にあるものを出力
const bool MouseKeyConf::get_motion( const int id, guint& motion, bool& ctrl, bool& shift, bool& alt, bool& dblclick, bool& trpclick )
{
    std::vector< MouseKeyItem >::iterator it = m_vec_items.begin();
    for( ; it != m_vec_items.end(); ++it ){

        if( (*it).get_id() == id ){
            motion = (*it).get_motion();
            ctrl = (*it).get_ctrl();
            shift = (*it).get_shift();
            alt = (*it).get_alt();
            dblclick = (*it).get_dblclick();
            trpclick = (*it).get_trpclick();
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
    std::vector< MouseKeyItem >::iterator it = m_vec_items.begin();
    for( ; it != m_vec_items.end(); ++it ){
        if( (*it).equal( motion, ctrl, shift, alt, dblclick, trpclick ) == id ) return true;
    }

    return false;
}



// 同じモード内でモーションが重複していないかチェック
const std::vector< int >  MouseKeyConf::check_conflict( const int mode, const std::string& str_motion )
{
    std::vector< int > vec_ids;

    std::vector< MouseKeyItem >::iterator it = m_vec_items.begin();
    for( ; it != m_vec_items.end(); ++it ){

        const int id = (*it).is_activated( mode, str_motion );
        if( id != CONTROL::None ) vec_ids.push_back( id );
    }

    return vec_ids;
}


// IDから操作文字列取得
const std::string MouseKeyConf::get_str_motions( const int id )
{
    std::string motions;

    std::vector< MouseKeyItem >::iterator it = m_vec_items.begin();
    for( ; it != m_vec_items.end(); ++it ){

        if( (*it).get_id() == id ){
            if( ! motions.empty() ) motions += " ";
            motions += (*it).get_str_motion();
        }
    }

    return motions;
}


// IDからデフォルトの操作文字列取得
const std::string MouseKeyConf::get_default_motions( const int id )
{
    std::map< int, std::string >::iterator it = m_map_default_motions.find( id );
    if( it != m_map_default_motions.end() ) return ( *it ).second;

    return std::string();
}


// 設定ファイルから読み込んだモーションを登録
void MouseKeyConf::load_motions( JDLIB::ConfLoader& cf, const std::string& name, const std::string& default_motions )
{
    const int id = CONTROL::get_id( name );
    const std::string str_motions = cf.get_option_str( name, default_motions, 256 );

    set_motions( id, str_motions );
    set_default_motions( id, default_motions );
}


// 設定ファイルから読み込んだモーションを登録
// ver.2.0.2 以前との互換性のため Plus を + に置き換える
void MouseKeyConf::load_keymotions( JDLIB::ConfLoader& cf, const std::string& name, const std::string& default_motions )
{
    const int id = CONTROL::get_id( name );
    std::string str_motions = cf.get_option_str( name, default_motions, 256 );
    if( str_motions.find( "Plus" ) != std::string::npos ) str_motions = MISC::replace_str( str_motions, "Plus", "+" );

    set_motions( id, str_motions );
    set_default_motions( id, default_motions );
}


// スペースで区切られた複数の操作をデータベースに登録
void MouseKeyConf::set_motions( const int id, const std::string& str_motions )
{
    if( id == CONTROL::None ) return;

    const std::string name = CONTROL::get_name( id );
    if( name.empty() ) return;

    const int mode = CONTROL::get_mode( id );
    if( mode == CONTROL::MODE_ERROR ) return;

    std::list< std::string > list_motion = MISC::StringTokenizer( str_motions, ' ' );
    std::list< std::string >::iterator it = list_motion.begin();
    for( ; it != list_motion.end(); ++it ) set_one_motion_impl( id, mode, name, (*it) );
}


// デフォルト操作を登録
void MouseKeyConf::set_default_motions( const int id, const std::string& default_motions )
{
    if( id == CONTROL::None ) return;

    m_map_default_motions.insert( make_pair( id, default_motions ) );
}


// ひとつの操作をデータベースに登録
void MouseKeyConf::set_one_motion( const std::string& name, const std::string& str_motion )
{
    const int id = CONTROL::get_id( name );
    if( id == CONTROL::None ) return;

    const int mode = CONTROL::get_mode( id );
    if( mode == CONTROL::MODE_ERROR ) return;

    set_one_motion_impl( id, mode, name, str_motion );
}


// 指定したIDの操作を全て削除
// 削除したら true を返す
const bool MouseKeyConf::remove_motions( const int id )
{
    bool ret = false;

    std::vector< MouseKeyItem >::iterator it = m_vec_items.begin();
    for( ; it != m_vec_items.end(); ++it ){

        if( (*it).get_id() == id ){
            m_vec_items.erase( it );
            ret = true;
            break;
        }
    }

    if( ret ) remove_motions( id );

    return ret;
}
