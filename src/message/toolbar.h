// ライセンス: GPL2

// ツールバーのクラス

#ifndef _MESSAGE_TOOLBAR_H
#define _MESSAGE_TOOLBAR_H

#include <gtkmm.h>

#include "skeleton/toolbar.h"
#include "skeleton/imgtoolbutton.h"
#include "skeleton/label_entry.h"

namespace MESSAGE
{
    class MessageToolBarBase : public SKELETON::ToolBar
    {
        SKELETON::ImgToolButton* m_button_preview;

      public:

        MessageToolBarBase();
        virtual ~MessageToolBarBase(){}

        // 書き込みボタンをフォーカス
        void focus_writebutton();

      protected:

        SKELETON::ImgToolButton* get_button_preview();

      private:

        void slot_toggle_preview();
    };

    ///////////////////////////////////

    // 通常
    class MessageToolBar : public MessageToolBarBase
    {
        SKELETON::ImgToolButton m_button_insert_draft;
        SKELETON::ImgToolButton m_button_undo;

        // false ならスレ名ラベル、trueなら新規レス名entry表示
        bool m_show_entry_new_subject;

        Gtk::ToolItem* m_tool_new_subject;
        Gtk::Entry* m_entry_new_subject;

      public:

        MessageToolBar();
        virtual ~MessageToolBar(){}

        void show_entry_new_subject( bool show );
        std::string get_new_subject();

      protected:

        virtual void pack_buttons();

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
        virtual ~MessageToolBarPreview(){}

      protected:

        virtual void pack_buttons();
    };
}


#endif
