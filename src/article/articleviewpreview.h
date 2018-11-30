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

        const bool operate_view( const int control ) override;

      private:

        bool slot_button_press( std::string url, int res_number, GdkEventButton* event ) override;

        void goto_num( const int num_to, const int num_from ) override {}
    };
}

#endif
