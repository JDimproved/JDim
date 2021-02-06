// ライセンス: GPL2

// キャレットの座標とかを計算するクラス

#ifndef _CARET_H
#define _CARET_H

#include "layouttree.h"

namespace ARTICLE
{
    class CARET_POSITION {

      public:

        long x{};
        long y{};
        LAYOUT* layout{}; // キャレットの属するレイアウトノードへのポインタ
        long byte{}; // 何バイト目の文字の「前」か

        CARET_POSITION() noexcept = default;
        ~CARET_POSITION() noexcept = default;

        // キャレット座標計算関数
        void set( LAYOUT* _layout, long _byte,

                  // マウスポインタのx座標
                  long _x = 0,

                  // 文字の座標、幅、バイト数
                  long char_x = 0, long char_y = 0, long char_width = 0, long byte_char = 0
            ){

            layout = _layout;
            byte = _byte;
            y = char_y;

            // 文字の真ん中から左にマウスポインタがある
            if( _x <= char_x + char_width / 2 ){ 
                x = char_x;
            }
            
            // 文字の真ん中から右にマウスポインタあるなら次の文字の前にキャレットセット
            else{ 
                x = char_x + char_width;
                byte += byte_char;
            }
        }

        // 後は演算子
        
        bool operator != ( const CARET_POSITION& caret_pos ){

            if( ! layout && ! caret_pos.layout ) return false;
            if( ! layout || ! caret_pos.layout ) return true;
            
            if( layout->id_header != caret_pos.layout->id_header
                || layout->id != caret_pos.layout->id
                || byte != caret_pos.byte ) return true;

            return false;
        }

        bool operator == ( const CARET_POSITION& caret_pos ) { return ! ( *this != caret_pos ); }

        bool operator > ( const CARET_POSITION& caret_pos ){

            if( ! layout && ! caret_pos.layout ) return true;
            if( ! layout ) return false;
            if( ! caret_pos.layout ) return true;

            if( layout->id_header >  caret_pos.layout->id_header ) return true;
            if( layout->id_header <  caret_pos.layout->id_header ) return false;

            // ブロック同じ
            if( layout->id > caret_pos.layout->id ) return true;
            if( layout->id < caret_pos.layout->id ) return false;

            // ノード同じ
            if( byte > caret_pos.byte ) return true;

            return false;
        }

        bool operator < ( const CARET_POSITION& caret_pos ){
            return ! ( *this > caret_pos );
        }
    };

}

#endif
