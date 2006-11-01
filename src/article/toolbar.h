// ライセンス: 最新のGPL

// ツールバーのクラス
//
// ARTICLE::ArticleView* 以外では使わない
//

#ifndef _ARTICLE_TOOLBAR_H
#define _ARTICLE_TOOLBAR_H

#include <gtkmm.h>

#include "skeleton/imgbutton.h"
#include "skeleton/entry.h"

#include "controlutil.h"
#include "controlid.h"

namespace ARTICLE
{
    class ArticleToolBar : public Gtk::ScrolledWindow
    {
        friend class ArticleViewBase;
        friend class ArticleViewMain;
        friend class ArticleViewRes;
        friend class ArticleViewName;
        friend class ArticleViewID;
        friend class ArticleViewBM;
        friend class ArticleViewRefer;
        friend class ArticleViewURL;
        friend class ArticleViewDrawout;

        Gtk::Tooltips m_tooltip;

        Gtk::VBox m_vbox;

        // ラベル、ボタンバー
        Gtk::HBox m_buttonbar;
        Gtk::Entry m_label;

        Gtk::Button m_button_board;
        SKELETON::ImgButton m_button_favorite;
        SKELETON::ImgButton m_button_write;
        SKELETON::ImgButton m_button_close;
        SKELETON::ImgButton m_button_delete;
        SKELETON::ImgButton m_button_reload;
        SKELETON::ImgButton m_button_stop;
        SKELETON::ImgButton m_button_open_search;
        SKELETON::ImgButton m_button_preferences;

        // 検索バー
        Gtk::HBox m_searchbar;
        bool m_searchbar_shown;
        SKELETON::JDEntry m_entry_search;
        SKELETON::ImgButton m_button_close_search;
        SKELETON::ImgButton m_button_up_search;
        SKELETON::ImgButton m_button_down_search;
        SKELETON::ImgButton m_button_drawout_and;
        SKELETON::ImgButton m_button_drawout_or;
        SKELETON::ImgButton m_button_clear_hl;


        // 検索バー表示
        void show_searchbar()
        {
            if( ! m_searchbar_shown ){
                if( m_vbox.get_height() < 8 ) set_size_request( 8 ); // まだrealizeしていない
                else set_size_request( 8, m_vbox.get_height()*2 );
                m_vbox.pack_start( m_searchbar, Gtk::PACK_SHRINK );
                show_all_children();
                m_searchbar_shown = true;
            }
        }

        // 検索バーを消す
        void hide_searchbar()
        {
            if( m_searchbar_shown ){
                m_vbox.remove( m_searchbar );
                if( m_vbox.get_height() < 8 ) set_size_request( 8 ); // まだrealizeしていない
                else set_size_request( 8, m_vbox.get_height()/2 );
                show_all_children();
                m_searchbar_shown = false;
            }
        }


        void set_label( const std::string& label )
        {
            m_label.set_text( label );
            m_tooltip.set_tip( m_label, label );
        }

        // vboxがrealizeしたらラベル(Gtk::Entry)の背景色を変える
        void slot_vbox_realize()
        {
            Gdk::Color color_bg = m_vbox.get_style()->get_bg( Gtk::STATE_NORMAL );
            m_label.modify_base( Gtk::STATE_NORMAL, color_bg );

            color_bg = m_vbox.get_style()->get_bg( Gtk::STATE_ACTIVE );
            m_label.modify_base( Gtk::STATE_ACTIVE, color_bg );
        }

        ArticleToolBar() :

        m_button_favorite( Gtk::Stock::COPY ),
        m_button_write( Gtk::Stock::NEW ),
        m_button_close( Gtk::Stock::CLOSE ),
        m_button_delete( Gtk::Stock::DELETE ),
        m_button_reload( Gtk::Stock::REFRESH ),
        m_button_stop( Gtk::Stock::STOP ),
        m_button_open_search( Gtk::Stock::FIND ),
        m_button_preferences( Gtk::Stock::PREFERENCES ),

        m_searchbar_shown( 0 ),
        m_button_close_search( Gtk::Stock::UNDO ),
        m_button_up_search( Gtk::Stock::GO_UP ),
        m_button_down_search( Gtk::Stock::GO_DOWN ),
        m_button_drawout_and( Gtk::Stock::CUT ),
        m_button_drawout_or( Gtk::Stock::ADD ),
        m_button_clear_hl( Gtk::Stock::CLEAR )
        {
            m_vbox.signal_realize().connect( sigc::mem_fun(*this, &ArticleToolBar::slot_vbox_realize ) );

            // スレ名ラベル
            // Gtk::Label を使うと勝手にリサイズするときがあるので
            // 面倒でも　Gtk::Entry を使う。背景色は on_realize() で指定する。
            m_label.set_editable( false );
            m_label.set_activates_default( false );
            m_label.set_has_frame( false );

            m_button_board.set_focus_on_click( false );
            m_button_board.set_relief( Gtk::RELIEF_NONE );

            m_tooltip.set_tip( m_button_board, CONTROL::get_label_motion( CONTROL::OpenParentBoard ) );
            m_tooltip.set_tip( m_button_write, CONTROL::get_label_motion( CONTROL::WriteMessage ) );
            m_tooltip.set_tip( m_button_close, CONTROL::get_label_motion( CONTROL::Quit ) );
            m_tooltip.set_tip( m_button_reload, CONTROL::get_label_motion( CONTROL::Reload ) );
            m_tooltip.set_tip( m_button_delete, CONTROL::get_label_motion( CONTROL::Delete ) );
            m_tooltip.set_tip( m_button_favorite, CONTROL::get_label_motion( CONTROL::AppendFavorite ) );
            m_tooltip.set_tip( m_button_stop, CONTROL::get_label_motion( CONTROL::StopLoading ) );
            m_tooltip.set_tip( m_button_preferences, CONTROL::get_label_motion( CONTROL::Property ) );
            m_tooltip.set_tip( m_button_open_search, CONTROL::get_label_motion( CONTROL::Search ) );

            // ボタンとかラベルのバー
            m_buttonbar.pack_start( m_button_board, Gtk::PACK_SHRINK );
            m_buttonbar.pack_start( m_label, Gtk::PACK_EXPAND_WIDGET, 2 );
            m_buttonbar.pack_end( m_button_close, Gtk::PACK_SHRINK );
            m_buttonbar.pack_end( m_button_delete, Gtk::PACK_SHRINK );
            m_buttonbar.pack_end( m_button_preferences, Gtk::PACK_SHRINK );
            m_buttonbar.pack_end( m_button_favorite, Gtk::PACK_SHRINK );
            m_buttonbar.pack_end( m_button_write, Gtk::PACK_SHRINK );
            m_buttonbar.pack_end( m_button_stop, Gtk::PACK_SHRINK );
            m_buttonbar.pack_end( m_button_reload, Gtk::PACK_SHRINK );
            m_buttonbar.pack_end( m_button_open_search, Gtk::PACK_SHRINK );

            m_buttonbar.set_border_width( 1 );
            m_vbox.pack_start( m_buttonbar, Gtk::PACK_SHRINK );

            // 検索バー
            m_tooltip.set_tip( m_button_close_search, CONTROL::get_label_motion( CONTROL::CloseSearchBar ) );
            m_tooltip.set_tip( m_button_up_search, CONTROL::get_label_motion( CONTROL::SearchPrev ) );
            m_tooltip.set_tip( m_button_down_search, CONTROL::get_label_motion( CONTROL::SearchNext ) );
            m_tooltip.set_tip( m_button_drawout_and, CONTROL::get_label_motion( CONTROL::DrawOutAnd ) );
            m_tooltip.set_tip( m_button_drawout_or, CONTROL::get_label_motion( CONTROL::DrawOutOr ) );
            m_tooltip.set_tip( m_button_clear_hl, CONTROL::get_label_motion( CONTROL::HiLightOff ) );

            m_searchbar.pack_start( m_entry_search, Gtk::PACK_EXPAND_WIDGET );
            m_searchbar.pack_end( m_button_close_search, Gtk::PACK_SHRINK );
            m_searchbar.pack_end( m_button_clear_hl, Gtk::PACK_SHRINK );
            m_searchbar.pack_end( m_button_drawout_or, Gtk::PACK_SHRINK );
            m_searchbar.pack_end( m_button_drawout_and, Gtk::PACK_SHRINK );
            m_searchbar.pack_end( m_button_up_search, Gtk::PACK_SHRINK );
            m_searchbar.pack_end( m_button_down_search, Gtk::PACK_SHRINK );

            add( m_vbox );
            set_policy( Gtk::POLICY_NEVER, Gtk::POLICY_NEVER );
            set_size_request( 8 );
            show_all_children();
        }
        
        virtual ~ArticleToolBar(){}
    };
}


#endif
