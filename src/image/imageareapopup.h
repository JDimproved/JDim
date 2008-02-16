// ライセンス: GPL2

//
// ポップアップ画像クラス
//

#ifndef _IMAGEAREAPOPUP_H
#define _IMAGEAREAPOPUP_H

#include "imageareabase.h"

namespace IMAGE
{
    class ImageAreaPopup : public ImageAreaBase
    {
      public:

        ImageAreaPopup( const std::string& url );
        virtual ~ImageAreaPopup();
        
        virtual void show_image();
    };
}

#endif
