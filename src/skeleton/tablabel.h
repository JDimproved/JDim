// ライセンス: GPL2

#ifndef _TABLABEL_H
#define _TABLABEL_H

#include <gtkmm.h>
#include <string>

namespace SKELETON
{
    // マウス
    typedef sigc::signal< void > SIG_TAB_MOTION_EVENT;
    typedef sigc::signal< void > SIG_TAB_LEAVE_EVENT;

    // D&D
    typedef sigc::signal< void > SIG_TAB_DRAG_BEGIN;
    typedef sigc::signal< void, Gtk::SelectionData& > SIG_TAB_DRAG_DATA_GET;
    typedef sigc::signal< void > SIG_TAB_DRAG_END;

    class TabLabel : public Gtk::EventBox
    {
        SIG_TAB_MOTION_EVENT m_sig_tab_motion_event;
        SIG_TAB_LEAVE_EVENT m_sig_tab_leave_event;

        SIG_TAB_DRAG_BEGIN m_sig_tab_drag_begin;
        SIG_TAB_DRAG_DATA_GET m_sig_tab_drag_data_get;
        SIG_TAB_DRAG_END m_sig_tab_drag_end;

        int m_x;
        int m_y;
        int m_width;
        int m_height;

        std::string m_url;
        Gtk::HBox m_hbox;
        int m_id_icon;

        Gtk::Label m_label;

        // アイコン画像
        Gtk::Image* m_image;

        // ラベルに表示する文字列の全体
        std::string m_fulltext;

      public:

        TabLabel( const std::string& url );
        ~TabLabel();

        SIG_TAB_MOTION_EVENT sig_tab_motion_event(){ return  m_sig_tab_motion_event; }
        SIG_TAB_LEAVE_EVENT sig_tab_leave_event(){ return m_sig_tab_leave_event; }

        SIG_TAB_DRAG_BEGIN sig_tab_drag_begin() { return m_sig_tab_drag_begin; }
        SIG_TAB_DRAG_DATA_GET sig_tab_drag_data_get() { return m_sig_tab_drag_data_get; }
        SIG_TAB_DRAG_END sig_tab_drag_end() { return m_sig_tab_drag_end; }

        int get_tab_x() const { return m_x; }
        int get_tab_y() const { return m_y; }
        int get_tab_width() const { return m_width; }
        int get_tab_height() const { return m_height; }

        void set_tab_x( const int x ){ m_x = x; }
        void set_tab_y( const int y ){ m_y = y; }
        void set_tab_width( const int width ){ m_width = width; }
        void set_tab_height( const int height ){ m_height = height; }

        Pango::FontDescription get_label_font_description(){
            return m_label.get_pango_context()->get_font_description(); }

        const std::string& get_url(){ return m_url; }

        void set_dragable( bool dragable, int button );

        // 本体の横幅 - ラベルの横幅
        int get_label_margin();

        // カットしていない全体の文字列
        const std::string& get_fulltext() const { return m_fulltext; }
        void set_fulltext( const std::string& label ){ m_fulltext = label; }

        // アイコンセット
        void set_id_icon( const int id );
        int get_id_icon() const { return m_id_icon; }

        // タブの文字列の文字数をlngにセット
        void resize_tab( const unsigned int lng );

      private:

        bool on_motion_notify_event( GdkEventMotion* event ) override;
        bool on_leave_notify_event( GdkEventCrossing* event ) override;

        void on_drag_begin( const Glib::RefPtr< Gdk::DragContext >& context ) override;
        void on_drag_data_get( const Glib::RefPtr< Gdk::DragContext >& context,
                               Gtk::SelectionData& selection_data, guint info, guint time ) override;
        void on_drag_end( const Glib::RefPtr< Gdk::DragContext>& context ) override;
    }; 
}

#endif
