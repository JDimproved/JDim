// ライセンス: GPL2
//
// キー設定クラス
//

#ifndef _KEYCONFIG_H
#define _KEYCONFIG_H

#include "mousekeyconf.h"

#include <gtkmm.h>

namespace CONTROL
{
    class KeyConfig : public MouseKeyConf
    {
      public:

        KeyConfig();
        ~KeyConfig() noexcept;

        void load_conf() override;

        // editviewの操作をemacs風にする
        const bool is_emacs_mode();
        void toggle_emacs_mode();

        const bool is_toggled_tab_key(); // タブで開くキーを入れ替えているか
        void toggle_tab_key( const bool toggle ); // タブで開くキーを入れ替える

        // Gtk アクセラレーションキーを取得
        Gtk::AccelKey get_accelkey( const int id );

      private:

        // ひとつの操作をデータベースに登録
        void set_one_motion_impl( const int id, const int mode,
                                  const std::string& name, const std::string& str_motion ) override;
    };
}


#endif
