// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "undobuffer.h"

using namespace SKELETON;


UNDO_BUFFER::UNDO_BUFFER()
    : m_first{ true }
{
    m_vec_undo.push_back( UNDO_DATA() );
}


void UNDO_BUFFER::set_item( UNDO_ITEM item )
{
#ifdef _DEBUG
    std::cout << "UNDO_BUFFER::set_item\n";
#endif
    if( m_first ){
#ifdef _DEBUG
        std::cout << "reset_data\n";
#endif
        get_undo_data().clear();
        m_first = false;
    }

    get_undo_data().push_back( std::move( item ) );
#ifdef _DEBUG
    std::cout << "size = " << get_undo_data().size() << std::endl;
#endif
}


void UNDO_BUFFER::undo()
{
    if( get_enable_undo() ){
#ifdef _DEBUG
        std::cout << "UNDO_BUFFER::undo pos = " << m_pos << " max = " << m_max << std::endl;
#endif
        --m_pos;
        m_first = true;
        m_sig_undo.emit();
    }
}


void UNDO_BUFFER::redo()
{
    if( get_enable_redo() ){
#ifdef _DEBUG
        std::cout << "SKELETON::UNDO_BUFFER::redo pos = " << m_pos << " max = " << m_max << std::endl;
#endif
        ++m_pos;
        m_first = true;
        m_sig_redo.emit();
    }
}


//
// 選択中の行
//
void UNDO_BUFFER::set_list_info_selected( const CORE::DATA_INFO_LIST& list_info_selected )
{
#ifdef _DEBUG
    std::cout << "UNDO_BUFFER::set_list_info_selected\n";
#endif

    UNDO_ITEM item;
    item.list_info_selected = list_info_selected;
    set_item( std::move( item ) );
}


//
// 削除や追加した行
//
void UNDO_BUFFER::set_list_info( const CORE::DATA_INFO_LIST& list_info_append, const CORE::DATA_INFO_LIST& list_info_delete )
{
#ifdef _DEBUG
    std::cout << "UNDO_BUFFER::set_list_info pos = " << m_pos << " max = " << m_max << std::endl;
#endif

    UNDO_ITEM item;
    item.list_info_append = list_info_append;
    item.list_info_delete = list_info_delete;
    set_item( std::move( item ) );
}


//
// 名前を変更した行
//
void UNDO_BUFFER::set_name( const Gtk::TreePath& path_renamed, const Glib::ustring& name_new, const Glib::ustring& name_before )
{
#ifdef _DEBUG
    std::cout << "UNDO_BUFFER::set_name pos = " << m_pos << " max = " << m_max << std::endl
              << "path = " << path_renamed.to_string()
              << " new = " << name_new << " before = " << name_before << std::endl;
#endif

    UNDO_ITEM item;
    item.path_renamed = path_renamed;
    item.name_new = name_new;
    item.name_before = name_before;
    set_item( std::move( item ) );
}


//
// set_* でデータをセットしたら最後にcommitする
//
void UNDO_BUFFER::commit()
{
    if( m_first ) return;

#ifdef _DEBUG
    std::cout << "UNDO_BUFFER::commit pos = " << m_pos << " max = " << m_pos << std::endl;
#endif

    ++m_pos;
    m_max = m_pos;
    if( static_cast<int>(m_vec_undo.size()) == m_pos ) m_vec_undo.push_back( UNDO_DATA() );
    m_first = true;
    m_sig_commit.emit();

#ifdef _DEBUG
    std::cout << "-> pos = max = " << m_pos << std::endl;
#endif
}
