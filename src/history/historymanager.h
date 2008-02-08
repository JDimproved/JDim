// ライセンス: GPL2

//
// 履歴管理クラス
//

#ifndef _HISTORYMANAGER_H
#define _HISTORYMANAGER_H

#include <list>
#include <vector>
#include <string>

namespace Gtk
{
    class MenuItem;
};


namespace HISTORY
{
    class HistoryMenuThread;
    class HistoryMenuBoard;
    class HistoryMenuClose;
    class ViewHistory;
    class ViewHistoryItem;

    class History_Manager
    {
        HistoryMenuThread* m_menu_thread;
        HistoryMenuBoard* m_menu_board;
        HistoryMenuClose* m_menu_close;

        // View履歴
        std::list< ViewHistory* > m_view_histories;
        ViewHistory* m_last_viewhistory;

      public:

        History_Manager();
        virtual ~History_Manager();

        // メイン履歴
        Gtk::MenuItem* get_menu_thread();
        Gtk::MenuItem* get_menu_board();
        Gtk::MenuItem* get_menu_close();

        // メイン履歴追加
        void append_thread( const std::string& url, const std::string& name, const int type );
        void append_board( const std::string& url, const std::string& name, const int type );
        void append_close( const std::string& url, const std::string& name, const int type );

        // メイン履歴クリア
        void clear_thread();
        void clear_board();
        void clear_close();

        // メイン履歴更新
        void update_thread();
        void update_board();
        void update_close();

        //////////////////////////////////////////////////////////////////////////////
        //
        // View履歴

        // 作成 / 削除
        void create_viewhistory( const std::string& url );
        void delete_viewhistory( const std::string& url );

        // 現在表示中のViewのURL( url_old) を新しいURL( url_new )に変更
        const bool replace_current_url_viewhistory( const std::string& url_old, const std::string& url_new ); 

        // 履歴全体で url_old を url_new に変更
        void replace_url_viewhistory( const std::string& url_old, const std::string& url_new ); 

        // タイトル更新
        const bool replace_current_title_viewhistory( const std::string& url, const std::string& title );

        // item の取得
        std::vector< ViewHistoryItem* >& get_items_back_viewhistory( const std::string& url, const int count );
        std::vector< ViewHistoryItem* >& get_items_forward_viewhistory( const std::string& url, const int count );

        // 追加
        const bool append_viewhistory( const std::string& url_current, const std::string& url_append );

        // 戻る / 進む
        // exec = true のときは履歴の位置を変更する
        // false の時はURLの取得のみ
        const bool can_back_viewhistory( const std::string& url, const int count );
        const bool can_forward_viewhistory( const std::string& url, const int count );
        const ViewHistoryItem* back_viewhistory( const std::string& url, const int count, const bool exec ); 
        const ViewHistoryItem* forward_viewhistory( const std::string& url, const int count, const bool exec );

      private:

        // XML <-> View履歴変換
        void xml2viewhistory();
        void viewhistory2xml();

        // View履歴取得
        ViewHistory* get_viewhistory( const std::string& url );
    };



    ///////////////////////////////////////
    // インターフェース

    History_Manager* get_history_manager();
    void delete_history_manager();
}

#endif
