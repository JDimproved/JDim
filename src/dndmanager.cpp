// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"
#include "dndmanager.h"


CORE::DND_Manager* instance_dnd_manager = NULL;


CORE::DND_Manager* CORE::get_dnd_manager()
{
    if( ! instance_dnd_manager ) instance_dnd_manager = new DND_Manager();
    assert( instance_dnd_manager );

    return instance_dnd_manager;
}


void CORE::delete_dnd_manager()
{
    if( instance_dnd_manager ) delete instance_dnd_manager;
    instance_dnd_manager = NULL;
}


void CORE::DND_Begin( const std::string& url_from )
{
    CORE::DND_Manager* manager = CORE::get_dnd_manager();
    if( manager ) manager->begin( url_from );
}


void CORE::DND_End()
{
    CORE::DND_Manager* manager = CORE::get_dnd_manager();
    if( manager ) manager->end();
}


const bool CORE::DND_Now_dnd()
{
    CORE::DND_Manager* manager = CORE::get_dnd_manager();
    if( manager ) return manager->now_dnd();

    return false;
}


const std::string CORE::DND_Url_from()
{
    CORE::DND_Manager* manager = CORE::get_dnd_manager();
    if( manager ) return manager->url_from();

    return std::string();
}


///////////////////////////////////////////////

using namespace CORE;


DND_Manager::DND_Manager()
    : m_dnd( 0 )
{}


DND_Manager::~DND_Manager()
{
#ifdef _DEBUG
    std::cout << "CORE::~DND_Manager\n";
#endif
}


// DnD 開始
void DND_Manager::begin( const std::string& url_from )
{
    if( m_dnd ) return;
    m_dnd = true;
    m_url_from = url_from;
    m_sig_dnd_begin.emit();
}


// DnD終了
void DND_Manager::end()
{
    if( !m_dnd ) return;
    m_dnd = false;
    m_url_from = std::string();
    m_sig_dnd_end.emit();
}
