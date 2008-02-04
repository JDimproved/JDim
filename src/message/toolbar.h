// ライセンス: GPL2

// ツールバーのクラス

#ifndef _MESSAGE_TOOLBAR_H
#define _MESSAGE_TOOLBAR_H

#include <gtkmm.h>

#include "skeleton/toolbar.h"
#include "skeleton/imgbutton.h"
#include "skeleton/label_entry.h"

namespace MESSAGE
{
    class MessageToolBarBase : public SKELETON::ToolBar
    {
        SKELETON::ImgButton* m_button_preview;

      public:

        MessageToolBarBase();
        virtual ~MessageToolBarBase(){}

        // 書き込みボタンをフォーカス
        void focus_writebutton();

      protected:

        SKELETON::ImgButton* get_button_preview();

      private:

        void slot_toggle_preview();
    };

    ///////////////////////////////////

    // 通常
    class MessageToolBar : public MessageToolBarBase
    {
        SKELETON::ImgButton m_button_insert_draft;
        SKELETON::ImgButton m_button_undo;

        SKELETON::LabelEntry m_entry_subject;

      public:

        MessageToolBar();
        virtual ~MessageToolBar(){}

        SKELETON::LabelEntry* get_entry_subject(){ return &m_entry_subject; }

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
