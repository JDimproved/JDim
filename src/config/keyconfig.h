// ライセンス: GPL2
//
// キー設定クラス
//

#ifndef _KEYCONFIG_H
#define _KEYCONFIG_H

#include "mousekeyconf.h"

namespace CONFIG
{
    class KeyConfig : public MouseKeyConf
    {
      public:

        KeyConfig();
        virtual ~KeyConfig();

        // 操作文字列取得
        virtual const std::string get_str_motion( int id );

        // editviewの操作をemacs風にする
        const bool is_emacs_mode();
        void toggle_emacs_mode();

        const bool is_toggled_tab_key(); // タブで開くキーを入れ替えているか
        void toggle_tab_key( const bool toggle ); // タブで開くキーを入れ替える

      private:

        void load_conf();
        virtual void set_one_motion( const std::string& name, const std::string& str_motion );
    };

    KeyConfig* get_keyconfig();
    void delete_keyconfig();
}


#endif
