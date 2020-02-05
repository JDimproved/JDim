// ライセンス: GPL2

// 履歴ビュー

#ifndef _HISTORYVIEW_H
#define _HISTORYVIEW_H

#include "bbslistviewbase.h"

namespace BBSLIST
{
    class HistoryViewBase : public BBSListViewBase
    {
        std::string m_file_xml;

      public:

        HistoryViewBase( const std::string& url, const std::string& file_xml,
                         const std::string& arg1, const std::string& arg2 );
        ~HistoryViewBase();

        void show_view() override;

      protected:

        // xml保存
        void save_xml() override;

        Gtk::Menu* get_popupmenu( const std::string& url ) override;
    };

    ///////////////////////////////////////

    class HistoryThreadView : public HistoryViewBase
    {
      public:

        explicit HistoryThreadView( const std::string& url, const std::string& arg1 = {}, const std::string& arg2 = {} );
    };

    class HistoryCloseView : public HistoryViewBase
    {
      public:

        explicit HistoryCloseView( const std::string& url, const std::string& arg1 = {}, const std::string& arg2 = {} );
    };

    class HistoryBoardView : public HistoryViewBase
    {
      public:

        explicit HistoryBoardView( const std::string& url, const std::string& arg1 = {}, const std::string& arg2 = {} );
    };

    class HistoryCloseBoardView : public HistoryViewBase
    {
      public:

        explicit HistoryCloseBoardView( const std::string& url, const std::string& arg1 = {}, const std::string& arg2 = {} );
    };

    class HistoryCloseImgView : public HistoryViewBase
    {
      public:

        explicit HistoryCloseImgView( const std::string& url, const std::string& arg1 = {}, const std::string& arg2 = {} );
    };
}

#endif
