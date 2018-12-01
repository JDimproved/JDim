// ライセンス: GPL2
//
// 情報表示用
//

#ifndef _ARTICLEVIEWINFO_H
#define _ARTICLEVIEWINFO_H

#include "articleviewbase.h"

namespace ARTICLE
{
    class ArticleViewInfo : public ArticleViewBase
    {
      public:
        ArticleViewInfo( const std::string& url );
        ~ArticleViewInfo();

        // viewの操作をキャンセル
        virtual const bool operate_view( const int control ){ return false; }

      protected:

        // ポップアップメニューは表示しない
        virtual Gtk::Menu* get_popupmenu( const std::string& url ){ return NULL; }

      private:

        // ボタンプレスキャンセル
        virtual bool slot_button_press( std::string url, int res_number, GdkEventButton* event ){ return true; }

        // ポップアップ表示キャンセル
        virtual void slot_on_url( std::string url, std::string imgurl, int res_number ) {}

        virtual DrawAreaBase* create_drawarea();
    };
}

#endif
