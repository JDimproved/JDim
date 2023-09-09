// ライセンス: GPL2

// 文字列関係の関数

#ifndef _MISCUTIL_H
#define _MISCUTIL_H

#include "jdencoding.h"

#include <glibmm.h>

#include <cstring>
#include <list>
#include <string>
#include <string_view>
#include <vector>

namespace MISC
{
    // URLスキームタイプ
	enum
	{
		SCHEME_NONE,

		SCHEME_HTTP,
		SCHEME_FTP,
		SCHEME_TTP,
		SCHEME_TP,
                SCHEME_SSSP
	};

     // parse_html_form_data() の戻り値
     struct FormDatum
     {
         std::string name;
         std::string value;

         bool operator==( const FormDatum& rhs ) const { return name == rhs.name && value == rhs.value; }
     };

    // str を "\n" ごとに区切ってlistにして出力
    std::list< std::string > get_lines( const std::string& str );

    // strを空白または "" 単位で区切って list で出力
    std::list< std::string > split_line( const std::string& str );

    // strを delimで区切って list で出力
    std::list< std::string > StringTokenizer( const std::string& str, const char delim );

    // emacs lisp のリスト型を要素ごとにlistにして出力
    std::list< std::string > get_elisp_lists( const std::string& str );

    // list_inから空白行を除いてリストを返す
    std::list< std::string > remove_nullline_from_list( const std::list< std::string >& list_in );

    // list_inの各行から前後の空白を除いてリストを返す
    std::list< std::string > remove_space_from_list( const std::list< std::string >& list_in );

    // list_inからコメント行(#)を除いてリストを返す
    std::list< std::string > remove_commentline_from_list( const std::list< std::string >& list_in );

    // 空白と""で区切られた str_in の文字列をリストにして出力
    // \"は " に置換される
    // (例)  "aaa" "bbb" "\"ccc\""  → aaa と bbb と "ccc"
    std::list< std::string > strtolist( const std::string& str_in );

    // list_in の文字列リストを空白と""で区切ってストリングにして出力
    // "は \" に置換される
    // (例)  "aaa" "bbb" "\"ccc\""
    std::string listtostr( const std::list< std::string >& list_in );

    // list_in から空文字列を除き suffix でつなげて返す
    // 他のプログラミング言語にあるjoin()と動作が異なり返り値の末尾にもsuffixが付く
    // (例) {"aa", "", "bb", "cc"}, '!' -> "aa!bb!cc!"
    std::string concat_with_suffix( const std::list<std::string>& list_in, char suffix );

    /// str前後の半角スペース(U+0020)と全角スペース(U+3000)を削除
    std::string utf8_trim( std::string_view str );

    /// str前後の改行(\\r, \\n)、タブ(\\t)、スペース(U+0020)を削除
    std::string ascii_trim( const std::string& str );

    /// strからpatternで示された文字列を除く
    std::string remove_str( std::string_view str, std::string_view pattern );

    /// start 〜 end の範囲をstrから取り除く ( /* コメント */ など )
    std::string remove_str( std::string_view str, std::string_view start, std::string_view end );

    // 正規表現を使ってstr1からqueryで示された文字列を除く
    std::string remove_str_regex( const std::string& str1, const std::string& query );

    /// front_sep, back_sep に囲まれた文字列を切り出す
    std::string cut_str( const std::string& str, std::string_view front_sep, std::string_view back_sep );

    /// pattern を replacement に置き換える
    std::string replace_str( std::string_view str, std::string_view pattern, std::string_view replacement );

    /// 文字列をコピーし部分文字列 old を new_ に置換して返す (ASCIIだけignore case)
    std::string replace_casestr( const std::string& str, const std::string& old, const std::string& new_ );

    /// list_inから pattern を replacement に置き換えてリストを返す
    std::list<std::string> replace_str_list( const std::list<std::string>& list_in,
                                             std::string_view pattern, std::string_view replacement );

    /// str に含まれる改行文字(`\r\n`)を replace に置き換え
    std::string replace_newlines_to_str( const std::string& str, std::string_view replace );

    // " を \" に置き換え
    std::string replace_quot( const std::string& str );

    // \" を " に置き換え
    std::string recover_quot( const std::string& str );

    // 文字列(utf-8も) -> 整数変換
    // (例) "12３" -> 123
    // 入力: str
    // 出力:
    // dig: 桁数、0なら失敗
    // n : str から何バイト読み取ったか
    // 戻り値: 数値
    int str_to_uint( const char* str, size_t& dig, size_t& n );

    // listで指定した数字を文字に変換
    std::string intlisttostr( const std::list< int >& list_num );

    // 16進数表記文字をバイナリに変換する( 例 "E38182" -> 0xE38182 )
    // 出力 : char_out 
    // 戻り値: 変換に成功した chr_in のバイト数
    std::size_t chrtobin( std::string_view chr_in, char* chr_out );

    // strが半角でmaxsize文字を超えたらカットして後ろに...を付ける
    std::string cut_str( const std::string& str, const unsigned int maxsize );

    // 正規表現のメタ文字が含まれているか
    // escape == true ならエスケープを考慮 (例)  escape == true なら \+ → \+ のまま、falseなら \+ → \\\+
    bool has_regex_metachar( const std::string& str, const bool escape );

    // 正規表現のメタ文字をエスケープ
    // escape == true ならエスケープを考慮 (例)  escape == true なら \+ → \+ のまま、falseなら \+ → \\\+
    std::string regex_escape( const std::string& str, const bool escape );

    // 正規表現のメタ文字をアンエスケープ
    std::string regex_unescape( const std::string& str );

    // HTMLで特別な意味を持つ記号(& " < >)を文字実体参照へエスケープする
    // completely : URL中でもエスケープする( デフォルト = true )
    std::string html_escape( const std::string& str, const bool completely = true );

    // HTMLで特別な意味を持つ記号の文字実体参照(&quot; &amp; &lt; &gt;)をアンエスケープする
    std::string html_unescape( const std::string& str );

    // HTMLをプレーンテキストに変換する
    std::string to_plain( const std::string& html );

    // HTMLをPango Markupテキストに変換する
    std::string to_markup( const std::string& html );

    // HTML文字参照をデコード( completely=trueの場合は '&' '<' '>' '"' を含める )
    std::string chref_decode( std::string_view str, const bool completely = true );

    // URL中のスキームを判別する
    // 戻り値 : スキームタイプ
    // length    : "http://"等の文字数
    int is_url_scheme( const char* str_in, int* length = nullptr );
    int is_url_scheme_impl( const char* str_in, int* length );

    // URLとして扱う文字かどうか判別する
    // 基本 : 「!#$%&'()*+,-./0-9:;=?@A-Z_a-z~」
    // 拡張 : 「[]^|」
    //
    // "RFC 3986" : http://www.ietf.org/rfc/rfc3986.txt
    // "RFC 2396" : http://www.ietf.org/rfc/rfc2396.txt
    bool is_url_char( const char* str_in, const bool loose_url );

    /// URLに含まれるパーセントエンコーディングをバイト列にデコードする
    std::string url_decode( std::string_view url );

    /// 文字列(バイト列)をパーセント符号化して返す
    std::string url_encode( std::string_view str );

    /// UTF-8文字列をエンコーディング変換してからパーセント符号化して返す
    std::string url_encode( const std::string& utf8str, const Encoding encoding );

    /// application/x-www-form-urlencoded の形式でパーセント符号化する
    std::string url_encode_plus( std::string_view str );

    /// UTF-8文字列をエンコーディング変換してから application/x-www-form-urlencoded の形式でパーセント符号化する
    std::string url_encode_plus( const std::string& utf8str, const Encoding encoding );

    // BASE64
    std::string base64( const std::string& str );

    // 「&#数字;」形式の数字参照文字列の中の「数字」部分の文字列長
    //
    // in_char: 入力文字列、in_char[0] == "&" && in_char[1] == "#" であること
    // offset : 開始位置が返る
    //
    // 戻り値 : 「&#数字;」の中の数字の文字列の長さ、変換出来ないときは -1
    //
    // 例 : &#9999; なら 戻り値 = 4、 offset = 2
    //
    int spchar_number_ln( const char* in_char, int& offset );

    // 「&#数字;」形式の数値文字参照をコードポイント(char32_t)に変換する
    char32_t decode_spchar_number_raw( const char* in_char, const int offset, const int lng );

    // @brief 「&#数字;」形式の数値文字参照をコードポイント(char32_t)に変換する
    //
    // @details 数値文字参照の解析エラーとなる値は規定の値に変換して返す
    // @param[in] in_char 入力文字列、 `in_char[0] == "&" && in_char[1] == "#"` であること (not null)
    // @param[in] offset  spchar_number_ln() の戻り値
    // @param[in] lng     spchar_number_ln() の戻り値
    // @return 「&#数字;」の中の数字(char32_t型)
    // @remarks 最初に MISC::spchar_number_ln() を呼び出して `offset` と `lng` を取得すること
    //
    char32_t decode_spchar_number( const char* in_char, const int offset, const int lng );

    // コードポイントが数値文字参照の無効・解析エラーなら規定の値へ変換する
    char32_t sanitize_numeric_charref( const char32_t uch, char32_t* high_surrogate = nullptr );

    // str に含まれる「&#数字;」形式の数字参照文字列を全てユニーコード文字に変換する
    std::string decode_spchar_number( const std::string& str );

    // str を大文字化
    std::string toupper_str( const std::string& str );

    // list 内のアイテムを全部大文字化
    std::list< std::string > toupper_list( const std::list< std::string >& list_str );
    
    //str を小文字化
    std::string tolower_str( const std::string& str );

    // path からホスト名だけ取り出す
    // protocol = false のときはプロトコルを除く
    std::string get_hostname( const std::string& path, const bool protocol = true );

    // path からファイル名だけ取り出す
    std::string get_filename( const std::string& path );

    // path からファイル名を除いてディレクトリだけ取り出す
    std::string get_dir( const std::string& path );

    // 文字数を限定して環境変数の値を返す
    std::string getenv_limited( const char *name, const size_t size = 1 );

    // pathセパレータを / に置き換える
    std::string recover_path( const std::string& str );
    std::vector< std::string > recover_path( std::vector< std::string > list_str );

    // 文字列(utf-8)に全角英数字が含まれるか判定する
    bool has_widechar( const char* str );

    // 全角英数字(str1) -> 半角英数字(str2)
    // table_pos : 置き換えた文字列の位置
    void asc( const char* str1, std::string& str2, std::vector< int >& table_pos );

    // UTF-8文字列(str1) -> 正規化文字列(str2)
    // table_pos : 置き換えた文字列の位置
    void norm( const char* str1, std::string& str2, std::vector<int>* table_pos = nullptr );


    // URL中のスキームを判別する
    inline int is_url_scheme( const char* str_in, int* length )
    {
        // 候補になり得ない場合は以降の処理はしない
        if( *str_in != 'h' && *str_in != 'f' && *str_in != 't' && *str_in != 's' )
            return SCHEME_NONE;

        return is_url_scheme_impl( str_in, length );
    }

    // selfの先頭部分がstartsと等しいか（ヌル終端文字列バージョン）
    // Unicode正規化は行わなずバイト列として比較する
    bool starts_with( const char* self, const char* starts );

    /** @brief haystackの末尾がneedleと一致するか
     *
     * @param[in] haystack 末尾をチェックする
     * @param[in] needle   haystackの末尾と一致するか
     * @retval true  末尾が一致した、またはneedleが空文字列の場合
     * @retval false 末尾が不一致、またはhaystackがneedleより短い場合
     */
    constexpr bool ends_with( std::string_view haystack, std::string_view needle )
    {
        return haystack.size() >= needle.size()
            && haystack.compare( haystack.size() - needle.size(), needle.size(), needle ) == 0;
    }

    // HTMLからform要素を解析してinput,textarea要素の名前と値を返す
    std::vector<FormDatum> parse_html_form_data( const std::string& html );

    // HTMLのform要素から action属性(送信先URLのパス) を取得する
    // 2ch互換板に特化して実装しているため他の掲示板で期待した結果を返す保証はない
    // 詳細は実装やテストコードを参照
    std::string parse_html_form_action( const std::string& html );

    // HTMLのmeta要素からテキストのエンコーディング(charset)を取得する
    std::string parse_charset_from_html_meta( const std::string& html );

    // haystack の pos 以降から最初に needle と一致する位置を返す (ASCIIだけignore case)
    // 見つからない場合は std::string::npos を返す、needle が空文字列なら pos を返す
    std::size_t ascii_ignore_case_find( const std::string& haystack, std::string_view needle, std::size_t pos = 0 );
}


#endif

