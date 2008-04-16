// ライセンス: GPL2

//
// ポップアップウィンドウ
//
// SKELETON::Viewをポップアップ表示する。
// 表示するSKELETON::Viewはデストラクタでdeleteするので呼出元でdeleteしなくても良い
//

#ifndef _POPUPWIN_H
#define _POPUPWIN_H

#include "popupwinbase.h"
#include "view.h"


namespace SKELETON
{
    class PopupWin : public PopupWinBase
    {
        SIG_HIDE_POPUP m_sig_hide_popup;

        Gtk::Widget* m_parent;
        SKELETON::View* m_view;
        int m_mrg_y;  // ポップアップとマウスカーソルの間のマージン

    public:

        // m_view　からの hide シグナルをブリッジする
        SIG_HIDE_POPUP& sig_hide_popup() { return m_sig_hide_popup; }
        void slot_hide_popup(){ m_sig_hide_popup.emit(); }

        SKELETON::View* view(){ return m_view; }

        // ポップアップウィンドウの座標と幅と高さを計算して移動とリサイズ
        void slot_resize_popup()
        {
            if( ! m_view ) return;
    
            // マウス座標
            int x_mouse, y_mouse;
            Gdk::ModifierType mod;
            Gdk::Display::get_default()->get_pointer( x_mouse, y_mouse,  mod );

            // クライアントのサイズを取得
            const int width_client = m_view->width_client();
            const int height_client = m_view->height_client();

            const int width_desktop = m_parent->get_screen()->get_width();
            const int height_desktop = m_parent->get_screen()->get_height();

            // 幅(m_parentの幅を超えないように制限する)
            const int width_parent = m_parent->get_width();
            const int width_popup = width_client < width_parent ? width_client : width_parent;

            // x 座標
            int x_popup;
            if( x_mouse + width_popup <= width_desktop ) x_popup = x_mouse;
            else x_popup = MAX( 0, width_desktop - width_popup );

            // y 座標と高さ
            int y_popup;  
            int height_popup;
            if( y_mouse - ( height_client + m_mrg_y ) >= 0 ){  // 上にスペースがある
                y_popup = y_mouse - height_client - m_mrg_y;
                height_popup = height_client;
            }
            else if( y_mouse + m_mrg_y + height_client <= height_desktop ){ // 下にスペースがある
                y_popup = y_mouse + m_mrg_y;        
                height_popup = height_client;
            }
            else if( y_mouse > height_desktop/2 ){ // スペースは無いが上に表示
                y_popup = MAX( 0, y_mouse - ( height_client + m_mrg_y ) );
                height_popup = y_mouse - ( y_popup + m_mrg_y );
            }
            else{ // 下
                y_popup = y_mouse + m_mrg_y;        
                height_popup = height_desktop - y_popup;
            }

#ifdef _DEBUG
            std::cout << "PopupWin::slot_resize_popup : x = " << x_popup << " y = " << y_popup
                      << " w = " << width_popup << " h = " << height_popup << std::endl;
#endif
            move( x_popup, y_popup );
            resize( width_popup,  height_popup );
            show_all();
        }


        PopupWin( Gtk::Widget* parent, SKELETON::View* view, int mrg_y )
        : PopupWinBase( POPUPWIN_NOFRAME ),
              m_parent( parent ),
              m_view( view ),
              m_mrg_y( mrg_y )
        {
#ifdef _DEBUG
            std::cout << "PopupWin::PopupWin\n";
#endif

            m_view->sig_resize_popup().connect( sigc::mem_fun( *this, &PopupWin::slot_resize_popup ) );
            add( *m_view );
            m_view->sig_hide_popup().connect( sigc::mem_fun( *this, &PopupWin::slot_hide_popup ) );
            m_view->show_view();

            Gtk::Widget* toplevel = m_parent->get_toplevel();
            if( toplevel->is_toplevel() ) set_transient_for( *( ( Gtk::Window* )toplevel ) );
            slot_resize_popup();
        }

        ~PopupWin()
        {
            if( m_view ) delete m_view;
        }
    };
}


#endif
