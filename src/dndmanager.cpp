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


void CORE::DND_Begin()
{
    CORE::DND_Manager* manager = CORE::get_dnd_manager();
    if( manager ) manager->begin();
}


void CORE::DND_End()
{
    CORE::DND_Manager* manager = CORE::get_dnd_manager();
    if( manager ) manager->end();
}


bool CORE::DND_Now_dnd()
{
    CORE::DND_Manager* manager = CORE::get_dnd_manager();
    if( manager ) return manager->now_dnd();

    return false;
}
