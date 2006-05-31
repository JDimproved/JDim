// ライセンス: 最新のGPL
//
// マウスボタン設定
//

#ifndef _BUTTONCONFIG_H
#define _BUTTONCONFIG_H

#include "mousekeyconf.h"

namespace CONFIG
{
    class ButtonConfig : public MouseKeyConf
    {
      public:

        ButtonConfig();
        virtual ~ButtonConfig();

      private:

        void load_conf();
        virtual void set_one_motion( const std::string& name, const std::string& str_motion );
    };

    ButtonConfig* get_buttonconfig();
    void delete_buttonconfig();
}

#endif
