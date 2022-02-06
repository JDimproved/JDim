// ライセンス: GPL2

#ifndef _JDREGEX_H
#define _JDREGEX_H

#include "span.h"

#include <map>
#include <string>
#include <vector>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <glib.h>


namespace JDLIB
{
    class Regex;

    class RegexPattern
    {
        friend class Regex;

        GRegex* m_regex{};
        bool m_compiled{};
        bool m_newline{};
        bool m_wchar{};
        GError *m_error{};

    public:
        RegexPattern() noexcept = default;
        RegexPattern( const std::string& reg, const bool icase, const bool newline,
                      const bool usemigemo = false, const bool wchar = false );
        ~RegexPattern() noexcept;

        // regex_tを複製する方法がないためcopy禁止にする
        RegexPattern( const RegexPattern& ) = delete;
        RegexPattern& operator=( const RegexPattern& ) = delete;
        // moveは許可
        RegexPattern( RegexPattern&& ) noexcept;
        RegexPattern& operator=( RegexPattern&& ) noexcept;

        bool set( const std::string& reg, const bool icase, const bool newline,
                  const bool usemigemo = false, const bool wchar = false );
        void clear();
        bool compiled() const noexcept { return m_compiled; }
        std::string errstr() const;
    };


    class Regex
    {
        std::vector<int> m_pos;
        std::vector<std::string> m_results;
        std::map<std::string, int> m_named_numbers; // グループ名(名前付きキャプチャ)からグループ番号を取得する

        // 全角半角を区別しないときに使う変換用バッファ
        // 処理可能なバッファ長は regoff_t (= int) のサイズに制限される
        std::string m_target_asc;
        std::vector<int> m_table_pos;

    public:

        Regex() noexcept = default;
        ~Regex() noexcept = default;

        // notbol : 行頭マッチは必ず失敗する
        // noteol : 行末マッチは必ず失敗する
        bool match( const RegexPattern& creg, const std::string& target, const std::size_t offset,
                    const bool notbol = false, const bool noteol = false );

        // named_captures : 後で利用する名前付きキャプチャを登録する
        bool match( const RegexPattern& creg, const std::string& target, const std::size_t offset,
                    const bool notbol, const bool noteol,
                    JDLIB::span<const std::string> named_captures );

        // icase : 大文字小文字区別しない
        // newline :  . に改行をマッチさせない
        // usemigemo : migemo使用 (コンパイルオプションで指定する必要あり)
        // wchar : 全角半角の区別をしない
        bool exec( const std::string& reg, const std::string& target, const std::size_t offset,
                   const bool icase, const bool newline, const bool usemigemo = false,
                   const bool wchar = false ) {
            RegexPattern pattern( reg, icase, newline, usemigemo, wchar );
            return match( pattern, target, offset );
        }

        // マッチした文字列と \0〜\9 を置換する
        std::string replace( const std::string& repstr ) const;

        int length( std::size_t num ) const noexcept;
        int pos( std::size_t num ) const noexcept;
        std::string str( std::size_t num ) const;
        // 名前付きキャプチャ版の取得APIは実際に使う箇所がないため未実装

        // パターン中に名前が有れば名前付きキャプチャ、無ければグループ番号で取得する
        // どちらにもマッチしなかったときは空文字列を返す
        std::string named_or_num( const std::string& name, std::size_t fallback_num ) const;
    };
}

#endif
