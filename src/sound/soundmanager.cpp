// ライセンス: GPL2

#define _DEBUG
#include "jddebug.h"

#include "soundmanager.h"

#include "cache.h"

SOUND::SOUND_Manager* instance_sound_manager = NULL;

SOUND::SOUND_Manager* SOUND::get_sound_manager()
{
#ifdef USE_ALSA
    if( ! instance_sound_manager ) instance_sound_manager = new SOUND::SOUND_Manager();
    assert( instance_sound_manager );
#endif

    return instance_sound_manager;
}


void SOUND::delete_sound_manager()
{
#ifdef USE_ALSA
    if( instance_sound_manager ) delete instance_sound_manager;
    instance_sound_manager = NULL;
#endif
}


void SOUND::play( const int sound )
{
#ifdef USE_ALSA
    SOUND::get_sound_manager()->play( sound );
#endif
}


///////////////////////////////////////////////

#ifdef USE_ALSA

using namespace SOUND;


SOUND_Manager::SOUND_Manager()
{
#ifdef _DEBUG
    std::cout << "SOUND_Manager::SOUND_Manager\n";
#endif
}


SOUND_Manager::~SOUND_Manager()
{
#ifdef _DEBUG
    std::cout << "SOUND_Manager::~SOUND_Manager\n";
#endif

    m_playsound.stop();
}


void SOUND_Manager::play( const int sound )
{
#ifdef _DEBUG
    std::cout << "SOUND_Manager::play sound = " << sound << std::endl;
#endif

    std::string path_sound = CACHE::path_sound_root();

    switch( sound ){

        case SOUND_RES: path_sound += "res.wav"; break;
        case SOUND_NO: path_sound += "no.wav"; break;
        case SOUND_NEW: path_sound += "new.wav"; break;
        case SOUND_ERR: path_sound += "err.wav"; break;

        default: return;
    }

    m_playsound.play( path_sound );
}


#endif
