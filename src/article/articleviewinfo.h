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
        explicit ArticleViewInfo( const std::string& url );
        ~ArticleViewInfo();

        // viewの操作をキャンセル
        bool operate_view( const int control ) override { return false; }

      protected:

        // ポップアップメニューは表示しない
        Gtk::Menu* get_popupmenu( const std::string& url ) override { return nullptr; }

      private:

        // ボタンプレスキャンセル
        bool slot_button_press( const std::string& url, int res_number, GdkEventButton* ) override { return true; }

        // ポップアップ表示キャンセル
        void slot_on_url( const std::string& url, const std::string& imgurl, int res_number ) override {}

        DrawAreaBase* create_drawarea() override;
    };
}

#endif
