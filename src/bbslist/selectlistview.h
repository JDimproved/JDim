// ライセンス: GPL2

//
// お気に入り追加の時の選択ビュー
//

#ifndef _SELECTLISTVIEW_H
#define _SELECTLISTVIEW_H

#include "bbslistviewbase.h"

namespace BBSLIST
{
    // 親の SelectListDialog や EditListWin に送る信号
    typedef sigc::signal< void > SIG_CLOSE_DIALOG;
    typedef sigc::signal< void > SIG_FOCUS_ENTRY_SEARCH;

    class SelectListView : public BBSListViewBase
    {
        SIG_CLOSE_DIALOG m_sig_close_dialog;
        SIG_FOCUS_ENTRY_SEARCH m_sig_focus_entry_search;

      public:

        SelectListView( const std::string& url, const std::string& arg1 = std::string() , const std::string& arg2 = std::string() );
        ~SelectListView() noexcept {}

        SIG_CLOSE_DIALOG sig_close_dialog() { return m_sig_close_dialog; }
        SIG_FOCUS_ENTRY_SEARCH sig_focus_entry_search() { return m_sig_focus_entry_search; }

        void save_xml() override {}

        void close_view() override;
        const bool operate_view( const int control ) override;

      private:

        const bool open_row( Gtk::TreePath& path, const bool tab ) override;
        void switch_rightview() override {} // boardに移動しないようにキャンセル
        Gtk::Menu* get_popupmenu( const std::string& url ) override;
    };

}


#endif
