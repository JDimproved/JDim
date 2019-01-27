// ライセンス: GPL2

//
// css 管理クラス
//

#ifndef _CSSMANAGER_H
#define _CSSMANAGER_H

#include <map>
#include <list>
#include <vector>
#include <string>

#include "jdlib/heap.h"

namespace CORE
{
    // cssプロパティ
    struct CSS_PROPERTY
    {
        int align; // text-align

        int border_style;

        ////////////////////
        // 色
        // >=0  のときは色を塗る

        int color;    
        int bg_color;

        int border_left_color;
        int border_right_color;
        int border_top_color;
        int border_bottom_color;

        ////////////////////

        int padding_left;
        int padding_right;
        int padding_top;
        int padding_bottom;

        int padding_left_px;
        int padding_right_px;
        int padding_top_px;
        int padding_bottom_px;

        double padding_left_em;
        double padding_right_em;
        double padding_top_em;
        double padding_bottom_em;

        ////////////////////

        int mrg_left;
        int mrg_right;
        int mrg_top;
        int mrg_bottom;

        int mrg_left_px;
        int mrg_right_px;
        int mrg_top_px;
        int mrg_bottom_px;

        double mrg_left_em;
        double mrg_right_em;
        double mrg_top_em;
        double mrg_bottom_em;

        ////////////////////

        int border_left_width;
        int border_right_width;
        int border_top_width;
        int border_bottom_width;

        int border_left_width_px;
        int border_right_width_px;
        int border_top_width_px;
        int border_bottom_width_px;

        double border_left_width_em;
        double border_right_width_em;
        double border_top_width_em;
        double border_bottom_width_em;
    };


    // text-align
    enum{
        ALIGN_LEFT = 0,
        ALIGN_CENTER,
        ALIGN_RIGHT
    };


    // border-style
    enum{
        BORDER_NONE = 0,
        BORDER_SOLID
    };


    /////////////////////////////////////////


    // DOM構造
    struct DOM
    {
        int nodetype;
        int dat;
        char* chardat;
        int attr;

        DOM* next_dom;
    };

    // nodetype
    enum{
        DOMNODE_DIV,
        DOMNODE_BLOCK,
        DOMNODE_TEXT,
        DOMNODE_IMAGE,
    };

    // attribute flags
    enum{
        DOMATTR_NOBR = 1,
    };

    /////////////////////////////////////////

    
    class Css_Manager
    {
        JDLIB::HEAP m_heap;

        std::map< int, CSS_PROPERTY > m_css;
        std::vector< std::string > m_colors;
        std::list< std::string > m_css_class;

        DOM* m_dom;
        DOM* m_last_dom;

    public:

        Css_Manager();
        virtual ~Css_Manager() noexcept {}

        // ユーザ設定の色取得
        std::string get_color( int colorid );

        // ユーザ設定の色( 先頭は黒 )
        std::vector< std::string >& get_colors() { return m_colors; }

        // クラス名からID取得
        int get_classid( const std::string& classname );

        // プロパティ取得
        CSS_PROPERTY get_property( const int id );

        // 文字の高さを与えてemをセット
        void set_size( CSS_PROPERTY* css, double height );

        // DOM 取得
        const DOM* get_dom() const { return m_dom; }

      private:

        // クラス名登録
        int register_class( const std::string& classname );

        // cssプロパティ関係
        CSS_PROPERTY create_property( std::map< std::string, std::string >& css_pair );
        void set_property( const std::string& classname, const CSS_PROPERTY& css );
        void clear_property( CSS_PROPERTY* css );
        void set_default_css();
        bool read_css();

        // DOM関係
        DOM* create_domnode( int type );
        DOM* create_divnode( const std::string& classname );
        DOM* create_blocknode( int blockid );
        DOM* create_textnode( const char* text );
        DOM* create_imagenode();
        void set_default_html();
        bool read_html();
    };

    ///////////////////////////////////////
    // インターフェース

    Css_Manager* get_css_manager();
    void delete_css_manager();
}

#endif
