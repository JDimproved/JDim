// ライセンス: GPL2

#ifndef _MESSAGEVIEW_H
#define _MESSAGEVIEW_H

#include "messageviewbase.h"

namespace MESSAGE
{
    // 通常の書き込みビュー
    class MessageViewMain : public MessageViewBase
    {
      public:
        MessageViewMain( const std::string& url, const std::string& msg );
        ~MessageViewMain();

        void reload() override;

      private:
        void write_impl( const std::string& msg ) override;
        const std::string create_message() override;
    };


    // 新スレ立て用ビュー
    class MessageViewNew : public MessageViewBase
    {
      public:
        MessageViewNew( const std::string& url, const std::string& msg );
        ~MessageViewNew() noexcept {}

        void reload() override;

      private:
        void write_impl( const std::string& msg ) override;
        const std::string create_message() override;
    };

}


#endif
