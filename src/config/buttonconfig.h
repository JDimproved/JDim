// ライセンス: GPL2
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

        // 中ボタンでタブで開くか
        bool tab_midbutton();

        // タブで開くボタンを入れ替える
        void toggle_tab_button();

      private:

        void load_conf();
        virtual void set_one_motion( const std::string& name, const std::string& str_motion );
    };

    ButtonConfig* get_buttonconfig();
    void delete_buttonconfig();
}

#endif
