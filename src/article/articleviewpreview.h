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
        std::string m_url_messageview;

      public:
        ArticleViewPreview( const std::string& url );
        ~ArticleViewPreview();

        virtual const bool operate_view( const int control );

      private:

        virtual bool slot_button_press( std::string url, int res_number, GdkEventButton* event );

        virtual void goto_num( int num ){}
    };
}

#endif
