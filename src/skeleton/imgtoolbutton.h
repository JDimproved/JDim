// ライセンス: GPL2

// 画像つきツールボタン

#ifndef _IMGTOOLBUTTON_H
#define _IMGTOOLBUTTON_H

#include <gtkmm.h>
#include <string>
#include <type_traits>

#include "control/controlutil.h"
#include "icons/iconmanager.h"

namespace SKELETON
{
    template<typename Base>
    class ToolButtonExtension : public Base
    {
        static_assert( std::is_base_of<Gtk::ToolButton, Base>::value, "Base must inherit Gtk::ToolButton" );

    public:
        explicit ToolButtonExtension( const int iconid )
            : Base( *Gtk::manage( new Gtk::Image( ICON::get_icon( iconid ), Gtk::ICON_SIZE_SMALL_TOOLBAR ) ) )
        {}

        explicit ToolButtonExtension( const int iconid, const int controlid )
            : Base( *Gtk::manage( new Gtk::Image( ICON::get_icon( iconid ), Gtk::ICON_SIZE_SMALL_TOOLBAR ) ), CONTROL::get_label( controlid ) )
        {}

        explicit ToolButtonExtension( const int iconid, const Glib::ustring& label )
            : Base( *Gtk::manage( new Gtk::Image( ICON::get_icon( iconid ), Gtk::ICON_SIZE_SMALL_TOOLBAR ) ), label )
        {}

        ~ToolButtonExtension() noexcept override = default;
    };

    using ImgToolButton = ToolButtonExtension<Gtk::ToolButton>;
    using ImgToggleToolButton = ToolButtonExtension<Gtk::ToggleToolButton>;
}

#endif
