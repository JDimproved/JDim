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
        int m_mrg_x;  // ポップアップとマウスカーソルの間のマージン(水平方向)
        int m_mrg_y;  // ポップアップとマウスカーソルの間のマージン(垂直方向)

    public:

        PopupWin( Gtk::Widget* parent, SKELETON::View* view, const int mrg_x, const int mrg_y );
        ~PopupWin();

        // m_view　からの hide シグナルをブリッジする
        SIG_HIDE_POPUP& sig_hide_popup() { return m_sig_hide_popup; }
        void slot_hide_popup(){ m_sig_hide_popup.emit(); }

        SKELETON::View* view(){ return m_view; }

        // ポップアップウィンドウの座標と幅と高さを計算して移動とリサイズ
        void slot_resize_popup();
    };
}


#endif
