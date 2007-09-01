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

        const bool is_toggled_tab_button(); // タブで開くボタンを入れ替えているか
        void toggle_tab_button( const bool toggle ); // タブで開くボタンを入れ替える

        const bool is_popup_warpmode(); // ポップアップ表示の時にクリックでワープするか
        void toggle_popup_warpmode(); // ポップアップ表示の時にクリックでワープする


      private:

        void load_conf();
        virtual void set_one_motion( const std::string& name, const std::string& str_motion );
    };

    ButtonConfig* get_buttonconfig();
    void delete_buttonconfig();
}

#endif
