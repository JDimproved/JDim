// SPDX-License-Identifier: GPL-2.0-or-later

#include "dbimg/imgroot.h"

#include "dbimg/imginterface.h"

#include "gtest/gtest.h"


namespace {

/** @brief DBIMG::ImgRoot::get_type_ext() のテスト
 *
 * @note ImgRoot::get_type_ext() の動作は Urlreplace_Manager の設定に依存します。
 * そのため、テストケースで使うURLは予約済みのドメイン .test を使用することで
 * キャッシュや設定の影響を受けないようにします。
 */
class DBIMG_ImgRoot_GetTypeExtTest : public ::testing::Test {};

TEST_F(DBIMG_ImgRoot_GetTypeExtTest, empty_string)
{
    DBIMG::ImgRoot imgroot;
    EXPECT_EQ( DBIMG::T_UNKNOWN, imgroot.get_type_ext( "" ) );
}

TEST_F(DBIMG_ImgRoot_GetTypeExtTest, not_url)
{
    DBIMG::ImgRoot imgroot;
    EXPECT_EQ( DBIMG::T_UNKNOWN, imgroot.get_type_ext( "test-invalid" ) );
}

TEST_F(DBIMG_ImgRoot_GetTypeExtTest, too_short_string)
{
    DBIMG::ImgRoot imgroot;
    EXPECT_EQ( DBIMG::T_UNKNOWN, imgroot.get_type_ext( ".jpg" ) );
    EXPECT_EQ( DBIMG::T_JPG, imgroot.get_type_ext( ".jpeg" ) );
}

TEST_F(DBIMG_ImgRoot_GetTypeExtTest, not_image_url)
{
    DBIMG::ImgRoot imgroot;
    EXPECT_EQ( DBIMG::T_UNKNOWN, imgroot.get_type_ext( "http://jdim.test/plain.txt" ) );
    EXPECT_EQ( DBIMG::T_UNKNOWN, imgroot.get_type_ext( "http://jdim.test/page.html" ) );
    EXPECT_EQ( DBIMG::T_UNKNOWN, imgroot.get_type_ext( "http://jdim.test/directory.jpg/" ) );
}

TEST_F(DBIMG_ImgRoot_GetTypeExtTest, url_jpg)
{
    DBIMG::ImgRoot imgroot;
    EXPECT_EQ( DBIMG::T_JPG, imgroot.get_type_ext( "http://jdim.test/image.jpg" ) );
    EXPECT_EQ( DBIMG::T_JPG, imgroot.get_type_ext( "http://jdim.test/image.JPG" ) );
}

TEST_F(DBIMG_ImgRoot_GetTypeExtTest, url_jpeg)
{
    DBIMG::ImgRoot imgroot;
    EXPECT_EQ( DBIMG::T_JPG, imgroot.get_type_ext( "http://jdim.test/image.jpeg" ) );
    EXPECT_EQ( DBIMG::T_JPG, imgroot.get_type_ext( "http://jdim.test/image.JPEG" ) );
}

TEST_F(DBIMG_ImgRoot_GetTypeExtTest, url_png)
{
    DBIMG::ImgRoot imgroot;
    EXPECT_EQ( DBIMG::T_PNG, imgroot.get_type_ext( "http://jdim.test/image.png" ) );
    EXPECT_EQ( DBIMG::T_PNG, imgroot.get_type_ext( "http://jdim.test/image.PNG" ) );
}

TEST_F(DBIMG_ImgRoot_GetTypeExtTest, url_gif)
{
    DBIMG::ImgRoot imgroot;
    EXPECT_EQ( DBIMG::T_GIF, imgroot.get_type_ext( "http://jdim.test/image.gif" ) );
    EXPECT_EQ( DBIMG::T_GIF, imgroot.get_type_ext( "http://jdim.test/image.GIF" ) );
}

TEST_F(DBIMG_ImgRoot_GetTypeExtTest, url_bmp)
{
    DBIMG::ImgRoot imgroot;
    EXPECT_EQ( DBIMG::T_BMP, imgroot.get_type_ext( "http://jdim.test/image.bmp" ) );
    EXPECT_EQ( DBIMG::T_BMP, imgroot.get_type_ext( "http://jdim.test/image.BMP" ) );
}

TEST_F(DBIMG_ImgRoot_GetTypeExtTest, url_webp)
{
    DBIMG::ImgRoot imgroot;
    const int img_type{ imgroot.is_webp_support() ? DBIMG::T_WEBP : DBIMG::T_UNKNOWN };
    const char* message{ imgroot.is_avif_support() ? "webp is supported!" : "webp is not supported!" };
    EXPECT_EQ( img_type, imgroot.get_type_ext( "http://jdim.test/image.webp" ) ) << message;
    EXPECT_EQ( img_type, imgroot.get_type_ext( "http://jdim.test/image.WEBP" ) ) << message;
}

TEST_F(DBIMG_ImgRoot_GetTypeExtTest, url_avif)
{
    DBIMG::ImgRoot imgroot;
    const int img_type{ imgroot.is_avif_support() ? DBIMG::T_AVIF : DBIMG::T_UNKNOWN };
    const char* message{ imgroot.is_avif_support() ? "avif is supported!" : "avif is not supported!" };
    EXPECT_EQ( img_type, imgroot.get_type_ext( "http://jdim.test/image.avif" ) ) << message;
    EXPECT_EQ( img_type, imgroot.get_type_ext( "http://jdim.test/image.AVIF" ) ) << message;
}

TEST_F(DBIMG_ImgRoot_GetTypeExtTest, url_mixcase)
{
    DBIMG::ImgRoot imgroot;
    EXPECT_EQ( DBIMG::T_UNKNOWN, imgroot.get_type_ext( "http://jdim.test/image.Jpg" ) );
    EXPECT_EQ( DBIMG::T_UNKNOWN, imgroot.get_type_ext( "http://jdim.test/image.jPeg" ) );
    EXPECT_EQ( DBIMG::T_UNKNOWN, imgroot.get_type_ext( "http://jdim.test/image.pnG" ) );
    EXPECT_EQ( DBIMG::T_UNKNOWN, imgroot.get_type_ext( "http://jdim.test/image.Gif" ) );
    EXPECT_EQ( DBIMG::T_UNKNOWN, imgroot.get_type_ext( "http://jdim.test/image.bMp" ) );
    EXPECT_EQ( DBIMG::T_UNKNOWN, imgroot.get_type_ext( "http://jdim.test/image.webP" ) );
    EXPECT_EQ( DBIMG::T_UNKNOWN, imgroot.get_type_ext( "http://jdim.test/image.avIf" ) );
}

TEST_F(DBIMG_ImgRoot_GetTypeExtTest, url_with_parameters)
{
    DBIMG::ImgRoot imgroot;
    EXPECT_EQ( DBIMG::T_JPG, imgroot.get_type_ext( "http://jdim.test/image.jpg?" ) );
    EXPECT_EQ( DBIMG::T_JPG, imgroot.get_type_ext( "http://jdim.test/image.jpeg?foo=bar" ) );
    EXPECT_EQ( DBIMG::T_PNG, imgroot.get_type_ext( "http://jdim.test/image.png?hoge=1&moge=2" ) );
    EXPECT_EQ( DBIMG::T_GIF, imgroot.get_type_ext( "http://jdim.test/image.gif?a=b.jpg" ) );
}

TEST_F(DBIMG_ImgRoot_GetTypeExtTest, url_with_anchor)
{
    DBIMG::ImgRoot imgroot;
    EXPECT_EQ( DBIMG::T_JPG, imgroot.get_type_ext( "http://jdim.test/image.jpg#" ) );
    EXPECT_EQ( DBIMG::T_JPG, imgroot.get_type_ext( "http://jdim.test/image.jpeg#foo" ) );
    EXPECT_EQ( DBIMG::T_PNG, imgroot.get_type_ext( "http://jdim.test/image.png?hoge=1#moge" ) );
    EXPECT_EQ( DBIMG::T_GIF, imgroot.get_type_ext( "http://jdim.test/image.gif#b.jpg" ) );
}

} // namespace
