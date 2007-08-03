// ライセンス: GPL2
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

      private:
        virtual void pack_widget();

        virtual bool slot_button_press( std::string url, int res_number, GdkEventButton* event );

        virtual void goto_num( int num ){}
        virtual void slot_drawout_id(){}        
        virtual void slot_drawout_ref(){}        
    };
}

#endif
