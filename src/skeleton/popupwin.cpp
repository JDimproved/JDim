// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "popupwin.h"

#include "jdlib/miscgtk.h"

#ifdef GDK_WINDOWING_WAYLAND
#include <gdk/gdkwayland.h>
#endif

#include <algorithm>


using namespace SKELETON;


PopupWin::PopupWin( Gtk::Widget* parent, SKELETON::View* view, const int mrg_x, const int mrg_y )
    : PopupWinBase( POPUPWIN_NOFRAME )
    , m_parent{ parent }
    , m_view{ view }
    , m_mrg_x{ mrg_x }
    , m_mrg_y{ mrg_y }
{
#ifdef _DEBUG
    std::cout << "PopupWin::PopupWin\n";
#endif
    signal_realize().connect( sigc::mem_fun( *this, &PopupWin::slot_realize ) );

    m_view->sig_resize_popup().connect( sigc::mem_fun( *this, &PopupWin::slot_resize_popup ) );
    add( *m_view );
    m_view->sig_hide_popup().connect( sigc::mem_fun( *this, &PopupWin::slot_hide_popup ) );
    m_view->show_view();

    Gtk::Widget* toplevel = m_parent->get_toplevel();
    if( toplevel->get_is_toplevel() ) {
        set_transient_for( *dynamic_cast< Gtk::Window* >( toplevel ) );
#ifdef GDK_WINDOWING_WAYLAND
        m_running_on_wayland = GDK_IS_WAYLAND_DISPLAY( toplevel->get_window()->get_display()->gobj() );
#endif
    }
}


//
// ポップアップウィンドウの座標と幅と高さを計算して移動とリサイズ
//
void PopupWin::slot_resize_popup()
{
    if( ! m_view ) return;

    if( m_running_on_wayland ) {
        move_resize_wayland();
    }
    else {
        move_resize_conventional();
    }
    show_all();
}


/** @brief realize したときにポップアップの座標と幅と高さを計算してリサイズと移動を行う
 *
 * @details Wayland では realize すると GdkWindow が関連付けされて
 * gdk_window_move_to_rect() が実行できるようになる。
 */
void PopupWin::slot_realize()
{
    if( ! m_view ) return;

    if( m_running_on_wayland ) {
        move_resize_wayland();
    }
    else {
        move_resize_conventional();
    }
}


/**
 * @brief ポップアップウィンドウの座標と幅と高さを計算してリサイズと移動する (従来方式)
 *
 * @details X11で gdk_window_move_to_rect() を使うと挙動が変わるため従来の方法を使う。
 */
void PopupWin::move_resize_conventional()
{
    // ディスプレイの情報を取得する。
    auto display = Gdk::Display::get_default();

    // ディスプレイに対するマウス座標を取得する。
    int x_mouse, y_mouse;
    display->get_default_seat()->get_pointer()->get_position( x_mouse, y_mouse );

    Gtk::Window* parent_win = get_transient_for();

    // 以前は Gdk::Screen を使用してポップアップのサイズを計算していたが、
    // Gdk::Screen は複数モニターを一つの画面領域として扱うため、
    // マルチディスプレイ環境ではポップアップが画面を跨いで表示される問題があった。

    // 親ウインドウが配置されている Gdk::Monitor の作業領域からポップアップのサイズと位置を計算する。
    Gdk::Rectangle rect;
    const auto monitor = display->get_monitor_at_window( parent_win->get_window() );
    monitor->get_workarea( rect );
    const int width_desktop = rect.get_width();
    const int height_desktop = rect.get_height();

    // クライアントのサイズを取得
    const int width_client = m_view->width_client();
    const int height_client = m_view->height_client();

    // x 座標と幅
    const int width_popup = width_client;
    int x_popup;
    if( x_mouse + m_mrg_x + width_popup <= width_desktop ) x_popup = x_mouse + m_mrg_x;
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
    else if( m_view->get_popup_upside() || y_mouse > height_desktop/2 ){ // スペースは無いが上に表示
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
}


/** @brief ポップアップウィンドウの座標と幅と高さを計算してリサイズと移動する (Wayland)
 *
 * @details Waylandでポップアップを表示するときの動作を説明します。
 * GTK 3.24から導入された [`gdk_window_move_to_rect()`] を使用してWaylandでの配置を改善します。
 * X11環境では従来の `Gtk::Window::move()` による方法を維持します。
 *
 * ポップアップを表示するには `SKELETON::PopupWin` と `Gdk::Window` が関連づけられている必要があります。
 * Waylandでは、 `PopupWin` のコンストラクタを実行している間は `Gdk::Window` が関連付けされていないため、
 * `show_all()` をはじめ、 `Gdk::Window` が必要な操作が期待通りに動作しません。
 * そのため、 `Gdk::Window` が関連付けされたときに発行される `realize` シグナルに接続したシグナルハンドラで
 * ウインドウの操作を行うように修正します。 また、ポップアップの表示はコンストラクタを呼び出した後に行います。
 *
 * - `gdk_window_move_to_rect()` は [`GdkAnchorHints`] を使ってウインドウのサイズと配置を決定します。
 *   - 水平方向(X軸)はポップアップの幅を変えずディスプレイに収まるようにずらします。
 *   - 垂直方向(Y軸)はポップアップの高さをディスプレイに収まるようにサイズ調整します。
 *   デフォルトの動作では垂直方向が期待した位置に配置されないことがあるため、
 *   ディスプレイのサイズとマウスポインターの座標から手動で配置を指定します。
 *
 * - `gdk_window_move_to_rect()` はウインドウに追加された子要素のサイズからウインドウの表示サイズを算出します。
 *   ポップアップを自然なサイズで表示するため `SKELETON::View` にウィジェットの最小サイズと自然なサイズを返す
 *   virtual method を実装します。
 *
 * - `gdk_window_move_to_rect()` はウインドウが表示された状態では効果がありません。
 *   表示されているウインドウの移動を行うときは一度閉じます。
 *   また、サイズ変更するときは `Gtk::Window::resize()` を使います。
 *
 * [`gdk_window_move_to_rect()`]: https://docs.gtk.org/gdk3/method.Window.move_to_rect.html
 * [`GdkAnchorHints`]: https://docs.gtk.org/gdk3/flags.AnchorHints.html
 */
void PopupWin::move_resize_wayland()
{
    // 1. ディスプレイの情報を取得する

    auto display = Gdk::Display::get_default();

    // ディスプレイに対するマウス座標
    int x_mouse, y_mouse;
    display->get_default_seat()->get_pointer()->get_position( x_mouse, y_mouse );

    Gtk::Window* parent = get_transient_for();

    // ディスプレイのサイズ
    Gdk::Rectangle rect;
    const auto monitor = display->get_monitor_at_window( parent->get_window() );
    monitor->get_workarea( rect );
    const int height_desktop = rect.get_height();

    // クライアントのサイズを取得
    const int height_client = m_view->height_client();
    const int width_client = m_view->width_client();

    // 2. 環境の情報から方向と高さを計算する

    // GdkAnchorHints に GDK_ANCHOR_RESIZE_Y と GDK_ANCHOR_FLIP_Y を一緒に指定して
    // gdk_window_move_to_rect() に配置を任せると下の条件通りには動作しないため
    // ディスプレイのサイズとマウス座標から計算して GdkGravity を設定する
    GdkGravity rect_anchor; // 表示位置の角
    GdkGravity window_anchor; // ポップアップの角
    int height_popup; // ポップアップ表示中にリサイズするとき使う
    if( y_mouse - ( height_client + m_mrg_y ) >= 0 ) { // 上にスペースがある
        rect_anchor = GDK_GRAVITY_NORTH_WEST;
        window_anchor = GDK_GRAVITY_SOUTH_WEST;
        height_popup = height_client;
    }
    else if( y_mouse + m_mrg_y + height_client <= height_desktop ) { // 下にスペースがある
        rect_anchor = GDK_GRAVITY_SOUTH_WEST;
        window_anchor = GDK_GRAVITY_NORTH_WEST;
        height_popup = height_client;
    }
    else if( m_view->get_popup_upside() || y_mouse > height_desktop / 2 ) { // スペースは無いが上に表示
        rect_anchor = GDK_GRAVITY_NORTH_WEST;
        window_anchor = GDK_GRAVITY_SOUTH_WEST;
        const int y_popup = (std::max)( 0, y_mouse - ( height_client + m_mrg_y ) );
        height_popup = y_mouse - ( y_popup + m_mrg_y );
    }
    else { // スペースは無いが下に表示
        rect_anchor = GDK_GRAVITY_SOUTH_WEST;
        window_anchor = GDK_GRAVITY_NORTH_WEST;
        const int y_popup = y_mouse + m_mrg_y;
        height_popup = height_desktop - y_popup;
    }

    // 3. リサイズと移動を行う

    // ポップアップ表示位置を取得 (親ウインドウのマウス座標)
    MISC::get_pointer_at_window( parent->get_window(), x_mouse, y_mouse );
    // 表示位置を垂直に伸びる矩形で指定して、ポップアップとマウスポインターの間のマージンを作る
    const GdkRectangle rect_dest = { x_mouse, y_mouse - m_mrg_y, 1, m_mrg_y * 2 + 1 };

    // 水平方向(X軸)は、ディスプレイに収まるようにスライドさせる。
    // 垂直方向(Y軸)は、ウインドウ配置の方向と初期サイズを計算して調整する。
    constexpr GdkAnchorHints anchor_hints = GdkAnchorHints( GDK_ANCHOR_SLIDE_X | GDK_ANCHOR_RESIZE_Y );
    const int rect_anchor_dx = m_mrg_x;
    constexpr int rect_anchor_dy = 0;

    if( get_mapped() ) {
        // gdk_window_move_to_rect() は表示したポップアップのサイズ変更を無視するため resize() を使う
        resize( width_client, height_popup );

        // ウインドウを非表示にして呼び出さないと期待通り動作しない環境がある
        // https://gitlab.gnome.org/GNOME/gtk/-/issues/1986
        hide();
    }
    gdk_window_move_to_rect( get_window()->gobj(), &rect_dest, rect_anchor, window_anchor, anchor_hints,
                             rect_anchor_dx, rect_anchor_dy );
}
