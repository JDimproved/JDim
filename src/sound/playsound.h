// ライセンス: GPL2

// サウンド再生クラス

#ifndef _PLAYSOUND_H
#define _PLAYSOUND_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef USE_ALSA

#include <gtkmm.h>

#include "skeleton/dispatchable.h"
#include "jdlib/jdthread.h"

namespace SOUND
{
    // RIFF ( 8 byte )
    struct RIFFCHK
    {
        char id[ 4 ];       // = "RIFF"
        unsigned int size;    // 全体サイズ
    };

    // WAVEfmt ( 28 byte )
    struct WAVEFMTCHK
    {
        char id[ 8 ];          // "WAVEfmt "
        unsigned int size;      // チャンクサイズ
        unsigned short fmt;    // 種類( PCMは1 )
        unsigned short chn;
        unsigned int rate;
        unsigned int average;  // = rate * block ( byte )
        unsigned short block;  // = chn * bit / 8 ( byte )
        unsigned short bit;
    };

    // data ( 8 byte )
    struct DATACHK
    {
        char id[ 4 ];      // = "data"
        unsigned int size; // チャンクサイズ = PCMデータサイズ
    };


    class Play_Sound : public SKELETON::Dispatchable
    {
        JDLIB::Thread m_thread;
        std::string m_wavfile;
        bool m_stop;
        bool m_playing;

      public:

        Play_Sound();
        ~Play_Sound();

        bool is_playing() const { return m_playing; }

        void play( const std::string& wavfile );
        void stop();

      private:

        void wait();
        static void* launcher( void* );
        void play_wavfile();
        void callback_dispatch() override;

    };

}

#endif

#endif
