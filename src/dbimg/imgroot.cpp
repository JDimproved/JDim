// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "imgroot.h"
#include "img.h"
#include "delimgcachediag.h"
#include "imginterface.h"

#include "jdlib/confloader.h"
#include "jdlib/miscgtk.h"

#ifdef _DEBUG
#include "jdlib/misctime.h"
#endif

#include "cache.h"
#include "command.h"
#include "prefdiagfactory.h"
#include "urlreplacemanager.h"

#include <sys/time.h>

#include <bitset>
#include <charconv>
#include <ios>
#include <limits>
#include <sstream>
#include <string_view>


namespace DBIMG::ir {
/// @brief NG 画像ハッシュの設定ファイル名
constexpr const char* kDHashListFileName = "dhash_list.txt";

/** @brief NG 画像ハッシュのデフォルト設定
 *
 * @details 最後にマッチした日時は 2000-01-01 (Sat) 09:00:00 JST に設定。
 * ハッシュのソースURLはデフォルト設定の目印として U+2699 GEAR を追加した。
 *
 * NGの対象は大量にレスを投稿する荒らしが繰り返し書き込んでいたグロテスク・ゴア・残虐な画像。
 */
constexpr const char* kDefaultConfig =
    "1028040033BF4F7C A2A430B1B070E1E 30 0 946684800 \xE2\x9A\x99+https://i.imgur.com/qsO20Gk.jpg\n"
    "178FC74220E94A20 E8E8E86828694BDE 30 0 946684800 \xE2\x9A\x99+https://i.imgur.com/wzq202L.jpg\n"
    "1CE2D3C1FF7E98 3B1D395D4D0F0F07 30 0 946684800 \xE2\x9A\x99+https://i.imgur.com/vjijPhf.jpg\n"
    "1EFBFC7BF208E7FC 6F7BB63B524E774F 30 0 946684800 \xE2\x9A\x99+https://i.imgur.com/PgMJGnN.jpg\n"
    "1F7CC23143F7CCBE 4D76B3B9EBADAEB5 30 0 946684800 \xE2\x9A\x99+https://i.imgur.com/WFteJZP.jpg\n"
    "1FE7F321F079D 4D5D0D8990CC643D 30 0 946684800 \xE2\x9A\x99+https://i.imgur.com/0B8cU8D.jpg\n"
    "23E0039D80C1633F B7E68A8E0E232317 30 0 946684800 \xE2\x9A\x99+https://i.imgur.com/Qwvsb6e.jpg\n"
    "291A080018FF7F86 4D5E65771B3372F2 30 0 946684800 \xE2\x9A\x99+https://i.imgur.com/FYYmQwu.jpg\n"
    "3882A2E73C4B04FF F6325450D4B2787 30 0 946684800 \xE2\x9A\x99+https://i.imgur.com/WJikloU.jpg\n"
    "3F63030079FC3ECC 1371CDAC4C86A2EC 30 0 946684800 \xE2\x9A\x99+https://i.imgur.com/lSkTyBs.jpg\n"
    "4343061C38B46C 636226A48C94F448 30 0 946684800 \xE2\x9A\x99+https://i.imgur.com/4EIF09A.gif\n"
    "5540FBCF8974002 6D6AAB475536AE2 30 0 946684800 \xE2\x9A\x99+https://i.imgur.com/Eq8pFcA.gif\n"
    "6378061381C4667D 73CFE3C1E474313D 30 0 946684800 \xE2\x9A\x99+https://i.imgur.com/iDOOCc5.jpg\n"
    "79FFBF3EB1C0006C 18D5A0E0E0E0E424 30 0 946684800 \xE2\x9A\x99+https://i.imgur.com/OXQ59ju.jpg\n"
    "7CB3772F0EC190AE 777367666E7C3E26 30 0 946684800 \xE2\x9A\x99+https://i.imgur.com/0pNEpr3.jpg\n"
    "7E81C2A1703111C7 B2B8262446C746E7 30 0 946684800 \xE2\x9A\x99+https://i.imgur.com/IQRJ6Xq.jpg\n"
    "801C308FFF0FBEFF B0F23C2C0E4F67C 30 0 946684800 \xE2\x9A\x99+https://i.imgur.com/Osuen0U.jpg\n"
    "811D3E8AB74916FF D0E2B3B376F173A 30 0 946684800 \xE2\x9A\x99+https://i.imgur.com/fZRYVgw.jpg\n"
    "833C6FCEC0BFFF3F 670663C3939112A2 30 0 946684800 \xE2\x9A\x99+https://i.imgur.com/8ftQr1w.jpg\n"
    "837FFCF18000E 382C23474D0D5656 30 0 946684800 \xE2\x9A\x99+https://i.imgur.com/ZX5KXSN.jpg\n"
    "923727080000C0E0 E9E5EC494C4C3D2D 30 0 946684800 \xE2\x9A\x99+https://i.imgur.com/7y8F03y.jpg\n"
    "9D0100000000E1 BCD46033B038B030 30 0 946684800 \xE2\x9A\x99+https://i.imgur.com/L5teub6.jpg\n"
    "A33F7A45411D7F01 333370507090F0F0 30 0 946684800 \xE2\x9A\x99+https://i.imgur.com/aZdlsbt.jpg\n"
    "B1B1F74F7E0017F0 B838326A3C74612C 30 0 946684800 \xE2\x9A\x99+https://i.imgur.com/Y1jjgEE.jpg\n"
    "BC3E3EC7420A9F01 E2373736A293931 30 0 946684800 \xE2\x9A\x99+https://i.imgur.com/31x4INT.jpg\n"
    "BE367CF7C091C3E9 A6366AD1E9922A2E 30 0 946684800 \xE2\x9A\x99+https://i.imgur.com/eZb4Fc4.jpg\n"
    "BEFF47593FFDC7FF 3371715D1B556727 30 0 946684800 \xE2\x9A\x99+https://i.imgur.com/wUyhJXD.jpg\n"
    "BF7FE1C1FCBFCE43 8171F8E9CFC3C771 30 0 946684800 \xE2\x9A\x99+https://i.imgur.com/OIbTczq.jpg\n"
    "C0841F8B3701F97C 20871131313D2D14 30 0 946684800 \xE2\x9A\x99+https://i.imgur.com/TUjtfzL.jpg\n"
    "C0BE3C856FDF3649 6F3F3C7432143468 30 0 946684800 \xE2\x9A\x99+https://i.imgur.com/JR8NIaZ.jpg\n"
    "C1EFA790F9B70E7 F0F8C0D8FC63667 30 0 946684800 \xE2\x9A\x99+https://i.imgur.com/Qw4FWRT.jpg\n"
    "C2099C00FF7FC7C2 C8CC8E333B39735A 30 0 946684800 \xE2\x9A\x99+https://i.imgur.com/NElZob7.jpg\n"
    "C2CCC0851116FBFE C7CF91041E022B27 30 0 946684800 \xE2\x9A\x99+https://i.imgur.com/ujTeALn.jpg\n"
    "C563F0FEF736D10 437130F8F85868D0 30 0 946684800 \xE2\x9A\x99+https://i.imgur.com/t5hnSAS.jpg\n"
    "C78038BF478E7B78 C68E8F6767EF3D7C 30 0 946684800 \xE2\x9A\x99+https://i.imgur.com/946sigr.jpg\n"
    "CCC231839C1BFC61 C693AF2B1F4B6E63 30 0 946684800 \xE2\x9A\x99+https://i.imgur.com/VouEyc3.jpg\n"
    "CD933744FCE0 9515173237658E9B 30 0 946684800 \xE2\x9A\x99+https://i.imgur.com/rWpzvPL.jpg\n"
    "D8F7F6001BDDFF9 FCB636BFFDDF5B7B 30 0 946684800 \xE2\x9A\x99+https://i.imgur.com/RA9YiOK.jpg\n"
    "DC1CFF37F3BD9D 70711B1A3830442 30 0 946684800 \xE2\x9A\x99+https://i.imgur.com/BPKDY3I.jpg\n"
    "DFE3810D6C0E02B1 E824A406361B3B19 30 0 946684800 \xE2\x9A\x99+https://i.imgur.com/eaT3GmP.jpg\n"
    "E3320C33CE3C0003 37364C594E3E7173 30 0 946684800 \xE2\x9A\x99+https://i.imgur.com/pe3oWus.jpg\n"
    "E72C816360A4CCFE E3CD594946464A5A 30 0 946684800 \xE2\x9A\x99+https://i.imgur.com/WdpCuFp.jpg\n"
    "F1E8CC20071F3FF7 ACCFC793C7C3B76E 30 0 946684800 \xE2\x9A\x99+https://i.imgur.com/ciXyCsx.jpg\n"
    "F70000B070000B06 FCFC7CFB75F5F5F7 30 0 946684800 \xE2\x9A\x99+https://i.imgur.com/Vdjgk2P.jpg\n"
    "F77980FE04000040 FA5C2E3E3C3C7C7E 30 0 946684800 \xE2\x9A\x99+https://i.imgur.com/nQoL19D.jpg\n"
    "FC800461BFFF7C80 872323230B0E2CB1 30 0 946684800 \xE2\x9A\x99+https://i.imgur.com/8gJqNJM.jpg\n"
    "FD811DF2DF7ED8C1 B070626A686870B0 30 0 946684800 \xE2\x9A\x99+https://i.imgur.com/GEswgVB.jpg\n"
    "FEFE210104FF9D00 D2393868E43989B9 30 0 946684800 \xE2\x9A\x99+https://i.imgur.com/HRmhuay.jpg\n"
    ;
}


using namespace DBIMG;

ImgRoot::ImgRoot()
{
    // GdkPixbufが直接サポートしていない画像フォーマットは利用できるか確認する
    for( const Gdk::PixbufFormat& fmt : Gdk::Pixbuf::get_formats() ) {
        const Glib::ustring name = fmt.get_name();
        if( name == "webp" ) {
            m_webp_support = true;
        }
        else if( name == "avif" ) {
            m_avif_support = true;
        }
    }

    const std::string img_abone_imghash_list = CACHE::path_img_abone_root() + ir::kDHashListFileName;
    std::string contents;
    if( CACHE::file_exists( img_abone_imghash_list ) == CACHE::EXIST_FILE ) {
        // ファイルがあるときは内容の有無に関係なくデフォルト設定は使わない
        if( ! CACHE::load_rawdata( img_abone_imghash_list, contents ) ) return;
    }
    else {
        // 設定ファイルが見つからないときはデフォルト設定を読み込む
        contents = ir::kDefaultConfig;
    }

    load_imghash_list( contents );
}


ImgRoot::~ImgRoot()
{
    for( auto& key_val : m_map_img ) {
        key_val.second->terminate_load();
        key_val.second.reset(); // call unique_ptr::reset()
    }

    save_abone_imghash_list();
}


void ImgRoot::clock_in()
{
    if( m_list_wait.size() ){

        for( Img* img : m_list_wait ) img->clock_in();

        remove_clock_in();
    }
}


// 読み込み待ちのためクロックを回すImgクラスをセット/リセット
void ImgRoot::set_clock_in( Img* img )
{
    m_list_wait.push_back( img );
}

void ImgRoot::reset_clock_in( Img* img )
{
    // リストに登録しておいて remove_clock_in()で消す
    m_list_delwait.push_back( img );
}


void ImgRoot::remove_clock_in()
{
    if( m_list_delwait.size() ){

#ifdef _DEBUG
        std::cout << "ImgRoot::remove_clock_in\n";
#endif

        for( Img* img : m_list_delwait ) {
#ifdef _DEBUG
            std::cout << img->url() << std::endl;
#endif
            m_list_wait.remove( img );
        }

        m_list_delwait.clear();
    }
}


//
// Imgクラス取得
// データベースに無ければImgクラスを作る
//
Img* ImgRoot::get_img( const std::string& url )
{
    auto it = m_map_img.find( url );
    if( it != m_map_img.end() ) return it->second.get();

    // 無ければ作る
    auto uniq = std::make_unique<Img>( url );
    Img* img = uniq.get();
    m_map_img.emplace( url, std::move( uniq ) );
    return img;
}


//
// 画像データの先頭のシグネチャを見て画像のタイプを取得
// 画像ではない場合は T_NOIMG を返す
//
// また、拡張子が偽装されていると未対応の画像が読み込まれる場合がある
// 読み込みが未対応の場合は T_NOT_SUPPORT を返す
//
int ImgRoot::get_image_type( const unsigned char *sign ) const
{
    int type = T_NOIMG;

    // jpeg は FF D8
    if( sign[ 0 ] == 0xFF
        && sign[ 1 ] == 0xD8 ) type = T_JPG;

    // png は 0x89 0x50 0x4e 0x47 0xd 0xa 0x1a 0xa
    else if( sign[ 0 ] == 0x89
             && sign[ 1 ] == 0x50
             && sign[ 2 ] == 0x4e
             && sign[ 3 ] == 0x47
             && sign[ 4 ] == 0x0d
             && sign[ 5 ] == 0x0a
             && sign[ 6 ] == 0x1a
             && sign[ 7 ] == 0x0a ) type = T_PNG;

    // gif
    else if( sign[ 0 ] == 'G'
             && sign[ 1 ] == 'I'
             && sign[ 2 ] == 'F' ) type = T_GIF;

    // bitmap
    else if( sign[ 0 ] == 'B'
             && sign[ 1 ] == 'M' ) type = T_BMP;

    // webp
    else if( sign[ 0 ] == 'R'
             && sign[ 1 ] == 'I'
             && sign[ 2 ] == 'F'
             && sign[ 3 ] == 'F'

             && sign[ 8 ] == 'W'
             && sign[ 9 ] == 'E'
             && sign[ 10 ] == 'B'
             && sign[ 11 ] == 'P' ) type = m_webp_support ? T_WEBP : T_NOT_SUPPORT;

    // avif
    else if( sign[ 4 ] == 'f'
             && sign[ 5 ] == 't'
             && sign[ 6 ] == 'y'
             && sign[ 7 ] == 'p'
             && sign[ 8 ] == 'a'
             && sign[ 9 ] == 'v'
             && sign[ 10 ] == 'i'
             && sign[ 11 ] == 'f' ) type = m_avif_support ? T_AVIF : T_NOT_SUPPORT;

    return type;
}


//
// 拡張子からタイプを取得
// 画像ではない場合は T_UNKNOWN を返す
//
int ImgRoot::get_type_ext( const std::string& url ) const
{
    // is_xxx() を安全に実行できない短すぎるURLはタイプを判定せず T_UNKNOWN を返す
    if( url.size() < 5 ) return T_UNKNOWN;

    // Urlreplaceによる画像コントロールを取得する
    int imgctrl = CORE::get_urlreplace_manager()->get_imgctrl( url );

    // URLに拡張子があっても画像として扱わない
    if( imgctrl & CORE::IMGCTRL_FORCEBROWSER ) return T_UNKNOWN;

    // URLの引数とアンカー(フラグメント)を除外して判定する
    const auto trimmed = std::string_view{ url }.substr( 0, url.find_first_of( "#?" ) );

    if( is_jpg( trimmed ) ) return T_JPG;
    if( is_png( trimmed ) ) return T_PNG;
    if( is_gif( trimmed ) ) return T_GIF;
    if( is_bmp( trimmed ) ) return T_BMP;
    if( m_webp_support && is_webp( trimmed ) ) return T_WEBP;
    if( m_avif_support && is_avif( trimmed ) ) return T_AVIF;

    // URLに拡張子がない場合でも画像として扱うか
    if( imgctrl & CORE::IMGCTRL_FORCEIMAGE ) return T_FORCEIMAGE;

    return T_UNKNOWN;
}


/** @brief 画像URLの拡張子がJPEGか否か判定する
 *
 * @details 今のところ拡張子だけを見る
 * @note URLのチェックは呼び出し側で行う前提で実装しているので注意
 * @param[in] url 引数とアンカーを取り除いたURL.
 * @return URLの拡張子がJPEGならtrueを返す
 */
bool ImgRoot::is_jpg( std::string_view url )
{
    auto ext = url.substr( url.size() - 4 );
    if( ext == ".jpg" || ext == ".JPG" ) return true;

    ext = url.substr( url.size() - 5 );
    return ext == ".jpeg" || ext == ".JPEG";
}


/** @brief 画像URLの拡張子がPNGか否か判定する
 *
 * @see ImgRoot::is_jpg()
 */
bool ImgRoot::is_png( std::string_view url )
{
    const auto ext = url.substr( url.size() - 4 );
    return ext == ".png" || ext == ".PNG";
}


/** @brief 画像URLの拡張子がGIFか否か判定する
 *
 * @see ImgRoot::is_jpg()
 */
bool ImgRoot::is_gif( std::string_view url )
{
    const auto ext = url.substr( url.size() - 4 );
    return ext == ".gif" || ext == ".GIF";
}


/** @brief 画像URLの拡張子がBMPか否か判定する
 *
 * @see ImgRoot::is_jpg()
 */
bool ImgRoot::is_bmp( std::string_view url )
{
    const auto ext = url.substr( url.size() - 4 );
    return ext == ".bmp" || ext == ".BMP";
}


/** @brief 画像URLの拡張子がWebPか否か判定する
 *
 * @see ImgRoot::is_jpg()
 */
bool ImgRoot::is_webp( std::string_view url )
{
    const auto ext = url.substr( url.size() - 5 );
    return ext == ".webp" || ext == ".WEBP";
}


/** @brief 画像URLの拡張子がAVIFか否か判定する
 *
 * @see ImgRoot::is_jpg()
 */
bool ImgRoot::is_avif( std::string_view url )
{
    const auto ext = url.substr( url.size() - 5 );
    return ext == ".avif" || ext == ".AVIF";
}


//
// キャッシュ削除
//
void ImgRoot::delete_cache( const std::string& url )
{
#ifdef _DEBUG
    std::cout << "ImgRoot::delete_cache  url = " << url << std::endl;
#endif

    Img* img = get_img( url );
    if( img && img->is_protected() ) return;

    // キャッシュ削除
    std::string path = CACHE::path_img( url );
    if( CACHE::file_exists( path ) == CACHE::EXIST_FILE ) unlink( to_locale_cstr( path ) );

    // info 削除
    path = CACHE::path_img_info( url );
    if( CACHE::file_exists( path ) == CACHE::EXIST_FILE ) unlink( to_locale_cstr( path ) );

    // 再描画
    if( img ) img->reset(); // call Img::reset()
    CORE::core_set_command( "close_image", url );
    CORE::core_set_command( "redraw_article" );
    CORE::core_set_command( "redraw_message" );
}


//
// 全キャッシュ削除
//
// image/info フォルダにあるファイルを全て取得して
// 期限切れのファイルを削除
//
void ImgRoot::delete_all_files()
{
#ifdef _DEBUG
    std::cout << "ImgRoot::delete_all_files\n";
#endif

    auto pref = CORE::PrefDiagFactory( nullptr, CORE::PREFDIAG_DELIMG, "" );
    int ret = pref->run();
    if( ret != Gtk::RESPONSE_OK ) return;

    // 画像キャッシュ削除ダイアログ表示
    DelImgCacheDiag deldiag;
    deldiag.run();

    reset_imgs();

    CORE::core_set_command( "close_nocached_image_views" );
    CORE::core_set_command( "redraw_article" );
    CORE::core_set_command( "redraw_message" );
}


/** @brief 画像URLからハッシュ値を登録して画像をあぼーんする
 *
 * @param[in] url       画像URL
 * @param[in] threshold あぼーん判定のしきい値
 */
void ImgRoot::push_abone_imghash( const std::string& url, const int threshold )
{
    Img* img = get_img( url );
    if( ! img ) return;

    std::optional<DHash> dhash = img->get_dhash();
    if( ! dhash ) return;

    std::stringstream ss;
    ss << "NG 画像ハッシュ "
       << std::uppercase << std::hex << dhash->row_hash
       << " "
       << std::uppercase << std::hex << dhash->col_hash
       << std::nouppercase << std::dec
       << " (しきい値: " << threshold
       << ")<br>" << url
       << " のソースURLです。";

    img->set_abone( true, ss.str() );
    delete_cache( url );

    m_vec_abone_imghash.push_back( AboneImgHash{ *dhash, threshold, img->get_time_modified(), url } );
}


/** @brief 画像のハッシュ値がNG 画像のハッシュの設定にマッチするかテストする
 *
 * @details マッチしたら画像をあぼーんしてキャッシュを削除する。
 * @param[in] img ハッシュ値を比較する画像データ
 * @return NG 画像ハッシュにマッチしたらtrue
 */
bool ImgRoot::test_imghash( Img& img )
{
    std::optional<DHash> dhash = img.get_dhash();
    if( ! dhash.has_value() ) {
#ifdef _DEBUG
        std::cout << "ImgRoot::test_imghash not has value" << std::endl;
#endif
        return false;
    }
    if( img.is_protected() || img.is_force_mosaic() ) return false;

    std::stringstream ss;
    for( auto& [abone_dhash, threshold, last_matched, source_url] : m_vec_abone_imghash ) {
        if( threshold < 0 ) continue;

        const int distance = calc_hamming_distance( abone_dhash, *dhash );
        if( distance <= threshold ) {
            last_matched = std::time( nullptr );
            std::string url = img.url();
            ss.str("");
            ss << "NG 画像ハッシュ "
               << std::uppercase << std::hex << abone_dhash.row_hash
               << " "
               << std::uppercase << std::hex << abone_dhash.col_hash
               << std::nouppercase << std::dec
               << " (しきい値: " << threshold
               << ")<br>" << source_url
               << " の類似画像です。";
#ifdef JDIM_DEVTOOLS
            // 開発者向けの機能: あぼーん判定理由にハッシュ値を比較した計算結果を追加
            ss << "(相違度: " << distance << ")";
#endif
            img.set_abone( true, ss.str() );
            delete_cache( url );
#ifdef _DEBUG
            std::cout << "ImgRoot::test_imghash abone url = " << url << std::endl;
#endif
            return true;
        }
    }
#ifdef _DEBUG
    std::cout << "ImgRoot::test_imghash not hit url = " << img.url() << std::endl;
#endif
    return false;
}


/** @brief 文字列からNG 画像ハッシュの設定を読み込む
 *
 * @details NG 画像ハッシュの設定は改行文字で区切って読み込む。
 * @param[in] contents NG 画像ハッシュのリスト
 */
void ImgRoot::load_imghash_list( const std::string& contents )
{
    m_vec_abone_imghash.clear();

    // ファイルフォーマット
    // v0: ROW COL THD RESERVED TIME URL
    std::size_t line_start = 0;
    for( std::size_t line_end = contents.find( '\n' );
            line_end != std::string::npos;
            line_end = contents.find( '\n', line_start )) {

        auto row_end = contents.find( ' ', line_start );
        if( row_end == std::string::npos ) continue;
        auto col_end = contents.find( ' ', row_end + 1 );
        if( col_end == std::string::npos ) continue;
        auto thd_end = contents.find( ' ', col_end + 1 );
        if( thd_end == std::string::npos ) continue;
        auto reserved_end = contents.find( ' ', thd_end + 1 );
        if( reserved_end == std::string::npos ) continue;
        auto time_end = contents.find( ' ', reserved_end + 1 );
        if( time_end == std::string::npos ) continue;

        std::from_chars_result result;
        std::string_view elem;

        elem = std::string_view{ contents }.substr( line_start );
        std::uint64_t row_hash;
        result = std::from_chars( elem.data(), elem.data() + row_end, row_hash, 16 );
        if( result.ec != std::errc{} ) row_hash = 0;

        elem = std::string_view{ contents }.substr( row_end + 1 );
        std::uint64_t col_hash;
        result = std::from_chars( elem.data(), elem.data() + col_end, col_hash, 16 );
        if( result.ec != std::errc{} ) col_hash = 0;

        elem = std::string_view{ contents }.substr( col_end + 1 );
        int threshold;
        result = std::from_chars( elem.data(), elem.data() + thd_end, threshold, 10 );
        if( result.ec != std::errc{} ) threshold = -1;

        elem = std::string_view{ contents }.substr( reserved_end + 1 );
        std::time_t last_matched;
        result = std::from_chars( elem.data(), elem.data() + time_end, last_matched, 10 );
        if( result.ec != std::errc{} ) last_matched = 0;

        std::string source_url = contents.substr( time_end + 1, line_end - (time_end + 1) );

        m_vec_abone_imghash.push_back( AboneImgHash{ { row_hash, col_hash }, threshold, last_matched, source_url } );

        line_start = line_end + 1;
    }
}


/**
 * @brief NG 画像ハッシュの設定をキャッシュディレクトリにファイル保存する
 */
void ImgRoot::save_abone_imghash_list()
{
    std::stringstream ss;
    for( auto& [dhash, threshold, date_modified, source_url] : m_vec_abone_imghash ) {
        ss << std::uppercase << std::hex << dhash.row_hash << " "
           << std::uppercase << std::hex << dhash.col_hash << " "
           << std::nouppercase << std::dec << threshold << " "
           << DBIMG::kImgHashReserved << " "
           << date_modified << " "
           << source_url << "\n";
    }

    const std::string img_abone_imghash_list = CACHE::path_img_abone_root() + ir::kDHashListFileName;
    CACHE::save_rawdata( img_abone_imghash_list, ss.str() );
}


//
// Img クラスの情報をリセット
//
void ImgRoot::reset_imgs()
{
    for( auto& key_val : m_map_img ) {
        key_val.second->reset(); // call Img::reset()
    }
}


/** @brief 画像データから画像ハッシュ(dHash)を計算して返す
 *
 * @param[in] pixbuf ハッシュを計算する画像データ
 * @return 計算した画像ハッシュ
 */
DBIMG::DHash ImgRoot::calc_dhash_from_pixbuf( const Gdk::Pixbuf& pixbuf )
{
    // グレースケール化
    auto gray = MISC::convert_to_grayscale( pixbuf );

    // グレースケール画像を縮小する
    constexpr int bit_map_size = 9;
    auto bit_map = gray->scale_simple( bit_map_size, bit_map_size, Gdk::INTERP_BILINEAR );

    // ピクセルデータの取得
    const guint8* pixels = bit_map->get_pixels();
    const int rowstride = bit_map->get_rowstride();
    const int n_channels = bit_map->get_n_channels();

    // 隣のピクセルより小さければ1、そうでないなら0としてハッシュ化する
    std::uint64_t row_hash = 0;
    std::uint64_t col_hash = 0;
    for( int y = 0; y < 8; ++y ) {
        for( int x = 0; x < 8; ++x ) {
            const guint8* pos = pixels + y * rowstride + x * n_channels;
            const guint8* next_row = pixels + (y + 1) * rowstride + x * n_channels;
            const guint8* next_col = pixels + y * rowstride + (x + 1) * n_channels;

            const int row_bit = *pos < *next_row;
            row_hash = (row_hash << 1) | row_bit;
            const int col_bit = *pos < *next_col;
            col_hash = (col_hash << 1) | col_bit;
        }
    }

#ifdef _DEBUG
    std::cout << "ImgRoot::calc_dhash_from_pixbuf dhash row = " << std::hex << row_hash
              << ", col = " << col_hash << std::dec << std::endl;
#endif
    return DBIMG::DHash{ row_hash, col_hash };
}


/** @brief 2つのdHashからハミング距離を計算する
 *
 * @details 2つの画像が似ているほど計算値が低くなる。 0 は同一画像の判定。
 * @return 計算結果は [0, 127] の範囲
 */
int ImgRoot::calc_hamming_distance( const DBIMG::DHash& a, const DBIMG::DHash& b ) noexcept
{
    std::bitset<64> bset_row( a.row_hash ^ b.row_hash );
    std::bitset<64> bset_col( a.col_hash ^ b.col_hash );

    auto num_bits_different = bset_row.count() + bset_col.count();
    return static_cast<int>( num_bits_different );
}
