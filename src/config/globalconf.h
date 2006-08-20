// ライセンス: 最新のGPL
//
// グローバル設定
//

#ifndef _GLOBALCONF_H
#define _GLOBALCONF_H

#include <string>
#include <list>

namespace CONFIG
{
    // 設定読み込み
    const bool init_config();

    void save_conf();
    
    // 前回開いたviewを復元するか
    const bool get_restore_board();
    void set_restore_board( bool restore );
    const bool get_restore_article();
    void set_restore_article( bool restore );
    const bool get_restore_image();
    void set_restore_image( bool restore );

    // 色 ( RGB の順 ) 範囲は 0 - 65535
    const int* get_color_char();
    const int* get_color_separator();
    const int* get_color_back();
    const int* get_color_back_popup();
    const int* get_color_back_tree();
    const int* get_color_back_tree_board();
    
    void set_color_char( int* color );
    void set_color_separator( int* color );
    void set_color_back( int* color );
    void set_color_back_popup( int* color );
    void set_color_back_tree( int* color );
    void set_color_back_tree_board( int* color );

    // フォント
    const std::string& get_fontname_main();
    const std::string& get_fontname_popup();
    const std::string& get_fontname_tree();
    const std::string& get_fontname_tree_board();

    void set_fontname_main( const std::string& name );
    void set_fontname_popup( const std::string& name );
    void set_fontname_tree( const std::string& name );
    void set_fontname_tree_board( const std::string& name );

    // 2chの認証サーバ
    const std::string& get_url_login2ch();

    // bbsmenu.htmlのURL
    const std::string& get_url_bbsmenu();    

    // キャッシュのルートディレクトリ
    // キャッシュ構造は navi2ch の上位互換なので path_cacheroot = "~/.navi2ch/" とすればnavi2chとキャッシュを共有できる
    const std::string& get_path_cacheroot();

    // 2ch にアクセスするときのエージェント名
    const std::string& get_agent_for2ch();

    // 2ch にログインするときのX-2ch-UA
    const std::string& get_x_2ch_ua();

    // 読み込み用プロクシとポート番号
    const bool get_use_proxy_for2ch();
    const std::string& get_proxy_for2ch();
    const int get_proxy_port_for2ch();

    void set_use_proxy_for2ch( bool set );
    void set_proxy_for2ch( const std::string& proxy );
    void set_proxy_port_for2ch( int port );

    // 書き込み用プロクシとポート番号
    const bool get_use_proxy_for2ch_w();
    const std::string& get_proxy_for2ch_w();
    const int get_proxy_port_for2ch_w();

    void set_use_proxy_for2ch_w( bool set );
    void set_proxy_for2ch_w( const std::string& proxy );
    void set_proxy_port_for2ch_w( int port );

    // 2ch外にアクセスするときのエージェント名
    const std::string& get_agent_for_data();

    // 2chの外にアクセスするときのプロクシとポート番号
    const bool get_use_proxy_for_data();
    const std::string& get_proxy_for_data();
    const int get_proxy_port_for_data();

    void set_use_proxy_for_data( bool set );
    void set_proxy_for_data( const std::string& proxy );
    void set_proxy_port_for_data( int port );

    // ローダのバッファサイズ
    const int get_loader_bufsize();

    // ローダのタイムアウト値
    const int get_loader_timeout();
    const int get_loader_timeout_post();
    const int get_loader_timeout_img();

    // リンクをクリックしたときに実行するコマンド
    const std::string& get_command_openurl();
    void set_command_openurl( const std::string& command );

    // ブラウザ設定ダイアログのコンボボックスの番号
    const int get_brownsercombo_id();
    void set_brownsercombo_id( int id );

    // 画像ポップアップサイズ
    const int get_imgpopup_width();
    const int get_imgpopup_height();

    // 画像にモザイクかける
    const bool get_use_mosaic();
    void set_use_mosaic( bool mosaic );

    // 画像をデフォルトでウィンドウサイズに合わせる
    const bool get_zoom_to_fit();
    void set_zoom_to_fit( bool fit );

    const bool get_show_oldarticle();
    void set_show_oldarticle( bool showarticle );

    // ツリービューのスクロール量(行数)
    const int get_tree_scroll_size();

    // 板一覧でカテゴリを常にひとつだけ開く
    const bool get_open_one_category();

    // 書き込み時に書き込み確認ダイアログを出すかどうか
    const bool get_always_write_ok();
    void set_always_write_ok( bool write_ok );

    // ポップアップとカーソルの間のマージン
    const int get_margin_popup();

    // マウスジェスチャの判定開始半径
    const int get_mouse_radius();

    // 履歴の保持数
    const int get_history_size();

    // 0以上なら多重ポップアップの説明を表示する
    const int get_instruct_popup();

    // スレ表示の行間調整
    const double get_adjust_underline_pos();
    const double get_adjust_line_space();

    // スレ表示でリンクの下に下線を引く
    const bool get_draw_underline();

    // boardviewでのスレの全体あぼーん
    std::list< std::string >& get_list_abone_word_thread(); // ワード
    std::list< std::string >& get_list_abone_regex_thread(); // 正規表現

    void set_list_abone_word_thread( std::list< std::string >& word );
    void set_list_abone_regex_thread( std::list< std::string >& regex );

    // articleviewでのレスの全体あぼーん
    std::list< std::string >& get_list_abone_name(); // 名前
    std::list< std::string >& get_list_abone_word(); // ワード
    std::list< std::string >& get_list_abone_regex(); // 正規表現

    void set_list_abone_name( std::list< std::string >& name );
    void set_list_abone_word( std::list< std::string >& word );
    void set_list_abone_regex( std::list< std::string >& regex );
}


#endif
