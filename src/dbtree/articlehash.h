// ライセンス: GPL2

//
//  ArticleBaseのチェーン式ハッシュテーブルとイテレータ
//
//  コンパイルが遅くなるのでテンプレートを使用しないでArticleBase専用にした

#ifndef _ARTICLEHASH_H
#define _ARTICLEHASH_H

#include <vector>
#include <string>

namespace DBTREE
{
    class ArticleBase;
    class ArticleHashIterator;

    class ArticleHash
    {
        friend class ArticleHashIterator;

        size_t m_size;
        size_t m_min_hash;
        ArticleHashIterator* m_iterator;
        std::vector< std::vector< ArticleBase* > > m_table;

        // iterator 用変数
        size_t m_it_hash;
        size_t m_it_pos;
        size_t m_it_size;

      public:

        ArticleHash();
        virtual ~ArticleHash();

        size_t size() const { return m_size; }

        void push( ArticleBase* article );

        ArticleBase* find( const std::string& datbase, const std::string& id );

        const ArticleHashIterator begin();
        size_t end() const { return size(); }

      private:

        int get_hash( const std::string& id );

        // iterator 用関数
        ArticleBase* it_get();
        void it_inc();
        size_t it_size() const { return m_it_size; }
    };


    /////////////////////////////////////////////////////


    class ArticleHashIterator
    {
        ArticleHash* m_hashtable;

      public:

        ArticleHashIterator( ArticleHash* hashtable );

        ArticleBase* operator * ();
        ArticleBase* operator ++ ();
        bool operator != ( const size_t size );
    };

}

#endif
