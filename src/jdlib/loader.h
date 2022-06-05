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

#include <netdb.h>
#include <zlib.h>

#include <list>
#include <string>
#include <string_view>
#include <vector>


namespace SKELETON
{
    class Loadable;
}


namespace JDLIB
{
    /**
     * @brief Chunked transfer encodingで送られてきたデータをデコードする
     */
    class ChunkedDecoder
    {
        /// @brief デコーダーの状態
        enum class State
        {
            parse_size, ///< 初期状態、サイズ部の解析
            format_body, ///< データ部の整形
            check_body_cr, ///< データ部の後ろにあるCRのチェック
            check_body_lf, ///< データ部の後ろにあるLFのチェック
            completed, ///< 最後のチャンク(長さ0)を読み込んで処理が完了した
        };

        State m_state = State::parse_size; ///< デコーダーの状態
        std::size_t m_lng_leftdata{}; ///< データ部の残りサイズ
        std::string m_buf_sizepart; ///< 解析途中のサイズ部を保存しておくバッファ

    public:
        ChunkedDecoder() = default;
        ~ChunkedDecoder() noexcept = default;

        /// 最後のチャンク(長さ0)まで読み込んで処理が完了したか
        bool is_completed() const noexcept { return m_state == State::completed; }

        /// デコーダーの状態を初期化する
        void clear();
        /// chunked なデータを切りつめる
        bool decode( char* buf, std::size_t& read_size );
    };

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
        std::vector<char> m_buf;

        // zlib 用のバッファ
        unsigned long m_lng_buf_zlib_in;
        unsigned long m_lng_buf_zlib_out;
        std::vector<Bytef> m_buf_zlib_in;
        std::vector<Bytef> m_buf_zlib_out;

        // chunk 用変数
        bool m_use_chunk;
        ChunkedDecoder m_chunk_decoder;

        // zlib 用変数
        bool m_use_zlib;
        z_stream m_zstream;

    public:

        // low_priority = true の時はスレッド起動待ち状態になった時に、起動順のプライオリティを下げる
        explicit Loader( const bool low_priority );
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
        std::string create_msg_send() const;
        bool wait_recv_send( const int fd, const bool recv );
        bool send_connect( const int soc, std::string& errmsg );

        // ローディング終了処理
        void finish_loading();

        // ヘッダ用
        int receive_header( char* buf, size_t& read_size );
        bool analyze_header();
        std::string analyze_header_option( std::string_view option ) const;
        std::list< std::string > analyze_header_option_list( std::string_view option ) const;

        // unzip 用
        bool init_unzip();
        bool unzip( char* buf, std::size_t read_size );
    };

    // ローダの起動待ちキューにあるスレッドを実行しない
    // アプリ終了時にこの関数を呼び出さないとキューに登録されたスレッドが起動してしまうので注意
    void disable_pop_loader_queue();

    // mainの最後でローダが動いていないかチェックする関数
    void check_loader_alive();
}

#endif
