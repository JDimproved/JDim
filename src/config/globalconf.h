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

    // 各ビューと枠との間の余白
    const int get_view_margin();

    // フォント
    const std::string& get_fontname( int id );
    void set_fontname( int id, const std::string& fontname );

    // レスを参照するときに前に付ける文字
    const std::string& get_ref_prefix();

    // JD ホームページのアドレス
    const std::string& get_url_jdhp();

    // 2chの認証サーバ
    const std::string& get_url_login2ch();

    // bbsmenu.htmlのURL
    const std::string& get_url_bbsmenu();    

    // bbsmenu.html内にあるリンクは全て板とみなす
    const bool use_link_as_board();

    // スレタイ検索用メニュータイトルアドレス
    const std::string& get_menu_search_title();
    const std::string& get_url_search_title();

    // スレタイ検索用正規表現
    const std::string& get_regex_search_title();

    // WEB検索用メニュータイトルアドレス
    const std::string& get_menu_search_web();
    const std::string& get_url_search_web();

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
    const int get_loader_timeout_checkupdate();

    // ipv6使用
    const bool get_use_ipv6();
    void set_use_ipv6( bool set );

    // リンクをクリックしたときに実行するコマンド
    const std::string& get_command_openurl();
    void set_command_openurl( const std::string& command );

    // ブラウザ設定ダイアログのコンボボックスの番号
    const int get_browsercombo_id();
    void set_browsercombo_id( int id );

    // レス番号の上にマウスオーバーしたときに参照ポップアップ表示する
    const bool get_refpopup_by_mo();

    // 名前の上にマウスオーバーしたときにポップアップ表示する
    const bool get_namepopup_by_mo();

    // IDの上にマウスオーバーしたときにIDをポップアップ表示する
    const bool get_idpopup_by_mo();

    // 画像ポップアップサイズ
    const int get_imgpopup_width();
    const int get_imgpopup_height();

    // 画像ポップアップを使用する
    const bool get_use_image_popup();
    void set_use_image_popup( bool use );

    // 画像ビューを使用する
    const bool get_use_image_view();
    void set_use_image_view( bool image_view );

    // インライン画像表示をする
    const bool get_use_inline_image();
    void set_use_inline_image( bool inline_img );

    // 画像にモザイクかける
    const bool get_use_mosaic();
    void set_use_mosaic( bool mosaic );

    // 画像をデフォルトでウィンドウサイズに合わせる
    const bool get_zoom_to_fit();
    void set_zoom_to_fit( bool fit );

    // 画像キャッシュ削除の日数
    const int get_del_img_day();
    void set_del_img_day( int day );

    // 画像あぼーん削除の日数
    const int get_del_imgabone_day();
    void set_del_imgabone_day( int day );

    // ダウンロードする画像の最大ファイルサイズ(Mbyte)
    const int get_max_img_size();

    // 画像の最大サイズ(Mピクセル)
    const int get_max_img_pixel();

    // スレ一覧にdat落ちしたスレも表示する
    const bool get_show_oldarticle();
    void set_show_oldarticle( bool showarticle );

    // スレ一覧で指定した値(時間)よりも後に立てられたスレを新着とみなす
    const int get_newthread_hour();

    // スレ一覧でインクリメント検索をする
    const bool get_inc_search_board();

    // ツリービューのスクロール量(マウスホイール上下・行数)
    const int get_tree_scroll_size();

    // スレビューのスクロール量(マウスホイール上下・行数)
    const int get_scroll_size();

    // スレビューのスクロール量(キー上下・行数)
    const int get_key_scroll_size();

    // スレビューの高速スクロール量(キー上下・ ページ高 - 行高 * key_fastscroll_size )
    const int get_key_fastscroll_size();

    // スレビューでリロード後に一番下までスクロール
    const bool get_jump_after_reload();
    void set_jump_after_reload( bool set );

    // スレビューでリロード後に新着までスクロール
    const bool get_jump_new_after_reload();
    void set_jump_new_after_reload( bool set );

    // 実況モード
    const int get_live_mode();
    void set_live_mode( const int mode );

    // 実況速度
    const int get_live_speed();
    void set_live_speed( const int speed );

    // 実況のスクロールモードを切り替えるしきい値
    const int get_live_threshold();
    void set_live_threshode( const int th );

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

    // 非アクティブ時に書き込みビューを折りたたむ
    const bool get_fold_message();
    void set_fold_message( bool fold );

    // ポップアップとカーソルの間のマージン
    const int get_margin_popup();
    void set_margin_popup( int margin );

    // 画像ポップアップとカーソルの間のマージン
    const int get_margin_imgpopup();

    // マウスジェスチャの判定開始半径
    const int get_mouse_radius();

    // 履歴の保持数
    const int get_history_size();

    // AA履歴の保持数
    const int get_aahistory_size();

    // 0以上なら多重ポップアップの説明を表示する
    const int get_instruct_popup();

    // スレビューを開いたときにスレ一覧との切り替え方法を説明する
    const bool get_instruct_tglart();
    void set_instruct_tglart( bool set );

    // 画像ビューを開いたときにスレビューとの切り替え方法を説明する
    const bool get_instruct_tglimg();
    void set_instruct_tglimg( bool set );

    // 下線位置調整
    const double get_adjust_underline_pos();
    void set_adjust_underline_pos( double pos );

    // スレ表示の行間調整
    const double get_adjust_line_space();
    void set_adjust_line_space( double space );

    // スレ表示でリンクの下に下線を引く
    const bool get_draw_underline();

    // スレビューで文字幅の近似を厳密にする
    const bool get_strict_char_width();
    void set_strict_char_width( bool strictwidth );

    // スレビューで発言数(ID)をカウントする
    const bool get_check_id();

    // レス参照で色を変える回数
    const int get_num_reference_high();
    const int get_num_reference_low();

    // 発言数で色を変える回数
    const int get_num_id_high();
    const int get_num_id_low();

    // datのパース時にURL判定を甘くする(^なども含める)
    const bool get_loose_url();

    // ユーザーコマンドで選択できない項目を非表示にする
    const bool get_hide_usrcmd();

    // 指定した数よりもユーザーコマンドが多い場合はサブメニュー化する
    const int get_max_show_usrcmd();

    // スレビューで再読み込みボタンを押したときに全タブを更新する
    const bool get_reload_allthreads();

    // タブに表示する文字列の最小値
    const int get_tab_min_str();

    // タブにアイコンを表示するか
    const bool get_show_tab_icon();

    // ボタンをフラットにするか
    const bool get_flat_button();
    void set_flat_button( const bool set );

    // boardviewでのスレの全体あぼーん
    std::list< std::string >& get_list_abone_word_thread(); // ワード
    std::list< std::string >& get_list_abone_regex_thread(); // 正規表現

    void set_list_abone_word_thread( std::list< std::string >& word );
    void set_list_abone_regex_thread( std::list< std::string >& regex );

    const int get_abone_number_thread();
    void set_abone_number_thread( const int number );

    const int get_abone_hour_thread();
    void set_abone_hour_thread( const int hour );

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

    // 右ペーンが空の時にサイドバーを閉じるか
    const bool get_expand_sidebar();

#ifdef HAVE_MIGEMO_H
    // migemo-dictの場所
    const std::string& get_migemodict_path();
#endif
}


#endif
