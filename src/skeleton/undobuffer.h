// ライセンス: GPL2

// UNDO用バッファ

#include "sharedbuffer.h"

#include <gtkmm.h>

namespace SKELETON
{
    class UNDO_ITEM
    {
    public:

        CORE::DATA_INFO_LIST list_info_selected;

        CORE::DATA_INFO_LIST list_info_append;
        CORE::DATA_INFO_LIST list_info_delete;

        Gtk::TreePath path_renamed;
        Glib::ustring name_new;
        Glib::ustring name_before;

        UNDO_ITEM()
        {
            list_info_selected.clear();

            list_info_append.clear();
            list_info_delete.clear();

            path_renamed = Gtk::TreePath();
            name_new = std::string();
            name_before = std::string();
        }
    };


    typedef std::vector< UNDO_ITEM > UNDO_DATA;

    typedef sigc::signal< void > SIG_UNDO;
    typedef sigc::signal< void > SIG_REDO;
    typedef sigc::signal< void > SIG_COMMIT;

    class UNDO_BUFFER
    {
        std::vector< UNDO_DATA > m_vec_undo;

        int m_pos;
        int m_max;
        bool m_first;

        SIG_UNDO m_sig_undo;
        SIG_REDO m_sig_redo;
        SIG_COMMIT m_sig_commit;

      public:

        UNDO_BUFFER();
        virtual ~UNDO_BUFFER() noexcept {}

        SIG_UNDO sig_undo(){ return m_sig_undo; }
        SIG_REDO sig_redo(){ return m_sig_redo; }
        SIG_COMMIT sig_commit(){ return m_sig_commit; }

        UNDO_DATA& get_undo_data(){ return m_vec_undo[ m_pos ]; }

        bool get_enable_undo() const { return ( m_pos ); }
        bool get_enable_redo() const { return ( m_pos < m_max ); }

        void undo();
        void redo();

        // 選択中の行
        void set_list_info_selected( const CORE::DATA_INFO_LIST& list_info_selected );

        // 削除や追加した行
        void set_list_info( const CORE::DATA_INFO_LIST& list_info_append, const CORE::DATA_INFO_LIST& list_info_delete );

        // 名前を変更した行
        void set_name( const Gtk::TreePath& path_renamed, const Glib::ustring& name_new, const Glib::ustring& name_before );

        // set_* でデータをセットしたら最後にcommitする
        void commit();

      private:

        void set_item( UNDO_ITEM& item );
    };

}
