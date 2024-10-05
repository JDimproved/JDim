// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "mousekeyconf.h"
#include "mousekeyitem.h"
#include "controlutil.h"

#include "jdlib/miscutil.h"

#include "cache.h"

#include <algorithm>


using namespace CONTROL;


MouseKeyConf::MouseKeyConf()
{}


MouseKeyConf::~MouseKeyConf() noexcept = default;


// 設定ファイル保存
void MouseKeyConf::save_conf( const std::string& savefile )
{
#ifdef _DEBUG
    std::cout << "MouseKeyConf::save_conf " << savefile << std::endl;
#endif

    JDLIB::ConfLoader cf( savefile, std::string() );

    for( const auto& key_val : m_map_default_motions ) {

        const int id = key_val.first;
        const std::string name = CONTROL::get_name( id );
        std::string motions;
        for( const MouseKeyItem& item : m_vec_items ) {
            if( item.get_id() == id  ){
                if( !motions.empty() ) motions += " ";
                motions.append( item.get_str_motion() );
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
int MouseKeyConf::get_id( const int mode,
                          const guint motion, const bool ctrl, const bool shift, const bool alt,
                          const bool dblclick, const bool trpclick ) const
{
    int id = CONTROL::NoOperation;
    for( const MouseKeyItem& item : m_vec_items ) {

        id = item.is_activated( mode, motion, ctrl, shift, alt, dblclick, trpclick );
        if( id != CONTROL::NoOperation ) break;
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
bool MouseKeyConf::get_motion( const int id, guint& motion, bool& ctrl, bool& shift, bool& alt, bool& dblclick,
                               bool& trpclick ) const
{
    auto it = std::find_if( m_vec_items.cbegin(), m_vec_items.cend(),
                            [id]( const MouseKeyItem& i ) { return i.get_id() == id; } );
    if( it != m_vec_items.cend() ) {
        motion = it->get_motion();
        ctrl = it->get_ctrl();
        shift = it->get_shift();
        alt = it->get_alt();
        dblclick = it->get_dblclick();
        trpclick = it->get_trpclick();
        return true;
    }

    return false;
}




// ID が割り当てられているかチェック
bool MouseKeyConf::alloted( const int id,
                            const guint motion, const bool ctrl, const bool shift, const bool alt,
                            const bool dblclick, const bool trpclick ) const
{
    const auto equal = [&]( const MouseKeyItem& item ) {
        return item.equal( motion, ctrl, shift, alt, dblclick, trpclick ) == id;
    };

    return std::any_of( m_vec_items.cbegin(), m_vec_items.cend(), equal );
}



// 同じモード内でモーションが重複していないかチェック
std::vector< int > MouseKeyConf::check_conflict( const int mode, const std::string& str_motion ) const
{
    std::vector< int > vec_ids;

    for( const MouseKeyItem& item : m_vec_items ) {

        const int id = item.is_activated( mode, str_motion );
        if( id != CONTROL::NoOperation ) vec_ids.push_back( id );
    }

    return vec_ids;
}


// IDから操作文字列取得
std::string MouseKeyConf::get_str_motions( const int id ) const
{
    std::string motions;

    for( const MouseKeyItem& item : m_vec_items ) {

        if( item.get_id() == id ){
            if( ! motions.empty() ) motions += " ";
            motions.append( item.get_str_motion() );
        }
    }

    return motions;
}


// IDからデフォルトの操作文字列取得
std::string MouseKeyConf::get_default_motions( const int id ) const
{
    auto it = m_map_default_motions.find( id );
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
    if( id == CONTROL::NoOperation ) return;

    const std::string name = CONTROL::get_name( id );
    if( name.empty() ) return;

    const int mode = CONTROL::get_mode( id );
    if( mode == CONTROL::MODE_ERROR ) return;

    std::list< std::string > list_motion = MISC::StringTokenizer( str_motions, ' ' );
    for( const std::string& str_motion : list_motion ) set_one_motion_impl( id, mode, name, str_motion );
}


// デフォルト操作を登録
void MouseKeyConf::set_default_motions( const int id, const std::string& default_motions )
{
    if( id == CONTROL::NoOperation ) return;

    m_map_default_motions.insert( make_pair( id, default_motions ) );
}


// ひとつの操作をデータベースに登録
void MouseKeyConf::set_one_motion( const std::string& name, const std::string& str_motion )
{
    const int id = CONTROL::get_id( name );
    if( id == CONTROL::NoOperation ) return;

    const int mode = CONTROL::get_mode( id );
    if( mode == CONTROL::MODE_ERROR ) return;

    set_one_motion_impl( id, mode, name, str_motion );
}


// 指定したIDの操作を全て削除
// 削除したら true を返す
bool MouseKeyConf::remove_motions( const int id )
{
    // erase-remove idiom
    const auto it = std::remove_if( m_vec_items.begin(), m_vec_items.end(),
                                    [id]( const MouseKeyItem& i ) { return i.get_id() == id; } );
    if( it != m_vec_items.end() ) {
        m_vec_items.erase( it, m_vec_items.end() );
        return true;
    }
    return false;
}


// 設定を一時的にバックアップする (Not thread safe)
void MouseKeyConf::state_backup()
{
    m_backup_vec_items = m_vec_items;
    m_backup_map_default_motions = m_map_default_motions;
}


// バックアップした設定を復元する (Not thread safe)
void MouseKeyConf::state_restore()
{
    m_vec_items = m_backup_vec_items;
    m_map_default_motions = m_backup_map_default_motions;
}
