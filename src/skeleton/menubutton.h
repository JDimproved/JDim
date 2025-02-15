// ライセンス: GPL2

// メニュー付きボタン

#ifndef _MENUBUTTON_H
#define _MENUBUTTON_H

#include <gtkmm.h>

#include <memory>
#include <string>
#include <vector>


constexpr size_t MAX_MENU_SIZE = 20;

namespace SKELETON
{
    class MenuButton : public Gtk::Button
    {
        typedef sigc::signal< void > SIG_BUTTON_CLICKED;
        typedef sigc::signal< void, const int > SIG_SELECTED;

        SIG_BUTTON_CLICKED m_sig_clicked;
        SIG_SELECTED m_sig_selected;

        std::unique_ptr<Gtk::Menu> m_popupmenu;
        std::vector< Gtk::MenuItem* > m_menuitems;
        Gtk::Widget* m_label{};

        Gtk::Image* m_arrow{};

        bool m_on_arrow{};
        bool m_enable_sig_clicked;

        MenuButton( const bool show_arrow, Gtk::Widget* label, Gtk::PackOptions options = Gtk::PACK_EXPAND_WIDGET );

      public:

        MenuButton( const bool show_arrow, Gtk::Widget& label );

        MenuButton( const bool show_arrow , const int id );

        ~MenuButton() noexcept override = default;

      Gtk::Widget* get_label_widget(){ return m_label; }

      void set_tooltip_arrow( const std::string& tooltip );

      SIG_BUTTON_CLICKED signal_button_clicked(){ return m_sig_clicked; }

      // メニューが選択されたらemitされる
      SIG_SELECTED signal_selected(){ return m_sig_selected; }

      // メニュー項目追加
      void append_menu( std::vector< std::string >& items );

      // 矢印ボタン以外をクリックしたときにSIG_BUTTON_CLICKEDをemitする
      // false の時はボタンのどこを押してもメニューを表示する
      void set_enable_sig_clicked( const bool enable ){ m_enable_sig_clicked = enable; }

      void on_clicked() override;

      protected:

      // ポップアップメニュー表示
      virtual void show_popupmenu();

      private:

      void slot_menu_selected( int i );

      bool slot_enter( GdkEventCrossing* event );
      bool slot_leave( GdkEventCrossing* event );
      bool slot_motion( GdkEventMotion* event );
      void check_on_arrow( int ex );
    };
}

#endif
