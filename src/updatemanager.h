// ライセンス: GPL2

//
// 更新チェッククラス
//
// push_back() 更新チェックするスレのグループを指定してからrun()を呼び出すと更新チェックを開始する。
// stop()で停止する。
//

#ifndef _UPDATEMANAGER_H
#define _UPDATEMANAGER_H

#include <list>
#include <string>


namespace CORE
{
    class CheckItem
    {
      public:

        std::list< std::string > urllist; // 更新チェックするスレのアドレスのリスト

        CheckItem(){ urllist.clear(); }
    };

    class CheckUpdate_Manager
    {
        std::list< CheckItem > m_list_item;
        std::list< std::string > m_list_open;

        bool m_running;
        int m_total;
        std::string m_url_checking;

      public:

        CheckUpdate_Manager();
        virtual ~CheckUpdate_Manager() noexcept;

        void run();
        void stop();

        // 更新チェックする板やスレをセットする
        // open == true なら更新チェック終了時に url を開く( url が更新可能状態なら )
        void push_back( const std::string& url, const bool open );

        // 次のスレをチェック
        void pop_front();
    };

    ///////////////////////////////////////
    // インターフェース

    CheckUpdate_Manager* get_checkupdate_manager();
    void delete_checkupdate_manager();
}

#endif
