// ライセンス: GPL2
//
// コントロール系ユーティリティ関数
//

#ifndef _CONTROLUTIL_H
#define _CONTROLUTIL_H

#include <gtkmm.h>

namespace CONTROL
{
    class KeyConfig;

    KeyConfig* get_keyconfig();
    void delete_keyconfig();

    // 設定の一時的なバックアップと復元
    void bkup_keyconfig();
    void restore_keyconfig();

    void load_conf();
    void save_conf();

    // keysymはアスキー文字か
    const bool is_ascii( const guint keysym );

    // メニューにショートカットキーやマウスジェスチャを表示
    void set_menu_motion( Gtk::Menu* menu );

    // IDからモードを取得
    // 例えば id == CONTROL::Up の時は CONTROL::COMMONMOTION を返す
    const int get_mode( const int id );

    // キー名からkeysymを取得
    // 例えば keyname == "Space" の時は GDK_space を返す
    const guint get_keysym( const std::string& keyname );

    // keysymからキー名を取得
    // 例えば keysym == GDK_space の時は "Space"  を返す
    const std::string get_keyname( const guint keysym );

    // 操作名からID取得
    // 例えば name == "Up" の時は CONTROL::Up を返す
    const int get_id( const std::string& name );

    // IDから操作名取得
    // 例えば id == CONTROL::Up の時は "Up" を返す
    const std::string get_name( const int id );

    // IDからラベル取得
    // 例えば id == CONTROL::Up の時は "上移動" を返す
    const std::string get_label( const int id );

    // IDからショートカットを付けたラベルを取得
    // 例えば id == CONTROL::Save の時は "名前を付けて保存(_S)..." を返す
    const std::string get_label_with_mnemonic( const int id );

    // IDからキーボード操作を取得
    const std::string get_str_keymotions( const int id );

    // IDからデフォルトキーボード操作を取得
    const std::string get_default_keymotions( const int id );

    // スペースで区切られた複数のキーボード操作をデータベースに登録
    void set_keymotions( const std::string& name, const std::string& str_motions );

    // 指定したIDのキーボード操作を全て削除
    const bool remove_keymotions( const int id );

    // IDからマウス操作を取得
    const std::string get_str_mousemotions( const int id );

    // IDからキーボードとマウス操作の両方を取得
    const std::string get_str_motions( const int id );

    // IDからラベルと操作の両方を取得
    const std::string get_label_motions( const int id );

    // キーボード操作が重複していないか
    const int check_key_conflict( const int mode, const std::string& str_motion );
    const int check_key_conflict( const int mode,
                                  const guint keysym, const bool ctrl, const bool shift, const bool alt );

}

#endif
