// ライセンス: GPL2
//
// KeyConfig, MouseConfig, ButtonConfig 取得
//

#ifndef _GET_CONFIG_H
#define _GET_CONFIG_H

namespace CONTROL
{
    class KeyConfig;
    class MouseConfig;
    class ButtonConfig;

    KeyConfig* get_keyconfig();
    MouseConfig* get_mouseconfig();
    ButtonConfig* get_buttonconfig();

    void delete_keyconfig();
    void delete_mouseconfig();
    void delete_buttonconfig();
}

#endif
