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

    cancel_dispatch();
}


void Dispatchable::set_dispatchable( bool dispatchable )
{
    m_dispatchable = dispatchable;
    if( ! m_dispatchable ) cancel_dispatch();
}


void Dispatchable::dispatch()
{
    if( m_dispatchable ) CORE::get_dispmanager()->add( this );
}


void Dispatchable::cancel_dispatch()
{
    if( m_dispatchable ) CORE::get_dispmanager()->remove( this );
}
