// ライセンス: GPL2
//
// コントロール系ユーティリティ関数
//

#ifndef _CONTROLUTIL_H
#define _CONTROLUTIL_H

#include <gtkmm.h>

namespace SKELETON
{
    class Admin;
}


namespace CONTROL
{
    void load_conf();
    void save_conf();
    void delete_conf();


    ///////////////////////


    // keysymはアスキー文字か
    bool is_ascii( const guint keysym );

    // メニューにショートカットキーやマウスジェスチャを表示
    void set_menu_motion( Gtk::Menu* menu );

    // IDからモードを取得
    // 例えば id == CONTROL::Up の時は CONTROL::COMMONMOTION を返す
    int get_mode( const int id );

    // 操作モードIDからモード名取得
    // 例えば mode == CONTROL::MODE_COMMON の時は "共通" を返す
    const std::string get_mode_label( const int mode );

    // キー名からkeysymを取得
    // 例えば keyname == "Space" の時は GDK_space を返す
    guint get_keysym( const std::string& keyname );

    // keysymからキー名を取得
    // 例えば keysym == GDK_space の時は "Space"  を返す
    const std::string get_keyname( const guint keysym );

    // 操作名からID取得
    // 例えば name == "Up" の時は CONTROL::Up を返す
    int get_id( const std::string& name );

    // IDから操作名取得
    // 例えば id == CONTROL::Up の時は "Up" を返す
    const std::string get_name( const int id );

    // IDからラベル取得
    // 例えば id == CONTROL::Up の時は "上移動" を返す
    const std::string get_label( const int id );

    // IDからショートカットを付けたラベルを取得
    // 例えば id == CONTROL::Save の時は "名前を付けて保存(_S)..." を返す
    const std::string get_label_with_mnemonic( const int id );

    // IDからキーボードとマウスジェスチャの両方を取得
    const std::string get_str_motions( const int id );

    // IDからラベルと操作の両方を取得
    const std::string get_label_motions( const int id );

    // 共通操作
    bool operate_common( const int control, const std::string& url, SKELETON::Admin* admin );


    ///////////////////////


    // キーボード設定の一時的なバックアップと復元
    void bkup_keyconfig();
    void restore_keyconfig();

    // IDからキーボード操作を取得
    const std::string get_str_keymotions( const int id );

    // IDからデフォルトキーボード操作を取得
    const std::string get_default_keymotions( const int id );

    // スペースで区切られた複数のキーボード操作をデータベースに登録
    void set_keymotions( const int id, const std::string& str_motions );

    // 指定したIDのキーボード操作を全て削除
    bool remove_keymotions( const int id );

    // キーボード操作が重複していないか
    const std::vector< int > check_key_conflict( const int mode, const std::string& str_motion );

    // editviewの操作をemacs風にする
    bool is_emacs_mode();
    void toggle_emacs_mode();

    // 「タブで開く」キーを入れ替える
    bool is_toggled_tab_key();
    void toggle_tab_key( const bool toggle );

    // Gtk アクセラレーションキーを取得
    Gtk::AccelKey get_accelkey( const int id );


    ///////////////////////


    // マウスジェスチャ設定の一時的なバックアップと復元
    void bkup_mouseconfig();
    void restore_mouseconfig();

    // IDからマウスジェスチャを取得
    const std::string get_str_mousemotions( const int id );

    // IDからデフォルトマウスジェスチャを取得
    const std::string get_default_mousemotions( const int id );

    // スペースで区切られた複数のマウスジェスチャをデータベースに登録
    void set_mousemotions( const int id, const std::string& str_motions );

    // 指定したIDのマウスジェスチャを全て削除
    bool remove_mousemotions( const int id );

    // マウスジェスチャが重複していないか
    const std::vector< int > check_mouse_conflict( const int mode, const std::string& str_motion );


    ///////////////////////


    // ボタン設定の一時的なバックアップと復元
    void bkup_buttonconfig();
    void restore_buttonconfig();

    // IDからボタン設定を取得
    const std::string get_str_buttonmotions( const int id );

    // IDからデフォルトボタン設定を取得
    const std::string get_default_buttonmotions( const int id );

    // スペースで区切られた複数のボタン設定をデータベースに登録
    void set_buttonmotions( const int id, const std::string& str_motions );

    // 指定したIDのボタン設定を全て削除
    bool remove_buttonmotions( const int id );

    // ボタンが重複していないか
    const std::vector< int > check_button_conflict( const int mode, const std::string& str_motion );


    ///////////////////////


    // タブで開くボタンを入れ替える
    bool is_toggled_tab_button();
    void toggle_tab_button( const bool toggle );

    // ポップアップ表示の時にクリックでワープ
    bool is_popup_warpmode();
    void toggle_popup_warpmode();
}

#endif
