// ライセンス: GPL2
//
// D&Dによって行の編集が可能なtreeviewクラス
//
// set_editable_view() で true を指定すると編集可能になる
//

#ifndef _EDITTREEVIEW_H
#define _EDITTREEVIEW_H

#include "dragtreeview.h"

#include "jdlib/constptr.h"

#include "type.h"
#include "data_info.h"

#include <gtkmm.h>
#include <list>

namespace XML
{
    class Document;
}


namespace SKELETON
{
    // ソートのモード
    enum
    {
        SORT_BY_TYPE = 0,
        SORT_BY_NAME
    };

    // 他のwidgetからドロップされた
    typedef sigc::signal< void, const CORE::DATA_INFO_LIST& > SIG_DROPPED_FROM_OTHER;

    class EditColumns;
    class UNDO_BUFFER;

    class EditTreeView : public DragTreeView
    {
        SIG_DROPPED_FROM_OTHER m_sig_dropped_from_other;

        std::string m_url;
        Gtk::Window* m_parent_win;

        EditColumns& m_columns;
        JDLIB::ConstPtr< Gtk::CellRendererText > m_ren_text;

        // 編集可能
        bool m_editable;  

        // UNDO 用のバッファ
        UNDO_BUFFER* m_undo_buffer;

        // D&D用変数
        Gtk::TreePath m_drag_path_uline; // D&D時に下線を引いている行
        int m_dnd_counter; // D&D時のスクロールのタイミング用
        bool m_exec_drop; // D&D時のドロップ処理で挿入するか
        Gtk::TreeRow m_row_dest; // ドロップ先
        bool m_row_dest_before; // m_row_dest の前に挿入するか
        bool m_dropped_from_other; // 他のwidgetからドロップされた

        // スクロール用変数
        // 詳しくは EditTreeView::clock_in() を参照
        int m_pre_adjust_upper;
        Gtk::TreePath m_jump_path;
        int m_jump_count;

        // 更新された
        bool m_updated;

        // ドラッグがこのツリー上で行われている
        bool m_dragging_on_tree;

        // ディレクトリIDの最大値
        size_t m_max_dirid;

      public:

        // ColumnRecord として SKELETON::EditColumns を派生したものを使用すること
        EditTreeView( const std::string& url, const std::string& dndtarget, EditColumns& columns,
                      const bool use_usr_fontcolor, const std::string& fontname, const int colorid_text, const int colorid_bg, const int colorid_bg_even );

        EditTreeView( const std::string& url, const std::string& dndtarget, EditColumns& columns );

        ~EditTreeView();

        void clock_in() override;

        SIG_DROPPED_FROM_OTHER sig_dropped_from_other(){ return m_sig_dropped_from_other; }

        void set_parent_win( Gtk::Window* parent_win ){ m_parent_win = parent_win; }
        Gtk::Window* get_parent_win(){ return m_parent_win; }

        void set_undo_buffer( UNDO_BUFFER* undo_buffer ){ m_undo_buffer = undo_buffer; }

        bool is_updated() const { return m_updated; };
        void set_updated( const bool set ){ m_updated = set; }

        // treestoreのセット
        void set_treestore( const Glib::RefPtr< Gtk::TreeStore >& treestore );

        // xml -> tree 展開して treestore をセットする
        void xml2tree( XML::Document& document, Glib::RefPtr< Gtk::TreeStore >& treestore, const std::string& root_name );

        // tree -> XML 変換
        void tree2xml( XML::Document& document, const std::string& root_name );

        // 列の作成
        // ypad : 行間スペース
        Gtk::TreeViewColumn* create_column( const int ypad );

        // 編集可能にする
        void set_editable_view( const bool editable );

        // 指定した path のタイプは ディレクトリか
        bool is_dir( Gtk::TreeModel::iterator& it );
        bool is_dir( const Gtk::TreePath& path );

        // 次のディレクトリに移動
        void prev_dir();
        void next_dir();

        // 指定したアドレスの行が含まれているか
        bool exist_row( const std::string& url, const int type );

        // ディレクトリ内を全選択
        void select_all_dir( Gtk::TreePath path_dir );

        // 新規ディレクトリ作成
        Gtk::TreePath create_newdir( const Gtk::TreePath& path );

        // ディレクトリIDとパスを相互変換
        Gtk::TreePath dirid_to_path( const size_t dirid );
        size_t path_to_dirid( const Gtk::TreePath path );

        // コメント挿入
        Gtk::TreePath create_newcomment( const Gtk::TreePath& path );

        // pathで指定した行の名前の変更
        void rename_row( const Gtk::TreePath& path );
        bool is_renaming_row(){ return m_ren_text->property_editable(); }

        // list_info を path_dest 以下に追加
        //
        // list_info の各path にあらかじめ値をセットしておくこと
        // scroll = true なら追加した行にスクロールする
        // force = true なら m_editable が false でも追加
        // cancel_undo_commit = true なら undo バッファをコミットしない
        // check_dup == 0 ならチェックせず追加 1 なら重複チェックをして重複してたらダイアログ表示、2なら重複チェックして重複してたら追加しない
        //
        // (1) path_dest が empty なら一番最後
        // (2) before = true なら path_dest の前
        // (3) path_destがディレクトリなら path_dest の下
        // (4) そうでなければ path_dest の後
        CORE::DATA_INFO_LIST append_info( const CORE::DATA_INFO_LIST& list_info,
                                          const Gtk::TreePath& path_dest, const bool before, const bool scroll,
                                          const bool force, const bool cancel_undo_commit, int check_dup
            );

        // pathをまとめて削除
        // force = true なら m_editable が false でも削除
        void delete_path( std::list< Gtk::TreePath >& list_path, const bool force );

        // 選択した行をまとめて削除
        // force = true なら m_editable が false でも削除
        void delete_selected_rows( const bool force ) override;

        void undo();
        void redo();

        // 選択行をlist_infoにセットする
        // dir : true の時はディレクトリが選択されているときはディレクトリ内の行もlist_infoに再帰的にセットする
        void get_info_in_selection( CORE::DATA_INFO_LIST& list_info, const bool dir );

        // 一行追加
        // 戻り値は追加した行のpath
        // (1) path_dest が empty なら一番最後に作る
        // (2) before = true なら前に作る
        // (3) path_dest がディレクトリかつ sudir == true なら path_dest の下に追加。
        // (4) そうでなければ path_dest の後に追加
        Gtk::TreePath append_one_row( const std::string& url, const std::string& name, const int type,
                                      const size_t dirid, const std::string& data,
                                      const Gtk::TreePath& path_dest, const bool before, const bool subdir );

        // ソート実行
        void sort( const Gtk::TreePath& path, const int mode );

      protected:

        bool on_drag_motion( const Glib::RefPtr<Gdk::DragContext>& context, int x, int y, guint time ) override;
        void on_drag_leave( const Glib::RefPtr< Gdk::DragContext >& context, guint time ) override;
        bool on_drag_drop( const Glib::RefPtr<Gdk::DragContext>& context, int x, int y, guint time ) override;
        void on_drag_data_get( const Glib::RefPtr< Gdk::DragContext >& context,
                               Gtk::SelectionData& selection_data, guint info, guint time ) override;
        void on_drag_data_received( const Glib::RefPtr< Gdk::DragContext >& context, int x, int y,
                                    const Gtk::SelectionData& selection_data, guint info, guint time ) override;
        void on_drag_data_delete( const Glib::RefPtr<Gdk::DragContext>& context ) override;
        void on_drag_end( const Glib::RefPtr< Gdk::DragContext>& context ) override;

      private:

        // path にスクロール
        void set_scroll( const Gtk::TreePath& path );

        // set_model()をprivate化して使用不可にする。代わりにset_treestore()を使用すること
        void set_model( const Glib::RefPtr< Gtk::TreeModel >& model ){ Gtk::TreeView::set_model( model ); }

        void setup();

        // ディレクトリIDの最大値を取得
        void get_max_dirid();

        // ディレクトリにIDをセットする
        void set_dirid();

        // 全てのツリーに m_columns.m_expand の値をセットする( tree2xml()で使用 )
        void set_expanded_row( Glib::RefPtr< Gtk::TreeStore >& treestore, const Gtk::TreeModel::Children& children );

        void slot_ren_text_on_edited( const Glib::ustring& path, const Glib::ustring& text );
        void slot_ren_text_on_canceled();

        // ドラッグ中にマウスカーソルの下に下線を引く
        void draw_underline_while_dragging( Gtk::TreePath path );

        // draw == true なら pathに下線を引く
        void draw_underline( const Gtk::TreePath& path, bool const draw );

        // list_info の各要素の path を path_dest 以下に変更
        // list_info の各様の path にあらかじめ値をセットしておくこと
        // (1) path_dest が empty なら一番最後
        // (2) before = true なら path_dest の前
        // (3) path_destがディレクトリなら path_dest の下
        // (4) そうでなければ path_dest の後
        void replace_infopath( CORE::DATA_INFO_LIST& list_info,
                               const Gtk::TreePath& path_dest, const bool before );

        // ディレクトリ(path_dir)内の行を全てlist_infoにセットする
        void get_info_in_dir( CORE::DATA_INFO_LIST& list_info, const Gtk::TreePath& path_dir );

        // path から info を取得
        void path2info( CORE::DATA_INFO& info, const Gtk::TreePath& path );

        // list_info に示した行の親を再起的にexpandする
        // list_info の各要素の path にあらかじめ値をセットしておくこと
        void expand_rows( const CORE::DATA_INFO_LIST& list_info );

        // list_info に示した行を追加
        // list_info の各要素の path にあらかじめ値をセットしておくこと
        void append_rows( const CORE::DATA_INFO_LIST& list_info );

        // list_info に示した行を削除
        // list_info の各要素の path にあらかじめ値をセットしておくこと
        // 削除した後、path_select にカーソルを移動する(emptyの場合は移動しない)
        void delete_rows( const CORE::DATA_INFO_LIST& list_info, const Gtk::TreePath& path_select );

        // list_infoに示した行を選択
        void select_info( const CORE::DATA_INFO_LIST& list_info );
   };



    ////////////////////////////////

    // EditTreeViewの項目の反復子

    class EditTreeViewIterator
    {
        EditTreeView& m_treeview;
        EditColumns& m_columns;
        Gtk::TreePath::size_type m_depth;
        bool m_end;

        Gtk::TreePath m_path;

      public:

        // path から反復開始
        // path が empty の時はルートから反復する
        EditTreeViewIterator( EditTreeView& treeview, EditColumns& columns, const Gtk::TreePath path );

        Gtk::TreeModel::Row operator * ();
        Gtk::TreePath get_path() const { return m_path; }

        void operator ++ ();

        bool end() const { return m_end; }
    };
}

#endif
