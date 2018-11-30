// ライセンス: GPL2

//
// 記事投稿クラス
//

#ifndef _POST_H
#define _POST_H

#include "skeleton/loadable.h"

#include <gtkmm.h>
#include <string>

namespace SKELETON
{
    class MsgDiag;
}


namespace MESSAGE
{
    class Post : public SKELETON::Loadable
    {
        // ポスト終了シグナル
        typedef sigc::signal< void > SIG_FIN;
        SIG_FIN m_sig_fin;

        // 親widget
        Gtk::Widget* m_parent;

        std::string m_url;
        std::string m_msg;
        std::string m_return_html;
        std::string m_errmsg;
        char* m_rawdata;
        size_t m_lng_rawdata;

        int m_count; // 書き込み確認時の永久ループ防止用
        bool m_subbbs; // true なら subbbs.cgiにpostする
        bool m_new_article; // 新スレ作成

        // 書き込んでいますのダイアログ
        SKELETON::MsgDiag* m_writingdiag;

      public:

        Post( Gtk::Widget* parent, const std::string& url, const std::string& msg, bool new_article );
        ~Post();
        SIG_FIN sig_fin() const { return m_sig_fin; }
        const std::string& get_return_html() const { return m_return_html;}
        const std::string& get_errmsg() const { return m_errmsg; }

        void post_msg();

        // 書き込み中ダイアログ表示
        void show_writingdiag( const bool show_buttons );

      private:

        void clear();
        void emit_sigfin();
        void slot_response( int id );

        void receive_data( const char* data, size_t size ) override;
        void receive_finish() override;
    };
    
}

#endif
