// ライセンス: GPL2

// 画像つきメニュー付きボタン

#ifndef _IMGMENUBUTTON_H
#define _IMGMENUBUTTON_H

#include <gtkmm.h>
#include <string>
#include <vector>

enum
{
    MAX_MENU_SIZE = 20
};

namespace SKELETON
{
    class ImgMenuButton : public Gtk::Button
    {
        typedef sigc::signal< void > SIG_BUTTON_CLICKED;
        typedef sigc::signal< void, const int > SIG_SELECTED;

        SIG_BUTTON_CLICKED m_sig_clicked;
        SIG_SELECTED m_sig_selected;

        Gtk::Menu* m_popupmenu;
        std::vector< Gtk::MenuItem* > m_menuitems;
        Gtk::Image* m_img;
        Gtk::Arrow* m_arrow;

        bool m_on_arrow;

      public:

      ImgMenuButton( const Gtk::StockID& stock_id,
                     const Gtk::BuiltinIconSize icon_size = Gtk::ICON_SIZE_MENU );
      ImgMenuButton();

      virtual ~ImgMenuButton();

      SIG_BUTTON_CLICKED signal_button_clicked(){ return m_sig_clicked; }

      // メニューが選択されたらemitされる
      SIG_SELECTED signal_selected(){ return m_sig_selected; }

      // メニュー項目追加
      void append_menu( std::vector< std::string >& items );

      protected:

      // ポップアップメニュー表示
      virtual void show_popupmenu();

      private:

      void setup();

      void slot_menu_selected( int i );

      void on_clicked();
      void slot_popup_pos( int& x, int& y, bool& push_in );

      bool slot_enter( GdkEventCrossing* event );
      bool slot_leave( GdkEventCrossing* event );
      bool slot_motion( GdkEventMotion* event );
      void check_on_arrow( int ex );
    };
}

#endif
