// ライセンス: GPL2

// 画像データクラス

#ifndef _IMG_H
#define _IMG_H

#include "imghash.h"

#include "skeleton/loadable.h"

#include <optional>
#include <string>
#include <string_view>


namespace Gtk
{
    class Window;
}

namespace DBIMG
{
    class Img : public SKELETON::Loadable
    {
        std::string m_url;
        std::string m_url_alt; // リダイレクトなどで使用するアドレス
        int m_count_redirect{}; // リダイレクト回数

        int m_imgctrl; // 画像コントロール
        int m_type; // 画像タイプ

        // 幅、高さ
        int m_width;
        int m_height;

        // 埋め込み画像の幅、高さ
        int m_width_emb;
        int m_height_emb;

        // モザイク処理時に縮小するサイズ
        int m_width_mosaic;
        int m_height_mosaic;

        bool m_mosaic; // モザイクかける
        bool m_force_mosaic{}; ///< true なら強制的にモザイク表示するモード
        bool m_zoom_to_fit; // windowにサイズをあわせる
        int m_size; // 画像の大きさ(パーセントで)
        bool m_protect; // true ならキャッシュを保護する( delete_cache()で削除しない )
        std::string m_refurl; // 参照元のスレのURL
        bool m_abone; // あぼーんされている

        // 読み込み待ち
        bool m_wait;
        int m_wait_counter;

        // 保存用ファイルハンドラ
        FILE* m_fout{};

        std::optional<DHash> m_dhash{}; ///< 画像のハッシュ値
        std::string m_abone_reason; ///< あぼーんした理由のテキスト

      public:

        explicit Img( const std::string& url );
        ~Img() override;

        void clock_in();

        void reset();
        void clear();

        const std::string& url() const { return m_url; }
        std::string get_cache_path() const;

        bool is_wait() const { return m_wait; }

        int get_imgctrl() const { return m_imgctrl; }

        int get_type() const { return m_type; }
        void set_type( const int type ){ m_type = type; }

        // 高さ、幅
        int get_width() const { return m_width; }
        int get_height() const { return m_height; }

        // スレ埋め込み画像の高さ、幅
        int get_width_emb();
        int get_height_emb();

        // モザイク処理時に縮小するサイズ
        int get_width_mosaic();
        int get_height_mosaic();

        bool is_cached() const;

        bool get_abone() const { return m_abone; }
        void set_abone( bool abone, const std::string& abone_reason = {} );

        bool get_mosaic() const { return m_mosaic; }
        void set_mosaic( const bool mosaic );

        bool is_force_mosaic() const noexcept { return m_force_mosaic; }

        void show_large_img();

        bool is_zoom_to_fit() const { return m_zoom_to_fit; }
        void set_zoom_to_fit( bool fit ) { m_zoom_to_fit = fit; }

        // 表示倍率
        // ファイルサイズ(byte)は JDLIB::Loadable::total_length() で取得
        int get_size() const { return m_size; }
        void set_size( int size ) { m_size = size; }

        const std::string& get_refurl() const { return m_refurl; }

        bool is_protected() const { return m_protect; }
        void set_protect( bool protect );

        // 拡張子が偽装されているか
        bool is_fake() const;

        void set_dhash( const DHash& dhash );
        const std::optional<DHash>& get_dhash() const noexcept { return m_dhash; }

        const std::string& get_abone_reason() const noexcept { return m_abone_reason; }

        // ロード開始
        // receive_data()　と receive_finish() がコールバックされる
        // refurl : 参照元のスレのアドレス
        // mosaic_mode : モザイク表示するか
        //               ( 0: モザイク表示しない, 1: モザイク表示する, 2: 強制的にモザイク表示する )
        // waitsec: 指定した秒数経過後にロード開始
        void download_img( const std::string& refurl, const int mosaic_mode, const int waitsec );

        // ロード停止
        void stop_load() override;

        bool save( Gtk::Window* parent, const std::string& path_to );
        
      private:

        void receive_data( std::string_view buf ) override;
        void receive_finish() override;

        // ロード待ち状態セット/リセット
        bool set_wait( const std::string& refurl, const int mosaic_mode, const int waitsec );
        void reset_wait();

        // 埋め込み画像のサイズを計算
        void set_embedded_size();

        // モザイク処理時に縮小するサイズを経産
        void set_mosaic_size();

        void read_info();
        void save_info();
    };
}

#endif
