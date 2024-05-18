// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef JDIM_DBIMG_IMGHASH_H
#define JDIM_DBIMG_IMGHASH_H

#include <cstdint>
#include <ctime>
#include <string>


namespace DBIMG
{

/** @brief uint64_t のペアにハッシュ値を格納する */
struct DHash
{
    std::uint64_t row_hash; ///< 行ハッシュ
    std::uint64_t col_hash; ///< 列ハッシュ
};

/** @brief あぼーん処理で使うデータ */
struct AboneImgHash
{
    DHash dhash;              ///< ハッシュ値
    int threshold;            ///< あぼーん判定のしきい値
    std::time_t last_matched; ///< 最後にマッチした日時
    std::string source_url;   ///< ハッシュのソースURL
};

} // namespace DBIMG

#endif // JDIM_DBIMG_IMGHASH_H
