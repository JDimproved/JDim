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

#endif
