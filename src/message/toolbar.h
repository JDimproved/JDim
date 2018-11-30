// ライセンス: GPL2

// ツールバーのクラス

#ifndef _MESSAGE_TOOLBAR_H
#define _MESSAGE_TOOLBAR_H

#include <gtkmm.h>

#include "skeleton/toolbar.h"
#include "skeleton/label_entry.h"

namespace SKELETON
{
    class ImgToggleToolButton;
    class ImgToolButton;
}


namespace MESSAGE
{
    class MessageToolBarBase : public SKELETON::ToolBar
    {
        bool m_enable_slot;

        SKELETON::ImgToggleToolButton* m_button_preview;

      public:

        MessageToolBarBase();
        ~MessageToolBarBase() noexcept {}

        // previewボタンのトグル
        void set_active_previewbutton( const bool active );

      protected:

        SKELETON::ImgToggleToolButton* get_button_preview();

      private:

        void slot_toggle_preview();
    };

    ///////////////////////////////////

    // 通常
    class MessageToolBar : public MessageToolBarBase
    {
        SKELETON::ImgToolButton* m_button_insert_draft;
        SKELETON::ImgToolButton* m_button_undo;

        // false ならスレ名ラベル、trueなら新規レス名entry表示
        bool m_show_entry_new_subject;

        Gtk::ToolItem* m_tool_new_subject;
        Gtk::Entry* m_entry_new_subject;

      public:

        MessageToolBar();
        ~MessageToolBar() noexcept {}

        void show_entry_new_subject( bool show );
        std::string get_new_subject();
        void clear_new_subject();

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
        ~MessageToolBarPreview() noexcept {}

      protected:

        void pack_buttons() override;
    };
}


#endif
