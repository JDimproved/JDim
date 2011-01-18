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


void Dispatchable::set_dispatchable( const bool dispatchable )
{
    m_dispatchable = dispatchable;
    if( ! m_dispatchable ) CORE::get_dispmanager()->remove( this );
}


void Dispatchable::dispatch()
{
    if( m_dispatchable ) CORE::get_dispmanager()->add( this );
}
