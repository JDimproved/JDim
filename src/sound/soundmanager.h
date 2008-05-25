// ライセンス: GPL2

// サウンド管理クラス

#ifndef _SOUNDMANAGER_H
#define _SOUNDMANAGER_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "playsound.h"

#include <vector>

namespace SOUND
{

    enum{
        SOUND_RES = 0,
        SOUND_NO,
        SOUND_NEW,
        SOUND_ERR,

        NUM_SOUNDS
    };


#ifdef USE_ALSA

    class SOUND_Manager
    {
        Play_Sound m_playsound;

      public:

        SOUND_Manager();
        virtual ~SOUND_Manager();

        void play( const int sound );
    };

#else

    class SOUND_Manager
    {
      public:

        SOUND_Manager(){}
        virtual ~SOUND_Manager(){}
    };

#endif

    ///////////////////////////////////////
    // インターフェース

    SOUND_Manager* get_sound_manager();
    void delete_sound_manager();
    void play( const int sound );
}

#endif
