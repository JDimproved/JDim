// ライセンス: GPL2
//
// マウス設定
//

// マウスジェスチャ設定
//
//      8
//      ↑
//  4 ←  → 6
//      ↓
//      2 
//
// ( 例 ) ↑→↓← = 8624


#ifndef _MOUSECONFIG_H
#define _MOUSECONFIG_H

#include "mousekeyconf.h"

namespace CONTROL
{
    class MouseConfig : public MouseKeyConf
    {
      public:

        MouseConfig();
        virtual ~MouseConfig();

        virtual void load_conf();

        // 操作文字列取得
        virtual const std::string get_str_motions( const int id );

      private:

        // ひとつの操作をデータベースに登録
        virtual void set_one_motion_impl( const int id, const int mode, const std::string& name, const std::string& str_motion );
    };

    MouseConfig* get_mouseconfig();
    void delete_mouseconfig();
}

#endif
