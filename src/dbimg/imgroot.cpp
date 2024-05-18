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
    if( CACHE::file_exists( img_abone_imghash_list ) != CACHE::EXIST_FILE ) return;

    std::string contents;
    if( ! CACHE::load_rawdata( img_abone_imghash_list, contents ) ) return;

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
    return get_type_ext( url.c_str(), url.length() );
}


int ImgRoot::get_type_ext( const char* url, int n ) const
{
    // Urlreplaceによる画像コントロールを取得する
    int imgctrl = CORE::get_urlreplace_manager()->get_imgctrl( url );

    // URLに拡張子があっても画像として扱わない
    if( imgctrl & CORE::IMGCTRL_FORCEBROWSER ) return T_UNKNOWN;

    if( is_jpg( url, n ) ) return T_JPG;
    if( is_png( url, n ) ) return T_PNG;
    if( is_gif( url, n ) ) return T_GIF;
    if( is_bmp( url, n ) ) return T_BMP;
    if( m_webp_support && is_webp( url, n ) ) return T_WEBP;
    if( m_avif_support && is_avif( url, n ) ) return T_AVIF;

    // URLに拡張子がない場合でも画像として扱うか
    if( imgctrl & CORE::IMGCTRL_FORCEIMAGE ) return T_FORCEIMAGE;

    return T_UNKNOWN;
}


// 今のところ拡張子だけを見る
bool ImgRoot::is_jpg( const char* url, int n )
{
    // .jpg
    if( *( url + n -4 ) == '.' &&
        *( url + n -3 ) == 'j' &&
        *( url + n -2 ) == 'p' &&
        *( url + n -1 ) == 'g'  ) return true;

    if( *( url + n -4 ) == '.' &&
        *( url + n -3 ) == 'J' &&
        *( url + n -2 ) == 'P' &&
        *( url + n -1 ) == 'G'  ) return true;

    // .jpeg
    if( *( url + n -5 ) == '.' &&
        *( url + n -4 ) == 'j' &&
        *( url + n -3 ) == 'p' &&
        *( url + n -2 ) == 'e' &&
        *( url + n -1 ) == 'g'  ) return true;

    if( *( url + n -5 ) == '.' &&
        *( url + n -4 ) == 'J' &&
        *( url + n -3 ) == 'P' &&
        *( url + n -2 ) == 'E' &&
        *( url + n -1 ) == 'G'  ) return true;

    return false;
}

bool ImgRoot::is_png( const char* url, int n )
{
    // .png
    if( *( url + n -4 ) == '.' &&
        *( url + n -3 ) == 'p' &&
        *( url + n -2 ) == 'n' &&
        *( url + n -1 ) == 'g'  ) return true;

    if( *( url + n -4 ) == '.' &&
        *( url + n -3 ) == 'P' &&
        *( url + n -2 ) == 'N' &&
        *( url + n -1 ) == 'G'  ) return true;

    return false;
}

bool ImgRoot::is_gif( const char* url, int n )
{
    // .gif
    if( *( url + n -4 ) == '.' &&
        *( url + n -3 ) == 'g' &&
        *( url + n -2 ) == 'i' &&
        *( url + n -1 ) == 'f'  ) return true;

    if( *( url + n -4 ) == '.' &&
        *( url + n -3 ) == 'G' &&
        *( url + n -2 ) == 'I' &&
        *( url + n -1 ) == 'F'  ) return true;

    return false;
}

bool ImgRoot::is_bmp( const char* url, int n )
{
    // .bmp
    if( *( url + n -4 ) == '.' &&
        *( url + n -3 ) == 'b' &&
        *( url + n -2 ) == 'm' &&
        *( url + n -1 ) == 'p'  ) return true;

    if( *( url + n -4 ) == '.' &&
        *( url + n -3 ) == 'B' &&
        *( url + n -2 ) == 'M' &&
        *( url + n -1 ) == 'P'  ) return true;

    return false;
}


bool ImgRoot::is_webp( const char* url, int n )
{
    // .webp
    if( *( url + n -5 ) == '.' &&
        *( url + n -4 ) == 'w' &&
        *( url + n -3 ) == 'e' &&
        *( url + n -2 ) == 'b' &&
        *( url + n -1 ) == 'p'  ) return true;

    if( *( url + n -5 ) == '.' &&
        *( url + n -4 ) == 'W' &&
        *( url + n -3 ) == 'E' &&
        *( url + n -2 ) == 'B' &&
        *( url + n -1 ) == 'P'  ) return true;

    return false;
}


bool ImgRoot::is_avif( const char* url, int n )
{
    // .avif
    if( *( url + n -5 ) == '.' &&
        *( url + n -4 ) == 'a' &&
        *( url + n -3 ) == 'v' &&
        *( url + n -2 ) == 'i' &&
        *( url + n -1 ) == 'f'  ) return true;

    if( *( url + n -5 ) == '.' &&
        *( url + n -4 ) == 'A' &&
        *( url + n -3 ) == 'V' &&
        *( url + n -2 ) == 'I' &&
        *( url + n -1 ) == 'F'  ) return true;

    return false;
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

    img->set_abone( true );
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
    if( img.is_protected() ) return false;

    for( auto& [abone_dhash, threshold, last_matched, u] : m_vec_abone_imghash ) {
        if( threshold < 0 ) continue;

        if( calc_hamming_distance( abone_dhash, *dhash ) <= threshold ) {
            last_matched = std::time( nullptr );
            std::string url = img.url();
            img.set_abone( true );
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
