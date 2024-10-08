// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "imginterface.h"
#include "imgroot.h"
#include "img.h"

#include "cache.h"
#include "global.h"


// インスタンスは Core でひとつだけ作って、Coreのデストラクタでdeleteする
static DBIMG::ImgRoot *instance_dbimg_root = nullptr;


void DBIMG::create_root()
{
    if( ! instance_dbimg_root ) instance_dbimg_root = new DBIMG::ImgRoot();
}


void DBIMG::delete_root()
{
    if( instance_dbimg_root ) delete instance_dbimg_root;
}


bool DBIMG::is_webp_support()
{
    if( instance_dbimg_root ) return instance_dbimg_root->is_webp_support();
    return false;
}

bool DBIMG::is_avif_support()
{
    if( instance_dbimg_root ) return instance_dbimg_root->is_avif_support();
    return false;
}


void DBIMG::clock_in()
{
    if( instance_dbimg_root ) instance_dbimg_root->clock_in();
}

// 読み込み待ちのためクロックを回すImgクラスをセット/リセット
void DBIMG::set_clock_in( Img* img )
{
    if( instance_dbimg_root ) instance_dbimg_root->set_clock_in( img );
}

void DBIMG::reset_clock_in( Img* img )
{
    if( instance_dbimg_root ) instance_dbimg_root->reset_clock_in( img );
}


int DBIMG::get_type_ext( const std::string& url )
{
    if( instance_dbimg_root ) return instance_dbimg_root->get_type_ext( url );
    return T_UNKNOWN;
}


int DBIMG::get_image_type( const unsigned char *sign )
{
    if( instance_dbimg_root ) return instance_dbimg_root->get_image_type( sign );
    return T_UNKNOWN;
}


int DBIMG::get_type_real( const std::string& url )
{
    int type = T_UNKNOWN;
    const DBIMG::Img* img = DBIMG::get_img( url );
    if( img ) type = img->get_type();

    return type;
}


DBIMG::Img* DBIMG::get_img( const std::string& url )
{
    if( instance_dbimg_root ) return instance_dbimg_root->get_img( url );
    return nullptr;
}


std::string DBIMG::get_cache_path( const std::string& url )
{
    const DBIMG::Img* img = DBIMG::get_img( url );
    if( img ) return img->get_cache_path();

    return std::string();
}

std::string DBIMG::get_img_abone_reason( const std::string& url )
{
    const DBIMG::Img* img = DBIMG::get_img( url );
    if( img ) return img->get_abone_reason();

    return std::string{};
}


std::optional<DBIMG::DHash> DBIMG::get_dhash( const std::string& url )
{
    const DBIMG::Img* img = DBIMG::get_img( url );
    if( img ) return img->get_dhash();

    return std::nullopt;
}

JDLIB::span<const DBIMG::AboneImgHash> DBIMG::get_span_abone_imghash()
{
    if( ! instance_dbimg_root ) return JDLIB::span<const DBIMG::AboneImgHash>{};

    return instance_dbimg_root->get_vec_abone_imghash();
}

void DBIMG::push_abone_imghash( const std::string& url, const int threshold )
{
    if( ! instance_dbimg_root ) return;

    instance_dbimg_root->push_abone_imghash( url, threshold );
}

bool DBIMG::test_imghash( DBIMG::Img& img )
{
    if( ! instance_dbimg_root ) return false;

    return instance_dbimg_root->test_imghash( img );
}

void DBIMG::save_abone_imghash_list()
{
    if( ! instance_dbimg_root ) return;

    instance_dbimg_root->save_abone_imghash_list();
}

void DBIMG::update_abone_imghash_list( JDLIB::span<DBIMG::AboneImgHash> span )
{
    if( ! instance_dbimg_root ) return;

    instance_dbimg_root->update_abone_imghash_list( span );
}

DBIMG::DHash DBIMG::calc_dhash_from_pixbuf( const Gdk::Pixbuf& pixbuf )
{
    return DBIMG::ImgRoot::calc_dhash_from_pixbuf( pixbuf );
}


void DBIMG::download_img( const std::string& url, const std::string& refurl, const int mosaic_mode )
{
    DBIMG::Img* img = DBIMG::get_img( url );
    if( img ) img->download_img( refurl, mosaic_mode, 0 );
}

void DBIMG::download_img_wait( const std::string& url, const std::string& refurl, const bool mosaic, const int first )
{
    DBIMG::Img* img = DBIMG::get_img( url );
    if( img && instance_dbimg_root ){

        int wait = 0;
        if( ! first || instance_dbimg_root->get_wait_size() ) wait = ( instance_dbimg_root->get_wait_size() + 1 ) * WAITLOADIMG_SEC;

        img->download_img( refurl, mosaic, wait  );
    }
}


void DBIMG::stop_load( const std::string& url )
{
    DBIMG::Img* img = DBIMG::get_img( url );
    if( img ) img->stop_load();
}



bool DBIMG::save( const std::string& url, Gtk::Window* parent, const std::string& path_to )
{
    DBIMG::Img* img = DBIMG::get_img( url );
    if( img ) return img->save( parent, path_to );

    return true;
}


void DBIMG::delete_cache( const std::string& url )
{
    if( instance_dbimg_root ) instance_dbimg_root->delete_cache( url );
}


void DBIMG::delete_all_files()
{
    if( instance_dbimg_root ) instance_dbimg_root->delete_all_files();
}    


int DBIMG::get_width( const std::string& url )
{
    const DBIMG::Img* img = DBIMG::get_img( url );
    if( img ) return img->get_width();
    return 0;
}


int DBIMG::get_height( const std::string& url )
{
    const DBIMG::Img* img = DBIMG::get_img( url );
    if( img ) return img->get_height();
    return 0;
}


bool DBIMG::is_cached( const std::string& url )
{
    const DBIMG::Img* img = DBIMG::get_img( url );
    if( img ) return img->is_cached();
    return false;
}


bool DBIMG::get_abone( const std::string& url )
{
    const DBIMG::Img* img = DBIMG::get_img( url );
    if( img ) return img->get_abone();
    return false;
}


void DBIMG::set_abone( const std::string& url, bool abone )
{
    DBIMG::Img* img = DBIMG::get_img( url );
    if( img ) return img->set_abone( abone );
}


bool DBIMG::is_loading( const std::string& url )
{
    DBIMG::Img* img = DBIMG::get_img( url );
    if( img ) return img->is_loading();

    return false;
}

bool DBIMG::is_wait( const std::string& url )
{
    const DBIMG::Img* img = DBIMG::get_img( url );
    if( img ) return img->is_wait();

    return false;
}

int DBIMG::get_code( const std::string& url )
{
    DBIMG::Img* img = DBIMG::get_img( url );
    if( img ) return img->get_code();
    return 0;
}


std::string DBIMG::get_str_code( const std::string& url )
{
    DBIMG::Img* img = DBIMG::get_img( url );
    if( img ) return img->get_str_code();
    return std::string();
}


bool DBIMG::get_mosaic( const std::string& url )
{
    const DBIMG::Img* img = DBIMG::get_img( url );
    if( img ) return img->get_mosaic();
    return true;
}


void DBIMG::set_mosaic( const std::string& url, bool mosaic )
{
    DBIMG::Img* img = DBIMG::get_img( url );
    if( img ) img->set_mosaic( mosaic );
}


void DBIMG::show_large_img( const std::string& url )
{
    DBIMG::Img* img = DBIMG::get_img( url );
    if( img ) img->show_large_img();
}


bool DBIMG::is_zoom_to_fit( const std::string& url )
{
    const DBIMG::Img* img = DBIMG::get_img( url );
    if( img ) return img->is_zoom_to_fit();
    return true;
}


void DBIMG::set_zoom_to_fit( const std::string& url, bool fit )
{
    DBIMG::Img* img = DBIMG::get_img( url );
    if( img ) img->set_zoom_to_fit( fit );
}


int DBIMG::get_size( const std::string& url )
{
    const DBIMG::Img* img = DBIMG::get_img( url );
    if( img ) return img->get_size();
    return 100;
}


void DBIMG::set_size( const std::string& url, int size )
{
    DBIMG::Img* img = DBIMG::get_img( url );
    if( img ) img->set_size( size );
}


std::string DBIMG::get_refurl( const std::string& url )
{
    const DBIMG::Img* img = DBIMG::get_img( url );
    if( img ) return img->get_refurl();
    return std::string();
}


size_t DBIMG::byte( const std::string& url )
{
    DBIMG::Img* img = DBIMG::get_img( url );
    if( img ) return img->current_length();
    return 0;
}

size_t DBIMG::get_filesize( const std::string& url )
{
    DBIMG::Img* img = DBIMG::get_img( url );
    if( img ) return img->total_length();
    return 0;
}


bool DBIMG::is_protected( const std::string& url )
{
    const DBIMG::Img* img = DBIMG::get_img( url );
    if( img ) return img->is_protected();
    return true;
}

bool DBIMG::is_fake( const std::string& url )
{
    const DBIMG::Img* img = DBIMG::get_img( url );
    if( img ) return img->is_fake();
    return false;
}


void DBIMG::set_protect( const std::string& url, bool protect )
{
    DBIMG::Img* img = DBIMG::get_img( url );
    if( img ) img->set_protect( protect );
}
