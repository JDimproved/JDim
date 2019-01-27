// ライセンス: GPL2

//
// 画像アイコンクラス
//

#ifndef _IMAGEVIEWICON_H
#define _IMAGEVIEWICON_H

#include "imageviewbase.h"

namespace IMAGE
{
    class ImageViewIcon : public ImageViewBase
    {
        Gtk::EventBox* m_event_frame;

      public:
        ImageViewIcon( const std::string& url );
        ~ImageViewIcon();

        void clock_in() override;
        void focus_view() override;
        void focus_out() override;
        void show_view() override;

      protected:

        Gtk::Menu* get_popupmenu( const std::string& url ) override;
        bool slot_scroll_event( GdkEventScroll* event ) override;

      private:

        void switch_icon() override;

        void slot_drag_begin( const Glib::RefPtr<Gdk::DragContext>& context );
        void slot_drag_data_get( const Glib::RefPtr<Gdk::DragContext>& context,
                                 Gtk::SelectionData& selection_data, guint info, guint time );
        void slot_drag_data_received( const Glib::RefPtr<Gdk::DragContext>& context, int x, int y,
                                      const Gtk::SelectionData& selection_data, guint info, guint time );
        void slot_drag_end( const Glib::RefPtr< Gdk::DragContext >& context );
    };
}

#endif

