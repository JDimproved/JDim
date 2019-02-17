// ライセンス: GPL2
//
// マウスジェスチャ設定
//


#ifndef _MOUSECONFIG_H
#define _MOUSECONFIG_H

#include "mousekeyconf.h"

namespace CONTROL
{
    class MouseConfig : public MouseKeyConf
    {
      public:

        MouseConfig();
        ~MouseConfig() noexcept;

        void load_conf() override;

        // 操作文字列取得
        std::string get_str_motions( const int id ) override;

        // IDからデフォルトの操作文字列取得
        std::string get_default_motions( const int id ) override;

      private:

        // ひとつの操作をデータベースに登録
        void set_one_motion_impl( const int id, const int mode,
                                  const std::string& name, const std::string& str_motion ) override;
    };
}

#endif
