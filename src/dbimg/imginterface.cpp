// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "imginterface.h"
#include "imgroot.h"
#include "img.h"

#include "cache.h"

// インスタンスは Core でひとつだけ作って、Coreのデストラクタでdeleteする
DBIMG::ImgRoot *instance_dbimg_root = NULL;


void DBIMG::create_root()
{
    if( ! instance_dbimg_root ) instance_dbimg_root = new DBIMG::ImgRoot();
}


void DBIMG::delete_root()
{
    if( instance_dbimg_root ) delete instance_dbimg_root;
}


bool DBIMG::is_loadable( const std::string& url )
{
    if( instance_dbimg_root ) return instance_dbimg_root->is_loadable( url );

    return false;
}


bool DBIMG::is_loadable( const char* url, int n )
{
    if( instance_dbimg_root ) return instance_dbimg_root->is_loadable( url, n );

    return false;
}



DBIMG::Img* DBIMG::get_img( const std::string& url )
{
    if( instance_dbimg_root ) return instance_dbimg_root->get_img( url );
    return NULL;
}


void DBIMG::download_img( const std::string& url )
{
    DBIMG::Img* img = DBIMG::get_img( url );
    if( img ) img->download_img();
}


void DBIMG::stop_load( const std::string& url )
{
    DBIMG::Img* img = DBIMG::get_img( url );
    if( img ) img->stop_load();
}



bool DBIMG::save( const std::string& url, const std::string& path_to )
{
    DBIMG::Img* img = DBIMG::get_img( url );
    if( img ) return img->save( path_to );

    return true;
}


void DBIMG::delete_cache( const std::string& url )
{
    if( instance_dbimg_root ) instance_dbimg_root->delete_cache( url, true );
}


void DBIMG::delete_all_files()
{
    if( instance_dbimg_root ) instance_dbimg_root->delete_all_files();
}    


bool DBIMG::is_cached( const std::string& url )
{
    DBIMG::Img* img = DBIMG::get_img( url );
    if( img ) return img->is_cached();
    return false;
}


bool DBIMG::is_loading( const std::string& url )
{
    DBIMG::Img* img = DBIMG::get_img( url );
    if( img ) return img->is_loading();

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
    DBIMG::Img* img = DBIMG::get_img( url );
    if( img ) return img->get_mosaic();
    return true;
}


void DBIMG::set_mosaic( const std::string& url, bool mosaic )
{
    DBIMG::Img* img = DBIMG::get_img( url );
    if( img ) img->set_mosaic( mosaic );
}


bool DBIMG::is_zoom_to_fit( const std::string& url )
{
    DBIMG::Img* img = DBIMG::get_img( url );
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
    DBIMG::Img* img = DBIMG::get_img( url );
    if( img ) return img->get_size();
    return 100;
}


void DBIMG::set_size( const std::string& url, int size )
{
    DBIMG::Img* img = DBIMG::get_img( url );
    if( img ) img->set_size( size );
}


std::string DBIMG::refurl( const std::string& url )
{
    DBIMG::Img* img = DBIMG::get_img( url );
    if( img ) return img->refurl();
    return std::string();
}


void DBIMG::set_refurl( const std::string& url, const std::string& refurl )
{
    DBIMG::Img* img = DBIMG::get_img( url );
    if( img ) img->set_refurl( refurl );
}



size_t DBIMG::byte( const std::string& url )
{
    DBIMG::Img* img = DBIMG::get_img( url );
    if( img ) return img->current_length();
    return 0;
}


bool DBIMG::is_protected( const std::string& url )
{
    DBIMG::Img* img = DBIMG::get_img( url );
    if( img ) return img->is_protected();
    return true;
}


void DBIMG::set_protect( const std::string& url, bool protect )
{
    DBIMG::Img* img = DBIMG::get_img( url );
    if( img ) img->set_protect( protect );
}
