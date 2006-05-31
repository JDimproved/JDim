// ライセンス: 最新のGPL

#ifndef _JDREGEX_H
#define _JDREGEX_H

#include <string>
#include <vector>

namespace JDLIB
{
    class Regex
    {
        std::vector< int > m_pos;
        std::vector< std::string > m_results;

    public:

        Regex();
        ~Regex();

        // icase : true なら大小無視
        // newline : true なら . に改行をマッチさせない
        bool exec( const std::string reg, const std::string& target, unsigned int offset = 0
                   , bool icase = false, bool newline = true );
        const int pos( unsigned int num );
        const std::string str( unsigned int num );
    };
}

#endif
