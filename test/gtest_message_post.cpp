// SPDX-License-Identifier: GPL-2.0-or-later

#include "message/post.h"

#include "gtest/gtest.h"


namespace {

// PostTestを無名名前空間に定義
class PostTest : public ::testing::Test, public MESSAGE::Post
{
public:
    PostTest() : MESSAGE::Post(nullptr, "dummy_url", "dummy_msg", false) {} // ダミーURLで初期化
};

TEST_F(PostTest, process_error_message_handles_duplicate_link)
{
    // 同一リンクの重複除去を確認
    std::time_t samba_sec = 0;
    std::string errmsg = "<b>エラー: <a href=\"https://example.com\">https://example.com</a></b>";
    EXPECT_EQ( process_error_message( errmsg, samba_sec ), "<b>エラー:  https://example.com</b>" );
    EXPECT_EQ( samba_sec, 0 ); // sambaマッチなし
}

TEST_F(PostTest, process_error_message_preserves_different_link)
{
    // 異なるリンクのhrefとテキストを保持
    std::time_t samba_sec = 0;
    std::string errmsg = "<b>エラー: <a href=\"https://example.com\">詳細リンク</a></b>";
    EXPECT_EQ( process_error_message( errmsg, samba_sec ), "<b>エラー:  https://example.com 詳細リンク</b>" );
    EXPECT_EQ( samba_sec, 0 );
}

TEST_F(PostTest, process_error_message_handles_multiple_links)
{
    // 複数リンクの処理を確認
    std::time_t samba_sec = 0;
    std::string errmsg = "<b><a href=\"url1\">url1</a> <a href=\"url2\">text2</a></b>";
    EXPECT_EQ( process_error_message( errmsg, samba_sec ), "<b> url1  url2 text2</b>" );
}

TEST_F(PostTest, process_error_message_handles_empty_link)
{
    // 空リンクの処理を確認
    std::time_t samba_sec = 0;
    std::string errmsg = "<a href=\"url\"></a>";
    EXPECT_EQ( process_error_message( errmsg, samba_sec ), " url " );
}

TEST_F(PostTest, process_error_message_handles_samba_sec)
{
    // samba秒の抽出を確認
    std::time_t samba_sec = 0;
    std::string errmsg = "ERROR: Samba24:Caution 25 秒たたないと書けません。";
    EXPECT_EQ( process_error_message( errmsg, samba_sec ), "ERROR: Samba24:Caution 25 秒たたないと書けません。" );
    EXPECT_EQ( samba_sec, 25 );
}

TEST_F(PostTest, process_error_message_handles_no_tags)
{
    // タグなしメッセージの改行処理を確認
    std::time_t samba_sec = 0;
    std::string errmsg = "シンプルなエラー<br>メッセージ";
    EXPECT_EQ( process_error_message( errmsg, samba_sec ), "シンプルなエラー\nメッセージ" );
    EXPECT_EQ( samba_sec, 0 );
}

TEST_F(PostTest, process_error_message_handles_invalid_html)
{
    // 不正HTMLの処理を確認
    std::time_t samba_sec = 0;
    std::string errmsg = "<a href=\"url\">text</a><invalid>";
    EXPECT_EQ( process_error_message( errmsg, samba_sec ), " url text<invalid>" );
}

TEST_F(PostTest, process_error_message_handles_special_chars)
{
    // 特殊文字リンクの処理を確認
    std::time_t samba_sec = 0;
    std::string errmsg = "<a href=\"https://example.com/path%20with%20space\">path with space</a>";
    EXPECT_EQ( process_error_message( errmsg, samba_sec ), " https://example.com/path%20with%20space path with space" );
    EXPECT_EQ( samba_sec, 0 );
}

TEST_F(PostTest, process_error_message_handles_nested_links)
{
    // ネストされた<a>タグを外側から順に処理（hrefとテキストを連結）
    std::time_t samba_sec = 0;
    std::string errmsg = "<a href=\"outer\">outer<a href=\"inner\">inner</a></a>";
    EXPECT_EQ( process_error_message( errmsg, samba_sec ), " outer outer inner inner" );
    EXPECT_EQ( samba_sec, 0 );
}

TEST_F(PostTest, process_error_message_handles_empty_input)
{
    // 空入力の処理を確認
    std::time_t samba_sec = 0;
    std::string errmsg = "";
    EXPECT_EQ( process_error_message( errmsg, samba_sec ), "" );
    EXPECT_EQ( samba_sec, 0 );
}

TEST_F(PostTest, process_error_message_handles_multiple_br_hr)
{
    // 複数<br><hr>の改行処理を確認
    std::time_t samba_sec = 0;
    std::string errmsg = "エラー<br><br><a href=\"url\">url</a><hr><hr>";
    EXPECT_EQ( process_error_message( errmsg, samba_sec ), "エラー\n\n url\n-------------------\n\n-------------------\n" );
    EXPECT_EQ( samba_sec, 0 );
}

TEST_F(PostTest, process_error_message_handles_invalid_samba_sec)
{
    // 不正samba秒の処理を確認
    std::time_t samba_sec = 0;
    std::string errmsg = "ERROR: Samba24:Caution invalid 秒";
    EXPECT_EQ( process_error_message( errmsg, samba_sec ), "ERROR: Samba24:Caution invalid 秒" );
    EXPECT_EQ( samba_sec, 0 ); // atoiが不正値で0を返す
}

}
