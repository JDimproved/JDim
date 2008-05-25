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

    for( int i = 0; i < NUM_SOUNDS; ++i ){

        std::string path_sound = get_file( i );
        bool exist = false;

        if( CACHE::file_exists( path_sound ) == CACHE::EXIST_FILE ) exist = true;

#ifdef _DEBUG
        std::cout << path_sound << " " << exist << std::endl;
#endif        

        m_playable.push_back( exist );
    }
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
    if( ! m_playable[ sound ] ) return;

#ifdef _DEBUG
    std::cout << "SOUND_Manager::play sound = " << sound << std::endl;
#endif

    std::string path_sound = get_file( sound );
    if( ! path_sound.empty() ) m_playsound.play( path_sound );
}


std::string SOUND_Manager::get_file( const int sound )
{
    std::string path_sound = CACHE::path_sound_root();

    switch( sound ){

        case SOUND_RES: path_sound += "res.wav"; break;
        case SOUND_NO: path_sound += "no.wav"; break;
        case SOUND_NEW: path_sound += "new.wav"; break;
        case SOUND_ERR: path_sound += "err.wav"; break;

        default: return std::string();
    }

    return path_sound;
}



#endif
