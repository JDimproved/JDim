// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "dispatchable.h"

#include "dispatchmanager.h"

using namespace SKELETON;


Dispatchable::Dispatchable()
{}


Dispatchable::~Dispatchable()
{
#ifdef _DEBUG
    std::cout << "Dispatchable::~Dispatchable\n";
#endif

    cancel_dispatch();
}


void Dispatchable::dispatch( Dispatchable* dest )
{
    CORE::get_dispmanager()->add( dest );   
}


void Dispatchable::cancel_dispatch( Dispatchable* dest )
{
    CORE::get_dispmanager()->remove( dest );
}


void Dispatchable::dispatch()
{
    dispatch( this );
}


void Dispatchable::cancel_dispatch()
{
    cancel_dispatch( this );
}
