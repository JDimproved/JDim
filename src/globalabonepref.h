// ライセンス: GPL2

// 全体あぼーん設定ダイアログ

#ifndef _GLOBALABONPREF_H
#define _GLOBALABONPREF_H

#include "skeleton/prefdiag.h"
#include "skeleton/editview.h"

#include "config/globalconf.h"

#include "dbtree/interface.h"

#include "command.h"

namespace CORE
{
    class GlobalAbonePref : public SKELETON::PrefDiag
    {
        Gtk::Notebook m_notebook;
        SKELETON::EditView m_edit_name, m_edit_word, m_edit_regex;
        Gtk::Label m_label_warning;

        // OK押した
        void slot_ok_clicked() override
        {

            // 全体あぼーん再設定
            std::list< std::string > list_name = MISC::get_lines( m_edit_name.get_text() );
            std::list< std::string > list_word = MISC::get_lines( m_edit_word.get_text() );
            std::list< std::string > list_regex = MISC::get_lines( m_edit_regex.get_text() );

            CONFIG::set_list_abone_name( list_name );
            CONFIG::set_list_abone_word( list_word );
            CONFIG::set_list_abone_regex( list_regex );

            // あぼーん情報更新
            DBTREE::update_abone_all_article();
            CORE::core_set_command( "relayout_all_article" );
        }

      public:

        GlobalAbonePref( Gtk::Window* parent, const std::string& url )
        : SKELETON::PrefDiag( parent, url )
        {
            std::string str_name, str_word, str_regex;
            std::list< std::string >::iterator it;

            // name
            std::list< std::string > list_name = CONFIG::get_list_abone_name();
            for( it = list_name.begin(); it != list_name.end(); ++it ) if( ! ( *it ).empty() ) str_name += ( *it ) + "\n";
            m_edit_name.set_text( str_name );

            // word
            std::list< std::string > list_word = CONFIG::get_list_abone_word();
            for( it = list_word.begin(); it != list_word.end(); ++it ) if( ! ( *it ).empty() ) str_word += ( *it ) + "\n";
            m_edit_word.set_text( str_word );

            // regex
            std::list< std::string > list_regex = CONFIG::get_list_abone_regex();
            for( it = list_regex.begin(); it != list_regex.end(); ++it ) if( ! ( *it ).empty() ) str_regex += ( *it ) + "\n";
            m_edit_regex.set_text( str_regex );

            m_label_warning.set_text( "ここでのあぼーん設定は全板の全スレに適用されます。\n\n設定のし過ぎは全板の全スレの表示速度を低下させます。\n\n指定のし過ぎに気を付けてください。" );

            m_notebook.append_page( m_label_warning, "注意" );
            m_notebook.append_page( m_edit_name, "NG 名前" );
            m_notebook.append_page( m_edit_word, "NG ワード" );
            m_notebook.append_page( m_edit_regex, "NG 正規表現" );

            get_vbox()->pack_start( m_notebook );
            set_title( "全体あぼ〜ん設定" );
            resize( 600, 400 );
            show_all_children();
        }

        ~GlobalAbonePref() noexcept {}
    };

}

#endif
