// SPDX-License-Identifier: GPL-2.0-or-later
/**
 * @file bbsmenu.h
 * @brief BBSMENUの取得と板一覧のデータを構築する
 */
#ifndef JDIM_DBTREE_BBSMENU_H
#define JDIM_DBTREE_BBSMENU_H

#include "skeleton/loadable.h"
#include "xml/document.h"

#include <string>
#include <string_view>


namespace DBTREE
{
// clang 8以下ではnoexceptがついたdefault constructorがコンパイルできない
// 参考文献: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=86583
namespace {
#if defined(__clang__) && __clang_major__ < 9
constexpr bool kSupportNoexceptDefaultCtor = false;
#else
constexpr bool kSupportNoexceptDefaultCtor = true;
#endif
}


/** @brief BBSMENUの取得と板一覧のデータを構築するクラス
 *
 * @details BBSMENUのURLにアクセスしてHTMLを取得して解析を行いカテゴリと板を持つDOMを構築する。
 * DOMの内容は板一覧に追加されて管理・反映される。
 * また、DOMからXMLを生成してキャッシュディレクトリに保存・読み込みを行う。
 */
class BBSMenu : public SKELETON::Loadable
{
    std::string m_url;
    std::string m_name;
    std::string m_rawdata;

    XML::Document m_xml_document;

    /** @brief 読み込んだDOMをRootクラスから参照するためのシグナル
     *
     * @note Root は sigc::trackable を継承していないためsignalの接続解除が動作しない。
     * Root の寿命が尽きるとこのシグナルに接続された関数オブジェクトはdangling参照になるので注意。
     */
    sigc::signal<void, BBSMenu&> m_sig_analyze_board_xml;

public:

    /** @brief コンストラクタ
     *
     * @param[in] url  BBSMENUのURL
     * @param[in] name BBSMENUの名称
     */
    BBSMenu( std::string_view url, std::string_view name )
        : m_url{ url }
        , m_name{ name }
    {
    }

    BBSMenu( BBSMenu&& ) noexcept(kSupportNoexceptDefaultCtor)= default;
    BBSMenu( const BBSMenu& ) = delete;

    ~BBSMenu() noexcept = default;

    BBSMenu& operator=( BBSMenu&& ) noexcept(kSupportNoexceptDefaultCtor) = default;
    BBSMenu& operator=( const BBSMenu& ) = delete;

    /// @brief 外部BBSMENUは name の重複を許すため url のみで検索するときに使う
    bool equals( std::string_view url ) const noexcept { return m_url == url; }

    /// @brief url と name の両方で検索するときに使う
    bool equals( std::string_view url, std::string_view name ) const noexcept
    {
        return m_url == url && m_name == name;
    }

    /// @brief BBSMENUのURLを返す
    const std::string& get_url() const { return m_url; }
    /// @brief BBSMENUの名前を返す
    const std::string& get_name() const { return m_name; }
    /// @brief BBSMENUのXMLを保存するファイルパスを返す
    std::string path_bbsmenu_boards_xml() const;

    void reset( std::string_view url, std::string_view name )
    {
        m_url = url;
        m_name = name;

        m_rawdata.clear();
        m_xml_document.clear();
    }

    void load_cache();
    void download_bbsmenu();

    /// @brief 構築したDOMを返す
    const XML::Document& xml_document() const { return m_xml_document; }

    auto& sig_analyze_board_xml() { return m_sig_analyze_board_xml; }

private:

    // BBSMENUのダウンロード用関数
    void clear() { m_rawdata.clear(); }
    /** @brief BBSMENU のデータを受信したときのコールバック
     *
     * @param[in] buf 受信したデータ
     */
    void receive_data( std::string_view buf ) override { m_rawdata.append( buf ); }
    void receive_finish() override;

    /// @brief XML に含まれる板情報を取り出してデータベースを更新する
    void analyze_board_xml() { m_sig_analyze_board_xml.emit( *this ); }

    void bbsmenu2xml( const std::string& menu );
};

} // namespace DBTREE

#endif // JDIM_DBTREE_BBSMENU_H
