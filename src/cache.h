// ライセンス: 最新のGPL

// キャッシュ、ファイル操作まわり

#ifndef _CACHE_H
#define _CACHE_H

#include <string>
#include <list>

namespace CACHE
{
    /////////////////////////////////////////////////
    //
    // 設定ファイルのパス

    // 設定ファイル
    std::string path_conf();

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

    // お気に入り
    std::string path_xml_favorite();
    std::string path_xml_favorite_bkup();

    // 外部板設定ファイル( navi2ch 互換 )
    std::string path_etcboard();

    // ユーザーコマンド設定ファイル
    std::string path_usrcmd();

    // 履歴
    std::string path_xml_history();

    // 板履歴
    std::string path_xml_history_board();

    // 板移転情報
    std::string path_movetable();

    // キーボード設定
    std::string path_keyconf();

    // マウスジェスチャ設定
    std::string path_mouseconf();

    // マウスボタン設定
    std::string path_buttonconf();

    std::string path_board_root( const std::string& url );
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
    std::string filename_img( const std::string& url );
    std::string path_img( const std::string& url );
    std::string path_img_info( const std::string& url );


    /////////////////////////////////////////////////
    //
    // ユーティリティ関数

    // is_file_exists の戻り値
    enum
    {
        EXIST_FILE = 0,  // ファイル
        EXIST_DIR,       // ディレクトリ
        EXIST,           // 存在しない or 何か存在してる
        EXIST_ERROR      // エラー
    };

    // キャッシュの mkdir 関係
    bool mkdir_root();
    bool mkdir_imgroot();
    bool mkdir_parent_of_board( const std::string& url );
    bool mkdir_boardroot( const std::string& url );

    // 生データ読み書き
    size_t load_rawdata( const std::string& path, std::string& str );
    size_t load_rawdata( const std::string& path, char* data, size_t n );    
    bool save_rawdata( const std::string& path, const std::string& str );
    bool save_rawdata( const std::string& path, const char* data, size_t n );

    // ファイル情報
    long is_file_exists( const std::string& path );
    size_t get_filesize( const std::string& path );
    time_t get_filemtime( const std::string& path );

    // ファイル操作
    bool jdmkdir( const std::string& path );
    bool jdcopy( const std::string& file_from, const std::string& file_to );
    std::string open_save_diag( const std::string& file_from, const std::string& file_to );

    // dir ディレクトリ内のレギュラーファイルのリストを取得
    std::list< std::string > get_filelist( const std::string& dir );

    // dir ディレクトリ内のレギュラーファイルの合計サイズを取得
    int64_t get_dirsize( const std::string& dir );
}

#endif
