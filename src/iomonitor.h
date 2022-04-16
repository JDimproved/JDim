// License: GPL2
//
// FIFOを使ったプロセス間通信を行うクラス
//

#ifndef _IOMONITOR_H
#define _IOMONITOR_H

#include <glibmm.h>


namespace CORE
{
    enum
    {
        FIFO_OK = 0,
        FIFO_OPEN_ERROR,
        FIFO_CREATE_ERROR
    };

    class IOMonitor
    {
        // FIFOのファイルディスクリプタ
        int m_fifo_fd;
        // I/Oの架け橋
        Glib::RefPtr< Glib::IOChannel > m_iochannel;

        // FIFOファイル名
        std::string m_fifo_file;

        // FIFOの状態
        int m_fifo_stat;

        // メインプロセスか否か
        bool m_main_process{};

      private:

        // FIFOを削除する
        void delete_fifo();

        // FIFOに書き込まれたら呼び出される
        bool slot_ioin( Glib::IOCondition io_condition );

      public:

        IOMonitor();
        ~IOMonitor();

        /// FIFOの初期化
        void init_connection();

        // FIFOの状態を取得
        int get_fifo_stat() const noexcept { return m_fifo_stat; }

        // メインプロセスか否かを取得
        bool is_main_process() const noexcept { return m_main_process; }

        // FIFOに書き込み
        bool send_command( const std::string& command );
    };
}
#endif
