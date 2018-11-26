// ライセンス: GPL2

//
// ファイルローダ
//
// SKELETON::Loadable と組み合わせて使用する
//

#ifndef _LOADER_H
#define _LOADER_H

#include "loaderdata.h"
#include "jdthread.h"

#include <string>
#include <list>
#include <zlib.h>

#ifdef _WIN32
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <netdb.h>
#endif

// zlibが1.2よりバージョンが低いか判定する
#ifndef ZLIB_VERNUM
#define ZLIB_VERNUM 0x1000
#endif
#if ZLIB_VERNUM < 0x1200
#define USE_OLDZLIB
#endif


namespace SKELETON
{
    class Loadable;
}


namespace JDLIB
{
    class Loader
    {
        LOADERDATA m_data;
        struct addrinfo* m_addrinfo;

        bool m_stop; // = true にするとスレッド停止
        bool m_loading;
        JDLIB::Thread m_thread;
        SKELETON::Loadable* m_loadable;

        // スレッド起動待ち状態になった時に、起動順のプライオリティを下げる
        bool m_low_priority; 
        
        // 読み込みバッファ
        unsigned long m_lng_buf; 
        char* m_buf;

        // zlib 用のバッファ
        unsigned long m_lng_buf_zlib_in;
        unsigned long m_lng_buf_zlib_out;
        Bytef* m_buf_zlib_in;
        Bytef* m_buf_zlib_out;

        // chunk 用変数
        bool m_use_chunk;
        long m_status_chunk;
        char m_str_sizepart[ 64 ]; // サイズ部のバッファ。64byte以下と仮定(超えるとエラー)
        char* m_pos_sizepart;
        size_t m_lng_leftdata;
    
        // zlib 用変数
        bool m_use_zlib;
        z_stream m_zstream;
#ifdef USE_OLDZLIB
        bool m_check_gzheader;
#endif
        
    public:

        // low_priority = true の時はスレッド起動待ち状態になった時に、起動順のプライオリティを下げる
        Loader( const bool low_priority );
        ~Loader();

        bool is_loading() const { return m_loading; }
        const LOADERDATA& data() const { return m_data; }
        
        bool run( SKELETON::Loadable* cb, const LOADERDATA& data_in );
        void wait();
        void stop();
        
        bool get_low_priority() const { return m_low_priority; }

        void create_thread();

    private:

        static void* launcher( void* );

        void clear();
        void run_main();
        struct addrinfo* get_addrinfo( const std::string& hostname, const int port );
        const std::string create_msg_send();
        bool wait_recv_send( const int fd, const bool recv );
        bool send_connect( const int soc, std::string& errmsg );

        // ローディング終了処理
        void finish_loading();

        // ヘッダ用
        int receive_header( char* buf, size_t& read_size );
        bool analyze_header();
        std::string analyze_header_option( const std::string& option );
        std::list< std::string > analyze_header_option_list( const std::string& option );

        // chunk用
        bool skip_chunk( char* buf, size_t& read_size );

        // unzip 用
        bool init_unzip();
        bool unzip( char* buf, size_t& read_size );
#ifdef USE_OLDZLIB
        int check_gzheader( char* buf, size_t& size );
#endif        
    };

    // ローダの起動待ちキューにあるスレッドを実行しない
    // アプリ終了時にこの関数を呼び出さないとキューに登録されたスレッドが起動してしまうので注意
    void disable_pop_loader_queue();

    // mainの最後でローダが動いていないかチェックする関数
    void check_loader_alive();
}

#endif
