// ライセンス: 最新のGPL

// スレ一覧ビュー

#ifndef _BOARDVIEW_H
#define _BOARDVIEW_H

#include "skeleton/view.h"
#include "skeleton/treeview.h"

#include "toolbar.h"
#include "columns.h"

#include <gtkmm.h>

namespace DBTREE
{
    class ArticleBase;
}

namespace BOARD
{
    class BoardView : public SKELETON::View
    {
        SKELETON::JDTreeView m_treeview;
        BOARD::TreeColumns m_columns;
        Glib::RefPtr< Gtk::ListStore > m_liststore;
        Gtk::ScrolledWindow m_scrwin;

        // ダブルクリック状態
        bool m_dblclick;

        // ソートで使う変数
        int m_previous_col;
        bool m_ascend;

        // サーチで使う変数
        bool m_search_invert;
        std::string m_pre_query;
        
        // ツールバー
        BoardToolBar m_toolbar;

        // ポップアップメニュー用
        Gtk::TreeModel::Path m_path_selected;

    public:
        BoardView( const std::string& url, const std::string& arg1 = std::string() , const std::string& arg2 = std::string() );
        ~BoardView();

        virtual const std::string url_for_copy();

        // SKELETON::View の関数のオーバロード
        virtual void clock_in();
        virtual void reload();
        virtual void stop();
        virtual void show_view();
        virtual void redraw_view();
        virtual void relayout();
        virtual void update_view();
        virtual void focus_view();
        virtual void focus_out();
        virtual void close_view();
        virtual void delete_view();
        virtual void update_item( const std::string& id_dat );
        virtual void operate_view( const int& control );
        virtual void goto_top();
        virtual void goto_bottom();

        void row_up();
        void row_down();
        void page_up();
        void page_down();

      protected:

        virtual Gtk::Menu* get_popupmenu( const std::string& url );

    private:

        void save_column_width();

        // ソート用
        void slot_mark_clicked();        
        void slot_id_clicked();
        void slot_col_clicked( int col );
        int slot_compare_mark_val( const Gtk::TreeModel::iterator& a, const Gtk::TreeModel::iterator& b );        
        int slot_compare_num_id( const Gtk::TreeModel::iterator& a, const Gtk::TreeModel::iterator& b );
        int slot_compare_subject( const Gtk::TreeModel::iterator& a, const Gtk::TreeModel::iterator& b );
        int slot_compare_num_res( const Gtk::TreeModel::iterator& a, const Gtk::TreeModel::iterator& b );
        int slot_compare_num_load( const Gtk::TreeModel::iterator& a, const Gtk::TreeModel::iterator& b );
        int slot_compare_new( const Gtk::TreeModel::iterator& a, const Gtk::TreeModel::iterator& b );
        int slot_compare_since_t( const Gtk::TreeModel::iterator& a, const Gtk::TreeModel::iterator& b );
        int slot_compare_write_t( const Gtk::TreeModel::iterator& a, const Gtk::TreeModel::iterator& b );
        int slot_compare_speed( const Gtk::TreeModel::iterator& a, const Gtk::TreeModel::iterator& b );        

        int compare_drawbg( Gtk::TreeModel::Row& row_a, Gtk::TreeModel::Row& row_b );
        int compare_mark( Gtk::TreeModel::Row& row_a, Gtk::TreeModel::Row& row_b );
        int compare_id( Gtk::TreeModel::Row& row_a, Gtk::TreeModel::Row& row_b );
        int compare_row( int& num_a, int& num_b, Gtk::TreeModel::Row& row_a, Gtk::TreeModel::Row& row_b );

        // UI
        bool slot_button_press( GdkEventButton* event );
        bool slot_button_release( GdkEventButton* event );
        bool slot_motion_notify( GdkEventMotion* event );
        bool slot_key_press( GdkEventKey* event );
        bool slot_key_release( GdkEventKey* event );
        bool slot_scroll_event( GdkEventScroll* event );
        void slot_open_tab();
        void slot_favorite_thread();
        void slot_favorite_board();
        void slot_new_article();
        void slot_push_delete();
        void slot_push_favorite();
        void slot_unselect_all();
        void slot_copy_url();
        void slot_copy_title_url();
        void slot_open_browser();
        void slot_push_preferences();
        void slot_preferences_article();
        void slot_abone_thread();

        bool open_row( Gtk::TreePath& path, bool tab );
        void open_selected_rows();
        std::string path2daturl( const Gtk::TreePath& path );

        // 検索
        bool drawout();
        void search();
        void slot_push_down_search();
        void slot_push_up_search();
        void slot_entry_operate( int controlid );

        // d&d
        void slot_drag_begin();
        void slot_drag_end();
        
        void update_row_common( DBTREE::ArticleBase* art, Gtk::TreeModel::Row& row, int& id );
        std::string get_subject_from_path( Gtk::TreePath& path );
        template < typename ColumnType >
        std::string get_name_of_cell( Gtk::TreePath& path, const Gtk::TreeModelColumn< ColumnType >& column );

        void set_article_to_buffer();
        void set_board_to_buffer();
    };
};


#endif
