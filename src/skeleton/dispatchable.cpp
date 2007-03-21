// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "dispatchable.h"

#include "dispatchmanager.h"

using namespace SKELETON;


Dispatchable::Dispatchable()
    : m_dispatchable( true )
{}


Dispatchable::~Dispatchable()
{
#ifdef _DEBUG
    std::cout << "Dispatchable::~Dispatchable\n";
#endif

    set_dispatchable( false );
}


void Dispatchable::set_dispatchable( bool dispatchable )
{
    Glib::Mutex::Lock lock( m_mutex );
    m_dispatchable = dispatchable;
    if( ! m_dispatchable ) CORE::get_dispmanager()->remove( this );
}


void Dispatchable::dispatch()
{
    Glib::Mutex::Lock lock( m_mutex );
    if( m_dispatchable ) CORE::get_dispmanager()->add( this );
}
