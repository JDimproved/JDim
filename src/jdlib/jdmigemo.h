// ライセンス: GPL2

//
// ローマ字入力を日本語検索のための正規表現に変換する
// Thanks to 「テスト運用中」スレの18氏
//
// http://jd4linux.sourceforge.jp/cgi-bin/bbs/test/read.cgi/support/1149945056/18
//

#ifndef JD_MIGEMO_H
#define JD_MIGEMO_H

#ifdef HAVE_MIGEMO_H

#include <string>

namespace jdmigemo {

// inputのヌル文字'\0'または改行'\n'までを変換して返す
std::string convert(const char* input);

// 辞書を読み込めたらtrueを返す
// 既に読み込んでる場合は何もせずtrueを返す
bool init(const std::string& filename);

void close();

} // namespace jdmigemo

#endif // HAVE_MIGEMO_H
#endif // JD_MIGEMO_H
