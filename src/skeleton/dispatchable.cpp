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

    // 継承クラスのデストラクタ内でdispatchすると落ちるので
    // dispatch不可にする
    set_dispatchable( false );
}


void Dispatchable::dispatch( Dispatchable* dest )
{
    if( m_dispatchable ) CORE::get_dispmanager()->add( dest );   
}


void Dispatchable::cancel_dispatch( Dispatchable* dest )
{
    if( m_dispatchable ) CORE::get_dispmanager()->remove( dest );
}


void Dispatchable::dispatch()
{
    dispatch( this );
}


void Dispatchable::cancel_dispatch()
{
    cancel_dispatch( this );
}
