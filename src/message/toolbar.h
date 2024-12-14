// ライセンス: GPL2

// ツールバーのクラス

#ifndef _MESSAGE_TOOLBAR_H
#define _MESSAGE_TOOLBAR_H

#include <gtkmm.h>

#include "skeleton/toolbar.h"


namespace MESSAGE
{
    class MessageToolBarBase : public SKELETON::ToolBar
    {
        bool m_enable_slot;

        Gtk::ToggleToolButton* m_button_preview{};

      public:

        MessageToolBarBase();
        ~MessageToolBarBase() noexcept override = default;

        // previewボタンのトグル
        void set_active_previewbutton( const bool active );

        void reload_ui_icon() override;

      protected:

        Gtk::ToggleToolButton* get_button_preview();

      private:

        void slot_toggle_preview();
    };

    ///////////////////////////////////

    // 通常
    class MessageToolBar : public MessageToolBarBase
    {
        Gtk::ToolButton* m_button_insert_draft{};
        Gtk::ToolButton* m_button_undo{};

        // false ならスレ名ラベル、trueなら新規レス名entry表示
        bool m_show_entry_new_subject{};

        Gtk::ToolItem* m_tool_new_subject{};
        Gtk::Entry* m_entry_new_subject{};

      public:

        MessageToolBar();
        ~MessageToolBar() noexcept override;

        void show_entry_new_subject( bool show );
        std::string get_new_subject() const;
        void clear_new_subject();

        void reload_ui_icon() override;

      protected:

        void pack_buttons() override;

      private:

        void slot_undo_clicked();
        void slot_insert_draft_clicked();
    };

    ///////////////////////////////////

    // プレビュー用
    class MessageToolBarPreview : public MessageToolBarBase
    {
      public:

        MessageToolBarPreview();
        ~MessageToolBarPreview() noexcept override;

      protected:

        void pack_buttons() override;
    };
}


#endif
