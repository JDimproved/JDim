// ライセンス: GPL2
//
// グローバル設定
//

#ifndef _GLOBALCONF_H
#define _GLOBALCONF_H

#include <string>
#include <list>

namespace CONFIG
{
    class ConfigItems;

    ConfigItems* get_confitem();
    void delete_confitem();

    // 設定読み込み、書き込み
    const bool load_conf();
    void save_conf();

    // 設定の一時的なバックアップと復元
    void bkup_conf();
    void restore_conf();


    /////////////////////////////////////////////
    
    // 前回開いたviewを復元するか
    const bool get_restore_board();
    void set_restore_board( bool restore );
    const bool get_restore_article();
    void set_restore_article( bool restore );
    const bool get_restore_image();
    void set_restore_image( bool restore );

    // 色 ( # + 12桁の16進数 の形式 )
    const std::string& get_color( int id );
    void set_color( int id, const std::string& color );

    // ツリービューでgtkrcの設定を使用するか
    const bool get_use_tree_gtkrc();

    // ツリービューの行間スペース
    const int get_tree_ypad();

    // フォント
    const std::string& get_fontname( int id );
    void set_fontname( int id, const std::string& fontname );

    // レスを参照するときに前に付ける文字
    const std::string& get_ref_prefix();

    // 2chの認証サーバ
    const std::string& get_url_login2ch();

    // bbsmenu.htmlのURL
    const std::string& get_url_bbsmenu();    

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

    // ipv6使用
    const bool get_use_ipv6();

    // リンクをクリックしたときに実行するコマンド
    const std::string& get_command_openurl();
    void set_command_openurl( const std::string& command );

    // ブラウザ設定ダイアログのコンボボックスの番号
    const int get_brownsercombo_id();
    void set_brownsercombo_id( int id );

    // レス番号の上にマウスオーバーしたときに参照ポップアップ表示する
    const bool get_refpopup_by_mo();

    // 名前の上にマウスオーバーしたときにポップアップ表示する
    const bool get_namepopup_by_mo();

    // IDの上にマウスオーバーしたときにIDをポップアップ表示する
    const bool get_idpopup_by_mo();

    // 画像ポップアップサイズ
    const int get_imgpopup_width();
    const int get_imgpopup_height();

    // 画像ビューを使用する
    const bool get_use_image_view();
    void set_use_image_view( bool image_view );

    // 画像にモザイクかける
    const bool get_use_mosaic();
    void set_use_mosaic( bool mosaic );

    // 画像をデフォルトでウィンドウサイズに合わせる
    const bool get_zoom_to_fit();
    void set_zoom_to_fit( bool fit );

    // 画像キャッシュ削除の日数
    const int get_del_img_day();
    void set_del_img_day( int day );

    // ダウンロードする画像の最大サイズ(Mbyte)
    const int get_max_img_size();

    // スレ一覧にdat落ちしたスレも表示する
    const bool get_show_oldarticle();
    void set_show_oldarticle( bool showarticle );

    // スレ一覧で指定した値(時間)よりも後に立てられたスレを新着とみなす
    const int get_newthread_hour();

    // ツリービューのスクロール量(行数)
    const int get_tree_scroll_size();

    // スレビューのスクロール量
    const int get_scroll_size();

    // 板一覧でカテゴリを常にひとつだけ開く
    const bool get_open_one_category();

    // 書き込み時に書き込み確認ダイアログを出すかどうか
    const bool get_always_write_ok();
    void set_always_write_ok( bool write_ok );

    // 書き込みログを保存
    const bool get_save_postlog();
    void set_save_postlog( bool save );

    // 書き込み中のダイアログを表示しない
    const bool get_hide_writing_dialog();

    // ポップアップとカーソルの間のマージン
    const int get_margin_popup();
    void set_margin_popup( int margin );

    // マウスジェスチャの判定開始半径
    const int get_mouse_radius();

    // 履歴の保持数
    const int get_history_size();

    // 0以上なら多重ポップアップの説明を表示する
    const int get_instruct_popup();

    // スレビューを開いたときにスレ一覧との切り替え方法を説明する
    const bool get_instruct_tglart();
    void set_instruct_tglart( bool set );

    // 画像ビューを開いたときにスレビューとの切り替え方法を説明する
    const bool get_instruct_tglimg();
    void set_instruct_tglimg( bool set );

    // スレ表示の行間調整
    const double get_adjust_underline_pos();
    const double get_adjust_line_space();

    // スレ表示でリンクの下に下線を引く
    const bool get_draw_underline();

    // スレビューで文字幅の近似を厳密にする
    const bool get_strict_char_width();
    void set_strict_char_width( bool strictwidth );

    // datのパース時にURL判定を甘くする(^なども含める)
    const bool get_loose_url();

    // ユーザーコマンドで選択できない項目を非表示にする
    const bool get_hide_usrcmd();

    // 指定した数よりもユーザーコマンドが多い場合はサブメニュー化する
    const int get_max_show_usrcmd();

    // タブに表示する文字列の最小値
    const int get_tab_min_str();

    // タブにアイコンを表示するか
    const bool get_show_tab_icon();

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

    // デフォルトで透明、連鎖あぼーんをするか
    const bool get_abone_transparent();
    void set_abone_transparent( bool set );

    const bool get_abone_chain();
    void set_abone_chain( bool set );
}


#endif
