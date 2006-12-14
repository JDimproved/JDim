// ライセンス: GPL2

//
// ファイルローダ
//
// このまま使うよりも SKELETON::Loadable と組み合わせた方が楽
//

#ifndef _LOADER_H
#define _LOADER_H

#include "loaderdata.h"

#include <string>
#include <list>
#include <zlib.h>

#include <netdb.h>
#include <gtkmm.h>


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
        pthread_t m_thread;
        SKELETON::Loadable* m_loadable;
        
        // 読み込みバッファ
        unsigned long m_lng_buf; 
        char* m_buf;

        // zlib 用のバッファ
        unsigned long m_lng_buf_zlib_in; 
        unsigned long m_lng_buf_zlib_out;; 
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

        Loader();
        ~Loader();

        const bool is_loading() const { return m_loading; }
        const LOADERDATA& data() const { return m_data; }
        
        bool run( SKELETON::Loadable* cb, const LOADERDATA& data_in );
        void wait();
        void stop(){ m_stop = true; }

    private:

        static void* launcher( void* );

        void clear();
        void run_main();
        struct addrinfo* get_addrinfo( const std::string& hostname, int port );
        std::string create_msg_send();
        bool wait_recv_send( int fd, bool recv );

        // ヘッダ用
        bool receive_header( char* buf, size_t& read_size );
        bool analyze_header();
        std::string analyze_header_option( char* option );
        std::list< std::string > analyze_header_option_list( char* option );

        // chunk用
        bool skip_chunk( char* buf, size_t& read_size );

        // unzip 用
        bool init_unzip();
        bool unzip( char* buf, size_t& read_size );
#ifdef USE_OLDZLIB
        int check_gzheader( char* buf, size_t& size );
#endif        
    };


    // ローダ作成関係

    int token();
    void get_token();
    void return_token();

    // loader作成関数
    Loader* create_loader();

    // mainの最後でローダが動いていないかチェックする関数
    void check_loader_alive();
}

#endif
