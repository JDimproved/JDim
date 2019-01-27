// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "tfidf.h"

#include "dbtree/articlebase.h"

#include "global.h"

#include <set>
#include <math.h>


//
// 単語ベクトル作成
//
void MISC::tfidf_create_vec_words( VEC_WORDS& vec_words, const Glib::ustring& document )
{
    const int n = document.length() - 1;

#ifdef _DEBUG
    std::cout << "tfidf_create_vec_words\n";
    std::cout << "doc = " << document.raw() << std::endl;
    std::cout << "n = " << n << std::endl;
#endif

    if( n <= 0 ) return;

    std::set< Glib::ustring > set_words;

    for( int i = 0; i < n; ++i ){

        Glib::ustring word = document.substr( i, 2 );
        if( set_words.find( word ) == set_words.end() ){

            set_words.insert( word );
            vec_words.push_back( word );
        }
    }

#ifdef _DEBUG
//    for( int i = 0; i < (int)vec_words.size(); ++i ) std::cout << vec_words[ i ].raw() << std::endl;
#endif
}


//
// IDF計算 (実際には頻度計算)
//
// vec_idf はあらかじめ resize しておくこと
//
void MISC::tfidf_create_vec_idf( VEC_IDF& vec_idf, const Glib::ustring& document, const VEC_WORDS& vec_words )
{
    const int n = vec_words.size();

#ifdef _DEBUG
//    std::cout << "MISC::tfidf_create_vec_idf n = " << n << std::endl;
#endif

    if( ! n || n != (int)vec_idf.size() ) return;

    for( int i = 0; i < n; ++i ){
        if( document.find( vec_words[ i ] ) != Glib::ustring::npos ) vec_idf[ i ] += 1;
    }
}


//
// documnet に対する TFIDF ベクトル計算
//
// vec_tfidf はあらかじめ resize しておくこと
//
void MISC::tfidf_calc_vec_tfifd( VEC_TFIDF& vec_tfidf, const Glib::ustring& document,
                           const VEC_IDF& vec_idf, const VEC_WORDS& vec_words )
{
    const int n = vec_words.size();
    const int n_doc = document.size() - 1;

#ifdef _DEBUG
    std::cout << "tfidf_calc_vec_tfidf\n";
    std::cout << "doc = " << document.raw() << std::endl;
    std::cout << "n = " << n << " n_doc = " << n_doc << std::endl;
#endif

    if( ! n || n_doc <= 0 || n != (int)vec_tfidf.size() ) return;

    double total = 0;
    for( int i = 0; i < n; ++i ){

        if( document.find( vec_words[ i ] ) == Glib::ustring::npos ){
            vec_tfidf[ i ] = 0;
            continue;
        }

        int hit = 0;
        for( int j = 0; j < n_doc; ++j ) if( document.substr( j, 2 ) == vec_words[ i ] ) ++hit;
        vec_tfidf[ i ] = hit;

        total += hit;
    }

    for( int i = 0; i < n; ++i ){

#ifdef _DEBUG
//        std::cout << vec_words[ i ].raw() << " : hit = " << (int)vec_tfidf[ i ];
#endif

        if( total ){
            vec_tfidf[ i ] /= total;
            vec_tfidf[ i ] *= vec_idf[ i ];
        }
        else vec_tfidf[ i ] = 0;

#ifdef _DEBUG
//        std::cout << " tfidf = " << vec_tfidf[ i ] << std::endl;
#endif
    }
}


//
// 相関計算
//
double MISC::tfidf_cos_similarity( const VEC_TFIDF& vec_tfidf1, const VEC_TFIDF& vec_tfidf2 )
{
    const int n = vec_tfidf1.size();

#ifdef _DEBUG
    std::cout << "MISC::tfidf_cos_similarity n = " << n << std::endl;
#endif

    if( ! n || n != (int)vec_tfidf1.size() ) return 0;

    double product = 0;
    double lng1 = 0;
    double lng2 = 0;

    for( int i = 0; i < n; ++i ){
        product += vec_tfidf1[ i ] * vec_tfidf2[ i ];
        lng1 += vec_tfidf1[ i ] * vec_tfidf1[ i ];
        lng2 += vec_tfidf2[ i ] * vec_tfidf2[ i ];
    }

    if( lng1 == 0 ) return 0;
    if( lng2 == 0 ) return 0;

    const double ret = product / sqrt( lng1 * lng2 );

#ifdef _DEBUG
    std::cout << "similarity = " << ret << std::endl;
#endif

    return ret;
}


//
// スレ一覧からIDF 計算
//
void MISC::tfidf_create_vec_idf_from_board( VEC_IDF& vec_idf,
                                            const Glib::ustring& subject_src, const std::vector< DBTREE::ArticleBase* >& list_subject, const VEC_WORDS& vec_words )
{
#ifdef _DEBUG
    std::cout << "MISC::tfidf_create_vec_idf_from_board\n";
#endif

    if( subject_src.empty() || ! list_subject.size() || ! vec_words.size() ) return;

    vec_idf.resize( vec_words.size() );
    MISC::tfidf_create_vec_idf( vec_idf, subject_src, vec_words );

    int D = 1;
    std::vector< DBTREE::ArticleBase* >::const_iterator it = list_subject.begin();
    for( ; it != list_subject.end(); ++it ){

        // DAT落ちのスレは除く
        if( ( *it )->get_status() & STATUS_OLD ) continue;

        const Glib::ustring subject = ( *it )->get_subject();
        if( subject != subject_src ){

            MISC::tfidf_create_vec_idf( vec_idf, subject, vec_words );
            ++D;
        }
    }
    for( int i = 0; i < (int)vec_words.size(); ++i ){

#ifdef _DEBUG
        std::cout << vec_words[ i ].raw() << " hit = " << (int)vec_idf[ i ] << " / " << D;
#endif

        vec_idf[ i ] = log( D / vec_idf[ i ] );

#ifdef _DEBUG
        std::cout << " idf = " << vec_idf[ i ] << std::endl;
#endif
    }
}



// str1 と str2 間のレーベンシュタイン距離
double MISC::leven( std::vector< std::vector< int > >& dist,
                    const Glib::ustring& str1, const Glib::ustring& str2 )
{
    const size_t maxlng = dist.size() -1;
    const size_t lng1 = MIN( maxlng, str1.length() );
    const size_t lng2 = MIN( maxlng, str2.length() );
    const gunichar sp_wide = Glib::ustring( "　" )[0];

#ifdef _DEBUG
    std::cout << "MISC::leven< str1 = " << str1.raw() << " lng1 = " << lng1 << std::endl
              << "str2 = " << str2.raw() << " lng2 = " << lng2 << std::endl
              << "maxlng = " << maxlng << std::endl;
#endif

    dist[ 0 ][ 0 ] = 0;
    for( size_t i = 1, cost = 0; i <= lng1; ++i ){

        const gunichar c = str1[ i-1 ];

        // 半角、全角空白の挿入コストは0
        if( c != ' ' && c != sp_wide ) ++cost; 

        dist[ i ][ 0 ] = cost;
    }
    for( size_t i = 1, cost = 0; i <= lng2; ++i ){

        const gunichar c = str2[ i-1 ];

        // 半角、空白の削除コストは0
        if( c != ' ' && c != sp_wide ) ++cost; 

        dist[ 0 ][ i ] = cost;
    }

    for( size_t i = 1; i <= lng1; ++i ){
        for( size_t j = 1; j <= lng2; ++j ){        

            const gunichar c1 = str1[ i-1 ];
            const gunichar c2 = str2[ j-1 ];

            int cost_replace = dist[ i-1 ][ j-1 ];
            if( c1 != c2 ) cost_replace += 1;

            int cost_insert = dist[ i-1 ][ j ];
            // 半角、空白の挿入コストは0
            if( c1 != ' ' && c1 != sp_wide ) cost_insert += 1;

            int cost_delete = dist[ i ][ j-1 ];
            // 半角、空白の削除コストは0
            if( c2 != ' ' && c2 != sp_wide ) cost_delete += 1;

            dist[ i ][ j ] = MIN( cost_replace, MIN( cost_insert, cost_delete ) );
        }
    }

#ifdef _DEBUG
    for( size_t i = 0; i <= lng1; ++i ){
        for( size_t j = 0; j <= lng2; ++j ) std::cout << dist[ i ][ j ] << " ";
        std::cout << std::endl;
    }
#endif

    // 0 - 1 の範囲に正規化
    return ( double )dist[ lng1 ][ lng2 ] / MAX( dist[ lng1 ][ 0 ], dist[ 0 ][ lng2 ] );
}

