// ライセンス: GPL2

// ツールバーのクラス

#ifndef _MESSAGE_TOOLBAR_H
#define _MESSAGE_TOOLBAR_H

#include <gtkmm.h>

#include "skeleton/toolbar.h"
#include "skeleton/imgbutton.h"
#include "skeleton/label_entry.h"
#include "skeleton/imgtogglebutton.h"

namespace MESSAGE
{
    class MessageToolBar : public SKELETON::ToolBar
    {
        friend class MessageViewBase;

        SKELETON::ImgButton m_button_write;
        SKELETON::ImgButton m_button_open;
        SKELETON::ImgButton m_button_undo;
        SKELETON::ImgToggleButton m_button_not_close;
        SKELETON::ImgToggleButton m_button_preview;

        SKELETON::LabelEntry m_entry_subject;

      public:

        MessageToolBar( const std::string& boardname ); 
        virtual ~MessageToolBar(){}

      protected:

        virtual void pack_buttons();
    };
}


#endif
