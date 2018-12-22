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
    class Menu;
    class MenuItem;
}


namespace HISTORY
{
    class HistoryMenu;
    class ViewHistory;
    struct ViewHistoryItem;

    class History_Manager
    {
        // 履歴メニュー
        HistoryMenu* m_menu_thread;
        HistoryMenu* m_menu_board;
        HistoryMenu* m_menu_close;
        HistoryMenu* m_menu_closeboard;
        HistoryMenu* m_menu_closeimg;

        // View履歴
        std::list< ViewHistory* > m_view_histories;
        ViewHistory* m_last_viewhistory;

      public:

        History_Manager();
        virtual ~History_Manager();

        // 履歴メニュー取得
        Gtk::MenuItem* get_menu_thread();
        Gtk::MenuItem* get_menu_board();
        Gtk::MenuItem* get_menu_close();
        Gtk::MenuItem* get_menu_closeboard();
        Gtk::MenuItem* get_menu_closeimg();

        // url_history で指定した履歴に追加
        void append_history( const std::string& url_history, const std::string& url, const std::string& name, const int type );

        // url_history で指定した履歴の先頭を復元
        void restore_history( const std::string& url_history );

        // url_history で指定した履歴を全クリア
        void remove_allhistories( const std::string& url_history );

        // url_history で指定した履歴メニューのラベルを更新
        void set_menulabel( const std::string& url_history );


        //////////////////////////////////////////////////////////////////////////////
        //
        // View履歴

      public:

        // View履歴をXMLに変換して保存
        void viewhistory2xml();

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

        // XMLを読み込んで View履歴に変換
        void xml2viewhistory();

        // View履歴取得
        ViewHistory* get_viewhistory( const std::string& url );
    };



    ///////////////////////////////////////
    // インターフェース

    History_Manager* get_history_manager();
    void delete_history_manager();

    // url_history で指定した履歴に追加
    void append_history( const std::string& url_history, const std::string& url, const std::string& name, const int type );

    // url_history で指定した履歴の先頭を復元
    void restore_history( const std::string& url_history );

    // url_history で指定した履歴を全クリア
    void remove_allhistories( const std::string& url_history );
}

#endif
