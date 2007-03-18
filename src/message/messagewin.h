// ライセンス: GPL2

#ifndef _MESSAGEWIN_H
#define _MESSAGEWIN_H

#include <gtkmm.h>

#include "skeleton/window.h"

namespace MESSAGE
{
    class MessageWin : public SKELETON::JDWindow
    {
      public:

        MessageWin();
        virtual ~MessageWin();

        virtual void focus_in();

      protected:
        virtual bool on_delete_event( GdkEventAny* event );
    };
}


#endif
