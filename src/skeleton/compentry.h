// ライセンス: GPL2
//
// 補完 Entryクラス
//

#ifndef _COMPENTRY_H
#define _COMPENTRY_H

#include "entry.h"
#include "popupwinbase.h"
#include "treeviewbase.h"
#include "imgbutton.h"

#include <gtkmm.h>

namespace SKELETON
{
    class CompletionEntry : public Gtk::HBox
    {
        typedef sigc::signal< void, int > SIG_OPERATE;
        typedef sigc::signal< void > SIG_ACTIVATE;
        typedef sigc::signal< void > SIG_CHANGED;

        SIG_OPERATE m_sig_operate;
        SIG_ACTIVATE m_sig_activate;
        SIG_CHANGED m_sig_changed;

        int m_mode;

        JDEntry m_entry;
        bool m_enable_changed;
        bool m_focused;

        // ポップアップ
        bool m_show_popup;
        PopupWinBase m_popup_win;
        Gtk::ScrolledWindow m_scr_win;
        JDTreeViewBase m_treeview;
        Gtk::TreeModelColumn< Glib::ustring > m_column;
        Gtk::TreeModel::ColumnRecord m_column_record;
        Glib::RefPtr< Gtk::ListStore > m_liststore;

      public:

        SIG_OPERATE signal_operate(){ return m_sig_operate; }
        SIG_ACTIVATE signal_activate(){ return m_sig_activate; }
        SIG_CHANGED signal_changed(){ return m_sig_changed; }

        void add_mode( int mode ){ m_entry.add_mode( mode ); }

        // 補完実行
        bool completion();

        Glib::ustring get_text();
        void set_text( const Glib::ustring& text );
        void grab_focus();

        void modify_font( Pango::FontDescription& pfd ){ m_entry.modify_font( pfd ); }

      protected:
        CompletionEntry( int mode );
        virtual ~CompletionEntry();

      private:

        // ポップアップ表示
        void show_popup( bool show_all );

        // ポップアップ閉じる
        void hide_popup();

        // entryでキーが押された
        void slot_entry_key_press( int keyval );

        // entryでボタンを押した
        void slot_entry_button_press( GdkEventButton* event );

        // entry操作
        void slot_entry_operate( int controlid );

        // entry からsignal_activateを受け取った
        void slot_entry_acivate();

        // entry からsignal_changedを受け取った
        void slot_entry_changed();

        // entryのフォーカスが外れた
        bool slot_entry_focus_out( GdkEventFocus* );

        // ポップアップ内をマウスを動かした
        bool slot_treeview_motion( GdkEventMotion* );

        // ポップアップクリック
        bool slot_treeview_button_release( GdkEventButton* );
    };


    // 検索用
    class SearchEntry : public CompletionEntry
    {
      public:
        SearchEntry();
    };


    // 書き込み名前
    class NameEntry : public CompletionEntry
    {
      public:
        NameEntry();
    };

    // 書き込みメール
    class MailEntry : public CompletionEntry
    {
      public:
        MailEntry();
    };
}


#endif
