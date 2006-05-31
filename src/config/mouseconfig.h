// ライセンス: 最新のGPL
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

namespace CONFIG
{
    class MouseConfig : public MouseKeyConf
    {
      public:

        MouseConfig();
        virtual ~MouseConfig();

        // 操作文字列取得
        virtual const std::string get_str_motion( int id );

      private:

        void load_conf();
        virtual void set_one_motion( const std::string& name, const std::string& str_motion );
    };

    MouseConfig* get_mouseconfig();
    void delete_mouseconfig();
}

#endif
