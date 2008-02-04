// ライセンス: GPL2

//
// 板ビュークラスのベースクラス
//

#ifndef _BBSLISTVIEWBASE_H
#define _BBSLISTVIEWBASE_H

#include "skeleton/view.h"
#include "skeleton/treeview.h"

#include "jdlib/constptr.h"
#include "xml/document.h"

#include "columns.h"

#include <gtkmm.h>

namespace SKELETON
{
    class Admin;
};


namespace BBSLIST
{
    class BBSListViewBase : public SKELETON::View
    {
      private:

        Glib::RefPtr< Gtk::TreeStore > m_treestore;
        SKELETON::JDTreeView m_treeview;

        bool m_ready_tree; // ツリーがセットされているならtrue

        BBSLIST::TreeColumns m_columns;

        Gtk::ScrolledWindow m_scrwin;
        JDLIB::ConstPtr< Gtk::CellRendererText > m_ren_text;

        // ダブルクリック状態
        bool m_dblclick;

        // ポップアップメニュー用
        Gtk::TreeModel::Path m_path_selected;

        // クロック入力されたときにtreeview のスクロールバーを指定した位置に移動する
        // clock_in()の説明を参照
        int m_jump_y;

        // D&D 用
        int m_dnd_counter;
        Gtk::TreePath m_drag_path_uline;;

        // サーチで使う変数
        bool m_search_invert;
        std::string m_pre_query;

        // fucus_viewを一回キャンセルする
        // ポップアップメニューが消えたときにfocus_viewが呼び出されるが
        // 名前の変更など都合の悪いときはキャンセルする
        bool m_cancel_focus;

        // あるフォルダを開いたときに他のフォルダを閉じる
        bool m_expand_collapse;
        bool m_cancel_expand; // signal_row_expanded() をキャンセルする
        bool m_expanding; // 行を開いている最中にtrueにしてsignal_row_collapsed()をキャンセルする

      protected:

        // Viewが所属するAdminクラス
        virtual SKELETON::Admin* get_admin();

        // DOM共有オブジェクト
        XML::Document m_document;

        Glib::RefPtr< Gtk::TreeStore >& get_treestore() { return m_treestore; }
        SKELETON::JDTreeView& get_treeview() { return  m_treeview; }
        const bool& get_ready_tree() const{ return m_ready_tree; }
        void set_expand_collapse( bool set ){ m_expand_collapse = set; }

        virtual void activate_act_before_popupmenu( const std::string& url );

        void append_from_buffer( Gtk::TreeModel::Path path, bool after, bool scroll );
        void delete_selected_rows();

        // tree <-> XML( DOM )変換
        void tree2xml( const std::string& root_name );
        void xml2tree( const std::string& root_name, const std::string& xml = std::string() );

        // 移転があったときに行に含まれるURLを変更する
        void update_urls();

        // アイコン表示の切り替え
        void toggle_icon( const std::string& url );

        // path からその行のタイプを取得
        int path2type( const Gtk::TreePath& path );
        // row からタイプを取得
        int row2type( const Gtk::TreeModel::Row& row );

        // お気に入りにアイテム追加
        // あらかじめ共有バッファに追加するデータをセットしておくこと
        void append_item();

        // xml保存
        virtual void save_xml( bool backup ){}

        // remove_dir != empty()の時はその名前のディレクトリを削除する
        void save_xml_impl( const std::string& file, const std::string& root, const std::string& remove_dir );

      public:

        BBSListViewBase( const std::string& url, const std::string& arg1 = std::string() , const std::string& arg2 = std::string() );
        virtual ~BBSListViewBase();

        // SKELETON::View の関数のオーバロード
        virtual const std::string url_for_copy(){ return std::string(); }

        virtual bool set_command( const std::string& command, const std::string& arg = std::string() );

        virtual void shutdown();

        virtual void clock_in();
        virtual void relayout();
        virtual void focus_view();
        virtual void focus_out();
        virtual void close_view();
        virtual void update_item( const std::string& );
        virtual void operate_view( const int& control );
        virtual void goto_top();
        virtual void goto_bottom();

        // 検索
        virtual void exec_search();
        virtual void up_search();
        virtual void down_search();
        virtual void operate_search( const std::string& controlid );

        // selectdialogで使う
        Gtk::TreePath get_current_path() { return m_treeview.get_current_path(); }
        void copy_treestore( Glib::RefPtr< Gtk::TreeStore >& store );

      private:

        void row_up();
        void row_down();
        void page_up();
        void page_down();
        void expand_all_dir( Gtk::TreeModel::Path path );
        void select_all_dir( Gtk::TreeModel::Path path );
        void check_update_dir( Gtk::TreeModel::Path path );
        void check_update_root( const Gtk::TreeModel::Children& children );
        void check_update_root( const bool tab_open = false );

        bool slot_button_press( GdkEventButton* event );
        bool slot_button_release( GdkEventButton* event );
        bool slot_motion_notify( GdkEventMotion* event );
        bool slot_key_press( GdkEventKey* event );
        bool slot_key_release( GdkEventKey* event );
        bool slot_scroll_event( GdkEventScroll* event );
        void slot_open_tab();
        void slot_open_browser();
        void slot_append_favorite();
        void slot_newdir();
        void slot_newcomment();
        void slot_rename();
        void slot_copy_url();
        void slot_copy_title_url();
        void slot_select_all_dir();
        void slot_select_all();
        void slot_check_update_dir();
        void slot_check_update_open_dir();
        void slot_cancel_check_update();
        void slot_search_cache_board();
        void slot_preferences_board();
        void slot_preferences_article();
        void slot_preferences_image();
        void slot_row_exp( const Gtk::TreeModel::iterator& it, const Gtk::TreeModel::Path& path );
        void slot_row_col( const Gtk::TreeModel::iterator& it, const Gtk::TreeModel::Path& path );        
        void slot_ren_text_on_edited( const Glib::ustring& path, const Glib::ustring& text );
        void slot_ren_text_on_canceled();
        void slot_checkupdate_selected_rows();
        void slot_checkupdate_open_selected_rows();

        // D&D 関係
        void slot_drag_begin();
        void slot_drag_motion( Gtk::TreeModel::Path path );
        void slot_drag_drop( Gtk::TreeModel::Path path );
        void slot_drag_end();
        void slot_receive_dnd_begin();
        void slot_receive_dnd_end();

        Gtk::TreeViewColumn* create_column();
        virtual const bool open_row( Gtk::TreePath& path, const bool tab );
        virtual void switch_rightview();
        void open_selected_rows();
        void checkupdate_selected_rows();
        Glib::ustring path2url( const Gtk::TreePath& path );
        Glib::ustring row2url( const Gtk::TreeModel::Row& row );
        Glib::ustring path2name( const Gtk::TreePath& path );
        bool is_dir( Gtk::TreeModel::iterator& it );
        bool is_dir( const Gtk::TreePath& path );

        Gtk::TreeModel::Path append_row( const std::string& url, const std::string& name, int type,
                                         Gtk::TreeModel::Path path_dest = Gtk::TreeModel::Path()
                                         , bool subdir = true, bool after = true );

        bool copy_row( Gtk::TreeModel::iterator& src, Gtk::TreeModel::iterator& dest, bool subdir, bool after = true );
        void move_selected_row( const Gtk::TreePath& path, bool after );

        void draw_underline( const Gtk::TreePath& path, bool draw );
        void show_status();

        void set_info_to_sharedbuffer( Gtk::TreePath& path );

        // 全てのツリーに m_columns.m_expand の値をセットする
        void set_expanded_row( const Gtk::TreeModel::Children& children );
    };
};


#endif
