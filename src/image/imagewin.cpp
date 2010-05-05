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
    : SKELETON::JDWindow( CONFIG::get_fold_image() ),
      m_tab( NULL )
{
#ifdef _DEBUG
    std::cout << "ImageWin::ImageWin x y w h = "
              << get_x_win() << " " << get_y_win()
              << " " << get_width_win() << " " << get_height_win() << std::endl;
#endif

    init_win();
    pack_remove_end( false, get_statbar(), Gtk::PACK_SHRINK );

    if( ! CONFIG::get_fold_image() ) set_transient_for( *CORE::get_mainwindow() );

    show_all_children();
}


ImageWin::~ImageWin()
{
#ifdef _DEBUG
    std::cout << "ImageWin::~ImageWin window size : x = " << get_x_win() << " y = " << get_y_win()
              << " w = " << get_width_win() << " h = " << get_height_win() << " max = " << is_maximized_win() << std::endl;
#endif

    set_shown_win( false );
    CORE::core_set_command( "restore_focus" );
}


const int ImageWin::get_x_win()
{
    return SESSION::get_x_win_img();
}

const int ImageWin::get_y_win()
{
    return SESSION::get_y_win_img();
}

void ImageWin::set_x_win( int x )
{
    SESSION::set_x_win_img( x );
}

void ImageWin::set_y_win( int y )
{
    SESSION::set_y_win_img( y );
}

const int ImageWin::get_width_win()
{
    return SESSION::get_width_win_img();
}

const int ImageWin::get_height_win()
{
    return SESSION::get_height_win_img();
}

void ImageWin::set_width_win( int width )
{
    SESSION::set_width_win_img( width );
}

void ImageWin::set_height_win( int height )
{
    SESSION::set_height_win_img( height );
}

const bool ImageWin::is_focus_win()
{
    return SESSION::is_focus_win_img();
}

void ImageWin::set_focus_win( bool set )
{
    SESSION::set_focus_win_img( set );
}


const bool ImageWin::is_maximized_win()
{
    return SESSION::is_maximized_win_img();
}

void ImageWin::set_maximized_win( bool set )
{
    SESSION::set_maximized_win_img( set );
}


const bool ImageWin::is_iconified_win()
{
    return SESSION::is_iconified_win_img();
}

void ImageWin::set_iconified_win( bool set )
{
    SESSION::set_iconified_win_img( set );
}


const bool ImageWin::is_shown_win()
{
    return SESSION::is_shown_win_img();
}


void ImageWin::set_shown_win( bool set )
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
