// ライセンス: GPL2

//
// ロード可能クラスの基底クラス
// JDLIB::Loaderを使ってデータを受信する
//
// ・start_load() でロード開始。JDLIB::LOADERDATA data を引数に渡す
// ・stop_load() で停止
//
// ロード中はreceive_data()がコールバックされてデータが送られる
// ロードが終わると receive_finish() がコールバックされる
//
// ・ delete する前に terminate_load() を明示的に呼び出すこと(terminate_load()のコメントを参照)
//
// 詳しい流れは次の通り
/*

(1) start_load() の JDLIB::create_loader() でローダが作成される
(2) m_loader->run() でロード開始

--------ここから別スレッド内で実行

(3) ローダがデータを受け取るとreceive()がコールバックされてhttpコードやサイズを取得する
(4) receive()からreceive_data()が呼び出される 
(5) ロードが終了したらfinish()が呼ばれて  Dispatchable::dispatch()により
    ディスパッチャを使ってメインスレッドに制御を戻す

--------ここからメインスレッドに戻る

(6) ディスパッチャ経由で Dispatchable::callback_dispatch() が呼び出される
(7) クッキー、更新時刻などを取得する
(8) delete_loader()でローダの停止を待って削除する
(9) receive_finish()を呼び出す 

注意点

・receive_data()は別スレッド内で実行される
・receive_finish() はメインスレッド内で実行される
・receive_finish()が呼ばれた時点で既にローダは削除されているので明示的に削除する必要はない

*/


#ifndef _LOADABLE_H
#define _LOADABLE_H

#include "dispatchable.h"

#include <gtkmm.h>
#include <list>
#include <ctime>

namespace JDLIB
{
    class Loader;
    class LOADERDATA;
}


namespace SKELETON
{
    class Loadable : public Dispatchable
    {
        JDLIB::Loader* m_loader;

        bool m_low_priority; 

        // ローダからコピーしたデータ
        int m_code;
        std::string m_str_code;
        std::string m_contenttype;
        std::string m_date_modified;
        std::list< std::string > m_cookies;
        std::string m_location;
        size_t m_total_length;
        size_t m_current_length;

      public:

        Loadable();
        virtual ~Loadable();

        // HTTPコードなどの完全クリア
        void clear_load_data();

        // ロード中かどうか
        const bool is_loading() const;

        const int get_code() const { return m_code; }
        void set_code( int code ){ m_code = code; }

        const std::string& get_str_code() const { return m_str_code; }
        void set_str_code( const std::string& str_code ){ m_str_code = str_code; }

        const std::string& get_contenttype() const { return m_contenttype; }

        const std::list< std::string >& cookies() { return m_cookies; }
        const std::string& location() const { return m_location; }

        const size_t total_length() const { return m_total_length; }
        void set_total_length( int length ){ m_total_length = length; }

        const size_t current_length() const { return m_current_length; }
        void set_current_length( int length ){ m_current_length = length; }

        // 更新時刻関係
        const time_t get_time_modified();
        const std::string& get_date_modified() const { return m_date_modified; }
        void set_date_modified( const std::string& date ){ m_date_modified = date; }

        // ローダーからコールバックされてコードなどを取得してから
        // receive_data() を呼び出す
        void receive( const char* data, size_t size );

        // ディスパッチャ経由で Dispatchable::callback_dispatch()、 receive_finish()を呼ぶ
        // ロード直後に直接 receive_finish() は呼ばないこと
        void finish();

        // ロード開始
        // パラメータは Loader::run()の説明をみること
        bool start_load( const JDLIB::LOADERDATA& data );

        // ロード停止
        virtual void stop_load();

        // ロード強制停止
        // loadableを delete する前に terminate_load() を呼び出さないと
        // スレッド実行中にメモリなどが初期化されてしまうため落ちる時がある
        // デストラクタ内から terminate_load() しても落ちる時があるので
        // デストラクタの外から呼び出すこと
        void terminate_load();

        // ローダがスレッド起動待ち状態になった時に、起動順のプライオリティを下げる
        void set_priority_low(){ m_low_priority = true; }
        
      private:

        virtual void receive_data( const char* , size_t ){};
        virtual void receive_finish(){};

        void delete_loader();
        virtual void callback_dispatch();

        const int get_loader_code();
        const std::string get_loader_str_code();
        const std::string get_loader_contenttype();
        const std::string get_loader_modified();
        const std::list< std::string > get_loader_cookies();
        const std::string get_loader_location();
        const size_t get_loader_length();
    };
}

#endif
