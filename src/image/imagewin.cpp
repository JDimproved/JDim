// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "imageadmin.h"
#include "imagewin.h"

#include "jdlib/miscgtk.h"

#include "config/globalconf.h"

#include "session.h"
#include "command.h"

using namespace IMAGE;


ImageWin::ImageWin()
    : SKELETON::JDWindow( CONFIG::get_fold_image() )
{
#ifdef _DEBUG
    std::cout << "ImageWin::ImageWin x y w h = "
              << ImageWin::get_x_win() << " " << ImageWin::get_y_win()
              << " " << ImageWin::get_width_win() << " " << ImageWin::get_height_win() << std::endl;
#endif

    init_win();
    pack_remove_end( false, get_statbar(), Gtk::PACK_SHRINK );

    if( ! CONFIG::get_fold_image() ) set_transient_for( *CORE::get_mainwindow() );

    show_all_children();
}


ImageWin::~ImageWin()
{
#ifdef _DEBUG
    std::cout << "ImageWin::~ImageWin window size : x = " << ImageWin::get_x_win() << " y = " << ImageWin::get_y_win()
              << " w = " << ImageWin::get_width_win() << " h = " << ImageWin::get_height_win()
              << " max = " << ImageWin::is_maximized_win() << std::endl;
#endif

    ImageWin::set_shown_win( false );
    CORE::core_set_command( "restore_focus" );
}


int ImageWin::get_x_win()
{
    return SESSION::get_x_win_img();
}

int ImageWin::get_y_win()
{
    return SESSION::get_y_win_img();
}

void ImageWin::set_x_win( const int x )
{
    SESSION::set_x_win_img( x );
}

void ImageWin::set_y_win( const int y )
{
    SESSION::set_y_win_img( y );
}

int ImageWin::get_width_win()
{
    return SESSION::get_width_win_img();
}

int ImageWin::get_height_win()
{
    return SESSION::get_height_win_img();
}

void ImageWin::set_width_win( const int width )
{
    SESSION::set_width_win_img( width );
}

void ImageWin::set_height_win( const int height )
{
    SESSION::set_height_win_img( height );
}

bool ImageWin::is_focus_win()
{
    return SESSION::is_focus_win_img();
}

void ImageWin::set_focus_win( const bool set )
{
    SESSION::set_focus_win_img( set );
}


bool ImageWin::is_maximized_win()
{
    return SESSION::is_maximized_win_img();
}

void ImageWin::set_maximized_win( const bool set )
{
    SESSION::set_maximized_win_img( set );
}


bool ImageWin::is_iconified_win()
{
    return SESSION::is_iconified_win_img();
}

void ImageWin::set_iconified_win( const bool set )
{
    SESSION::set_iconified_win_img( set );
}


bool ImageWin::is_shown_win()
{
    return SESSION::is_shown_win_img();
}


void ImageWin::set_shown_win( const bool set )
{
    SESSION::set_shown_win_img( set );
}

void ImageWin::switch_admin()
{
    CORE::core_set_command( "switch_image" );    
}


void ImageWin::pack_remove_tab( bool unpack, Widget& tab )
{
    m_tab = &tab;
    get_vbox().pack_remove_start( unpack, tab, Gtk::PACK_SHRINK );
}
