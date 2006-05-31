// ライセンス: 最新のGPL

//
// メイン画像クラス
//

#ifndef _IMAGEAREA_H
#define _IMAGEAREA_H

#include "imageareabase.h"

namespace IMAGE
{
    class ImageAreaMain : public ImageAreaBase
    {
      public:
        ImageAreaMain( const std::string& url );

        virtual void show_image();
    };
}

#endif
