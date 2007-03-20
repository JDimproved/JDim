// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "dispatchmanager.h"

#include "skeleton/dispatchable.h"


CORE::DispatchManager* instance_dispmanager = NULL;

CORE::DispatchManager* CORE::get_dispmanager()
{
    if( ! instance_dispmanager ) instance_dispmanager = new CORE::DispatchManager();
    return instance_dispmanager;
}

void CORE::delete_dispatchmanager()
{
    if( instance_dispmanager ) delete instance_dispmanager;
    instance_dispmanager = NULL;
}


//////////////////////


using namespace CORE;


DispatchManager::DispatchManager()
{
    m_dispatch.connect( sigc::mem_fun( *this, &DispatchManager::slot_dispatch ) );
}


DispatchManager::~DispatchManager()
{
#ifdef _DEBUG
    std::cout << "DispatchManager::~DispatchManager size = " << m_children.size() << std::endl;
#endif
}


void DispatchManager::add( SKELETON::Dispatchable* child )
{
    std::list< SKELETON::Dispatchable* >::iterator it = m_children.begin();
    for( ; it != m_children.end(); ++it ){
        if( *it == child ) return;
    }

    m_children.push_back( child );
    m_dispatch.emit();

#ifdef _DEBUG
    std::cout << "DispatchManager::add size = " << m_children.size() << std::endl;
#endif
}


void DispatchManager::remove( SKELETON::Dispatchable* child )
{
    size_t size = m_children.size();
    if( ! size  ) return;

    m_children.remove( child );

#ifdef _DEBUG
    if( size != m_children.size() ) std::cout << "!!!!!!!\nDispatchManager::remove size "
                                              << size << " -> " << m_children.size() << "\n!!!!!!!\n";
#endif
}


void DispatchManager::slot_dispatch()
{
    // リストに登録されている Dispatchable の callback_dispatch()をまとめて実行
    std::list< SKELETON::Dispatchable* > children = m_children;
    m_children.clear();

    while( children.size() ){
        SKELETON::Dispatchable* child = *( children.begin() );
        child->callback_dispatch();
        children.remove( child );
    }

#ifdef _DEBUG
    std::cout << "DispatchManager::slot_dispatch size = " << m_children.size() << std::endl;
#endif
}
