// ライセンス: GPL2

// サウンド再生クラス

#ifndef _PLAYSOUND_H
#define _PLAYSOUND_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef USE_ALSA

#include "skeleton/dispatchable.h"
#include "jdlib/jdthread.h"

#include <gtkmm.h>

#include <cstdint>


namespace SOUND
{
    // RIFF ( 8 byte )
    struct RIFFCHK
    {
        char id[ 4 ];       // = "RIFF"
        std::uint32_t size; // 全体サイズ
    };

    // WAVEfmt ( 28 byte )
    struct WAVEFMTCHK
    {
        char id[ 8 ];          // "WAVEfmt "
        std::uint32_t size;    // チャンクサイズ
        std::uint16_t fmt;     // 種類( PCMは1 )
        std::uint16_t chn;
        std::uint32_t rate;
        std::uint32_t average; // = rate * block ( byte )
        std::uint16_t block;   // = chn * bit / 8 ( byte )
        std::uint16_t bit;
    };

    // data ( 8 byte )
    struct DATACHK
    {
        char id[ 4 ];       // = "data"
        std::uint32_t size; // チャンクサイズ = PCMデータサイズ
    };


    class Play_Sound : public SKELETON::Dispatchable
    {
        JDLIB::Thread m_thread;
        std::string m_wavfile;
        bool m_stop{};
        bool m_playing{};

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
