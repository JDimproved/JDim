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

#include <gtkmm.h>

namespace XML
{
    class Document;
}


namespace SKELETON
{
    class EditColumns;

    class EditTreeView : public DragTreeView
    {
        typedef sigc::signal< void, Gtk::TreeModel::Path, const bool > SIG_DRAG_DROP;

        SIG_DRAG_DROP m_sig_drag_drop;

        EditColumns& m_columns;
        JDLIB::ConstPtr< Gtk::CellRendererText > m_ren_text;

        bool m_editable;  // 編集可能

        // D&D時に下線を引くために使用
        int m_dnd_counter;
        Gtk::TreePath m_drag_path_uline;

        // 更新された
        bool m_updated;

      public:

        SIG_DRAG_DROP sig_drag_drop() { return m_sig_drag_drop; }

        // ColumnRecord として SKELETON::EditColumns を派生したものを使用すること
        EditTreeView( EditColumns& columns,
                    const bool use_usr_fontcolor, const std::string& fontname, const int colorid_text, const int colorid_bg, const int colorid_bg_even );
        EditTreeView( EditColumns& columns );

        virtual ~EditTreeView();

        virtual void clock_in();

        const bool is_updated() const { return m_updated; };
        void set_updated( const bool set ){ m_updated = set; }

        // treestoreのセット
        void set_treestore( const Glib::RefPtr< Gtk::TreeStore >& treestore );

        // xml -> tree 展開して treestore をセットする
        void xml2tree( XML::Document& document, Glib::RefPtr< Gtk::TreeStore >& treestore, const std::string& root_name );

        // tree -> XML 変換
        void tree2xml( XML::Document& document, const std::string& root_name );

        // 列の作成
        Gtk::TreeViewColumn* create_column( const int ypad );

        // 編集可能にする
        void set_editable_view( const bool editable );

        // 指定した path のタイプは ディレクトリか
        const bool is_dir( Gtk::TreeModel::iterator& it );
        const bool is_dir( const Gtk::TreePath& path );

        // ディレクトリ内を全選択
        void select_all_dir( Gtk::TreeModel::Path path_dir );

        // 新規ディレクトリ作成
        const Gtk::TreePath create_newdir( const Gtk::TreePath& path );

        // コメント挿入
        const Gtk::TreePath create_newcomment( const Gtk::TreePath& path );

        // pathで指定した行の名前の変更
        void rename_row( const Gtk::TreePath& path );
        const bool is_renaming_row(){ return m_ren_text->property_editable(); }

        // 行追加
        // after = false ならpath_dest の前に追加する( デフォルト after = true )
        // path_dest がNULLなら一番最後に作る
        // path_dest がディレクトリであり、かつ subdir = true なら path_dest の下に追加。
        // path_dest がディレクトリでない、または subdir = falseなら path_dest の後に追加
        // 戻り値は追加した行のpath
        const Gtk::TreeModel::Path append_row( const std::string& url, const std::string& name, const Glib::ustring data, const int type,
                                               const Gtk::TreeModel::Path path_dest,
                                               const bool subdir, const bool after );

        // 選択した行をまとめて削除
        void delete_selected_rows();

      protected:

        virtual bool on_drag_motion( const Glib::RefPtr<Gdk::DragContext>& context, int x, int y, guint time );
        virtual bool on_drag_drop( const Glib::RefPtr<Gdk::DragContext>& context, int x, int y, guint time );
        virtual void on_drag_end( const Glib::RefPtr< Gdk::DragContext>& context );

      private:

        // set_model()を使用不可にする。代わりにset_treestore()を使用すること
        void set_model( const Glib::RefPtr< Gtk::TreeModel >& model ){ Gtk::TreeView::set_model( model ); }

        void setup();

        // 全てのツリーに m_columns.m_expand の値をセットする( tree2xml()で使用 )
        void set_expanded_row( Glib::RefPtr< Gtk::TreeStore >& treestore, const Gtk::TreeModel::Children& children );

        void slot_ren_text_on_edited( const Glib::ustring& path, const Glib::ustring& text );
        void slot_ren_text_on_canceled();

        // 行の再帰コピー
        // dest が NULL なら一番下にappend
        // subdir = true　かつdestがディレクトリならサブディレクトリをその下に作ってそこにコピーする。false ならdestの後にコピー
        // after = false の場合はdestの前に挿入する
        // 成功したら dest にコピーした行のiteratorが入る
        const bool copy_row( const Gtk::TreeModel::iterator& src, Gtk::TreeModel::iterator& dest, const bool subdir, const bool after );

        // 選択した行をpathの所にまとめて移動
        // after = true なら path の後に移動。falseなら前
        void move_selected_row( const Gtk::TreePath& path, bool after );

        // ドラッグ中にマウスカーソルの下に下線を引く
        void draw_underline_while_dragging( Gtk::TreePath path );

        // draw == true なら pathに下線を引く
        void draw_underline( const Gtk::TreePath& path, bool draw );

        // D&Dマネージャから D&D 終了シグナルを受けたときに呼び出される
        void slot_receive_dnd_end();
   };
}

#endif
