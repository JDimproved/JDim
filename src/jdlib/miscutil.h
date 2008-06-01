// ライセンス: GPL2

// 文字列関係の関数

#ifndef _MISCUTIL_H
#define _MISCUTIL_H

#include <string>
#include <list>


namespace MISC
{
    // URLスキームタイプ
	enum
	{
		SCHEME_NONE,

		SCHEME_HTTP,
		SCHEME_FTP,
		SCHEME_TTP,
		SCHEME_TP
	};

     // get_ucs2mode()の戻り値
     enum
     {
         UCS2MODE_BASIC_LATIN = 0,
         UCS2MODE_HIRA,
         UCS2MODE_KATA,

         UCS2MODE_OTHER
     };

    // str を "\n" ごとに区切ってlistにして出力
    std::list< std::string > get_lines( const std::string& str );

    // strを空白または "" 単位で区切って list で出力
    std::list< std::string > split_line( const std::string& str );

    // strを delimで区切って list で出力
    std::list< std::string > StringTokenizer( const std::string& str, char delim );

    // emacs lisp のリスト型を要素ごとにlistにして出力
    std::list< std::string > get_elisp_lists( const std::string& str );

    // list_inから空白行を除いてリストを返す
    std::list< std::string > remove_nullline_from_list( std::list< std::string >& list_in );

    // list_inの各行から前後の空白を除いてリストを返す
    std::list< std::string > remove_space_from_list( std::list< std::string >& list_in );

    // list_inからコメント行(#)を除いてリストを返す
    std::list< std::string > remove_commentline_from_list( std::list< std::string >& list_in );

    // 空白と""で区切られた str_in の文字列をリストにして出力
    // \"は " に置換される
    // (例)  "aaa" "bbb" "\"ccc\""  → aaa と bbb と "ccc"
    std::list< std::string > strtolist( std::string& str_in );

    // list_in の文字列リストを空白と""で区切ってストリングにして出力
    // "は \" に置換される
    // (例)  "aaa" "bbb" "\"ccc\""
    std::string listtostr( std::list< std::string >& list_in );

    // strの前後の空白削除
    std::string remove_space( const std::string& str );

    // str前後の改行、タブ、スペースを削除
    std::string remove_spaces( const std::string& str );

    // str1からstr2で示された文字列を除く
    std::string remove_str( const std::string& str1, const std::string& str2 );

    // start 〜 end の範囲をstrから取り除く ( /* コメント */ など )
    std::string remove_str( const std::string& str, const std::string& start, const std::string& end );

    // 正規表現を使ってstr1からqueryで示された文字列を除く
    std::string remove_str_regex( const std::string& str1, const std::string& query );

    // str1, str2 に囲まれた文字列を切り出す
    std::string cut_str( const std::string& str, const std::string& str1, const std::string& str2 );

    // str1 を str2 に置き換え
    std::string replace_str( const std::string& str, const std::string& str1, const std::string& str2 );

    // list_inから str1 を str2 に置き換えてリストを返す
    std::list< std::string > replace_str_list( std::list< std::string >& list_in, const std::string& str1, const std::string& str2 );

    // str_in に含まれる改行文字を replace に置き換え
    std::string replace_newlines_to_str( const std::string& str_in, const std::string& replace );

    // " を \" に置き換え
    std::string replace_quot( const std::string& str );

    // \" を " に置き換え
    std::string recover_quot( const std::string& str );

    // str 中に含まれている str2 の 数を返す
    int count_str( const std::string& str, const std::string& str2 );
    
    //文字 -> 整数変換
    int str_to_uint( const char* str, unsigned int& dig, unsigned int& n );

    // 数字　-> 文字変換
    std::string itostr( int n );

    // listで指定した数字を文字に変換
    std::string intlisttostr( std::list< int >& list_num );

    // 16進数表記文字をバイナリに変換する
    size_t chrtobin( const char* chr_in, char* chr_out );

    // strが半角でmaxsize文字を超えたらカットして後ろに...を付ける
    std::string cut_str( const std::string& str, unsigned int maxsize );

    // HTMLエスケープ
    std::string html_escape( const std::string& str, const bool include_url = true );

    // HTMLアンエスケープ
    std::string html_unescape( const std::string& str );

    // URL中のスキームを判別する
    int is_url_scheme( const char* str_in, int* length = NULL );

    // URLとして扱う文字かどうか判別する
    bool is_url_char( const char* str_in, const bool loose_url );

    // URLデコード
    std::string url_decode( const std::string& url );

    // urlエンコード
    std::string url_encode( const char* str, size_t n );

    // 文字コードを変換して url エンコード
    // str は UTF-8 であること
    std::string charset_url_encode( const std::string& str, const std::string& charset );

    // 文字コード変換して url エンコード
    // ただし半角スペースのところを+に置き換えて区切る
    std::string charset_url_encode_split( const std::string& str, const std::string& charset );

    // BASE64
    std::string base64( const std::string& str );

    // 文字コードを coding_from から coding_to に変換
    // 遅いので連続的な処理が必要な時は使わないこと
    std::string Iconv( const std::string& str, const std::string& coding_from, const std::string& coding_to );

    // utf-8 -> ucs2 変換
    int utf8toucs2( const char* utfstr, int& byte );

    // ucs2 の種類
    int get_ucs2mode( const int ucs2 );

    // ucs2 -> utf8 変換
    int ucs2toutf8( int ucs2, char* utfstr );

    // str を大文字化
    std::string toupper_str( const std::string& str );

    // list 内のアイテムを全部大文字化
    std::list< std::string > toupper_list( std::list< std::string >& list_str );
    
    //str を小文字化
    std::string tolower_str( const std::string& str );

    // path からホスト名だけ取り出す
    // protocol = false のときはプロトコルを除く
    std::string get_hostname( const std::string& path, bool protocol = true );

    // path からファイル名だけ取り出す
    std::string get_filename( const std::string& path );

    // path からファイル名を除いてディレクトリだけ取り出す
    std::string get_dir( const std::string& path );

    // SVNリビジョンとして表示する文字列を返す
    std::string get_svn_revision( const char* rev = NULL );

    // 文字数を限定して環境変数の値を返す
    std::string getenv_limited( const char *name, const size_t size = 1 );
}


#endif
