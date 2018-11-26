// ライセンス: GPL2

//
// TF-IDF
//

#ifndef _TFIDF_H
#define _TFIDF_H

#include <gtkmm.h>
#include <vector>

namespace DBTREE
{
    class ArticleBase;
}

namespace MISC
{
    typedef std::vector< double > VEC_TFIDF;
    typedef std::vector< Glib::ustring > VEC_WORDS;
    typedef std::vector< double > VEC_IDF;

    // 単語ベクトル作成
    void tfidf_create_vec_words( VEC_WORDS& vec_words, const Glib::ustring& document );

    // IDF計算 (実際には頻度計算)
    // vec_idf はあらかじめ resize しておくこと
    void tfidf_create_vec_idf( VEC_IDF& vec_idf, const Glib::ustring& document, const VEC_WORDS& vec_words );

    // documnet に対する TFIDF ベクトル計算
    // vec_tfidf はあらかじめ resize しておくこと
    void tfidf_calc_vec_tfifd( VEC_TFIDF& vec_tfidf, const Glib::ustring& document,
                               const VEC_IDF& vec_idf, const VEC_WORDS& vec_words );

    // tfidf1 と tfidf2 の相関計算
    double tfidf_cos_similarity( const VEC_TFIDF& vec_tfidf1, const VEC_TFIDF& vec_tfidf2 );

    // スレ一覧からIDF 計算
    void tfidf_create_vec_idf_from_board( VEC_IDF& vec_idf,
                                          const Glib::ustring& subject_src, const std::vector< DBTREE::ArticleBase* >& list_subject, const VEC_WORDS& vec_words );


    // str1 と str2 間のレーベンシュタイン距離
    // スレ一覧を使用できない場合に使う
    double leven( std::vector< std::vector< int > >& dist,
                  const Glib::ustring& str1, const Glib::ustring& str2 );
}

#endif
