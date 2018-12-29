// ライセンス: GPL2

// 全体スレあぼーん設定ダイアログ

#ifndef _GLOBALABONTHREADPREF_H
#define _GLOBALABONTHREADPREF_H

#include "skeleton/prefdiag.h"
#include "skeleton/editview.h"
#include "skeleton/spinbutton.h"

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

        Gtk::VBox m_vbox_abone_thread;
        Gtk::Label m_label_abone_thread;

        Gtk::HBox m_hbox_number;
        Gtk::Label m_label_number;
        SKELETON::SpinButton m_spin_number;

        Gtk::HBox m_hbox_hour;
        Gtk::Label m_label_hour;
        SKELETON::SpinButton m_spin_hour;

        // OK押した
        void slot_ok_clicked() override
        {

            // 全体あぼーん再設定

            // スレ数、時間
            CONFIG::set_abone_number_thread( m_spin_number.get_value_as_int() );
            CONFIG::set_abone_hour_thread( m_spin_hour.get_value_as_int() );

            // word
            std::list< std::string > list_word = MISC::get_lines( m_edit_word.get_text() );

            // regex
            std::list< std::string > list_regex = MISC::get_lines( m_edit_regex.get_text() );

            CONFIG::set_list_abone_word_thread( list_word );
            CONFIG::set_list_abone_regex_thread( list_regex );

            // スレ一覧再描画
            DBTREE::update_abone_thread();
        }

      public:

        GlobalAboneThreadPref( Gtk::Window* parent, const std::string& url )
        : SKELETON::PrefDiag( parent, url )
        {
            std::string str_name, str_word, str_regex;
            std::list< std::string >::iterator it;

            // スレ数、時間
            m_label_abone_thread.set_text( "以下の数字が0の時は未設定になります。\nまたキャッシュにログがあるスレはあぼ〜んされません。\n\n" );

            m_label_number.set_text( "レス以上のスレをあぼ〜ん" );
            m_spin_number.set_range( 0, 9999 );
            m_spin_number.set_increments( 1, 1 );
            m_spin_number.set_value( CONFIG::get_abone_number_thread() );
            
            m_hbox_number.set_spacing( 4 );
            m_hbox_number.pack_start( m_spin_number, Gtk::PACK_SHRINK );
            m_hbox_number.pack_start( m_label_number, Gtk::PACK_SHRINK );

            set_activate_entry( m_spin_number );

            m_label_hour.set_text( "時間以上スレ立てから経過したスレをあぼ〜ん" );
            m_spin_hour.set_range( 0, 9999 );
            m_spin_hour.set_increments( 1, 1 );
            m_spin_hour.set_value( CONFIG::get_abone_hour_thread() );
            
            m_hbox_hour.set_spacing( 4 );
            m_hbox_hour.pack_start( m_spin_hour, Gtk::PACK_SHRINK );
            m_hbox_hour.pack_start( m_label_hour, Gtk::PACK_SHRINK );

            set_activate_entry( m_spin_hour );

            m_vbox_abone_thread.set_border_width( 16 );
            m_vbox_abone_thread.set_spacing( 8 );
            m_vbox_abone_thread.pack_start( m_label_abone_thread, Gtk::PACK_SHRINK );
            m_vbox_abone_thread.pack_start( m_hbox_number, Gtk::PACK_SHRINK );
            m_vbox_abone_thread.pack_start( m_hbox_hour, Gtk::PACK_SHRINK );

            // word
            std::list< std::string > list_word = CONFIG::get_list_abone_word_thread();
            for( it = list_word.begin(); it != list_word.end(); ++it ) if( ! ( *it ).empty() ) str_word += ( *it ) + "\n";
            m_edit_word.set_text( str_word );

            // regex
            std::list< std::string > list_regex = CONFIG::get_list_abone_regex_thread();
            for( it = list_regex.begin(); it != list_regex.end(); ++it ) if( ! ( *it ).empty() ) str_regex += ( *it ) + "\n";
            m_edit_regex.set_text( str_regex );

            m_label_warning.set_text( "ここでのあぼーん設定は全板のスレ一覧に適用されます。\n\n設定のし過ぎは全板の全スレ一覧表示速度を低下させます。\n\n指定のし過ぎに気を付けてください。" );

            m_notebook.append_page( m_label_warning, "注意" );
            m_notebook.append_page( m_vbox_abone_thread, "一般" );
            m_notebook.append_page( m_edit_word, "NG ワード" );
            m_notebook.append_page( m_edit_regex, "NG 正規表現" );

            get_vbox()->pack_start( m_notebook );
            set_title( "全体スレあぼ〜ん設定" );
            resize( 600, 400 );
            show_all_children();
        }

        ~GlobalAboneThreadPref() noexcept {}
    };

}

#endif
