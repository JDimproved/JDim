// ライセンス: GPL2

// キャッシュ、ファイル操作まわり

#ifndef _CACHE_H
#define _CACHE_H

#include <glib.h>
#include <string>
#include <list>
#include <vector>
#include <ctime>

#ifdef _WIN32
#define ENV_HOME "USERPROFILE"
#else
#define ENV_HOME "HOME"
#endif

// GNU/Hurd doen't have PATH_MAX
#ifndef PATH_MAX
#define PATH_MAX 1024
#endif

// UTF-8からロケールでエンコードされた文字列に変換
#ifdef _WIN32
#define to_locale_cstr( path ) Glib::locale_from_utf8((path)).c_str()
#else
#define to_locale_cstr( path ) (path).c_str()
#endif

namespace Gtk
{
    class Window;
    class FileChooserDialog;
}

namespace CACHE
{
    /////////////////////////////////////////////////
    //
    // 設定ファイルのパス

    // 設定ファイル
    std::string path_conf();
    std::string path_conf_bkup();
    std::string path_conf_old();  // 旧ファイル

    // セッション情報ファイル
    std::string path_session();

    // ロックファイル
    std::string path_lock();

    // パスワード保存ファイル
    std::string path_passwd( const std::string& basename );

    // キャッシュルートの絶対パス
    std::string path_root();

    // 板
    std::string path_xml_listmain();
    std::string path_xml_listmain_bkup();
    std::string path_xml_listmain_old();

    // お気に入り
    std::string path_xml_favorite();
    std::string path_xml_favorite_bkup();
    std::string path_xml_favorite_old();

    // 外部板設定ファイル( navi2ch 互換 )
    std::string path_etcboard();

    // ユーザーコマンド設定ファイル
    std::string path_usrcmd();
    std::string path_usrcmd_old();

    // リンクフィルタ
    std::string path_linkfilter();

    // URL変換設定ファイル
    std::string path_urlreplace();

    // スレ履歴
    std::string path_xml_history();

    // 板履歴
    std::string path_xml_history_board();

    // 最近閉じたスレの履歴
    std::string path_xml_history_close();

    // 最近閉じた板の履歴
    std::string path_xml_history_closeboard();

    // 最近閉じた画像の履歴
    std::string path_xml_history_closeimg();

    // View履歴
    std::string path_xml_history_view();

    // 板移転情報
    std::string path_movetable();

    // キーボード設定
    std::string path_keyconf();

    // マウスジェスチャ設定
    std::string path_mouseconf();

    // マウスボタン設定
    std::string path_buttonconf();

    std::string path_board_root( const std::string& url );
    std::string path_board_root_fast( const std::string& boardbase );
    std::string path_article_summary( const std::string& url );
    std::string path_board_info( const std::string& url );
    std::string path_jdboard_info( const std::string& url );
    std::string path_article_info_root( const std::string& url );
    std::string path_article_info( const std::string& url, const std::string& id );
    std::string path_article_ext_info( const std::string& url, const std::string& id );        
    std::string path_dat( const std::string& url );

    // 画像関係
    std::string path_img_root();
    std::string path_img_info_root();
    std::string path_img_protect_root();
    std::string path_img_protect_info_root();
    std::string path_img_abone_root();
    std::string filename_img( const std::string& url );
    std::string filename_img_info( const std::string& url );
    std::string path_img( const std::string& url );
    std::string path_img_info( const std::string& url );
    std::string path_img_protect( const std::string& url );
    std::string path_img_protect_info( const std::string& url );
    std::string path_img_abone( const std::string& url );

    // AA
    std::string path_aalist(); // アスキーアートファイル
    std::string path_aadir();  // アスキーアートファイル格納用ディレクトリ .jd/aa/
    std::string path_aahistory();  // AAの使用履歴ファイル

    // テーマのルートパス
    const std::string path_theme_root();

    // アイコンテーマのルートパス
    const std::string path_theme_icon_root();

    // css
    std::string path_css();
    std::string path_css_old();

    // html
    std::string path_reshtml();
    std::string path_reshtml_old();

    // ログ
    std::string path_logroot();
    std::string path_postlog(); // 書き込みログ
    std::string path_msglog(); // メッセージログ（-lオプション）

    // 検索や名前などの補完情報
    std::string path_completion( int mode );

    // サウンドファイルのルートパス
    std::string path_sound_root();

    /////////////////////////////////////////////////
    //
    // ユーティリティ関数

    // file_exists の戻り値
    enum
    {
        EXIST_FILE = 0,  // ファイル
        EXIST_DIR,       // ディレクトリ
        EXIST_FIFO,      // FIFO
        EXIST,           // 存在しない or 何か存在してる
        EXIST_ERROR      // エラー
    };

    // ファイルタイプ( file_open_diag() で使用 )
    enum
    {
        FILE_TYPE_ALL = 0,
        FILE_TYPE_TEXT,
        FILE_TYPE_DAT
    };

    // キャッシュの mkdir 関係
    bool mkdir_root();
    bool mkdir_imgroot();
    bool mkdir_imgroot_favorite();
    bool mkdir_parent_of_board( const std::string& url );
    bool mkdir_boardroot( const std::string& url );
    bool mkdir_logroot();

    // 生データ読み書き
    size_t load_rawdata( const std::string& path, std::string& str );
    size_t load_rawdata( const std::string& path, char* data, const size_t n );    
    size_t save_rawdata( const std::string& path, const std::string& str, const bool append = false );
    size_t save_rawdata( const std::string& path, const char* data, const size_t n, const bool append = false );

    // ファイル情報
    const int file_exists( const std::string& path );
    size_t get_filesize( const std::string& path );
    time_t get_filemtime( const std::string& path );
    const bool set_filemtime( const std::string& path, const time_t mtime );

    // ファイル操作
    bool jdmkdir( const std::string& path );
    bool jdcopy( const std::string& file_from, const std::string& file_to );
    bool jdmv( const std::string& file_from, const std::string& file_to );

    // 保存ダイアログを表示して file_from を file_to に保存する
    std::string copy_file( Gtk::Window* parent, const std::string& file_from, const std::string& file_to, const int type );

    // ファイル選択ダイアログにフィルタ追加
    void add_filter_to_diag( Gtk::FileChooserDialog& diag, const int type );

    // ファイル選択ダイアログを表示する
    // parent == NULL の時はメインウィンドウをparentにする
    // open_path はデフォルトの参照先
    // multi == true なら複数選択可能
    // 戻り値は選択されたファイルのpathのリスト
    std::vector< std::string > open_load_diag( Gtk::Window* parent, const std::string& open_path, const int type, const bool multi );

    // 保存ファイル選択ダイアログを表示する
    std::string open_save_diag( Gtk::Window* parent, const std::string& dir, const std::string& name, const int type );

    // dir ディレクトリ内のレギュラーファイルのリストを取得
    std::list< std::string > get_filelist( const std::string& dir );

    // dir ディレクトリ内のレギュラーファイルの合計サイズを取得
    guint64 get_dirsize( const std::string& dir );

    // 相対パスから絶対パスを取得してファイルが存在すれば絶対パスを返す
    // ファイルが存在しない場合は std::string() を返す
    const std::string get_realpath( const std::string& path );
}

#endif
