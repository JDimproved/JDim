// ライセンス: 最新のGPL
//
// 書き込みなどのプレビュー 
//

#ifndef _ARTICLEVIEWPREVIEW_H
#define _ARTICLEVIEWPREVIEW_H

#include "articleviewbase.h"

namespace ARTICLE
{
    class ArticleViewPreview : public ArticleViewBase
    {
      public:
        ArticleViewPreview( const std::string& url );
        ~ArticleViewPreview();

        virtual void operate_view( const int& control );
        virtual void append_dat( const std::string& dat, int num = 0 );

      private:
        virtual void pack_widget();

        virtual void goto_num( int num ){}
        virtual void slot_drawout_id(){}        
        virtual void slot_drawout_ref(){}        
    };
}

#endif
