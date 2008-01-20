// ライセンス: GPL2
//
// ツールバーの基底クラス
//

#ifndef _TOOLBAR_H
#define _TOOLBAR_H

#include <gtkmm.h>


namespace SKELETON
{
    class Admin;
    class ImgButton;
    class BackForwardButton;

    class ToolBar : public Gtk::VBox
    {
        SKELETON::Admin* m_admin;
        std::string m_url;

        // ツールバー表示状態
        bool m_toolbar_shown;

        Gtk::Tooltips m_tooltip;
        Gtk::ScrolledWindow m_scrwin;
        Gtk::HBox m_buttonbar;

        SKELETON::ImgButton* m_button_close;
        SKELETON::BackForwardButton* m_button_back;
        SKELETON::BackForwardButton* m_button_forward;

      public:

        ToolBar( Admin* admin );
        virtual ~ToolBar(){}

        void set_url( const std::string& url );

        bool is_empty();

        // ツールバー表示
        void show_toolbar();

        // ツールバー非表示
        void hide_toolbar();

        // タブのロック
        void lock();

        // タブのアンロック
        void unlock();

        // 更新
        void update();

      protected:

        // ボタンのパッキング
        virtual void pack_buttons() = 0;
        void unpack_buttons();

        Gtk::HBox& get_buttonbar(){ return m_buttonbar; }

        Gtk::Button* get_button_close();
        Gtk::Button* get_button_back();
        Gtk::Button* get_button_forward();

        void pack_separator();
        void set_tooltip( Gtk::Widget& widget, const std::string& tip ){ m_tooltip.set_tip( widget, tip ); }

      private:

        void slot_clicked_close();

        void slot_clicked_back();
        void slot_selected_back( const int i );

        void slot_clicked_forward();
        void slot_selected_forward( const int i );
    };
}


#endif
