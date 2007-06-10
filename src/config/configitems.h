// ライセンス: GPL2
//
// 設定項目クラス
//

#ifndef _CONFIGITEMS_H
#define _CONFIGITEMS_H

#include <string>
#include <list>
#include <vector>

namespace CONFIG
{
    class ConfigItems
    {

    public:

        // 前回開いたviewを復元するか
        bool restore_board;
        bool restore_article;
        bool restore_image;

        // フォント
        std::vector< std::string > fontname;

        // レスを参照するときに前に付ける文字
        std::string ref_prefix;

        // ref_prefix の後のスペースの数
        int ref_prefix_space;

        // キャッシュのルートディレクトリ
        // 旧バージョンとの互換のため残しているだけで使用していない
        std::string path_cacheroot;

        // 読み込み用プロクシとポート番号
        bool use_proxy_for2ch;
        std::string proxy_for2ch;
        int proxy_port_for2ch;

        // 書き込み用プロクシとポート番号
        bool use_proxy_for2ch_w;
        std::string proxy_for2ch_w;
        int proxy_port_for2ch_w;

        // 2chの外にアクセスするときのプロクシとポート番号
        bool use_proxy_for_data;
        std::string proxy_for_data;
        int proxy_port_for_data;

        // 2ch にアクセスするときのエージェント名
        std::string agent_for2ch;

        // 2ch外にアクセスするときのエージェント名
        std::string agent_for_data;

        // 2ch にログインするときのX-2ch-UA
        std::string x_2ch_ua;

        // ローダのバッファサイズ
        int loader_bufsize;

        // ローダのタイムアウト値
        int loader_timeout;
        int loader_timeout_post;
        int loader_timeout_img;

        // ipv6使用
        bool use_ipv6;

        // リンクをクリックしたときに実行するコマンド
        std::string command_openurl;

        // ブラウザ設定ダイアログのコンボボックスの番号
        int browsercombo_id;

        // レス番号の上にマウスオーバーしたときに参照ポップアップ表示する
        bool refpopup_by_mo;

        // 名前の上にマウスオーバーしたときにポップアップ表示する
        bool namepopup_by_mo;

        // IDの上にマウスオーバーしたときにIDをポップアップ表示する
        bool idpopup_by_mo;

        // 画像ポップアップサイズ
        int imgpopup_width;
        int imgpopup_height;

        // 画像ビューを使用する
        bool use_image_view;

        // インライン画像を表示する
        bool use_inline_image;

        // 画像にモザイクかける
        bool use_mosaic;

        // 画像をデフォルトでウィンドウサイズに合わせる
        bool zoom_to_fit;

        // 画像キャッシュ削除の日数
        int del_img_day;

        // ダウンロードする画像の最大サイズ(Mbyte)        
        int max_img_size;

        // JD ホームページのアドレス
        std::string url_jdhp;

        // 2chの認証サーバ
        std::string url_login2ch;

        // bbsmenu.htmlのURL
        std::string url_bbsmenu;

        // 色
        std::vector< std::string > str_color;

        // ツリービューでgtkrcの設定を使用するか
        bool use_tree_gtkrc;

        // ツリービューの行間スペース
        int tree_ypad;

        // boardビューで古いスレも表示
        bool show_oldarticle;

        // スレ一覧で指定した値(時間)よりも後に立てられたスレを新着とみなす
        int newthread_hour;

        // ツリービューのスクロール量(行数)
        int tree_scroll_size;

        // スレビューのスクロール量
        int scroll_size;

        // スレビューのスクロール量(キー上下)
        int key_scroll_size;

        // 板一覧でカテゴリを常にひとつだけ開く
        bool open_one_category;

        // 書き込み時に書き込み確認ダイアログを出すかどうか
        bool always_write_ok;

        // 書き込みログを保存
        bool save_postlog;

        // 「書き込み中」のダイアログを表示しない
        bool hide_writing_dialog;

        // ポップアップとカーソルの間のマージン
        int margin_popup;

        // 画像ポップアップとカーソルの間のマージン
        int margin_imgpopup;

        // マウスジェスチャの判定開始半径
        int mouse_radius;

        // 履歴の保持数        
        int history_size;

        // 0以上なら多重ポップアップの説明を表示する
        int instruct_popup;

        // スレビューを開いたときにスレ一覧との切り替え方法を説明する
        bool instruct_tglart;
        bool instruct_tglart_end;

        // 画像ビューを開いたときにスレビューとの切り替え方法を説明する        
        bool instruct_tglimg;
        bool instruct_tglimg_end;

        // スレ表示の行間調整
        double adjust_underline_pos;
        double adjust_line_space;

        // リンク下線を表示
        bool draw_underline;

        // スレビューで文字幅の近似を厳密にする        
        bool strict_char_width;

        // datのパース時にURL判定を甘くする(^なども含める)
        bool loose_url;

        // ユーザーコマンドで選択できない項目を非表示にする
        bool hide_usrcmd;

        // 指定した数よりもユーザーコマンドが多い場合はサブメニュー化する
        int max_show_usrcmd;

        // スレビューで再読み込みボタンを押したときに全タブを更新する
        bool reload_allthreads;

        // タブに表示する文字列の最小値        
        int tab_min_str;

        // タブにアイコンを表示するか
        bool show_tab_icon;

        // スレ あぼーん word
        std::list< std::string > list_abone_word_thread;

        // スレ あぼーん regex
        std::list< std::string > list_abone_regex_thread;

        // あぼーん name
        std::list< std::string > list_abone_name;

        // あぼーん word
        std::list< std::string > list_abone_word;

        // あぼーん regex        
        std::list< std::string > list_abone_regex;

        // デフォルトで透明、連鎖あぼーんをするか
        bool abone_transparent;
        bool abone_chain;

        // 右ペーンが空の時にサイドバーを閉じるか
        bool expand_sidebar;

#ifdef HAVE_MIGEMO_H
        // migemo-dictの場所
        std::string migemodict_path;
#endif

        /////////////////////////


        ConfigItems();
        virtual ~ConfigItems();

        // 設定読み込み
        const bool load();

        // 保存
        void save();

      private:

        void save_impl( const std::string& path );
    };
    
}

#endif
