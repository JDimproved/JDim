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

        HistoryThreadView( const std::string& url, const std::string& arg1 = std::string() , const std::string& arg2 = std::string() );
    };

    class HistoryCloseView : public HistoryViewBase
    {
      public:

        HistoryCloseView( const std::string& url, const std::string& arg1 = std::string() , const std::string& arg2 = std::string() );
    };

    class HistoryBoardView : public HistoryViewBase
    {
      public:

        HistoryBoardView( const std::string& url, const std::string& arg1 = std::string() , const std::string& arg2 = std::string() );
    };

    class HistoryCloseBoardView : public HistoryViewBase
    {
      public:

        HistoryCloseBoardView( const std::string& url, const std::string& arg1 = std::string() , const std::string& arg2 = std::string() );
    };

    class HistoryCloseImgView : public HistoryViewBase
    {
      public:

        HistoryCloseImgView( const std::string& url, const std::string& arg1 = std::string() , const std::string& arg2 = std::string() );
    };
}

#endif
