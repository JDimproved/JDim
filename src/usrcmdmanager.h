// ライセンス: GPL2

//
// ユーザーコマンドの管理クラス
//

#ifndef _USRCMDMANAGER_H
#define _USRCMDMANAGER_H

#include "xml/document.h"

#include <string>
#include <vector>

#define ROOT_NODE_NAME_USRCMD "usrcmdlist"

namespace CORE
{
    class Usrcmd_Manager
    {
        XML::Document m_document;

        int m_size;

        std::vector< std::string > m_list_cmd;

    public:

        Usrcmd_Manager();
        virtual ~Usrcmd_Manager() noexcept {}

        XML::Document& xml_document() { return m_document; }
        void analyze_xml();
        void save_xml();

        int get_size() const { return m_size; }

        // 実行
        void exec( const int comnum, // コマンド番号
                   const std::string& url,
                   const std::string& link,
                   const std::string& selection, // 選択文字
                   const int number // レス番号
            );

        void exec( const std::string command, // コマンド
                   const std::string& url,
                   const std::string& link,
                   const std::string& selection, // 選択文字
                   const int number // レス番号
            );


        // コマンド置換
        // cmdの$URLをurl, $LINKをlink, $TEXT*をtext, $NUMBERをnumberで置き換えて出力
        // text は UTF-8 であること
        std::string replace_cmd( const std::string& cmd,
                                 const std::string& url,
                                 const std::string& link,
                                 const std::string& text,
                                 const int number );

        // ユーザコマンドメニューの作成
        std::string create_usrcmd_menu( Glib::RefPtr< Gtk::ActionGroup >& action_group );
        std::string create_usrcmd_menu( Glib::RefPtr< Gtk::ActionGroup >& action_group,
                                        XML::Dom* dom, int& dirno, int& cmdno );

        Glib::RefPtr< Gtk::Action > get_action( Glib::RefPtr< Gtk::ActionGroup >& action_group, const int num );

        // 選択不可かどうか判断して visible や sensitive を切り替える
        void toggle_sensitive( Glib::RefPtr< Gtk::ActionGroup >& action_group,
                               const std::string& url_article,
                               const std::string& url_link,
                               const std::string& str_select );
      private:

        void txt2xml();

        bool show_replacetextdiag( std::string& texti, const std::string& title );
        void set_cmd( const std::string& cmd );

        bool is_sensitive( int num, const std::string& link, const std::string& selection );
        bool is_hide( int num, const std::string& url );
    };

    ///////////////////////////////////////
    // インターフェース

    Usrcmd_Manager* get_usrcmd_manager();
    void delete_usrcmd_manager();
}


#endif
