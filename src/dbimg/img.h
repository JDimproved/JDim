// ライセンス: 最新のGPL

// 画像データクラス

#ifndef _IMG_H
#define _IMG_H

#include "skeleton/loadable.h"

#include <string>

namespace DBIMG
{
    // 画像タイプ
    enum{
        T_NOIMG = 0,
        T_JPG,
        T_PNG,
        T_GIF,
        T_UNKNOWN
    };

    class Img : public SKELETON::Loadable
    {
        std::string m_url;

        int m_type; // 画像タイプ
        bool m_mosaic; // モザイクかける
        bool m_zoom_to_fit; // windowにサイズをあわせる
        int m_size; // 画像の大きさ(パーセントで)
        bool m_protect; // true ならキャッシュを保護する( delete_cache()で削除しない )
        std::string m_refurl; // 参照元URL

        // 保存用ファイルハンドラ
        FILE* m_fout;

        // ファイル判定用のシグネチャ
        unsigned char m_sign[ 16 ];
        
      public:
        Img( const std::string& url );
        ~Img();

        void clear();

        const std::string& url() const { return m_url; }
        const bool is_cached();

        const bool get_mosaic() const { return m_mosaic; }
        void set_mosaic( bool mosaic );

        const bool is_zoom_to_fit() const { return m_zoom_to_fit; }
        void set_zoom_to_fit( bool fit ) { m_zoom_to_fit = fit; }

        // 表示倍率
        // ファイルサイズ(byte)は JDLIB::Loadable::total_length() で取得
        const int get_size() const { return m_size; }
        void set_size( int size ) { m_size = size; }

        const bool is_protected() const { return m_protect; }
        const std::string& refurl() const { return m_refurl; }
        void set_refurl( const std::string& refurl ){ m_refurl = refurl; }

        void set_protect( bool protect );

        void download_img();
        bool save( const std::string& path_to );
        
      private:

        virtual void receive_data( const char* data, size_t size );
        virtual void receive_finish();

        void read_info();
        void save_info();
    };
}

#endif
