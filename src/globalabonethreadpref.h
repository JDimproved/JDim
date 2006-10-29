// ライセンス: 最新のGPL

// 全体スレあぼーん設定ダイアログ

#ifndef _GLOBALABONTHREADPREF_H
#define _GLOBALABONTHREADPREF_H

#include "skeleton/prefdiag.h"
#include "skeleton/editview.h"

#include "config/globalconf.h"

#include "dbtree/interface.h"

#include "command.h"

namespace CORE
{
    class GlobalAboneThreadPref : public SKELETON::PrefDiag
    {
        Gtk::Notebook m_notebook;
        SKELETON::EditView m_edit_word, m_edit_regex;
        Gtk::Label m_label_warning;

        // OK押した
        virtual void slot_ok_clicked(){

            // 全体あぼーん再設定
            std::list< std::string > list_word = MISC::get_lines( m_edit_word.get_text() );
            std::list< std::string > list_regex = MISC::get_lines( m_edit_regex.get_text() );

            CONFIG::set_list_abone_word_thread( list_word );
            CONFIG::set_list_abone_regex_thread( list_regex );

            // あぼーん情報更新
            DBTREE::update_abone_all_board();  // 板の再描画も行われる
        }

      public:

        GlobalAboneThreadPref( const std::string& url )
        : SKELETON::PrefDiag( url )
        {
            std::string str_name, str_word, str_regex;
            std::list< std::string >::iterator it;

            // word
            std::list< std::string > list_word = CONFIG::get_list_abone_word_thread();
            for( it = list_word.begin(); it != list_word.end(); ++it ) if( ! ( *it ).empty() ) str_word += ( *it ) + "\n";
            m_edit_word.set_text( str_word );

            // regex
            std::list< std::string > list_regex = CONFIG::get_list_abone_regex_thread();
            for( it = list_regex.begin(); it != list_regex.end(); ++it ) if( ! ( *it ).empty() ) str_regex += ( *it ) + "\n";
            m_edit_regex.set_text( str_regex );

            m_label_warning.set_text( "全体スレあぼ〜んはスレ一覧表示速度の低下を招きます。指定のし過ぎに気を付けてください。" );

            m_notebook.append_page( m_label_warning, "注意" );
            m_notebook.append_page( m_edit_word, "NG ワード" );
            m_notebook.append_page( m_edit_regex, "NG 正規表現" );

            get_vbox()->pack_start( m_notebook );
            set_title( "全体スレあぼ〜ん設定" );
            resize( 600, 400 );
            show_all_children();
        }

        virtual ~GlobalAboneThreadPref(){}
    };

}

#endif
