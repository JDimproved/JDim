// ライセンス: GPL2

//
// 板ビュークラスのベースクラス
//

#ifndef _BBSLISTVIEWBASE_H
#define _BBSLISTVIEWBASE_H

#include "skeleton/view.h"
#include "skeleton/edittreeview.h"

#include "xml/document.h"

#include "jdlib/hash_set.h"

#include "columns.h"

#include <gtkmm.h>

namespace SKELETON
{
    class Admin;
}


#define SUBDIR_ETCLIST "外部板"


namespace BBSLIST
{
    class EditListWin;

    class BBSListViewBase : public SKELETON::View
    {
        Glib::RefPtr< Gtk::TreeStore > m_treestore;
        SKELETON::EditTreeView m_treeview;

        // DOM共有オブジェクト
        XML::Document m_document;

        std::string m_date_modified;

        bool m_ready_tree; // ツリーがセットされているならtrue

        BBSLIST::TreeColumns m_columns;

        Gtk::ScrolledWindow m_scrwin;

        // クリック状態
        bool m_clicked;

        // ダブルクリック状態
        bool m_dblclicked;

        // ポップアップメニュー用
        Gtk::TreeModel::Path m_path_selected;

        // クロック入力されたときにtreeview のスクロールバーを指定した位置に移動する
        // clock_in()の説明を参照
        int m_jump_y;

        // サーチで使う変数
        bool m_search_invert;
        std::string m_pre_query;

        bool m_open_only_onedir; // あるフォルダを開いたときに他のフォルダを閉じる
        bool m_cancel_expand; // signal_row_expanded() をキャンセルする
        bool m_expanding; // 行を開いている最中にtrueにしてsignal_row_collapsed()をキャンセルする

        // ツリーに含まれているスレのURLを入れる hash_set
        // toggle_articleicon() で使用する
        JDLIB::hash_set_thread m_set_thread;

        // ツリーに含まれている板のURLを入れる set
        // toggle_boardicon() で使用する
        std::set< std::string > m_set_board;

        // ツリーに含まれている画像のURLを入れる hash_set
        std::set< std::string > m_set_image;

        EditListWin* m_editlistwin;

        // スレを追加したときにそのスレにしおりを付ける
        bool m_set_bookmark;

      protected:

        // Viewが所属するAdminクラス
        SKELETON::Admin* get_admin() override;

        // treeviewのD&Dによる編集を可能にする
        void set_editable( const bool editable );

        void set_search_invert( const bool invert ){ m_search_invert = invert; }

        // スレを追加したときにそのスレにしおりを付ける
        void set_bookmark( const bool set ){ m_set_bookmark = set; }

        // DOM共有オブジェクト
        XML::Document& get_document(){ return m_document;}
        void set_document( const XML::Document& document){ m_document = document;}

        Glib::RefPtr< Gtk::TreeStore >& get_treestore() { return m_treestore; }
        SKELETON::EditTreeView& get_treeview() { return  m_treeview; }
        const bool& get_ready_tree() const{ return m_ready_tree; }
        void set_open_only_onedir( const bool set ){ m_open_only_onedir = set; }

        void activate_act_before_popupmenu( const std::string& url ) override;

        // tree <-> XML( DOM )変換
        void tree2xml( const std::string& root_name );
        void xml2tree( const std::string& root_name, const std::string& xml = std::string() );

        // 外部板のディレクトリか
        bool is_etcdir( Gtk::TreePath path );

        // 外部板か
        bool is_etcboard( Gtk::TreeModel::iterator& it );
        bool is_etcboard( Gtk::TreePath path );

        // 起動時や移転があったときなどに行に含まれるURlを変更する
        void update_urls();

        // アイコン表示(スレ)の切り替え
        void toggle_articleicon( const std::string& url );

        // アイコン表示(板)の切り替え
        void toggle_boardicon( const std::string& url );

        // URLを選択
        void select_item( const std::string& url );

        // スレの url と 名前を変更
        void replace_thread( const std::string& url, const std::string& url_new );

        // path からその行のタイプを取得
        int path2type( const Gtk::TreePath& path );

        // row からタイプを取得
        int row2type( const Gtk::TreeModel::Row& row );

        // row -> name 変換
        Glib::ustring row2name( const Gtk::TreeModel::Row& row );

        // row -> url 変換
        // 板の場合は boardbase
        // スレの場合は dat 型のアドレスを返す
        Glib::ustring row2url( const Gtk::TreeModel::Row& row );

        // row -> dirid 変換
        size_t row2dirid( const Gtk::TreeModel::Row& row );

        // path からその行の名前を取得
        Glib::ustring path2name( const Gtk::TreePath& path );

        // path からその行のURLを取得
        Glib::ustring path2rawurl( const Gtk::TreePath& path );
        Glib::ustring path2url( const Gtk::TreePath& path ); // 移転をチェックするバージョン

        // url で指定した項目を削除
        void remove_item( const std::string& url );

        // 先頭項目を削除
        void remove_headitem();

        // 全項目を削除
        void remove_allitems();

        // ツリーの編集ウィンドウを開く
        void edit_tree();

        // xml保存
        virtual void save_xml() = 0;

        // remove_dir != empty()の時はその名前のディレクトリを削除する
        void save_xml_impl( const std::string& file, const std::string& root, const std::string& remove_dir );

        // idからポップアップメニュー取得
        Gtk::Menu* id2popupmenu( const std::string& id );

      public:

        BBSListViewBase( const std::string& url, const std::string& arg1 = std::string() , const std::string& arg2 = std::string() );
        ~BBSListViewBase();

        //
        // SKELETON::View の関数のオーバロード
        //

        void save_session() override;

        // 親ウィンドウをセット
        void set_parent_win( Gtk::Window* parent_win ) override;

        std::string url_for_copy() override { return {}; }

        bool set_command( const std::string& command,
                          const std::string& arg1 = {},
                          const std::string& arg2 = {} ) override;

        void clock_in() override;

        // キーを押した
        bool slot_key_press( GdkEventKey* event ) override;

        void stop() override;
        void redraw_view() override;
        void relayout() override;  // 色やフォントなどの変更
        void focus_view() override;
        void focus_out() override;
        void close_view() override;
        void delete_view() override;

        // ツリー内の全ての項目をURLを新しいアドレスに変更 ( id は未使用 )
        void update_item( const std::string& url, const std::string& id ) override;

        bool operate_view( const int control ) override;
        void goto_top() override;
        void goto_bottom() override;

        // 検索
        void exec_search() override;
        void up_search() override;
        void down_search() override;
        void operate_search( const std::string& controlid ) override;

        // 挿入先ダイアログを表示してアイテム追加
        // あらかじめ共有バッファに追加するデータをセットしておくこと
        void append_item();

        // 履歴のセット
        // 先頭にアイテムを追加する。ツリーにアイテムが含まれている場合は移動する
        // あらかじめ共有バッファに追加するデータをセットしておくこと
        void append_history();

        // 履歴を DATA_INFO_LIST 型で取得
        void get_history( CORE::DATA_INFO_LIST& info_list );

        // 指定したidのディレクトリに含まれるスレのアドレスを取得
        void get_threads( const size_t dirid, std::vector< std::string >& list_url );

        // 指定したidのディレクトリの名前を取得
        std::string get_dirname( const int dirid );

        // selectdialogで使う
        Gtk::TreePath get_current_path() { return m_treeview.get_current_path(); }
        void copy_treestore( Glib::RefPtr< Gtk::TreeStore >& store );

        // undo, redo
        void undo();
        void redo();

      private:

        void set_fgcolor_of_comment( const Gtk::TreeModel::Children& children );

        void row_up();
        void row_down();
        void page_up();
        void page_down();
        void prev_dir();
        void next_dir();
        void expand_all_dir( Gtk::TreeModel::Path path );

        // ディレクトリ以下を更新チェック
        // root : true ならルートから検索する。falseの場合は m_path_selected にパスをセットしておくこと
        // open : チェック後に更新していたら開く
        void check_update_dir( const bool root, const bool open );


        bool slot_button_press( GdkEventButton* event );
        bool slot_button_release( GdkEventButton* event );
        bool slot_motion_notify( GdkEventMotion* event );
        bool slot_key_release( GdkEventKey* event );
        bool slot_scroll_event( GdkEventScroll* event );
        void slot_dropped_from_other( const CORE::DATA_INFO_LIST& list_info );

        void slot_open_tab();
        void slot_open_browser();
        void slot_open_cache_browser();
        void slot_append_favorite();
        void slot_newdir();
        void slot_newcomment();
        void slot_newetcboard();
        void slot_moveetcboard();
        void slot_rename();
        void slot_copy_url();
        void slot_copy_title_url();
        void slot_select_all_dir();
        void slot_select_all();
        void slot_check_update_dir();
        void slot_check_update_open_dir();
        void slot_opendir_as_board();
        void slot_create_vboard();
        void slot_search_cache_board();
        void slot_import_dat();
        void slot_preferences_board();
        void slot_preferences_article();
        void slot_preferences_image();
        void slot_row_exp( const Gtk::TreeModel::iterator& it, const Gtk::TreeModel::Path& path );
        void slot_row_col( const Gtk::TreeModel::iterator& it, const Gtk::TreeModel::Path& path );        
        void slot_checkupdate_selected_rows();
        void slot_checkupdate_open_selected_rows();
        void slot_sort( const int mode );

        virtual void delete_view_impl();

        // 外部板追加/編集
        void add_newetcboard( const bool move, const std::string& _url, const std::string& _name, const std::string& _id, const std::string& _passwd );

        // D&D 関係
        void slot_drag_drop( Gtk::TreeModel::Path path, const bool after );

        virtual bool open_row( Gtk::TreePath& path, const bool tab );
        virtual void switch_rightview();
        void open_selected_rows();
        void checkupdate_selected_rows( const bool open );

        void show_status();

        // ツリーの編集ウィンドウが閉じた
        void slot_hide_editlistwin();
    };
}


#endif
