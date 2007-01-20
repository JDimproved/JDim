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
        virtual ~MessageViewMain();

        virtual void reload();

      private:
        virtual void write();
        virtual std::string create_message();
    };


    // 新スレ立て用ビュー
    class MessageViewNew : public MessageViewBase
    {
      public:
        MessageViewNew( const std::string& url, const std::string& msg );
        virtual ~MessageViewNew(){}

        virtual void reload();

      private:
        virtual void write();
        virtual std::string create_message();
    };

}

    

#endif
