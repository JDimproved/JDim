// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "boardadmin.h"
#include "boardviewnext.h"

#include "dbtree/interface.h"
#include "dbtree/articlebase.h"

#include "skeleton/msgdiag.h"

#include "jdlib/tfidf.h"

#include "config/globalconf.h"

#include "session.h"

using namespace BOARD;


BoardViewNext::BoardViewNext( const std::string& url, const std::string& url_pre_article )
    : BoardViewBase( url, false ),
      m_url_pre_article( url_pre_article )
{
    set_writeable( false );

#ifdef _DEBUG
    std::cout << "BoardViewNext::BoardViewNext : url = " << get_url() << std::endl;
#endif
}


BoardViewNext::~BoardViewNext()
{
#ifdef _DEBUG
    std::cout << "BoardViewNext::~BoardViewNext : url = " << get_url() << std::endl;
#endif
}


//
// リロード
//
void BoardViewNext::reload()
{
    // オフライン
    if( ! SESSION::is_online() ){
        SKELETON::MsgDiag mdiag( get_parent_win(), "オフラインです" );
        mdiag.run();
        return;
    }

    show_view();
}


//
// view更新
//
// subject.txtのロードが終わったら呼ばれる
//
void BoardViewNext::update_view()
{
    std::vector< NEXT_ITEM > next_items;

    update_by_tfidf( next_items );

    std::vector< DBTREE::ArticleBase* >list_article;
    std::vector< NEXT_ITEM >::iterator it_next_items = next_items.begin();
    for( ; it_next_items != next_items.end(); ++it_next_items ) list_article.push_back( ( *it_next_items ).article );

    const bool loading_fin = true;
    update_view_impl( list_article, loading_fin );
}


//
// TFIDFで次スレ検索
//
void BoardViewNext::update_by_tfidf( std::vector< NEXT_ITEM >& next_items )
{
    const Glib::ustring subject_src = DBTREE::article_subject( m_url_pre_article );
    const time_t since_src = DBTREE::article_since_time( m_url_pre_article );

#ifdef _DEBUG
    const int code = DBTREE::board_code( get_url_board() );
    std::cout << "BoardViewNext::update_by_tfidf " << get_url()
              << " code = " << code << " " << subject_src << std::endl
              << "since_src = " << since_src << std::endl;
#endif    

    // 高速化のためデータベースに直接アクセス
    const std::vector< DBTREE::ArticleBase* >& list_subject = DBTREE::board_list_subject( get_url_board() );
    if( ! list_subject.size() ) return;

    // 単語ベクトル作成
    MISC::VEC_WORDS vec_words;
    MISC::tfidf_create_vec_words( vec_words, subject_src );

    // IDFベクトル計算
    MISC::VEC_IDF vec_idf;
    MISC::tfidf_create_vec_idf_from_board( vec_idf, subject_src, list_subject, vec_words );

    // subject_src の TFIDFベクトル計算
    MISC::VEC_TFIDF vec_tfidf_src;
    MISC::VEC_TFIDF vec_tfidf;
    vec_tfidf_src.resize( vec_words.size() );
    vec_tfidf.resize( vec_words.size() );
    MISC::tfidf_calc_vec_tfifd( vec_tfidf_src, subject_src, vec_idf, vec_words );

    // 類似度検索
    std::vector< DBTREE::ArticleBase* >::const_iterator it = list_subject.begin();
    for( ; it != list_subject.end(); ++it ){

        NEXT_ITEM item;

        // 読み込み済みのスレは除外
        item.article = ( *it );
        if( item.article->get_number_load() ) continue;

        item.since = item.article->get_since_time();

        const Glib::ustring subject = item.article->get_subject();
        MISC::tfidf_calc_vec_tfifd( vec_tfidf, subject, vec_idf, vec_words );
        item.value = ( int )( MISC::tfidf_cos_similarity( vec_tfidf_src, vec_tfidf ) * 10 + .5 );
        if( item.value >= CONFIG::get_threshold_next() ){

#ifdef _DEBUG
            std::cout << item.value << " , " << item.since << " | " << subject << std::endl;
#endif

            std::vector< NEXT_ITEM >::iterator it_next_items = next_items.begin();
            for( ; it_next_items != next_items.end(); ++it_next_items ){

                // next が src よりも以前に立てられて、item が src よりも後に立てられた
                // 時は item を前に挿入する
                if( ( *it_next_items ).since < since_src && item.since > since_src ){

                    next_items.insert( it_next_items, item );
                    break;
                }
                else if(  ( ( *it_next_items ).since > since_src && item.since > since_src ) 
                          || ( ( *it_next_items ).since < since_src && item.since < since_src ) ){ 

                    // value -> since の優先度で挿入
                    if( ( *it_next_items ).value < item.value ){

                        next_items.insert( it_next_items, item );
                        break;
                    }
                    else if( ( *it_next_items ).value == item.value ){

                        if( ( *it_next_items ).since > item.since ){

                            next_items.insert( it_next_items, item );
                            break;
                        }
                    }
                }

            }
            if( it_next_items == next_items.end() ) next_items.push_back( item );
        }
    }
}


void BoardViewNext::slot_abone_thread()
{
    SKELETON::MsgDiag mdiag( get_parent_win(), "次スレ検索ではあぼ〜ん出来ません" );
    mdiag.run();
    return;
}


//
// 板名更新
//
void BoardViewNext::update_boardname()
{
    const std::string title = "[ 次スレ検索 ] - " + DBTREE::article_subject( m_url_pre_article );

    // ウィンドウタイトル表示
    set_title( title );
    BOARD::get_admin()->set_command( "set_title", get_url(), get_title() );

    // タブに名前をセット
    BOARD::get_admin()->set_command( "set_tablabel", get_url(), title );
}


//
// デフォルトのソート状態
//
int BoardViewNext::get_default_sort_column()
{
    return COL_ID;
}

int BoardViewNext::get_default_view_sort_mode()
{
    return SORTMODE_ASCEND;
}

int BoardViewNext::get_default_view_sort_pre_column()
{
    return COL_ID;
}

int BoardViewNext::get_default_view_sort_pre_mode()
{
    return SORTMODE_ASCEND;
}
