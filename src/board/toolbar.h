// ライセンス: GPL2

// ツールバーのクラス

#ifndef _BOARD_TOOLBAR_H
#define _BOARD_TOOLBAR_H

#include <gtkmm.h>

#include "skeleton/imgbutton.h"
#include "skeleton/entry.h"

#include "icons/iconmanager.h"

#include "controlutil.h"
#include "controlid.h"

namespace BOARD
{
    class BoardToolBar : public Gtk::VBox
    {

        friend class BoardView;

        Gtk::ScrolledWindow m_scrwin;
        Gtk::HBox m_buttonbar;
        bool m_toolbar_shown;
        SKELETON::JDEntry m_entry_search;
        SKELETON::ImgButton m_button_close;
        SKELETON::ImgButton m_button_reload;
        SKELETON::ImgButton m_button_delete;
        SKELETON::ImgButton m_button_stop;
        SKELETON::ImgButton m_button_favorite;
        SKELETON::ImgButton m_button_up_search;
        SKELETON::ImgButton m_button_down_search;
        SKELETON::ImgButton m_button_search_cache;
        SKELETON::ImgButton m_button_new_article;
        SKELETON::ImgButton m_button_preferences;

        Gtk::Tooltips m_tooltip;

        // ツールバーを表示
        void show_toolbar()
        {
            if( ! m_toolbar_shown ){
                pack_start( m_scrwin, Gtk::PACK_SHRINK );
                show_all_children();
                m_toolbar_shown = true;
            }
        }

        // ツールバーを隠す
        void hide_toolbar()
        {
            if( m_toolbar_shown ){
                remove( m_scrwin );
                show_all_children();
                m_toolbar_shown = false;
            }
        }


        BoardToolBar( bool show_bar ) :
        m_toolbar_shown( false ),
        m_button_close( Gtk::Stock::CLOSE ),
        m_button_reload( Gtk::Stock::REFRESH ),
        m_button_delete( Gtk::Stock::DELETE ),
        m_button_stop( Gtk::Stock::STOP ),
        m_button_favorite( Gtk::Stock::COPY ),
        m_button_up_search( Gtk::Stock::GO_UP ),
        m_button_down_search( Gtk::Stock::GO_DOWN ),
        m_button_search_cache( Gtk::Stock::FIND ),
        m_button_new_article( ICON::WRITE ),
        m_button_preferences( Gtk::Stock::PREFERENCES )
        {
            m_tooltip.set_tip( m_button_close, CONTROL::get_label_motion( CONTROL::Quit ) );
            m_tooltip.set_tip( m_button_reload, CONTROL::get_label_motion( CONTROL::Reload ) );
            m_tooltip.set_tip( m_button_delete, CONTROL::get_label_motion( CONTROL::Delete ) );
            m_tooltip.set_tip( m_button_stop, CONTROL::get_label_motion( CONTROL::StopLoading ) );
            m_tooltip.set_tip( m_button_favorite, CONTROL::get_label_motion( CONTROL::AppendFavorite )
                + "\n\nまたは板のタブか選択したスレをお気に入りに直接Ｄ＆Ｄする" );
            m_tooltip.set_tip( m_button_up_search, CONTROL::get_label_motion( CONTROL::SearchPrev ) );
            m_tooltip.set_tip( m_button_down_search, CONTROL::get_label_motion( CONTROL::SearchNext ) );
            m_tooltip.set_tip( m_button_search_cache, CONTROL::get_label_motion( CONTROL::SearchCache ) );
            m_tooltip.set_tip( m_button_new_article, CONTROL::get_label_motion( CONTROL::NewArticle ) );
            m_tooltip.set_tip( m_button_preferences, CONTROL::get_label_motion( CONTROL::Property )  );
        
            m_buttonbar.pack_start( m_button_new_article, Gtk::PACK_SHRINK );
            m_buttonbar.pack_start( m_entry_search );
            m_buttonbar.pack_end( m_button_close, Gtk::PACK_SHRINK );    
            m_buttonbar.pack_end( m_button_delete, Gtk::PACK_SHRINK );
            m_buttonbar.pack_end( m_button_preferences, Gtk::PACK_SHRINK );
            m_buttonbar.pack_end( m_button_favorite, Gtk::PACK_SHRINK );
            m_buttonbar.pack_end( m_button_stop, Gtk::PACK_SHRINK );
            m_buttonbar.pack_end( m_button_reload, Gtk::PACK_SHRINK );
            m_buttonbar.pack_end( m_button_search_cache, Gtk::PACK_SHRINK );
            m_buttonbar.pack_end( m_button_up_search, Gtk::PACK_SHRINK );
            m_buttonbar.pack_end( m_button_down_search, Gtk::PACK_SHRINK );

            m_entry_search.add_mode( CONTROL::MODE_BOARD );

            m_buttonbar.set_border_width( 1 );
            m_scrwin.add( m_buttonbar );
            m_scrwin.set_policy( Gtk::POLICY_NEVER, Gtk::POLICY_NEVER );

            set_size_request( 8 );
            if( show_bar ) show_toolbar();
        }

    };
}


#endif
