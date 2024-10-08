// ライセンス: GPL2
//
// マウスジェスチャ、キーボード設定ダイアログの基底クラス
//

#ifndef _MOUSEKEYPREFPREF_H
#define _MOUSEKEYPREFPREF_H

#include "skeleton/prefdiag.h"

#include "control.h"

#include <memory>


namespace CONTROL
{
    enum
    {
        INPUTDIAG_MODE_BUTTON = 1,
        INPUTDIAG_MODE_MOUSE = 2,
        INPUTDIAG_MODE_KEY = 4
    };


    //
    // キーやマウス入力ダイアログの基底クラス
    //
    class InputDiag : public SKELETON::PrefDiag
    {
        int m_id;
        int m_mode;
        int m_controlmode;
        std::string m_str_motion;

        Gtk::Label m_label;
        Gtk::HBox m_hbox;

        CONTROL::Control m_control;

      public:

        InputDiag( Gtk::Window* parent, const std::string& url,
                   const int id, const std::string& target, const int mode );

        const std::string& get_str_motion() const { return m_str_motion; }
        void set_str_motion( const std::string& motion ){ m_str_motion = motion; }

      protected:

        int get_id() const { return m_id; }

        std::string get_key_label() const;
        std::string get_mouse_label() const;
        std::string get_button_label() const;

      private:

        bool on_key_press_event (GdkEventKey* event) override;
        bool on_button_press_event( GdkEventButton* event ) override;
        bool on_button_release_event( GdkEventButton* event ) override;
        bool on_motion_notify_event( GdkEventMotion* event ) override;
    };


    ///////////////////////////////////////


    class MouseKeyDiagColumn : public Gtk::TreeModel::ColumnRecord
    {
      public:

        Gtk::TreeModelColumn< std::string >  m_col_motion;

        MouseKeyDiagColumn()
        {
            add( m_col_motion );
        }
    };


    //
    // 個別のマウスジェスチャ、ショートカットキー設定ダイアログの基底クラス
    //
    class MouseKeyDiag : public SKELETON::PrefDiag
    {
        int m_id;
        int m_controlmode;
        bool m_single{};

        Gtk::TreeView m_treeview;
        Glib::RefPtr< Gtk::ListStore > m_liststore;
        MouseKeyDiagColumn m_columns;
        Gtk::ScrolledWindow m_scrollwin;

        Gtk::Label m_label;

        Gtk::Button m_button_delete;
        Gtk::Button m_button_add;
        Gtk::Button m_button_reset;
        Gtk::VButtonBox m_vbuttonbox;

        Gtk::HBox m_hbox;

      public:

        MouseKeyDiag( Gtk::Window* parent, const std::string& url,
                      const int id, const std::string& target, const std::string& str_motions );

        std::string get_str_motions();

      protected:

        int get_id() const { return m_id; }
        int get_controlmode() const { return m_controlmode; }
        int get_count() const { return m_liststore->children().size(); }

        int get_single() const { return m_single; }
        void set_single( bool single ){ m_single = single; }

        virtual std::unique_ptr<InputDiag> create_inputdiag() = 0;
        virtual std::string get_default_motions( const int id ) = 0;
        virtual std::vector< int > check_conflict( const int mode, const std::string& str_motion ) = 0;

      private:

        Gtk::TreeModel::Row append_row( const std::string& motion );

        // 入力ダイアログを表示
        std::string show_inputdiag( bool is_append );

        // 行をダブルクリック
        void slot_row_activated( const Gtk::TreeModel::Path& path, Gtk::TreeViewColumn* column );

        // 行削除
        void slot_delete();

        // 行追加
        void slot_add();

        // デフォルトに戻す
        void slot_reset();
    };


    ///////////////////////////////////////


    class MouseKeyTreeColumn : public Gtk::TreeModel::ColumnRecord
    {
      public:

        Gtk::TreeModelColumn< std::string >  m_col_label;
        Gtk::TreeModelColumn< std::string >  m_col_motions;
        Gtk::TreeModelColumn< int > m_col_id;
        Gtk::TreeModelColumn< bool > m_col_drawbg;

        MouseKeyTreeColumn()
        {
            add( m_col_label );
            add( m_col_motions );
            add( m_col_id );
            add( m_col_drawbg );
        }
    };


    //
    // マウスジェスチャ、キーボード設定ダイアログの基底クラス
    //
    class MouseKeyPref : public SKELETON::PrefDiag
    {
        Gtk::Box m_hbox_search;
        Gtk::ToggleButton m_toggle_search;
        Glib::RefPtr<Glib::Binding> m_binding_search; ///< ToggleButtonとSearchBarをバインドする
        Gtk::SearchBar m_search_bar;
        Gtk::SearchEntry m_search_entry;
        Gtk::TreeView m_treeview;
        Glib::RefPtr< Gtk::ListStore > m_liststore;
        Glib::RefPtr<Gtk::TreeModelFilter> m_model_filter;
        MouseKeyTreeColumn m_columns;
        Gtk::ScrolledWindow m_scrollwin;

        Gtk::HBox m_hbox;
        Gtk::Button m_button_reset;

        Gtk::Label m_label;

      public:

        MouseKeyPref( Gtk::Window* parent, const std::string& url, const std::string& target );

      protected:

        Glib::RefPtr< Gtk::ListStore >& get_liststore(){ return m_liststore; }
        MouseKeyTreeColumn& get_colums(){ return  m_columns; }

        void append_row( const int id, const std::string& label = std::string() );
        void append_comment_row( const std::string& comment );

        virtual std::unique_ptr<MouseKeyDiag> create_setting_diag( const int id, const std::string& str_motions ) = 0;
        virtual std::string get_str_motions( const int id ) = 0;
        virtual std::string get_default_motions( const int id ) = 0;
        virtual void set_motions( const int id, const std::string& str_motions ) = 0;
        virtual bool remove_motions( const int id ) = 0;

      private:

        void slot_reset();
        void slot_row_activated( const Gtk::TreeModel::Path& path, Gtk::TreeViewColumn* column );
        void slot_cell_data( Gtk::CellRenderer* cell, const Gtk::TreeModel::iterator& it );
        bool slot_key_press_event( GdkEventKey* event );
        void slot_entry_changed();
        bool slot_visible_func( const Gtk::TreeModel::const_iterator& iter );
    };

}
#endif
