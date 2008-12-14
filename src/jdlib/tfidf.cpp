// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "tfidf.h"

#include "dbtree/articlebase.h"

#include "global.h"

#include <set>


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
const double MISC::tfidf_cos_similarity( const VEC_TFIDF& vec_tfidf1, const VEC_TFIDF& vec_tfidf2 )
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
#include <iostream>
void MISC::tfidf_create_vec_idf_from_board( VEC_IDF& vec_idf,
                                            const Glib::ustring& subject_src, const std::list< DBTREE::ArticleBase* >& list_subject, const VEC_WORDS& vec_words )
{
//#ifdef _DEBUG
    std::cout << "MISC::tfidf_create_vec_idf_from_board\n";
//#endif

    if( subject_src.empty() || ! list_subject.size() || ! vec_words.size() ) return;

    vec_idf.resize( vec_words.size() );
    MISC::tfidf_create_vec_idf( vec_idf, subject_src, vec_words );

    int D = 1;
    std::list< DBTREE::ArticleBase* >::const_iterator it = list_subject.begin();
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

//#ifdef _DEBUG
        std::cout << vec_words[ i ].raw() << " hit = " << (int)vec_idf[ i ] << " / " << D;
//#endif

        vec_idf[ i ] = log( D / vec_idf[ i ] );

//#ifdef _DEBUG
        std::cout << " idf = " << vec_idf[ i ] << std::endl;
//#endif
    }
}
