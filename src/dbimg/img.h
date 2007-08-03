// ライセンス: GPL2

// 画像データクラス

#ifndef _IMG_H
#define _IMG_H

#include "skeleton/loadable.h"

#include <string>

namespace Gtk
{
    class Window;
}

namespace DBIMG
{
    // 画像タイプ
    enum{
        T_NOIMG = 0,
        T_JPG,
        T_PNG,
        T_GIF,
        T_LARGE,
        T_OPENFAILED,
        T_WRITEFAILED,
        T_UNKNOWN
    };

    class Img : public SKELETON::Loadable
    {
        std::string m_url;
        std::string m_url_alt; // リダイレクトなどで使用するアドレス
        int m_count_redirect; // リダイレクト回数

        int m_type; // 画像タイプ
        int m_width;
        int m_height;
        bool m_mosaic; // モザイクかける
        bool m_zoom_to_fit; // windowにサイズをあわせる
        int m_size; // 画像の大きさ(パーセントで)
        bool m_protect; // true ならキャッシュを保護する( delete_cache()で削除しない )
        std::string m_refurl; // 参照元URL
        bool m_abone; // あぼーんされている

        // 保存用ファイルハンドラ
        FILE* m_fout;
        
      public:
        Img( const std::string& url );
        ~Img();

        void reset();
        void clear();

        const std::string& url() const { return m_url; }
        std::string get_cache_path();

        const int get_type() const { return m_type; }

        const int get_width();
        const int get_height();

        const bool is_cached();

        const bool get_abone() const { return m_abone; }
        void set_abone( bool abone );

        const bool get_mosaic() const { return m_mosaic; }
        void set_mosaic( bool mosaic );

        const bool is_zoom_to_fit() const { return m_zoom_to_fit; }
        void set_zoom_to_fit( bool fit ) { m_zoom_to_fit = fit; }

        // 表示倍率
        // ファイルサイズ(byte)は JDLIB::Loadable::total_length() で取得
        const int get_size() const { return m_size; }
        void set_size( int size ) { m_size = size; }

        const bool is_protected() const { return m_protect; }
        const std::string& get_refurl() const { return m_refurl; }

        void set_protect( bool protect );

        void download_img( const std::string refurl );
        bool save( Gtk::Window* parent, const std::string& path_to );
        
      private:

        virtual void receive_data( const char* data, size_t size );
        virtual void receive_finish();

        void read_info();
        void save_info();
    };
}

#endif
