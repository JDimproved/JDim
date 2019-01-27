// ライセンス: GPL2

//
// 書き込みログの管理クラス
//

#ifndef _LOGMANAGER_H
#define _LOGMANAGER_H

#include <list>
#include <string>

namespace MESSAGE
{
    class LogItem;

    class Log_Manager
    {
        std::list< LogItem* > m_logitems;

      public:

        Log_Manager();
        virtual ~Log_Manager();

        int size() const { return m_logitems.size(); }
        bool has_items( const std::string& url, const bool newthread );
        void remove_items( const std::string& url );

        // messageが自分の書き込んだものかチェックする
        // newthread == true の時は新スレの>>1のチェック
        // headsize > 0 の時は先頭の headsize 文字だけを比較
        bool check_write( const std::string& url, const bool newthread, const char* msg_in, const size_t headsize );

        // 自分の書き込みの判定用データの保存
        void push_logitem( const std::string& url, const bool newthread, const std::string& msg );

        // ログの保存
        void save( const std::string& url,
                   const std::string& subject,  const std::string& msg, const std::string& name, const std::string& mail );

        //　書き込みログ取得
        const std::string get_post_log( const int num );

        // ログファイル( log/postlog-* ) の最大数
        int get_max_num_of_log();

        // ログ削除
        void clear_post_log();
    };

    ///////////////////////////////////////
    // インターフェース

    Log_Manager* get_log_manager();
    void delete_log_manager();
}

#endif
