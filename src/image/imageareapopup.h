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

        explicit ImageAreaPopup( const std::string& url );
        ~ImageAreaPopup() override;

        void show_image() override;
    };
}

#endif
