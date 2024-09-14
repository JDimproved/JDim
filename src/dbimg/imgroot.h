// ライセンス: GPL2

// 画像データベースのルートクラス

#ifndef _IMGROOT_H
#define _IMGROOT_H

#include "imghash.h"

#include "jdlib/span.h"

#include <list>
#include <map>
#include <memory>
#include <string>
#include <string_view>
#include <vector>


namespace Gdk
{
    class Pixbuf;
}

namespace DBIMG
{
    class Img;

    class ImgRoot
    {
        std::map<std::string, std::unique_ptr<Img>> m_map_img;
        std::vector<AboneImgHash> m_vec_abone_imghash;
        std::list< Img* > m_list_wait; // ロード待ち状態のImgクラス
        std::list< Img* > m_list_delwait; // ロード待ち状態のImgクラスを削除する時の一時変数
        bool m_webp_support{}; // WebP の読み込みに対応しているか
        bool m_avif_support{}; // AVIF の読み込みに対応しているか

      public:
        ImgRoot();
        ~ImgRoot();

        bool is_webp_support() const noexcept { return m_webp_support; }
        bool is_avif_support() const noexcept { return m_avif_support; }

        void clock_in();

        // ロード待ちのためクロックを回すImgクラスをセット/リセット
        void set_clock_in( Img* img );
        void reset_clock_in( Img* img );
        void remove_clock_in();
        int get_wait_size() const noexcept { return m_list_wait.size(); }

        // Imgクラス取得(無ければ作成)
        Img* get_img( const std::string& url );

        // 検索(無ければnullptr)
        Img* search_img( const std::string& url ) = delete;

        // 画像データの先頭のシグネチャを見て画像のタイプを取得
        // 画像ではない場合は T_NOIMG を返す
        // 読み込みが未対応の場合は T_NOT_SUPPORT を返す
        int get_image_type( const unsigned char *sign ) const;

        // 拡張子から画像タイプを取得
        // 画像ではない場合は T_UNKNOWN を返す
        int get_type_ext( const std::string& url ) const;

        // キャッシュ削除
        void delete_cache( const std::string& url );

        // 全キャッシュ削除
        void delete_all_files();

        const std::vector<AboneImgHash>& get_vec_abone_imghash() const noexcept { return m_vec_abone_imghash; }
        void push_abone_imghash( const std::string& url, const int threshold );
        bool test_imghash( Img& img );
        void load_imghash_list( const std::string& contents );
        void save_abone_imghash_list();
        void update_abone_imghash_list( JDLIB::span<AboneImgHash> span )
        {
            m_vec_abone_imghash.assign( span.begin(), span.end() );
        }

        static DBIMG::DHash calc_dhash_from_pixbuf( const Gdk::Pixbuf& pixbuf );
        static int calc_hamming_distance( const DBIMG::DHash& a, const DBIMG::DHash& b ) noexcept;

      private:

        static bool is_jpg( std::string_view url );
        static bool is_png( std::string_view url );
        static bool is_gif( std::string_view url );
        static bool is_bmp( std::string_view url );
        static bool is_webp( std::string_view url );
        static bool is_avif( std::string_view url );

        void reset_imgs();
    };
}

#endif
