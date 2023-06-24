// ライセンス: GPL2
//
// タブの切り替えメニュー
//
#ifndef SKELETON_TABSWITCHMENU_H
#define SKELETON_TABSWITCHMENU_H

#include <gtkmm.h>

#include <vector>

namespace SKELETON
{
    class DragableNoteBook;

    /**
     * @brief タブを切り替えるメニューのモデル
     *
     * SKELETON::Admin でメニュー項目の部品として使われる
     */
    class TabSwitchMenu : public Gio::Menu
    {
        /// Admin メンバー変数への参照で所有しない
        DragableNoteBook* m_parentnote;
        std::vector<Glib::RefPtr<Gio::MenuItem>> m_items;
        bool m_deactivated;

      public:

        static Glib::RefPtr<TabSwitchMenu> create( DragableNoteBook* notebook );

        explicit TabSwitchMenu( DragableNoteBook* notebook );
        ~TabSwitchMenu() noexcept = default;

        /// メニュー項目を作り直してラベルとアイコンを更新する
        void update_labels_and_icons();
        /// メニュー項目を作り直してアイコンを更新する
        void update_icons();
        /// メニューがスクリーンから消されるときに呼び出す
        void deactivate();

      private:

        /// メニュー項目を必要な分だけ確保しておき使い回す
        void alloc_items();
    };
}

#endif
