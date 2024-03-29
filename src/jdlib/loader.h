// ライセンス: GPL2

//
// ファイルローダ
//
// SKELETON::Loadable と組み合わせて使用する
//

#ifndef _LOADER_H
#define _LOADER_H

#include "loaderdata.h"

#include <zlib.h>

#include <atomic>
#include <functional>
#include <list>
#include <optional>
#include <string>
#include <string_view>
#include <thread>
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


    /**
     * @brief gzip圧縮で送られてきたデータを展開する
     *
     * @details 展開した内容はコールバックに渡して処理する
     */
    class GzipDecoder
    {
    public:
        /** @brief 展開したデータを処理するコールバック関数のラッパー
         *
         * @param[in] expan      展開したデータへのポインター
         * @param[in] expan_size データのバイトサイズ
         */
        using CallbackWrapper = std::function<void(const char* expan, std::size_t expan_size)>;

    private:
        static constexpr std::size_t kMargin = 64;

        z_stream m_zstream;
        // zlib 用のバッファ
        std::vector<Bytef> m_buf_gzip_in;
        std::vector<Bytef> m_buf_gzip_out;
        std::size_t m_lng_gzip_in{};
        std::size_t m_lng_gzip_out{};

        CallbackWrapper m_callback;

        bool m_use_gzip{};

    public:
        GzipDecoder() = default;
        ~GzipDecoder() noexcept { clear(); }

        /// デコーダーが使われているか
        bool is_decoding() const noexcept { return m_use_gzip; }

        /// デコーダーの状態を開放する
        void clear();
        /// zlib 初期化
        bool setup( std::size_t lng_buf, CallbackWrapper callback );
        /// gzip圧縮されたデータを展開してcallbackに渡す
        std::optional<std::size_t> feed( const char* gzip, const std::size_t gzip_size );
    };


    class Loader
    {
        /// @brief HTTPヘッダー解析の結果を表す列挙型
        enum class HeaderParse
        {
            not_finished, ///< 解析は終わってない
            success, ///< 解析に成功した
            failure, ///< 解析に失敗した
        };

        LOADERDATA m_data;

        std::atomic<bool> m_stop{}; // = true にするとスレッド停止
        bool m_loading{};
        std::thread m_thread;
        SKELETON::Loadable* m_loadable;

        // スレッド起動待ち状態になった時に、起動順のプライオリティを下げる
        bool m_low_priority; 
        
        // 読み込みバッファ
        unsigned long m_lng_buf; 
        std::vector<char> m_buf;

        // chunk 用変数
        bool m_use_chunk;
        ChunkedDecoder m_chunk_decoder;

        // gzip 用変数
        GzipDecoder m_gzip_decoder;

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

        void clear();
        void run_main();
        std::string create_msg_send() const;

        // ローディング終了処理
        void finish_loading();

        // ヘッダ用
        HeaderParse receive_header( char* buf, size_t& read_size );
        bool analyze_header();
        std::string analyze_header_option( std::string_view option ) const;
        std::list< std::string > analyze_header_option_list( std::string_view option ) const;
    };

    // ローダの起動待ちキューにあるスレッドを実行しない
    // アプリ終了時にこの関数を呼び出さないとキューに登録されたスレッドが起動してしまうので注意
    void disable_pop_loader_queue();

    // mainの最後でローダが動いていないかチェックする関数
    void check_loader_alive();
}

#endif
