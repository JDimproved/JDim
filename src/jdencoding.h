// SPDX-License-Identifier: GPL-2.0-only
/** @file jdencoding.h
 *
 * @details 文字エンコーディングを表す列挙型
 */

#ifndef JDENCODING_H
#define JDENCODING_H

/** @brief 文字エンコーディングを表す列挙型
 *
 * @details 日本語の文章に使われている文字エンコーディングをリストする
 */
enum class Encoding
{
    unknown = 0, ///< 不明. エンコーディング名は LATIN1 (ISO-8859-1) として扱う
    ascii, ///< ASCII
    eucjp, ///< WindowsのShift_JISに合わせたEUC-JP (EUCJP-MS)
    jis,   ///< JISコード (ISO-2022-JP)
    sjis,  ///< WindowsのShift_JIS (MS932)
    utf8,  ///< UTF-8
};


/** @brief テキストエンコーディングを判定する方法を表す定数をまとめた構造体
 *
 * @details 0 より小さい値は無効な値にするため定義しない
 */
struct EncodingAnalysisMethod
{
    static constexpr const int use_default = 0; ///< デフォルト設定を使う
    static constexpr const int http_header = 1; ///< HTTPヘッダーのエンコーディング情報を使う
    static constexpr const int guess = 2; ///< テキストからエンコーディングを推測する
    static constexpr const int max = guess;
};

#endif
