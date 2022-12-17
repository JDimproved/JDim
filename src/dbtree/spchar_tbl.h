// ライセンス: GPL2
//
/**
 * @file spchar_tbl.h
 * @brief 名前付き文字参照とUTF-8文字列の変換テーブル
 *
 * @details 名前付き文字参照の参考文献
 * * https://html.spec.whatwg.org/entities.json
 * * https://dev.w3.org/html5/spec-LC/named-character-references.html
 *
 * @note char型の文字列リテラルに含まれるユニバーサル文字名(`\uXXXX`)は
 * 実装定義のマルチバイト文字コードにエンコードされるためUTF-8のバイト表現を直接使っている。@n
 * u8プレフィックスはC++20から`char8_t`が追加される影響でエラーとなるため使っていない。
 *
 * @note 検索の高速化のため UCSTBL の配列は entity の辞書順に並べてある
 */

#ifndef _SPCHAR_TBL_H
#define _SPCHAR_TBL_H

#include "jdlib/span.h"

#include <string_view>


/**
 * @brief 名前付き文字参照 <-> UTF-8文字列の変換テーブル
 *
 * @details 文字参照の中には2つのコードポイントへ変換されるものも存在する。
 * 変換処理を簡略化するためスカラー値(`char16_t` や `char32_t`)ではなくUTF-8の文字列でデータを保持する。
 */
struct UCSTBL
{
    std::string_view entity; ///< Named Character Reference
    std::string_view utf8; ///< UTF-8文字列 (1〜2文字)
};


static constexpr UCSTBL const ucstbl_A[] = {
    { "AElig;", "\xC3\x86" }, // U+00C6
    { "AMP;", "\x26" }, // U+0026
    { "Aacute;", "\xC3\x81" }, // U+00C1
    { "Abreve;", "\xC4\x82" }, // U+0102
    { "Acirc;", "\xC3\x82" }, // U+00C2
    { "Acy;", "\xD0\x90" }, // U+0410
    { "Afr;", "\xF0\x9D\x94\x84" }, // U+1D504
    { "Agrave;", "\xC3\x80" }, // U+00C0
    { "Alpha;", "\xCE\x91" }, // U+0391
    { "Amacr;", "\xC4\x80" }, // U+0100
    { "And;", "\xE2\xA9\x93" }, // U+2A53
    { "Aogon;", "\xC4\x84" }, // U+0104
    { "Aopf;", "\xF0\x9D\x94\xB8" }, // U+1D538
    { "ApplyFunction;", "\xE2\x81\xA1" }, // U+2061
    { "Aring;", "\xC3\x85" }, // U+00C5
    { "Ascr;", "\xF0\x9D\x92\x9C" }, // U+1D49C
    { "Assign;", "\xE2\x89\x94" }, // U+2254
    { "Atilde;", "\xC3\x83" }, // U+00C3
    { "Auml;", "\xC3\x84" }, // U+00C4
};

static constexpr UCSTBL const ucstbl_B[] = {
    { "Backslash;", "\xE2\x88\x96" }, // U+2216
    { "Barv;", "\xE2\xAB\xA7" }, // U+2AE7
    { "Barwed;", "\xE2\x8C\x86" }, // U+2306
    { "Bcy;", "\xD0\x91" }, // U+0411
    { "Because;", "\xE2\x88\xB5" }, // U+2235
    { "Bernoullis;", "\xE2\x84\xAC" }, // U+212C
    { "Beta;", "\xCE\x92" }, // U+0392
    { "Bfr;", "\xF0\x9D\x94\x85" }, // U+1D505
    { "Bopf;", "\xF0\x9D\x94\xB9" }, // U+1D539
    { "Breve;", "\xCB\x98" }, // U+02D8
    { "Bscr;", "\xE2\x84\xAC" }, // U+212C
    { "Bumpeq;", "\xE2\x89\x8E" }, // U+224E
};

static constexpr UCSTBL const ucstbl_C[] = {
    { "CHcy;", "\xD0\xA7" }, // U+0427
    { "COPY;", "\xC2\xA9" }, // U+00A9
    { "Cacute;", "\xC4\x86" }, // U+0106
    { "Cap;", "\xE2\x8B\x92" }, // U+22D2
    { "CapitalDifferentialD;", "\xE2\x85\x85" }, // U+2145
    { "Cayleys;", "\xE2\x84\xAD" }, // U+212D
    { "Ccaron;", "\xC4\x8C" }, // U+010C
    { "Ccedil;", "\xC3\x87" }, // U+00C7
    { "Ccirc;", "\xC4\x88" }, // U+0108
    { "Cconint;", "\xE2\x88\xB0" }, // U+2230
    { "Cdot;", "\xC4\x8A" }, // U+010A
    { "Cedilla;", "\xC2\xB8" }, // U+00B8
    { "CenterDot;", "\xC2\xB7" }, // U+00B7
    { "Cfr;", "\xE2\x84\xAD" }, // U+212D
    { "Chi;", "\xCE\xA7" }, // U+03A7
    { "CircleDot;", "\xE2\x8A\x99" }, // U+2299
    { "CircleMinus;", "\xE2\x8A\x96" }, // U+2296
    { "CirclePlus;", "\xE2\x8A\x95" }, // U+2295
    { "CircleTimes;", "\xE2\x8A\x97" }, // U+2297
    { "ClockwiseContourIntegral;", "\xE2\x88\xB2" }, // U+2232
    { "CloseCurlyDoubleQuote;", "\xE2\x80\x9D" }, // U+201D
    { "CloseCurlyQuote;", "\xE2\x80\x99" }, // U+2019
    { "Colon;", "\xE2\x88\xB7" }, // U+2237
    { "Colone;", "\xE2\xA9\xB4" }, // U+2A74
    { "Congruent;", "\xE2\x89\xA1" }, // U+2261
    { "Conint;", "\xE2\x88\xAF" }, // U+222F
    { "ContourIntegral;", "\xE2\x88\xAE" }, // U+222E
    { "Copf;", "\xE2\x84\x82" }, // U+2102
    { "Coproduct;", "\xE2\x88\x90" }, // U+2210
    { "CounterClockwiseContourIntegral;", "\xE2\x88\xB3" }, // U+2233
    { "Cross;", "\xE2\xA8\xAF" }, // U+2A2F
    { "Cscr;", "\xF0\x9D\x92\x9E" }, // U+1D49E
    { "Cup;", "\xE2\x8B\x93" }, // U+22D3
    { "CupCap;", "\xE2\x89\x8D" }, // U+224D
};

static constexpr UCSTBL const ucstbl_D[] = {
    { "DD;", "\xE2\x85\x85" }, // U+2145
    { "DDotrahd;", "\xE2\xA4\x91" }, // U+2911
    { "DJcy;", "\xD0\x82" }, // U+0402
    { "DScy;", "\xD0\x85" }, // U+0405
    { "DZcy;", "\xD0\x8F" }, // U+040F
    { "Dagger;", "\xE2\x80\xA1" }, // U+2021
    { "Darr;", "\xE2\x86\xA1" }, // U+21A1
    { "Dashv;", "\xE2\xAB\xA4" }, // U+2AE4
    { "Dcaron;", "\xC4\x8E" }, // U+010E
    { "Dcy;", "\xD0\x94" }, // U+0414
    { "Del;", "\xE2\x88\x87" }, // U+2207
    { "Delta;", "\xCE\x94" }, // U+0394
    { "Dfr;", "\xF0\x9D\x94\x87" }, // U+1D507
    { "DiacriticalAcute;", "\xC2\xB4" }, // U+00B4
    { "DiacriticalDot;", "\xCB\x99" }, // U+02D9
    { "DiacriticalDoubleAcute;", "\xCB\x9D" }, // U+02DD
    { "DiacriticalGrave;", "\x60" }, // U+0060
    { "DiacriticalTilde;", "\xCB\x9C" }, // U+02DC
    { "Diamond;", "\xE2\x8B\x84" }, // U+22C4
    { "DifferentialD;", "\xE2\x85\x86" }, // U+2146
    { "Dopf;", "\xF0\x9D\x94\xBB" }, // U+1D53B
    { "Dot;", "\xC2\xA8" }, // U+00A8
    { "DotDot;", "\xE2\x83\x9C" }, // U+20DC
    { "DotEqual;", "\xE2\x89\x90" }, // U+2250
    { "DoubleContourIntegral;", "\xE2\x88\xAF" }, // U+222F
    { "DoubleDot;", "\xC2\xA8" }, // U+00A8
    { "DoubleDownArrow;", "\xE2\x87\x93" }, // U+21D3
    { "DoubleLeftArrow;", "\xE2\x87\x90" }, // U+21D0
    { "DoubleLeftRightArrow;", "\xE2\x87\x94" }, // U+21D4
    { "DoubleLeftTee;", "\xE2\xAB\xA4" }, // U+2AE4
    { "DoubleLongLeftArrow;", "\xE2\x9F\xB8" }, // U+27F8
    { "DoubleLongLeftRightArrow;", "\xE2\x9F\xBA" }, // U+27FA
    { "DoubleLongRightArrow;", "\xE2\x9F\xB9" }, // U+27F9
    { "DoubleRightArrow;", "\xE2\x87\x92" }, // U+21D2
    { "DoubleRightTee;", "\xE2\x8A\xA8" }, // U+22A8
    { "DoubleUpArrow;", "\xE2\x87\x91" }, // U+21D1
    { "DoubleUpDownArrow;", "\xE2\x87\x95" }, // U+21D5
    { "DoubleVerticalBar;", "\xE2\x88\xA5" }, // U+2225
    { "DownArrow;", "\xE2\x86\x93" }, // U+2193
    { "DownArrowBar;", "\xE2\xA4\x93" }, // U+2913
    { "DownArrowUpArrow;", "\xE2\x87\xB5" }, // U+21F5
    { "DownBreve;", "\xCC\x91" }, // U+0311
    { "DownLeftRightVector;", "\xE2\xA5\x90" }, // U+2950
    { "DownLeftTeeVector;", "\xE2\xA5\x9E" }, // U+295E
    { "DownLeftVector;", "\xE2\x86\xBD" }, // U+21BD
    { "DownLeftVectorBar;", "\xE2\xA5\x96" }, // U+2956
    { "DownRightTeeVector;", "\xE2\xA5\x9F" }, // U+295F
    { "DownRightVector;", "\xE2\x87\x81" }, // U+21C1
    { "DownRightVectorBar;", "\xE2\xA5\x97" }, // U+2957
    { "DownTee;", "\xE2\x8A\xA4" }, // U+22A4
    { "DownTeeArrow;", "\xE2\x86\xA7" }, // U+21A7
    { "Downarrow;", "\xE2\x87\x93" }, // U+21D3
    { "Dscr;", "\xF0\x9D\x92\x9F" }, // U+1D49F
    { "Dstrok;", "\xC4\x90" }, // U+0110
};

static constexpr UCSTBL const ucstbl_E[] = {
    { "ENG;", "\xC5\x8A" }, // U+014A
    { "ETH;", "\xC3\x90" }, // U+00D0
    { "Eacute;", "\xC3\x89" }, // U+00C9
    { "Ecaron;", "\xC4\x9A" }, // U+011A
    { "Ecirc;", "\xC3\x8A" }, // U+00CA
    { "Ecy;", "\xD0\xAD" }, // U+042D
    { "Edot;", "\xC4\x96" }, // U+0116
    { "Efr;", "\xF0\x9D\x94\x88" }, // U+1D508
    { "Egrave;", "\xC3\x88" }, // U+00C8
    { "Element;", "\xE2\x88\x88" }, // U+2208
    { "Emacr;", "\xC4\x92" }, // U+0112
    { "EmptySmallSquare;", "\xE2\x97\xBB" }, // U+25FB
    { "EmptyVerySmallSquare;", "\xE2\x96\xAB" }, // U+25AB
    { "Eogon;", "\xC4\x98" }, // U+0118
    { "Eopf;", "\xF0\x9D\x94\xBC" }, // U+1D53C
    { "Epsilon;", "\xCE\x95" }, // U+0395
    { "Equal;", "\xE2\xA9\xB5" }, // U+2A75
    { "EqualTilde;", "\xE2\x89\x82" }, // U+2242
    { "Equilibrium;", "\xE2\x87\x8C" }, // U+21CC
    { "Escr;", "\xE2\x84\xB0" }, // U+2130
    { "Esim;", "\xE2\xA9\xB3" }, // U+2A73
    { "Eta;", "\xCE\x97" }, // U+0397
    { "Euml;", "\xC3\x8B" }, // U+00CB
    { "Exists;", "\xE2\x88\x83" }, // U+2203
    { "ExponentialE;", "\xE2\x85\x87" }, // U+2147
};

static constexpr UCSTBL const ucstbl_F[] = {
    { "Fcy;", "\xD0\xA4" }, // U+0424
    { "Ffr;", "\xF0\x9D\x94\x89" }, // U+1D509
    { "FilledSmallSquare;", "\xE2\x97\xBC" }, // U+25FC
    { "FilledVerySmallSquare;", "\xE2\x96\xAA" }, // U+25AA
    { "Fopf;", "\xF0\x9D\x94\xBD" }, // U+1D53D
    { "ForAll;", "\xE2\x88\x80" }, // U+2200
    { "Fouriertrf;", "\xE2\x84\xB1" }, // U+2131
    { "Fscr;", "\xE2\x84\xB1" }, // U+2131
};

static constexpr UCSTBL const ucstbl_G[] = {
    { "GJcy;", "\xD0\x83" }, // U+0403
    { "GT;", "\x3E" }, // U+003E
    { "Gamma;", "\xCE\x93" }, // U+0393
    { "Gammad;", "\xCF\x9C" }, // U+03DC
    { "Gbreve;", "\xC4\x9E" }, // U+011E
    { "Gcedil;", "\xC4\xA2" }, // U+0122
    { "Gcirc;", "\xC4\x9C" }, // U+011C
    { "Gcy;", "\xD0\x93" }, // U+0413
    { "Gdot;", "\xC4\xA0" }, // U+0120
    { "Gfr;", "\xF0\x9D\x94\x8A" }, // U+1D50A
    { "Gg;", "\xE2\x8B\x99" }, // U+22D9
    { "Gopf;", "\xF0\x9D\x94\xBE" }, // U+1D53E
    { "GreaterEqual;", "\xE2\x89\xA5" }, // U+2265
    { "GreaterEqualLess;", "\xE2\x8B\x9B" }, // U+22DB
    { "GreaterFullEqual;", "\xE2\x89\xA7" }, // U+2267
    { "GreaterGreater;", "\xE2\xAA\xA2" }, // U+2AA2
    { "GreaterLess;", "\xE2\x89\xB7" }, // U+2277
    { "GreaterSlantEqual;", "\xE2\xA9\xBE" }, // U+2A7E
    { "GreaterTilde;", "\xE2\x89\xB3" }, // U+2273
    { "Gscr;", "\xF0\x9D\x92\xA2" }, // U+1D4A2
    { "Gt;", "\xE2\x89\xAB" }, // U+226B
};

static constexpr UCSTBL const ucstbl_H[] = {
    { "HARDcy;", "\xD0\xAA" }, // U+042A
    { "Hacek;", "\xCB\x87" }, // U+02C7
    { "Hat;", "\x5E" }, // U+005E
    { "Hcirc;", "\xC4\xA4" }, // U+0124
    { "Hfr;", "\xE2\x84\x8C" }, // U+210C
    { "HilbertSpace;", "\xE2\x84\x8B" }, // U+210B
    { "Hopf;", "\xE2\x84\x8D" }, // U+210D
    { "HorizontalLine;", "\xE2\x94\x80" }, // U+2500
    { "Hscr;", "\xE2\x84\x8B" }, // U+210B
    { "Hstrok;", "\xC4\xA6" }, // U+0126
    { "HumpDownHump;", "\xE2\x89\x8E" }, // U+224E
    { "HumpEqual;", "\xE2\x89\x8F" }, // U+224F
};

static constexpr UCSTBL const ucstbl_I[] = {
    { "IEcy;", "\xD0\x95" }, // U+0415
    { "IJlig;", "\xC4\xB2" }, // U+0132
    { "IOcy;", "\xD0\x81" }, // U+0401
    { "Iacute;", "\xC3\x8D" }, // U+00CD
    { "Icirc;", "\xC3\x8E" }, // U+00CE
    { "Icy;", "\xD0\x98" }, // U+0418
    { "Idot;", "\xC4\xB0" }, // U+0130
    { "Ifr;", "\xE2\x84\x91" }, // U+2111
    { "Igrave;", "\xC3\x8C" }, // U+00CC
    { "Im;", "\xE2\x84\x91" }, // U+2111
    { "Imacr;", "\xC4\xAA" }, // U+012A
    { "ImaginaryI;", "\xE2\x85\x88" }, // U+2148
    { "Implies;", "\xE2\x87\x92" }, // U+21D2
    { "Int;", "\xE2\x88\xAC" }, // U+222C
    { "Integral;", "\xE2\x88\xAB" }, // U+222B
    { "Intersection;", "\xE2\x8B\x82" }, // U+22C2
    { "InvisibleComma;", "\xE2\x81\xA3" }, // U+2063
    { "InvisibleTimes;", "\xE2\x81\xA2" }, // U+2062
    { "Iogon;", "\xC4\xAE" }, // U+012E
    { "Iopf;", "\xF0\x9D\x95\x80" }, // U+1D540
    { "Iota;", "\xCE\x99" }, // U+0399
    { "Iscr;", "\xE2\x84\x90" }, // U+2110
    { "Itilde;", "\xC4\xA8" }, // U+0128
    { "Iukcy;", "\xD0\x86" }, // U+0406
    { "Iuml;", "\xC3\x8F" }, // U+00CF
};

static constexpr UCSTBL const ucstbl_J[] = {
    { "Jcirc;", "\xC4\xB4" }, // U+0134
    { "Jcy;", "\xD0\x99" }, // U+0419
    { "Jfr;", "\xF0\x9D\x94\x8D" }, // U+1D50D
    { "Jopf;", "\xF0\x9D\x95\x81" }, // U+1D541
    { "Jscr;", "\xF0\x9D\x92\xA5" }, // U+1D4A5
    { "Jsercy;", "\xD0\x88" }, // U+0408
    { "Jukcy;", "\xD0\x84" }, // U+0404
};

static constexpr UCSTBL const ucstbl_K[] = {
    { "KHcy;", "\xD0\xA5" }, // U+0425
    { "KJcy;", "\xD0\x8C" }, // U+040C
    { "Kappa;", "\xCE\x9A" }, // U+039A
    { "Kcedil;", "\xC4\xB6" }, // U+0136
    { "Kcy;", "\xD0\x9A" }, // U+041A
    { "Kfr;", "\xF0\x9D\x94\x8E" }, // U+1D50E
    { "Kopf;", "\xF0\x9D\x95\x82" }, // U+1D542
    { "Kscr;", "\xF0\x9D\x92\xA6" }, // U+1D4A6
};

static constexpr UCSTBL const ucstbl_L[] = {
    { "LJcy;", "\xD0\x89" }, // U+0409
    { "LT;", "\x3C" }, // U+003C
    { "Lacute;", "\xC4\xB9" }, // U+0139
    { "Lambda;", "\xCE\x9B" }, // U+039B
    { "Lang;", "\xE2\x9F\xAA" }, // U+27EA
    { "Laplacetrf;", "\xE2\x84\x92" }, // U+2112
    { "Larr;", "\xE2\x86\x9E" }, // U+219E
    { "Lcaron;", "\xC4\xBD" }, // U+013D
    { "Lcedil;", "\xC4\xBB" }, // U+013B
    { "Lcy;", "\xD0\x9B" }, // U+041B
    { "LeftAngleBracket;", "\xE2\x9F\xA8" }, // U+27E8
    { "LeftArrow;", "\xE2\x86\x90" }, // U+2190
    { "LeftArrowBar;", "\xE2\x87\xA4" }, // U+21E4
    { "LeftArrowRightArrow;", "\xE2\x87\x86" }, // U+21C6
    { "LeftCeiling;", "\xE2\x8C\x88" }, // U+2308
    { "LeftDoubleBracket;", "\xE2\x9F\xA6" }, // U+27E6
    { "LeftDownTeeVector;", "\xE2\xA5\xA1" }, // U+2961
    { "LeftDownVector;", "\xE2\x87\x83" }, // U+21C3
    { "LeftDownVectorBar;", "\xE2\xA5\x99" }, // U+2959
    { "LeftFloor;", "\xE2\x8C\x8A" }, // U+230A
    { "LeftRightArrow;", "\xE2\x86\x94" }, // U+2194
    { "LeftRightVector;", "\xE2\xA5\x8E" }, // U+294E
    { "LeftTee;", "\xE2\x8A\xA3" }, // U+22A3
    { "LeftTeeArrow;", "\xE2\x86\xA4" }, // U+21A4
    { "LeftTeeVector;", "\xE2\xA5\x9A" }, // U+295A
    { "LeftTriangle;", "\xE2\x8A\xB2" }, // U+22B2
    { "LeftTriangleBar;", "\xE2\xA7\x8F" }, // U+29CF
    { "LeftTriangleEqual;", "\xE2\x8A\xB4" }, // U+22B4
    { "LeftUpDownVector;", "\xE2\xA5\x91" }, // U+2951
    { "LeftUpTeeVector;", "\xE2\xA5\xA0" }, // U+2960
    { "LeftUpVector;", "\xE2\x86\xBF" }, // U+21BF
    { "LeftUpVectorBar;", "\xE2\xA5\x98" }, // U+2958
    { "LeftVector;", "\xE2\x86\xBC" }, // U+21BC
    { "LeftVectorBar;", "\xE2\xA5\x92" }, // U+2952
    { "Leftarrow;", "\xE2\x87\x90" }, // U+21D0
    { "Leftrightarrow;", "\xE2\x87\x94" }, // U+21D4
    { "LessEqualGreater;", "\xE2\x8B\x9A" }, // U+22DA
    { "LessFullEqual;", "\xE2\x89\xA6" }, // U+2266
    { "LessGreater;", "\xE2\x89\xB6" }, // U+2276
    { "LessLess;", "\xE2\xAA\xA1" }, // U+2AA1
    { "LessSlantEqual;", "\xE2\xA9\xBD" }, // U+2A7D
    { "LessTilde;", "\xE2\x89\xB2" }, // U+2272
    { "Lfr;", "\xF0\x9D\x94\x8F" }, // U+1D50F
    { "Ll;", "\xE2\x8B\x98" }, // U+22D8
    { "Lleftarrow;", "\xE2\x87\x9A" }, // U+21DA
    { "Lmidot;", "\xC4\xBF" }, // U+013F
    { "LongLeftArrow;", "\xE2\x9F\xB5" }, // U+27F5
    { "LongLeftRightArrow;", "\xE2\x9F\xB7" }, // U+27F7
    { "LongRightArrow;", "\xE2\x9F\xB6" }, // U+27F6
    { "Longleftarrow;", "\xE2\x9F\xB8" }, // U+27F8
    { "Longleftrightarrow;", "\xE2\x9F\xBA" }, // U+27FA
    { "Longrightarrow;", "\xE2\x9F\xB9" }, // U+27F9
    { "Lopf;", "\xF0\x9D\x95\x83" }, // U+1D543
    { "LowerLeftArrow;", "\xE2\x86\x99" }, // U+2199
    { "LowerRightArrow;", "\xE2\x86\x98" }, // U+2198
    { "Lscr;", "\xE2\x84\x92" }, // U+2112
    { "Lsh;", "\xE2\x86\xB0" }, // U+21B0
    { "Lstrok;", "\xC5\x81" }, // U+0141
    { "Lt;", "\xE2\x89\xAA" }, // U+226A
};

static constexpr UCSTBL const ucstbl_M[] = {
    { "Map;", "\xE2\xA4\x85" }, // U+2905
    { "Mcy;", "\xD0\x9C" }, // U+041C
    { "MediumSpace;", "\xE2\x81\x9F" }, // U+205F
    { "Mellintrf;", "\xE2\x84\xB3" }, // U+2133
    { "Mfr;", "\xF0\x9D\x94\x90" }, // U+1D510
    { "MinusPlus;", "\xE2\x88\x93" }, // U+2213
    { "Mopf;", "\xF0\x9D\x95\x84" }, // U+1D544
    { "Mscr;", "\xE2\x84\xB3" }, // U+2133
    { "Mu;", "\xCE\x9C" }, // U+039C
};

static constexpr UCSTBL const ucstbl_N[] = {
    { "NJcy;", "\xD0\x8A" }, // U+040A
    { "Nacute;", "\xC5\x83" }, // U+0143
    { "Ncaron;", "\xC5\x87" }, // U+0147
    { "Ncedil;", "\xC5\x85" }, // U+0145
    { "Ncy;", "\xD0\x9D" }, // U+041D
    { "NegativeMediumSpace;", "\xE2\x80\x8B" }, // U+200B ZERO WIDTH SPACE
    { "NegativeThickSpace;", "\xE2\x80\x8B" }, // U+200B ZERO WIDTH SPACE
    { "NegativeThinSpace;", "\xE2\x80\x8B" }, // U+200B ZERO WIDTH SPACE
    { "NegativeVeryThinSpace;", "\xE2\x80\x8B" }, // U+200B ZERO WIDTH SPACE
    { "NestedGreaterGreater;", "\xE2\x89\xAB" }, // U+226B
    { "NestedLessLess;", "\xE2\x89\xAA" }, // U+226A
    { "NewLine;", "\x0A" }, // U+000A
    { "Nfr;", "\xF0\x9D\x94\x91" }, // U+1D511
    { "NoBreak;", "\xE2\x81\xA0" }, // U+2060
    { "NonBreakingSpace;", "\xC2\xA0" }, // U+00A0
    { "Nopf;", "\xE2\x84\x95" }, // U+2115
    { "Not;", "\xE2\xAB\xAC" }, // U+2AEC
    { "NotCongruent;", "\xE2\x89\xA2" }, // U+2262
    { "NotCupCap;", "\xE2\x89\xAD" }, // U+226D
    { "NotDoubleVerticalBar;", "\xE2\x88\xA6" }, // U+2226
    { "NotElement;", "\xE2\x88\x89" }, // U+2209
    { "NotEqual;", "\xE2\x89\xA0" }, // U+2260
    { "NotEqualTilde;", "\xE2\x89\x82\xCC\xB8" }, // U+2242 U+0338
    { "NotExists;", "\xE2\x88\x84" }, // U+2204
    { "NotGreater;", "\xE2\x89\xAF" }, // U+226F
    { "NotGreaterEqual;", "\xE2\x89\xB1" }, // U+2271
    { "NotGreaterFullEqual;", "\xE2\x89\xA7\xCC\xB8" }, // U+2267 U+0338
    { "NotGreaterGreater;", "\xE2\x89\xAB\xCC\xB8" }, // U+226B U+0338
    { "NotGreaterLess;", "\xE2\x89\xB9" }, // U+2279
    { "NotGreaterSlantEqual;", "\xE2\xA9\xBE\xCC\xB8" }, // U+2A7E U+0338
    { "NotGreaterTilde;", "\xE2\x89\xB5" }, // U+2275
    { "NotHumpDownHump;", "\xE2\x89\x8E\xCC\xB8" }, // U+224E U+0338
    { "NotHumpEqual;", "\xE2\x89\x8F\xCC\xB8" }, // U+224F U+0338
    { "NotLeftTriangle;", "\xE2\x8B\xAA" }, // U+22EA
    { "NotLeftTriangleBar;", "\xE2\xA7\x8F\xCC\xB8" }, // U+29CF U+0338
    { "NotLeftTriangleEqual;", "\xE2\x8B\xAC" }, // U+22EC
    { "NotLess;", "\xE2\x89\xAE" }, // U+226E
    { "NotLessEqual;", "\xE2\x89\xB0" }, // U+2270
    { "NotLessGreater;", "\xE2\x89\xB8" }, // U+2278
    { "NotLessLess;", "\xE2\x89\xAA\xCC\xB8" }, // U+226A U+0338
    { "NotLessSlantEqual;", "\xE2\xA9\xBD\xCC\xB8" }, // U+2A7D U+0338
    { "NotLessTilde;", "\xE2\x89\xB4" }, // U+2274
    { "NotNestedGreaterGreater;", "\xE2\xAA\xA2\xCC\xB8" }, // U+2AA2 U+0338
    { "NotNestedLessLess;", "\xE2\xAA\xA1\xCC\xB8" }, // U+2AA1 U+0338
    { "NotPrecedes;", "\xE2\x8A\x80" }, // U+2280
    { "NotPrecedesEqual;", "\xE2\xAA\xAF\xCC\xB8" }, // U+2AAF U+0338
    { "NotPrecedesSlantEqual;", "\xE2\x8B\xA0" }, // U+22E0
    { "NotReverseElement;", "\xE2\x88\x8C" }, // U+220C
    { "NotRightTriangle;", "\xE2\x8B\xAB" }, // U+22EB
    { "NotRightTriangleBar;", "\xE2\xA7\x90\xCC\xB8" }, // U+29D0 U+0338
    { "NotRightTriangleEqual;", "\xE2\x8B\xAD" }, // U+22ED
    { "NotSquareSubset;", "\xE2\x8A\x8F\xCC\xB8" }, // U+228F U+0338
    { "NotSquareSubsetEqual;", "\xE2\x8B\xA2" }, // U+22E2
    { "NotSquareSuperset;", "\xE2\x8A\x90\xCC\xB8" }, // U+2290 U+0338
    { "NotSquareSupersetEqual;", "\xE2\x8B\xA3" }, // U+22E3
    { "NotSubset;", "\xE2\x8A\x82\xE2\x83\x92" }, // U+2282 U+20D2
    { "NotSubsetEqual;", "\xE2\x8A\x88" }, // U+2288
    { "NotSucceeds;", "\xE2\x8A\x81" }, // U+2281
    { "NotSucceedsEqual;", "\xE2\xAA\xB0\xCC\xB8" }, // U+2AB0 U+0338
    { "NotSucceedsSlantEqual;", "\xE2\x8B\xA1" }, // U+22E1
    { "NotSucceedsTilde;", "\xE2\x89\xBF\xCC\xB8" }, // U+227F U+0338
    { "NotSuperset;", "\xE2\x8A\x83\xE2\x83\x92" }, // U+2283 U+20D2
    { "NotSupersetEqual;", "\xE2\x8A\x89" }, // U+2289
    { "NotTilde;", "\xE2\x89\x81" }, // U+2241
    { "NotTildeEqual;", "\xE2\x89\x84" }, // U+2244
    { "NotTildeFullEqual;", "\xE2\x89\x87" }, // U+2247
    { "NotTildeTilde;", "\xE2\x89\x89" }, // U+2249
    { "NotVerticalBar;", "\xE2\x88\xA4" }, // U+2224
    { "Nscr;", "\xF0\x9D\x92\xA9" }, // U+1D4A9
    { "Ntilde;", "\xC3\x91" }, // U+00D1
    { "Nu;", "\xCE\x9D" }, // U+039D
};

static constexpr UCSTBL const ucstbl_O[] = {
    { "OElig;", "\xC5\x92" }, // U+0152
    { "Oacute;", "\xC3\x93" }, // U+00D3
    { "Ocirc;", "\xC3\x94" }, // U+00D4
    { "Ocy;", "\xD0\x9E" }, // U+041E
    { "Odblac;", "\xC5\x90" }, // U+0150
    { "Ofr;", "\xF0\x9D\x94\x92" }, // U+1D512
    { "Ograve;", "\xC3\x92" }, // U+00D2
    { "Omacr;", "\xC5\x8C" }, // U+014C
    { "Omega;", "\xCE\xA9" }, // U+03A9
    { "Omicron;", "\xCE\x9F" }, // U+039F
    { "Oopf;", "\xF0\x9D\x95\x86" }, // U+1D546
    { "OpenCurlyDoubleQuote;", "\xE2\x80\x9C" }, // U+201C
    { "OpenCurlyQuote;", "\xE2\x80\x98" }, // U+2018
    { "Or;", "\xE2\xA9\x94" }, // U+2A54
    { "Oscr;", "\xF0\x9D\x92\xAA" }, // U+1D4AA
    { "Oslash;", "\xC3\x98" }, // U+00D8
    { "Otilde;", "\xC3\x95" }, // U+00D5
    { "Otimes;", "\xE2\xA8\xB7" }, // U+2A37
    { "Ouml;", "\xC3\x96" }, // U+00D6
    { "OverBar;", "\xE2\x80\xBE" }, // U+203E
    { "OverBrace;", "\xE2\x8F\x9E" }, // U+23DE
    { "OverBracket;", "\xE2\x8E\xB4" }, // U+23B4
    { "OverParenthesis;", "\xE2\x8F\x9C" }, // U+23DC
};

static constexpr UCSTBL const ucstbl_P[] = {
    { "PartialD;", "\xE2\x88\x82" }, // U+2202
    { "Pcy;", "\xD0\x9F" }, // U+041F
    { "Pfr;", "\xF0\x9D\x94\x93" }, // U+1D513
    { "Phi;", "\xCE\xA6" }, // U+03A6
    { "Pi;", "\xCE\xA0" }, // U+03A0
    { "PlusMinus;", "\xC2\xB1" }, // U+00B1
    { "Poincareplane;", "\xE2\x84\x8C" }, // U+210C
    { "Popf;", "\xE2\x84\x99" }, // U+2119
    { "Pr;", "\xE2\xAA\xBB" }, // U+2ABB
    { "Precedes;", "\xE2\x89\xBA" }, // U+227A
    { "PrecedesEqual;", "\xE2\xAA\xAF" }, // U+2AAF
    { "PrecedesSlantEqual;", "\xE2\x89\xBC" }, // U+227C
    { "PrecedesTilde;", "\xE2\x89\xBE" }, // U+227E
    { "Prime;", "\xE2\x80\xB3" }, // U+2033
    { "Product;", "\xE2\x88\x8F" }, // U+220F
    { "Proportion;", "\xE2\x88\xB7" }, // U+2237
    { "Proportional;", "\xE2\x88\x9D" }, // U+221D
    { "Pscr;", "\xF0\x9D\x92\xAB" }, // U+1D4AB
    { "Psi;", "\xCE\xA8" }, // U+03A8
};

static constexpr UCSTBL const ucstbl_Q[] = {
    { "QUOT;", "\x22" }, // U+0022
    { "Qfr;", "\xF0\x9D\x94\x94" }, // U+1D514
    { "Qopf;", "\xE2\x84\x9A" }, // U+211A
    { "Qscr;", "\xF0\x9D\x92\xAC" }, // U+1D4AC
};

static constexpr UCSTBL const ucstbl_R[] = {
    { "RBarr;", "\xE2\xA4\x90" }, // U+2910
    { "REG;", "\xC2\xAE" }, // U+00AE
    { "Racute;", "\xC5\x94" }, // U+0154
    { "Rang;", "\xE2\x9F\xAB" }, // U+27EB
    { "Rarr;", "\xE2\x86\xA0" }, // U+21A0
    { "Rarrtl;", "\xE2\xA4\x96" }, // U+2916
    { "Rcaron;", "\xC5\x98" }, // U+0158
    { "Rcedil;", "\xC5\x96" }, // U+0156
    { "Rcy;", "\xD0\xA0" }, // U+0420
    { "Re;", "\xE2\x84\x9C" }, // U+211C
    { "ReverseElement;", "\xE2\x88\x8B" }, // U+220B
    { "ReverseEquilibrium;", "\xE2\x87\x8B" }, // U+21CB
    { "ReverseUpEquilibrium;", "\xE2\xA5\xAF" }, // U+296F
    { "Rfr;", "\xE2\x84\x9C" }, // U+211C
    { "Rho;", "\xCE\xA1" }, // U+03A1
    { "RightAngleBracket;", "\xE2\x9F\xA9" }, // U+27E9
    { "RightArrow;", "\xE2\x86\x92" }, // U+2192
    { "RightArrowBar;", "\xE2\x87\xA5" }, // U+21E5
    { "RightArrowLeftArrow;", "\xE2\x87\x84" }, // U+21C4
    { "RightCeiling;", "\xE2\x8C\x89" }, // U+2309
    { "RightDoubleBracket;", "\xE2\x9F\xA7" }, // U+27E7
    { "RightDownTeeVector;", "\xE2\xA5\x9D" }, // U+295D
    { "RightDownVector;", "\xE2\x87\x82" }, // U+21C2
    { "RightDownVectorBar;", "\xE2\xA5\x95" }, // U+2955
    { "RightFloor;", "\xE2\x8C\x8B" }, // U+230B
    { "RightTee;", "\xE2\x8A\xA2" }, // U+22A2
    { "RightTeeArrow;", "\xE2\x86\xA6" }, // U+21A6
    { "RightTeeVector;", "\xE2\xA5\x9B" }, // U+295B
    { "RightTriangle;", "\xE2\x8A\xB3" }, // U+22B3
    { "RightTriangleBar;", "\xE2\xA7\x90" }, // U+29D0
    { "RightTriangleEqual;", "\xE2\x8A\xB5" }, // U+22B5
    { "RightUpDownVector;", "\xE2\xA5\x8F" }, // U+294F
    { "RightUpTeeVector;", "\xE2\xA5\x9C" }, // U+295C
    { "RightUpVector;", "\xE2\x86\xBE" }, // U+21BE
    { "RightUpVectorBar;", "\xE2\xA5\x94" }, // U+2954
    { "RightVector;", "\xE2\x87\x80" }, // U+21C0
    { "RightVectorBar;", "\xE2\xA5\x93" }, // U+2953
    { "Rightarrow;", "\xE2\x87\x92" }, // U+21D2
    { "Ropf;", "\xE2\x84\x9D" }, // U+211D
    { "RoundImplies;", "\xE2\xA5\xB0" }, // U+2970
    { "Rrightarrow;", "\xE2\x87\x9B" }, // U+21DB
    { "Rscr;", "\xE2\x84\x9B" }, // U+211B
    { "Rsh;", "\xE2\x86\xB1" }, // U+21B1
    { "RuleDelayed;", "\xE2\xA7\xB4" }, // U+29F4
};

static constexpr UCSTBL const ucstbl_S[] = {
    { "SHCHcy;", "\xD0\xA9" }, // U+0429
    { "SHcy;", "\xD0\xA8" }, // U+0428
    { "SOFTcy;", "\xD0\xAC" }, // U+042C
    { "Sacute;", "\xC5\x9A" }, // U+015A
    { "Sc;", "\xE2\xAA\xBC" }, // U+2ABC
    { "Scaron;", "\xC5\xA0" }, // U+0160
    { "Scedil;", "\xC5\x9E" }, // U+015E
    { "Scirc;", "\xC5\x9C" }, // U+015C
    { "Scy;", "\xD0\xA1" }, // U+0421
    { "Sfr;", "\xF0\x9D\x94\x96" }, // U+1D516
    { "ShortDownArrow;", "\xE2\x86\x93" }, // U+2193
    { "ShortLeftArrow;", "\xE2\x86\x90" }, // U+2190
    { "ShortRightArrow;", "\xE2\x86\x92" }, // U+2192
    { "ShortUpArrow;", "\xE2\x86\x91" }, // U+2191
    { "Sigma;", "\xCE\xA3" }, // U+03A3
    { "SmallCircle;", "\xE2\x88\x98" }, // U+2218
    { "Sopf;", "\xF0\x9D\x95\x8A" }, // U+1D54A
    { "Sqrt;", "\xE2\x88\x9A" }, // U+221A
    { "Square;", "\xE2\x96\xA1" }, // U+25A1
    { "SquareIntersection;", "\xE2\x8A\x93" }, // U+2293
    { "SquareSubset;", "\xE2\x8A\x8F" }, // U+228F
    { "SquareSubsetEqual;", "\xE2\x8A\x91" }, // U+2291
    { "SquareSuperset;", "\xE2\x8A\x90" }, // U+2290
    { "SquareSupersetEqual;", "\xE2\x8A\x92" }, // U+2292
    { "SquareUnion;", "\xE2\x8A\x94" }, // U+2294
    { "Sscr;", "\xF0\x9D\x92\xAE" }, // U+1D4AE
    { "Star;", "\xE2\x8B\x86" }, // U+22C6
    { "Sub;", "\xE2\x8B\x90" }, // U+22D0
    { "Subset;", "\xE2\x8B\x90" }, // U+22D0
    { "SubsetEqual;", "\xE2\x8A\x86" }, // U+2286
    { "Succeeds;", "\xE2\x89\xBB" }, // U+227B
    { "SucceedsEqual;", "\xE2\xAA\xB0" }, // U+2AB0
    { "SucceedsSlantEqual;", "\xE2\x89\xBD" }, // U+227D
    { "SucceedsTilde;", "\xE2\x89\xBF" }, // U+227F
    { "SuchThat;", "\xE2\x88\x8B" }, // U+220B
    { "Sum;", "\xE2\x88\x91" }, // U+2211
    { "Sup;", "\xE2\x8B\x91" }, // U+22D1
    { "Superset;", "\xE2\x8A\x83" }, // U+2283
    { "SupersetEqual;", "\xE2\x8A\x87" }, // U+2287
    { "Supset;", "\xE2\x8B\x91" }, // U+22D1
};

static constexpr UCSTBL const ucstbl_T[] = {
    { "THORN;", "\xC3\x9E" }, // U+00DE
    { "TRADE;", "\xE2\x84\xA2" }, // U+2122
    { "TSHcy;", "\xD0\x8B" }, // U+040B
    { "TScy;", "\xD0\xA6" }, // U+0426
    { "Tab;", "\x09" }, // U+0009
    { "Tau;", "\xCE\xA4" }, // U+03A4
    { "Tcaron;", "\xC5\xA4" }, // U+0164
    { "Tcedil;", "\xC5\xA2" }, // U+0162
    { "Tcy;", "\xD0\xA2" }, // U+0422
    { "Tfr;", "\xF0\x9D\x94\x97" }, // U+1D517
    { "Therefore;", "\xE2\x88\xB4" }, // U+2234
    { "Theta;", "\xCE\x98" }, // U+0398
    { "ThickSpace;", "\xE2\x81\x9F\xE2\x80\x8A" }, // U+205F U+200A
    { "ThinSpace;", "\xE2\x80\x89" }, // U+2009
    { "Tilde;", "\xE2\x88\xBC" }, // U+223C
    { "TildeEqual;", "\xE2\x89\x83" }, // U+2243
    { "TildeFullEqual;", "\xE2\x89\x85" }, // U+2245
    { "TildeTilde;", "\xE2\x89\x88" }, // U+2248
    { "Topf;", "\xF0\x9D\x95\x8B" }, // U+1D54B
    { "TripleDot;", "\xE2\x83\x9B" }, // U+20DB
    { "Tscr;", "\xF0\x9D\x92\xAF" }, // U+1D4AF
    { "Tstrok;", "\xC5\xA6" }, // U+0166
};

static constexpr UCSTBL const ucstbl_U[] = {
    { "Uacute;", "\xC3\x9A" }, // U+00DA
    { "Uarr;", "\xE2\x86\x9F" }, // U+219F
    { "Uarrocir;", "\xE2\xA5\x89" }, // U+2949
    { "Ubrcy;", "\xD0\x8E" }, // U+040E
    { "Ubreve;", "\xC5\xAC" }, // U+016C
    { "Ucirc;", "\xC3\x9B" }, // U+00DB
    { "Ucy;", "\xD0\xA3" }, // U+0423
    { "Udblac;", "\xC5\xB0" }, // U+0170
    { "Ufr;", "\xF0\x9D\x94\x98" }, // U+1D518
    { "Ugrave;", "\xC3\x99" }, // U+00D9
    { "Umacr;", "\xC5\xAA" }, // U+016A
    { "UnderBar;", "\x5F" }, // U+005F
    { "UnderBrace;", "\xE2\x8F\x9F" }, // U+23DF
    { "UnderBracket;", "\xE2\x8E\xB5" }, // U+23B5
    { "UnderParenthesis;", "\xE2\x8F\x9D" }, // U+23DD
    { "Union;", "\xE2\x8B\x83" }, // U+22C3
    { "UnionPlus;", "\xE2\x8A\x8E" }, // U+228E
    { "Uogon;", "\xC5\xB2" }, // U+0172
    { "Uopf;", "\xF0\x9D\x95\x8C" }, // U+1D54C
    { "UpArrow;", "\xE2\x86\x91" }, // U+2191
    { "UpArrowBar;", "\xE2\xA4\x92" }, // U+2912
    { "UpArrowDownArrow;", "\xE2\x87\x85" }, // U+21C5
    { "UpDownArrow;", "\xE2\x86\x95" }, // U+2195
    { "UpEquilibrium;", "\xE2\xA5\xAE" }, // U+296E
    { "UpTee;", "\xE2\x8A\xA5" }, // U+22A5
    { "UpTeeArrow;", "\xE2\x86\xA5" }, // U+21A5
    { "Uparrow;", "\xE2\x87\x91" }, // U+21D1
    { "Updownarrow;", "\xE2\x87\x95" }, // U+21D5
    { "UpperLeftArrow;", "\xE2\x86\x96" }, // U+2196
    { "UpperRightArrow;", "\xE2\x86\x97" }, // U+2197
    { "Upsi;", "\xCF\x92" }, // U+03D2
    { "Upsilon;", "\xCE\xA5" }, // U+03A5
    { "Uring;", "\xC5\xAE" }, // U+016E
    { "Uscr;", "\xF0\x9D\x92\xB0" }, // U+1D4B0
    { "Utilde;", "\xC5\xA8" }, // U+0168
    { "Uuml;", "\xC3\x9C" }, // U+00DC
};

static constexpr UCSTBL const ucstbl_V[] = {
    { "VDash;", "\xE2\x8A\xAB" }, // U+22AB
    { "Vbar;", "\xE2\xAB\xAB" }, // U+2AEB
    { "Vcy;", "\xD0\x92" }, // U+0412
    { "Vdash;", "\xE2\x8A\xA9" }, // U+22A9
    { "Vdashl;", "\xE2\xAB\xA6" }, // U+2AE6
    { "Vee;", "\xE2\x8B\x81" }, // U+22C1
    { "Verbar;", "\xE2\x80\x96" }, // U+2016
    { "Vert;", "\xE2\x80\x96" }, // U+2016
    { "VerticalBar;", "\xE2\x88\xA3" }, // U+2223
    { "VerticalLine;", "\x7C" }, // U+007C
    { "VerticalSeparator;", "\xE2\x9D\x98" }, // U+2758
    { "VerticalTilde;", "\xE2\x89\x80" }, // U+2240
    { "VeryThinSpace;", "\xE2\x80\x8A" }, // U+200A
    { "Vfr;", "\xF0\x9D\x94\x99" }, // U+1D519
    { "Vopf;", "\xF0\x9D\x95\x8D" }, // U+1D54D
    { "Vscr;", "\xF0\x9D\x92\xB1" }, // U+1D4B1
    { "Vvdash;", "\xE2\x8A\xAA" }, // U+22AA
};

static constexpr UCSTBL const ucstbl_W[] = {
    { "Wcirc;", "\xC5\xB4" }, // U+0174
    { "Wedge;", "\xE2\x8B\x80" }, // U+22C0
    { "Wfr;", "\xF0\x9D\x94\x9A" }, // U+1D51A
    { "Wopf;", "\xF0\x9D\x95\x8E" }, // U+1D54E
    { "Wscr;", "\xF0\x9D\x92\xB2" }, // U+1D4B2
};

static constexpr UCSTBL const ucstbl_X[] = {
    { "Xfr;", "\xF0\x9D\x94\x9B" }, // U+1D51B
    { "Xi;", "\xCE\x9E" }, // U+039E
    { "Xopf;", "\xF0\x9D\x95\x8F" }, // U+1D54F
    { "Xscr;", "\xF0\x9D\x92\xB3" }, // U+1D4B3
};

static constexpr UCSTBL const ucstbl_Y[] = {
    { "YAcy;", "\xD0\xAF" }, // U+042F
    { "YIcy;", "\xD0\x87" }, // U+0407
    { "YUcy;", "\xD0\xAE" }, // U+042E
    { "Yacute;", "\xC3\x9D" }, // U+00DD
    { "Ycirc;", "\xC5\xB6" }, // U+0176
    { "Ycy;", "\xD0\xAB" }, // U+042B
    { "Yfr;", "\xF0\x9D\x94\x9C" }, // U+1D51C
    { "Yopf;", "\xF0\x9D\x95\x90" }, // U+1D550
    { "Yscr;", "\xF0\x9D\x92\xB4" }, // U+1D4B4
    { "Yuml;", "\xC5\xB8" }, // U+0178
};

static constexpr UCSTBL const ucstbl_Z[] = {
    { "ZHcy;", "\xD0\x96" }, // U+0416
    { "Zacute;", "\xC5\xB9" }, // U+0179
    { "Zcaron;", "\xC5\xBD" }, // U+017D
    { "Zcy;", "\xD0\x97" }, // U+0417
    { "Zdot;", "\xC5\xBB" }, // U+017B
    { "ZeroWidthSpace;", "\xE2\x80\x8B" }, // U+200B ZERO WIDTH SPACE
    { "Zeta;", "\xCE\x96" }, // U+0396
    { "Zfr;", "\xE2\x84\xA8" }, // U+2128
    { "Zopf;", "\xE2\x84\xA4" }, // U+2124
    { "Zscr;", "\xF0\x9D\x92\xB5" }, // U+1D4B5
};

static constexpr UCSTBL const ucstbl_a[] = {
    { "aacute;", "\xC3\xA1" }, // U+00E1
    { "abreve;", "\xC4\x83" }, // U+0103
    { "ac;", "\xE2\x88\xBE" }, // U+223E
    { "acE;", "\xE2\x88\xBE\xCC\xB3" }, // U+223E U+0333
    { "acd;", "\xE2\x88\xBF" }, // U+223F
    { "acirc;", "\xC3\xA2" }, // U+00E2
    { "acute;", "\xC2\xB4" }, // U+00B4
    { "acy;", "\xD0\xB0" }, // U+0430
    { "aelig;", "\xC3\xA6" }, // U+00E6
    { "af;", "\xE2\x81\xA1" }, // U+2061
    { "afr;", "\xF0\x9D\x94\x9E" }, // U+1D51E
    { "agrave;", "\xC3\xA0" }, // U+00E0
    { "alefsym;", "\xE2\x84\xB5" }, // U+2135
    { "aleph;", "\xE2\x84\xB5" }, // U+2135
    { "alpha;", "\xCE\xB1" }, // U+03B1
    { "amacr;", "\xC4\x81" }, // U+0101
    { "amalg;", "\xE2\xA8\xBF" }, // U+2A3F
    { "amp;", "\x26" }, // U+0026
    { "and;", "\xE2\x88\xA7" }, // U+2227
    { "andand;", "\xE2\xA9\x95" }, // U+2A55
    { "andd;", "\xE2\xA9\x9C" }, // U+2A5C
    { "andslope;", "\xE2\xA9\x98" }, // U+2A58
    { "andv;", "\xE2\xA9\x9A" }, // U+2A5A
    { "ang;", "\xE2\x88\xA0" }, // U+2220
    { "ange;", "\xE2\xA6\xA4" }, // U+29A4
    { "angle;", "\xE2\x88\xA0" }, // U+2220
    { "angmsd;", "\xE2\x88\xA1" }, // U+2221
    { "angmsdaa;", "\xE2\xA6\xA8" }, // U+29A8
    { "angmsdab;", "\xE2\xA6\xA9" }, // U+29A9
    { "angmsdac;", "\xE2\xA6\xAA" }, // U+29AA
    { "angmsdad;", "\xE2\xA6\xAB" }, // U+29AB
    { "angmsdae;", "\xE2\xA6\xAC" }, // U+29AC
    { "angmsdaf;", "\xE2\xA6\xAD" }, // U+29AD
    { "angmsdag;", "\xE2\xA6\xAE" }, // U+29AE
    { "angmsdah;", "\xE2\xA6\xAF" }, // U+29AF
    { "angrt;", "\xE2\x88\x9F" }, // U+221F
    { "angrtvb;", "\xE2\x8A\xBE" }, // U+22BE
    { "angrtvbd;", "\xE2\xA6\x9D" }, // U+299D
    { "angsph;", "\xE2\x88\xA2" }, // U+2222
    { "angst;", "\xC3\x85" }, // U+00C5
    { "angzarr;", "\xE2\x8D\xBC" }, // U+237C
    { "aogon;", "\xC4\x85" }, // U+0105
    { "aopf;", "\xF0\x9D\x95\x92" }, // U+1D552
    { "ap;", "\xE2\x89\x88" }, // U+2248
    { "apE;", "\xE2\xA9\xB0" }, // U+2A70
    { "apacir;", "\xE2\xA9\xAF" }, // U+2A6F
    { "ape;", "\xE2\x89\x8A" }, // U+224A
    { "apid;", "\xE2\x89\x8B" }, // U+224B
    { "apos;", "\x27" }, // U+0027
    { "approx;", "\xE2\x89\x88" }, // U+2248
    { "approxeq;", "\xE2\x89\x8A" }, // U+224A
    { "aring;", "\xC3\xA5" }, // U+00E5
    { "ascr;", "\xF0\x9D\x92\xB6" }, // U+1D4B6
    { "ast;", "\x2A" }, // U+002A
    { "asymp;", "\xE2\x89\x88" }, // U+2248
    { "asympeq;", "\xE2\x89\x8D" }, // U+224D
    { "atilde;", "\xC3\xA3" }, // U+00E3
    { "auml;", "\xC3\xA4" }, // U+00E4
    { "awconint;", "\xE2\x88\xB3" }, // U+2233
    { "awint;", "\xE2\xA8\x91" }, // U+2A11
};

static constexpr UCSTBL const ucstbl_b[] = {
    { "bNot;", "\xE2\xAB\xAD" }, // U+2AED
    { "backcong;", "\xE2\x89\x8C" }, // U+224C
    { "backepsilon;", "\xCF\xB6" }, // U+03F6
    { "backprime;", "\xE2\x80\xB5" }, // U+2035
    { "backsim;", "\xE2\x88\xBD" }, // U+223D
    { "backsimeq;", "\xE2\x8B\x8D" }, // U+22CD
    { "barvee;", "\xE2\x8A\xBD" }, // U+22BD
    { "barwed;", "\xE2\x8C\x85" }, // U+2305
    { "barwedge;", "\xE2\x8C\x85" }, // U+2305
    { "bbrk;", "\xE2\x8E\xB5" }, // U+23B5
    { "bbrktbrk;", "\xE2\x8E\xB6" }, // U+23B6
    { "bcong;", "\xE2\x89\x8C" }, // U+224C
    { "bcy;", "\xD0\xB1" }, // U+0431
    { "bdquo;", "\xE2\x80\x9E" }, // U+201E
    { "becaus;", "\xE2\x88\xB5" }, // U+2235
    { "because;", "\xE2\x88\xB5" }, // U+2235
    { "bemptyv;", "\xE2\xA6\xB0" }, // U+29B0
    { "bepsi;", "\xCF\xB6" }, // U+03F6
    { "bernou;", "\xE2\x84\xAC" }, // U+212C
    { "beta;", "\xCE\xB2" }, // U+03B2
    { "beth;", "\xE2\x84\xB6" }, // U+2136
    { "between;", "\xE2\x89\xAC" }, // U+226C
    { "bfr;", "\xF0\x9D\x94\x9F" }, // U+1D51F
    { "bigcap;", "\xE2\x8B\x82" }, // U+22C2
    { "bigcirc;", "\xE2\x97\xAF" }, // U+25EF
    { "bigcup;", "\xE2\x8B\x83" }, // U+22C3
    { "bigodot;", "\xE2\xA8\x80" }, // U+2A00
    { "bigoplus;", "\xE2\xA8\x81" }, // U+2A01
    { "bigotimes;", "\xE2\xA8\x82" }, // U+2A02
    { "bigsqcup;", "\xE2\xA8\x86" }, // U+2A06
    { "bigstar;", "\xE2\x98\x85" }, // U+2605
    { "bigtriangledown;", "\xE2\x96\xBD" }, // U+25BD
    { "bigtriangleup;", "\xE2\x96\xB3" }, // U+25B3
    { "biguplus;", "\xE2\xA8\x84" }, // U+2A04
    { "bigvee;", "\xE2\x8B\x81" }, // U+22C1
    { "bigwedge;", "\xE2\x8B\x80" }, // U+22C0
    { "bkarow;", "\xE2\xA4\x8D" }, // U+290D
    { "blacklozenge;", "\xE2\xA7\xAB" }, // U+29EB
    { "blacksquare;", "\xE2\x96\xAA" }, // U+25AA
    { "blacktriangle;", "\xE2\x96\xB4" }, // U+25B4
    { "blacktriangledown;", "\xE2\x96\xBE" }, // U+25BE
    { "blacktriangleleft;", "\xE2\x97\x82" }, // U+25C2
    { "blacktriangleright;", "\xE2\x96\xB8" }, // U+25B8
    { "blank;", "\xE2\x90\xA3" }, // U+2423
    { "blk12;", "\xE2\x96\x92" }, // U+2592
    { "blk14;", "\xE2\x96\x91" }, // U+2591
    { "blk34;", "\xE2\x96\x93" }, // U+2593
    { "block;", "\xE2\x96\x88" }, // U+2588
    { "bne;", "\x3D\xE2\x83\xA5" }, // U+003D U+20E5
    { "bnequiv;", "\xE2\x89\xA1\xE2\x83\xA5" }, // U+2261 U+20E5
    { "bnot;", "\xE2\x8C\x90" }, // U+2310
    { "bopf;", "\xF0\x9D\x95\x93" }, // U+1D553
    { "bot;", "\xE2\x8A\xA5" }, // U+22A5
    { "bottom;", "\xE2\x8A\xA5" }, // U+22A5
    { "bowtie;", "\xE2\x8B\x88" }, // U+22C8
    { "boxDL;", "\xE2\x95\x97" }, // U+2557
    { "boxDR;", "\xE2\x95\x94" }, // U+2554
    { "boxDl;", "\xE2\x95\x96" }, // U+2556
    { "boxDr;", "\xE2\x95\x93" }, // U+2553
    { "boxH;", "\xE2\x95\x90" }, // U+2550
    { "boxHD;", "\xE2\x95\xA6" }, // U+2566
    { "boxHU;", "\xE2\x95\xA9" }, // U+2569
    { "boxHd;", "\xE2\x95\xA4" }, // U+2564
    { "boxHu;", "\xE2\x95\xA7" }, // U+2567
    { "boxUL;", "\xE2\x95\x9D" }, // U+255D
    { "boxUR;", "\xE2\x95\x9A" }, // U+255A
    { "boxUl;", "\xE2\x95\x9C" }, // U+255C
    { "boxUr;", "\xE2\x95\x99" }, // U+2559
    { "boxV;", "\xE2\x95\x91" }, // U+2551
    { "boxVH;", "\xE2\x95\xAC" }, // U+256C
    { "boxVL;", "\xE2\x95\xA3" }, // U+2563
    { "boxVR;", "\xE2\x95\xA0" }, // U+2560
    { "boxVh;", "\xE2\x95\xAB" }, // U+256B
    { "boxVl;", "\xE2\x95\xA2" }, // U+2562
    { "boxVr;", "\xE2\x95\x9F" }, // U+255F
    { "boxbox;", "\xE2\xA7\x89" }, // U+29C9
    { "boxdL;", "\xE2\x95\x95" }, // U+2555
    { "boxdR;", "\xE2\x95\x92" }, // U+2552
    { "boxdl;", "\xE2\x94\x90" }, // U+2510
    { "boxdr;", "\xE2\x94\x8C" }, // U+250C
    { "boxh;", "\xE2\x94\x80" }, // U+2500
    { "boxhD;", "\xE2\x95\xA5" }, // U+2565
    { "boxhU;", "\xE2\x95\xA8" }, // U+2568
    { "boxhd;", "\xE2\x94\xAC" }, // U+252C
    { "boxhu;", "\xE2\x94\xB4" }, // U+2534
    { "boxminus;", "\xE2\x8A\x9F" }, // U+229F
    { "boxplus;", "\xE2\x8A\x9E" }, // U+229E
    { "boxtimes;", "\xE2\x8A\xA0" }, // U+22A0
    { "boxuL;", "\xE2\x95\x9B" }, // U+255B
    { "boxuR;", "\xE2\x95\x98" }, // U+2558
    { "boxul;", "\xE2\x94\x98" }, // U+2518
    { "boxur;", "\xE2\x94\x94" }, // U+2514
    { "boxv;", "\xE2\x94\x82" }, // U+2502
    { "boxvH;", "\xE2\x95\xAA" }, // U+256A
    { "boxvL;", "\xE2\x95\xA1" }, // U+2561
    { "boxvR;", "\xE2\x95\x9E" }, // U+255E
    { "boxvh;", "\xE2\x94\xBC" }, // U+253C
    { "boxvl;", "\xE2\x94\xA4" }, // U+2524
    { "boxvr;", "\xE2\x94\x9C" }, // U+251C
    { "bprime;", "\xE2\x80\xB5" }, // U+2035
    { "breve;", "\xCB\x98" }, // U+02D8
    { "brvbar;", "\xC2\xA6" }, // U+00A6
    { "bscr;", "\xF0\x9D\x92\xB7" }, // U+1D4B7
    { "bsemi;", "\xE2\x81\x8F" }, // U+204F
    { "bsim;", "\xE2\x88\xBD" }, // U+223D
    { "bsime;", "\xE2\x8B\x8D" }, // U+22CD
    { "bsol;", "\x5C" }, // U+005C
    { "bsolb;", "\xE2\xA7\x85" }, // U+29C5
    { "bsolhsub;", "\xE2\x9F\x88" }, // U+27C8
    { "bull;", "\xE2\x80\xA2" }, // U+2022
    { "bullet;", "\xE2\x80\xA2" }, // U+2022
    { "bump;", "\xE2\x89\x8E" }, // U+224E
    { "bumpE;", "\xE2\xAA\xAE" }, // U+2AAE
    { "bumpe;", "\xE2\x89\x8F" }, // U+224F
    { "bumpeq;", "\xE2\x89\x8F" }, // U+224F
};

static constexpr UCSTBL const ucstbl_c[] = {
    { "cacute;", "\xC4\x87" }, // U+0107
    { "cap;", "\xE2\x88\xA9" }, // U+2229
    { "capand;", "\xE2\xA9\x84" }, // U+2A44
    { "capbrcup;", "\xE2\xA9\x89" }, // U+2A49
    { "capcap;", "\xE2\xA9\x8B" }, // U+2A4B
    { "capcup;", "\xE2\xA9\x87" }, // U+2A47
    { "capdot;", "\xE2\xA9\x80" }, // U+2A40
    { "caps;", "\xE2\x88\xA9\xEF\xB8\x80" }, // U+2229 U+FE00
    { "caret;", "\xE2\x81\x81" }, // U+2041
    { "caron;", "\xCB\x87" }, // U+02C7
    { "ccaps;", "\xE2\xA9\x8D" }, // U+2A4D
    { "ccaron;", "\xC4\x8D" }, // U+010D
    { "ccedil;", "\xC3\xA7" }, // U+00E7
    { "ccirc;", "\xC4\x89" }, // U+0109
    { "ccups;", "\xE2\xA9\x8C" }, // U+2A4C
    { "ccupssm;", "\xE2\xA9\x90" }, // U+2A50
    { "cdot;", "\xC4\x8B" }, // U+010B
    { "cedil;", "\xC2\xB8" }, // U+00B8
    { "cemptyv;", "\xE2\xA6\xB2" }, // U+29B2
    { "cent;", "\xC2\xA2" }, // U+00A2
    { "centerdot;", "\xC2\xB7" }, // U+00B7
    { "cfr;", "\xF0\x9D\x94\xA0" }, // U+1D520
    { "chcy;", "\xD1\x87" }, // U+0447
    { "check;", "\xE2\x9C\x93" }, // U+2713
    { "checkmark;", "\xE2\x9C\x93" }, // U+2713
    { "chi;", "\xCF\x87" }, // U+03C7
    { "cir;", "\xE2\x97\x8B" }, // U+25CB
    { "cirE;", "\xE2\xA7\x83" }, // U+29C3
    { "circ;", "\xCB\x86" }, // U+02C6
    { "circeq;", "\xE2\x89\x97" }, // U+2257
    { "circlearrowleft;", "\xE2\x86\xBA" }, // U+21BA
    { "circlearrowright;", "\xE2\x86\xBB" }, // U+21BB
    { "circledR;", "\xC2\xAE" }, // U+00AE
    { "circledS;", "\xE2\x93\x88" }, // U+24C8
    { "circledast;", "\xE2\x8A\x9B" }, // U+229B
    { "circledcirc;", "\xE2\x8A\x9A" }, // U+229A
    { "circleddash;", "\xE2\x8A\x9D" }, // U+229D
    { "cire;", "\xE2\x89\x97" }, // U+2257
    { "cirfnint;", "\xE2\xA8\x90" }, // U+2A10
    { "cirmid;", "\xE2\xAB\xAF" }, // U+2AEF
    { "cirscir;", "\xE2\xA7\x82" }, // U+29C2
    { "clubs;", "\xE2\x99\xA3" }, // U+2663
    { "clubsuit;", "\xE2\x99\xA3" }, // U+2663
    { "colon;", "\x3A" }, // U+003A
    { "colone;", "\xE2\x89\x94" }, // U+2254
    { "coloneq;", "\xE2\x89\x94" }, // U+2254
    { "comma;", "\x2C" }, // U+002C
    { "commat;", "\x40" }, // U+0040
    { "comp;", "\xE2\x88\x81" }, // U+2201
    { "compfn;", "\xE2\x88\x98" }, // U+2218
    { "complement;", "\xE2\x88\x81" }, // U+2201
    { "complexes;", "\xE2\x84\x82" }, // U+2102
    { "cong;", "\xE2\x89\x85" }, // U+2245
    { "congdot;", "\xE2\xA9\xAD" }, // U+2A6D
    { "conint;", "\xE2\x88\xAE" }, // U+222E
    { "copf;", "\xF0\x9D\x95\x94" }, // U+1D554
    { "coprod;", "\xE2\x88\x90" }, // U+2210
    { "copy;", "\xC2\xA9" }, // U+00A9
    { "copysr;", "\xE2\x84\x97" }, // U+2117
    { "crarr;", "\xE2\x86\xB5" }, // U+21B5
    { "cross;", "\xE2\x9C\x97" }, // U+2717
    { "cscr;", "\xF0\x9D\x92\xB8" }, // U+1D4B8
    { "csub;", "\xE2\xAB\x8F" }, // U+2ACF
    { "csube;", "\xE2\xAB\x91" }, // U+2AD1
    { "csup;", "\xE2\xAB\x90" }, // U+2AD0
    { "csupe;", "\xE2\xAB\x92" }, // U+2AD2
    { "ctdot;", "\xE2\x8B\xAF" }, // U+22EF
    { "cudarrl;", "\xE2\xA4\xB8" }, // U+2938
    { "cudarrr;", "\xE2\xA4\xB5" }, // U+2935
    { "cuepr;", "\xE2\x8B\x9E" }, // U+22DE
    { "cuesc;", "\xE2\x8B\x9F" }, // U+22DF
    { "cularr;", "\xE2\x86\xB6" }, // U+21B6
    { "cularrp;", "\xE2\xA4\xBD" }, // U+293D
    { "cup;", "\xE2\x88\xAA" }, // U+222A
    { "cupbrcap;", "\xE2\xA9\x88" }, // U+2A48
    { "cupcap;", "\xE2\xA9\x86" }, // U+2A46
    { "cupcup;", "\xE2\xA9\x8A" }, // U+2A4A
    { "cupdot;", "\xE2\x8A\x8D" }, // U+228D
    { "cupor;", "\xE2\xA9\x85" }, // U+2A45
    { "cups;", "\xE2\x88\xAA\xEF\xB8\x80" }, // U+222A U+FE00
    { "curarr;", "\xE2\x86\xB7" }, // U+21B7
    { "curarrm;", "\xE2\xA4\xBC" }, // U+293C
    { "curlyeqprec;", "\xE2\x8B\x9E" }, // U+22DE
    { "curlyeqsucc;", "\xE2\x8B\x9F" }, // U+22DF
    { "curlyvee;", "\xE2\x8B\x8E" }, // U+22CE
    { "curlywedge;", "\xE2\x8B\x8F" }, // U+22CF
    { "curren;", "\xC2\xA4" }, // U+00A4
    { "curvearrowleft;", "\xE2\x86\xB6" }, // U+21B6
    { "curvearrowright;", "\xE2\x86\xB7" }, // U+21B7
    { "cuvee;", "\xE2\x8B\x8E" }, // U+22CE
    { "cuwed;", "\xE2\x8B\x8F" }, // U+22CF
    { "cwconint;", "\xE2\x88\xB2" }, // U+2232
    { "cwint;", "\xE2\x88\xB1" }, // U+2231
    { "cylcty;", "\xE2\x8C\xAD" }, // U+232D
};

static constexpr UCSTBL const ucstbl_d[] = {
    { "dArr;", "\xE2\x87\x93" }, // U+21D3
    { "dHar;", "\xE2\xA5\xA5" }, // U+2965
    { "dagger;", "\xE2\x80\xA0" }, // U+2020
    { "daleth;", "\xE2\x84\xB8" }, // U+2138
    { "darr;", "\xE2\x86\x93" }, // U+2193
    { "dash;", "\xE2\x80\x90" }, // U+2010
    { "dashv;", "\xE2\x8A\xA3" }, // U+22A3
    { "dbkarow;", "\xE2\xA4\x8F" }, // U+290F
    { "dblac;", "\xCB\x9D" }, // U+02DD
    { "dcaron;", "\xC4\x8F" }, // U+010F
    { "dcy;", "\xD0\xB4" }, // U+0434
    { "dd;", "\xE2\x85\x86" }, // U+2146
    { "ddagger;", "\xE2\x80\xA1" }, // U+2021
    { "ddarr;", "\xE2\x87\x8A" }, // U+21CA
    { "ddotseq;", "\xE2\xA9\xB7" }, // U+2A77
    { "deg;", "\xC2\xB0" }, // U+00B0
    { "delta;", "\xCE\xB4" }, // U+03B4
    { "demptyv;", "\xE2\xA6\xB1" }, // U+29B1
    { "dfisht;", "\xE2\xA5\xBF" }, // U+297F
    { "dfr;", "\xF0\x9D\x94\xA1" }, // U+1D521
    { "dharl;", "\xE2\x87\x83" }, // U+21C3
    { "dharr;", "\xE2\x87\x82" }, // U+21C2
    { "diam;", "\xE2\x8B\x84" }, // U+22C4
    { "diamond;", "\xE2\x8B\x84" }, // U+22C4
    { "diamondsuit;", "\xE2\x99\xA6" }, // U+2666
    { "diams;", "\xE2\x99\xA6" }, // U+2666
    { "die;", "\xC2\xA8" }, // U+00A8
    { "digamma;", "\xCF\x9D" }, // U+03DD
    { "disin;", "\xE2\x8B\xB2" }, // U+22F2
    { "div;", "\xC3\xB7" }, // U+00F7
    { "divide;", "\xC3\xB7" }, // U+00F7
    { "divideontimes;", "\xE2\x8B\x87" }, // U+22C7
    { "divonx;", "\xE2\x8B\x87" }, // U+22C7
    { "djcy;", "\xD1\x92" }, // U+0452
    { "dlcorn;", "\xE2\x8C\x9E" }, // U+231E
    { "dlcrop;", "\xE2\x8C\x8D" }, // U+230D
    { "dollar;", "\x24" }, // U+0024
    { "dopf;", "\xF0\x9D\x95\x95" }, // U+1D555
    { "dot;", "\xCB\x99" }, // U+02D9
    { "doteq;", "\xE2\x89\x90" }, // U+2250
    { "doteqdot;", "\xE2\x89\x91" }, // U+2251
    { "dotminus;", "\xE2\x88\xB8" }, // U+2238
    { "dotplus;", "\xE2\x88\x94" }, // U+2214
    { "dotsquare;", "\xE2\x8A\xA1" }, // U+22A1
    { "doublebarwedge;", "\xE2\x8C\x86" }, // U+2306
    { "downarrow;", "\xE2\x86\x93" }, // U+2193
    { "downdownarrows;", "\xE2\x87\x8A" }, // U+21CA
    { "downharpoonleft;", "\xE2\x87\x83" }, // U+21C3
    { "downharpoonright;", "\xE2\x87\x82" }, // U+21C2
    { "drbkarow;", "\xE2\xA4\x90" }, // U+2910
    { "drcorn;", "\xE2\x8C\x9F" }, // U+231F
    { "drcrop;", "\xE2\x8C\x8C" }, // U+230C
    { "dscr;", "\xF0\x9D\x92\xB9" }, // U+1D4B9
    { "dscy;", "\xD1\x95" }, // U+0455
    { "dsol;", "\xE2\xA7\xB6" }, // U+29F6
    { "dstrok;", "\xC4\x91" }, // U+0111
    { "dtdot;", "\xE2\x8B\xB1" }, // U+22F1
    { "dtri;", "\xE2\x96\xBF" }, // U+25BF
    { "dtrif;", "\xE2\x96\xBE" }, // U+25BE
    { "duarr;", "\xE2\x87\xB5" }, // U+21F5
    { "duhar;", "\xE2\xA5\xAF" }, // U+296F
    { "dwangle;", "\xE2\xA6\xA6" }, // U+29A6
    { "dzcy;", "\xD1\x9F" }, // U+045F
    { "dzigrarr;", "\xE2\x9F\xBF" }, // U+27FF
};

static constexpr UCSTBL const ucstbl_e[] = {
    { "eDDot;", "\xE2\xA9\xB7" }, // U+2A77
    { "eDot;", "\xE2\x89\x91" }, // U+2251
    { "eacute;", "\xC3\xA9" }, // U+00E9
    { "easter;", "\xE2\xA9\xAE" }, // U+2A6E
    { "ecaron;", "\xC4\x9B" }, // U+011B
    { "ecir;", "\xE2\x89\x96" }, // U+2256
    { "ecirc;", "\xC3\xAA" }, // U+00EA
    { "ecolon;", "\xE2\x89\x95" }, // U+2255
    { "ecy;", "\xD1\x8D" }, // U+044D
    { "edot;", "\xC4\x97" }, // U+0117
    { "ee;", "\xE2\x85\x87" }, // U+2147
    { "efDot;", "\xE2\x89\x92" }, // U+2252
    { "efr;", "\xF0\x9D\x94\xA2" }, // U+1D522
    { "eg;", "\xE2\xAA\x9A" }, // U+2A9A
    { "egrave;", "\xC3\xA8" }, // U+00E8
    { "egs;", "\xE2\xAA\x96" }, // U+2A96
    { "egsdot;", "\xE2\xAA\x98" }, // U+2A98
    { "el;", "\xE2\xAA\x99" }, // U+2A99
    { "elinters;", "\xE2\x8F\xA7" }, // U+23E7
    { "ell;", "\xE2\x84\x93" }, // U+2113
    { "els;", "\xE2\xAA\x95" }, // U+2A95
    { "elsdot;", "\xE2\xAA\x97" }, // U+2A97
    { "emacr;", "\xC4\x93" }, // U+0113
    { "empty;", "\xE2\x88\x85" }, // U+2205
    { "emptyset;", "\xE2\x88\x85" }, // U+2205
    { "emptyv;", "\xE2\x88\x85" }, // U+2205
    { "emsp13;", "\xE2\x80\x84" }, // U+2004
    { "emsp14;", "\xE2\x80\x85" }, // U+2005
    { "emsp;", "\xE2\x80\x83" }, // U+2003
    { "eng;", "\xC5\x8B" }, // U+014B
    { "ensp;", "\xE2\x80\x82" }, // U+2002
    { "eogon;", "\xC4\x99" }, // U+0119
    { "eopf;", "\xF0\x9D\x95\x96" }, // U+1D556
    { "epar;", "\xE2\x8B\x95" }, // U+22D5
    { "eparsl;", "\xE2\xA7\xA3" }, // U+29E3
    { "eplus;", "\xE2\xA9\xB1" }, // U+2A71
    { "epsi;", "\xCE\xB5" }, // U+03B5
    { "epsilon;", "\xCE\xB5" }, // U+03B5
    { "epsiv;", "\xCF\xB5" }, // U+03F5
    { "eqcirc;", "\xE2\x89\x96" }, // U+2256
    { "eqcolon;", "\xE2\x89\x95" }, // U+2255
    { "eqsim;", "\xE2\x89\x82" }, // U+2242
    { "eqslantgtr;", "\xE2\xAA\x96" }, // U+2A96
    { "eqslantless;", "\xE2\xAA\x95" }, // U+2A95
    { "equals;", "\x3D" }, // U+003D
    { "equest;", "\xE2\x89\x9F" }, // U+225F
    { "equiv;", "\xE2\x89\xA1" }, // U+2261
    { "equivDD;", "\xE2\xA9\xB8" }, // U+2A78
    { "eqvparsl;", "\xE2\xA7\xA5" }, // U+29E5
    { "erDot;", "\xE2\x89\x93" }, // U+2253
    { "erarr;", "\xE2\xA5\xB1" }, // U+2971
    { "escr;", "\xE2\x84\xAF" }, // U+212F
    { "esdot;", "\xE2\x89\x90" }, // U+2250
    { "esim;", "\xE2\x89\x82" }, // U+2242
    { "eta;", "\xCE\xB7" }, // U+03B7
    { "eth;", "\xC3\xB0" }, // U+00F0
    { "euml;", "\xC3\xAB" }, // U+00EB
    { "euro;", "\xE2\x82\xAC" }, // U+20AC
    { "excl;", "\x21" }, // U+0021
    { "exist;", "\xE2\x88\x83" }, // U+2203
    { "expectation;", "\xE2\x84\xB0" }, // U+2130
    { "exponentiale;", "\xE2\x85\x87" }, // U+2147
};

static constexpr UCSTBL const ucstbl_f[] = {
    { "fallingdotseq;", "\xE2\x89\x92" }, // U+2252
    { "fcy;", "\xD1\x84" }, // U+0444
    { "female;", "\xE2\x99\x80" }, // U+2640
    { "ffilig;", "\xEF\xAC\x83" }, // U+FB03
    { "fflig;", "\xEF\xAC\x80" }, // U+FB00
    { "ffllig;", "\xEF\xAC\x84" }, // U+FB04
    { "ffr;", "\xF0\x9D\x94\xA3" }, // U+1D523
    { "filig;", "\xEF\xAC\x81" }, // U+FB01
    { "fjlig;", "\x66\x6A" }, // U+0066 U+006A
    { "flat;", "\xE2\x99\xAD" }, // U+266D
    { "fllig;", "\xEF\xAC\x82" }, // U+FB02
    { "fltns;", "\xE2\x96\xB1" }, // U+25B1
    { "fnof;", "\xC6\x92" }, // U+0192
    { "fopf;", "\xF0\x9D\x95\x97" }, // U+1D557
    { "forall;", "\xE2\x88\x80" }, // U+2200
    { "fork;", "\xE2\x8B\x94" }, // U+22D4
    { "forkv;", "\xE2\xAB\x99" }, // U+2AD9
    { "fpartint;", "\xE2\xA8\x8D" }, // U+2A0D
    { "frac12;", "\xC2\xBD" }, // U+00BD
    { "frac13;", "\xE2\x85\x93" }, // U+2153
    { "frac14;", "\xC2\xBC" }, // U+00BC
    { "frac15;", "\xE2\x85\x95" }, // U+2155
    { "frac16;", "\xE2\x85\x99" }, // U+2159
    { "frac18;", "\xE2\x85\x9B" }, // U+215B
    { "frac23;", "\xE2\x85\x94" }, // U+2154
    { "frac25;", "\xE2\x85\x96" }, // U+2156
    { "frac34;", "\xC2\xBE" }, // U+00BE
    { "frac35;", "\xE2\x85\x97" }, // U+2157
    { "frac38;", "\xE2\x85\x9C" }, // U+215C
    { "frac45;", "\xE2\x85\x98" }, // U+2158
    { "frac56;", "\xE2\x85\x9A" }, // U+215A
    { "frac58;", "\xE2\x85\x9D" }, // U+215D
    { "frac78;", "\xE2\x85\x9E" }, // U+215E
    { "frasl;", "\xE2\x81\x84" }, // U+2044
    { "frown;", "\xE2\x8C\xA2" }, // U+2322
    { "fscr;", "\xF0\x9D\x92\xBB" }, // U+1D4BB
};

static constexpr UCSTBL const ucstbl_g[] = {
    { "gE;", "\xE2\x89\xA7" }, // U+2267
    { "gEl;", "\xE2\xAA\x8C" }, // U+2A8C
    { "gacute;", "\xC7\xB5" }, // U+01F5
    { "gamma;", "\xCE\xB3" }, // U+03B3
    { "gammad;", "\xCF\x9D" }, // U+03DD
    { "gap;", "\xE2\xAA\x86" }, // U+2A86
    { "gbreve;", "\xC4\x9F" }, // U+011F
    { "gcirc;", "\xC4\x9D" }, // U+011D
    { "gcy;", "\xD0\xB3" }, // U+0433
    { "gdot;", "\xC4\xA1" }, // U+0121
    { "ge;", "\xE2\x89\xA5" }, // U+2265
    { "gel;", "\xE2\x8B\x9B" }, // U+22DB
    { "geq;", "\xE2\x89\xA5" }, // U+2265
    { "geqq;", "\xE2\x89\xA7" }, // U+2267
    { "geqslant;", "\xE2\xA9\xBE" }, // U+2A7E
    { "ges;", "\xE2\xA9\xBE" }, // U+2A7E
    { "gescc;", "\xE2\xAA\xA9" }, // U+2AA9
    { "gesdot;", "\xE2\xAA\x80" }, // U+2A80
    { "gesdoto;", "\xE2\xAA\x82" }, // U+2A82
    { "gesdotol;", "\xE2\xAA\x84" }, // U+2A84
    { "gesl;", "\xE2\x8B\x9B\xEF\xB8\x80" }, // U+22DB U+FE00
    { "gesles;", "\xE2\xAA\x94" }, // U+2A94
    { "gfr;", "\xF0\x9D\x94\xA4" }, // U+1D524
    { "gg;", "\xE2\x89\xAB" }, // U+226B
    { "ggg;", "\xE2\x8B\x99" }, // U+22D9
    { "gimel;", "\xE2\x84\xB7" }, // U+2137
    { "gjcy;", "\xD1\x93" }, // U+0453
    { "gl;", "\xE2\x89\xB7" }, // U+2277
    { "glE;", "\xE2\xAA\x92" }, // U+2A92
    { "gla;", "\xE2\xAA\xA5" }, // U+2AA5
    { "glj;", "\xE2\xAA\xA4" }, // U+2AA4
    { "gnE;", "\xE2\x89\xA9" }, // U+2269
    { "gnap;", "\xE2\xAA\x8A" }, // U+2A8A
    { "gnapprox;", "\xE2\xAA\x8A" }, // U+2A8A
    { "gne;", "\xE2\xAA\x88" }, // U+2A88
    { "gneq;", "\xE2\xAA\x88" }, // U+2A88
    { "gneqq;", "\xE2\x89\xA9" }, // U+2269
    { "gnsim;", "\xE2\x8B\xA7" }, // U+22E7
    { "gopf;", "\xF0\x9D\x95\x98" }, // U+1D558
    { "grave;", "\x60" }, // U+0060
    { "gscr;", "\xE2\x84\x8A" }, // U+210A
    { "gsim;", "\xE2\x89\xB3" }, // U+2273
    { "gsime;", "\xE2\xAA\x8E" }, // U+2A8E
    { "gsiml;", "\xE2\xAA\x90" }, // U+2A90
    { "gt;", "\x3E" }, // U+003E
    { "gtcc;", "\xE2\xAA\xA7" }, // U+2AA7
    { "gtcir;", "\xE2\xA9\xBA" }, // U+2A7A
    { "gtdot;", "\xE2\x8B\x97" }, // U+22D7
    { "gtlPar;", "\xE2\xA6\x95" }, // U+2995
    { "gtquest;", "\xE2\xA9\xBC" }, // U+2A7C
    { "gtrapprox;", "\xE2\xAA\x86" }, // U+2A86
    { "gtrarr;", "\xE2\xA5\xB8" }, // U+2978
    { "gtrdot;", "\xE2\x8B\x97" }, // U+22D7
    { "gtreqless;", "\xE2\x8B\x9B" }, // U+22DB
    { "gtreqqless;", "\xE2\xAA\x8C" }, // U+2A8C
    { "gtrless;", "\xE2\x89\xB7" }, // U+2277
    { "gtrsim;", "\xE2\x89\xB3" }, // U+2273
    { "gvertneqq;", "\xE2\x89\xA9\xEF\xB8\x80" }, // U+2269 U+FE00
    { "gvnE;", "\xE2\x89\xA9\xEF\xB8\x80" }, // U+2269 U+FE00
};

static constexpr UCSTBL const ucstbl_h[] = {
    { "hArr;", "\xE2\x87\x94" }, // U+21D4
    { "hairsp;", "\xE2\x80\x8A" }, // U+200A
    { "half;", "\xC2\xBD" }, // U+00BD
    { "hamilt;", "\xE2\x84\x8B" }, // U+210B
    { "hardcy;", "\xD1\x8A" }, // U+044A
    { "harr;", "\xE2\x86\x94" }, // U+2194
    { "harrcir;", "\xE2\xA5\x88" }, // U+2948
    { "harrw;", "\xE2\x86\xAD" }, // U+21AD
    { "hbar;", "\xE2\x84\x8F" }, // U+210F
    { "hcirc;", "\xC4\xA5" }, // U+0125
    { "hearts;", "\xE2\x99\xA5" }, // U+2665
    { "heartsuit;", "\xE2\x99\xA5" }, // U+2665
    { "hellip;", "\xE2\x80\xA6" }, // U+2026
    { "hercon;", "\xE2\x8A\xB9" }, // U+22B9
    { "hfr;", "\xF0\x9D\x94\xA5" }, // U+1D525
    { "hksearow;", "\xE2\xA4\xA5" }, // U+2925
    { "hkswarow;", "\xE2\xA4\xA6" }, // U+2926
    { "hoarr;", "\xE2\x87\xBF" }, // U+21FF
    { "homtht;", "\xE2\x88\xBB" }, // U+223B
    { "hookleftarrow;", "\xE2\x86\xA9" }, // U+21A9
    { "hookrightarrow;", "\xE2\x86\xAA" }, // U+21AA
    { "hopf;", "\xF0\x9D\x95\x99" }, // U+1D559
    { "horbar;", "\xE2\x80\x95" }, // U+2015
    { "hscr;", "\xF0\x9D\x92\xBD" }, // U+1D4BD
    { "hslash;", "\xE2\x84\x8F" }, // U+210F
    { "hstrok;", "\xC4\xA7" }, // U+0127
    { "hybull;", "\xE2\x81\x83" }, // U+2043
    { "hyphen;", "\xE2\x80\x90" }, // U+2010
};

static constexpr UCSTBL const ucstbl_i[] = {
    { "iacute;", "\xC3\xAD" }, // U+00ED
    { "ic;", "\xE2\x81\xA3" }, // U+2063
    { "icirc;", "\xC3\xAE" }, // U+00EE
    { "icy;", "\xD0\xB8" }, // U+0438
    { "iecy;", "\xD0\xB5" }, // U+0435
    { "iexcl;", "\xC2\xA1" }, // U+00A1
    { "iff;", "\xE2\x87\x94" }, // U+21D4
    { "ifr;", "\xF0\x9D\x94\xA6" }, // U+1D526
    { "igrave;", "\xC3\xAC" }, // U+00EC
    { "ii;", "\xE2\x85\x88" }, // U+2148
    { "iiiint;", "\xE2\xA8\x8C" }, // U+2A0C
    { "iiint;", "\xE2\x88\xAD" }, // U+222D
    { "iinfin;", "\xE2\xA7\x9C" }, // U+29DC
    { "iiota;", "\xE2\x84\xA9" }, // U+2129
    { "ijlig;", "\xC4\xB3" }, // U+0133
    { "imacr;", "\xC4\xAB" }, // U+012B
    { "image;", "\xE2\x84\x91" }, // U+2111
    { "imagline;", "\xE2\x84\x90" }, // U+2110
    { "imagpart;", "\xE2\x84\x91" }, // U+2111
    { "imath;", "\xC4\xB1" }, // U+0131
    { "imof;", "\xE2\x8A\xB7" }, // U+22B7
    { "imped;", "\xC6\xB5" }, // U+01B5
    { "in;", "\xE2\x88\x88" }, // U+2208
    { "incare;", "\xE2\x84\x85" }, // U+2105
    { "infin;", "\xE2\x88\x9E" }, // U+221E
    { "infintie;", "\xE2\xA7\x9D" }, // U+29DD
    { "inodot;", "\xC4\xB1" }, // U+0131
    { "int;", "\xE2\x88\xAB" }, // U+222B
    { "intcal;", "\xE2\x8A\xBA" }, // U+22BA
    { "integers;", "\xE2\x84\xA4" }, // U+2124
    { "intercal;", "\xE2\x8A\xBA" }, // U+22BA
    { "intlarhk;", "\xE2\xA8\x97" }, // U+2A17
    { "intprod;", "\xE2\xA8\xBC" }, // U+2A3C
    { "iocy;", "\xD1\x91" }, // U+0451
    { "iogon;", "\xC4\xAF" }, // U+012F
    { "iopf;", "\xF0\x9D\x95\x9A" }, // U+1D55A
    { "iota;", "\xCE\xB9" }, // U+03B9
    { "iprod;", "\xE2\xA8\xBC" }, // U+2A3C
    { "iquest;", "\xC2\xBF" }, // U+00BF
    { "iscr;", "\xF0\x9D\x92\xBE" }, // U+1D4BE
    { "isin;", "\xE2\x88\x88" }, // U+2208
    { "isinE;", "\xE2\x8B\xB9" }, // U+22F9
    { "isindot;", "\xE2\x8B\xB5" }, // U+22F5
    { "isins;", "\xE2\x8B\xB4" }, // U+22F4
    { "isinsv;", "\xE2\x8B\xB3" }, // U+22F3
    { "isinv;", "\xE2\x88\x88" }, // U+2208
    { "it;", "\xE2\x81\xA2" }, // U+2062
    { "itilde;", "\xC4\xA9" }, // U+0129
    { "iukcy;", "\xD1\x96" }, // U+0456
    { "iuml;", "\xC3\xAF" }, // U+00EF
};

static constexpr UCSTBL const ucstbl_j[] = {
    { "jcirc;", "\xC4\xB5" }, // U+0135
    { "jcy;", "\xD0\xB9" }, // U+0439
    { "jfr;", "\xF0\x9D\x94\xA7" }, // U+1D527
    { "jmath;", "\xC8\xB7" }, // U+0237
    { "jopf;", "\xF0\x9D\x95\x9B" }, // U+1D55B
    { "jscr;", "\xF0\x9D\x92\xBF" }, // U+1D4BF
    { "jsercy;", "\xD1\x98" }, // U+0458
    { "jukcy;", "\xD1\x94" }, // U+0454
};

static constexpr UCSTBL const ucstbl_k[] = {
    { "kappa;", "\xCE\xBA" }, // U+03BA
    { "kappav;", "\xCF\xB0" }, // U+03F0
    { "kcedil;", "\xC4\xB7" }, // U+0137
    { "kcy;", "\xD0\xBA" }, // U+043A
    { "kfr;", "\xF0\x9D\x94\xA8" }, // U+1D528
    { "kgreen;", "\xC4\xB8" }, // U+0138
    { "khcy;", "\xD1\x85" }, // U+0445
    { "kjcy;", "\xD1\x9C" }, // U+045C
    { "kopf;", "\xF0\x9D\x95\x9C" }, // U+1D55C
    { "kscr;", "\xF0\x9D\x93\x80" }, // U+1D4C0
};

static constexpr UCSTBL const ucstbl_l[] = {
    { "lAarr;", "\xE2\x87\x9A" }, // U+21DA
    { "lArr;", "\xE2\x87\x90" }, // U+21D0
    { "lAtail;", "\xE2\xA4\x9B" }, // U+291B
    { "lBarr;", "\xE2\xA4\x8E" }, // U+290E
    { "lE;", "\xE2\x89\xA6" }, // U+2266
    { "lEg;", "\xE2\xAA\x8B" }, // U+2A8B
    { "lHar;", "\xE2\xA5\xA2" }, // U+2962
    { "lacute;", "\xC4\xBA" }, // U+013A
    { "laemptyv;", "\xE2\xA6\xB4" }, // U+29B4
    { "lagran;", "\xE2\x84\x92" }, // U+2112
    { "lambda;", "\xCE\xBB" }, // U+03BB
    { "lang;", "\xE2\x9F\xA8" }, // U+27E8
    { "langd;", "\xE2\xA6\x91" }, // U+2991
    { "langle;", "\xE2\x9F\xA8" }, // U+27E8
    { "lap;", "\xE2\xAA\x85" }, // U+2A85
    { "laquo;", "\xC2\xAB" }, // U+00AB
    { "larr;", "\xE2\x86\x90" }, // U+2190
    { "larrb;", "\xE2\x87\xA4" }, // U+21E4
    { "larrbfs;", "\xE2\xA4\x9F" }, // U+291F
    { "larrfs;", "\xE2\xA4\x9D" }, // U+291D
    { "larrhk;", "\xE2\x86\xA9" }, // U+21A9
    { "larrlp;", "\xE2\x86\xAB" }, // U+21AB
    { "larrpl;", "\xE2\xA4\xB9" }, // U+2939
    { "larrsim;", "\xE2\xA5\xB3" }, // U+2973
    { "larrtl;", "\xE2\x86\xA2" }, // U+21A2
    { "lat;", "\xE2\xAA\xAB" }, // U+2AAB
    { "latail;", "\xE2\xA4\x99" }, // U+2919
    { "late;", "\xE2\xAA\xAD" }, // U+2AAD
    { "lates;", "\xE2\xAA\xAD\xEF\xB8\x80" }, // U+2AAD U+FE00
    { "lbarr;", "\xE2\xA4\x8C" }, // U+290C
    { "lbbrk;", "\xE2\x9D\xB2" }, // U+2772
    { "lbrace;", "\x7B" }, // U+007B
    { "lbrack;", "\x5B" }, // U+005B
    { "lbrke;", "\xE2\xA6\x8B" }, // U+298B
    { "lbrksld;", "\xE2\xA6\x8F" }, // U+298F
    { "lbrkslu;", "\xE2\xA6\x8D" }, // U+298D
    { "lcaron;", "\xC4\xBE" }, // U+013E
    { "lcedil;", "\xC4\xBC" }, // U+013C
    { "lceil;", "\xE2\x8C\x88" }, // U+2308
    { "lcub;", "\x7B" }, // U+007B
    { "lcy;", "\xD0\xBB" }, // U+043B
    { "ldca;", "\xE2\xA4\xB6" }, // U+2936
    { "ldquo;", "\xE2\x80\x9C" }, // U+201C
    { "ldquor;", "\xE2\x80\x9E" }, // U+201E
    { "ldrdhar;", "\xE2\xA5\xA7" }, // U+2967
    { "ldrushar;", "\xE2\xA5\x8B" }, // U+294B
    { "ldsh;", "\xE2\x86\xB2" }, // U+21B2
    { "le;", "\xE2\x89\xA4" }, // U+2264
    { "leftarrow;", "\xE2\x86\x90" }, // U+2190
    { "leftarrowtail;", "\xE2\x86\xA2" }, // U+21A2
    { "leftharpoondown;", "\xE2\x86\xBD" }, // U+21BD
    { "leftharpoonup;", "\xE2\x86\xBC" }, // U+21BC
    { "leftleftarrows;", "\xE2\x87\x87" }, // U+21C7
    { "leftrightarrow;", "\xE2\x86\x94" }, // U+2194
    { "leftrightarrows;", "\xE2\x87\x86" }, // U+21C6
    { "leftrightharpoons;", "\xE2\x87\x8B" }, // U+21CB
    { "leftrightsquigarrow;", "\xE2\x86\xAD" }, // U+21AD
    { "leftthreetimes;", "\xE2\x8B\x8B" }, // U+22CB
    { "leg;", "\xE2\x8B\x9A" }, // U+22DA
    { "leq;", "\xE2\x89\xA4" }, // U+2264
    { "leqq;", "\xE2\x89\xA6" }, // U+2266
    { "leqslant;", "\xE2\xA9\xBD" }, // U+2A7D
    { "les;", "\xE2\xA9\xBD" }, // U+2A7D
    { "lescc;", "\xE2\xAA\xA8" }, // U+2AA8
    { "lesdot;", "\xE2\xA9\xBF" }, // U+2A7F
    { "lesdoto;", "\xE2\xAA\x81" }, // U+2A81
    { "lesdotor;", "\xE2\xAA\x83" }, // U+2A83
    { "lesg;", "\xE2\x8B\x9A\xEF\xB8\x80" }, // U+22DA U+FE00
    { "lesges;", "\xE2\xAA\x93" }, // U+2A93
    { "lessapprox;", "\xE2\xAA\x85" }, // U+2A85
    { "lessdot;", "\xE2\x8B\x96" }, // U+22D6
    { "lesseqgtr;", "\xE2\x8B\x9A" }, // U+22DA
    { "lesseqqgtr;", "\xE2\xAA\x8B" }, // U+2A8B
    { "lessgtr;", "\xE2\x89\xB6" }, // U+2276
    { "lesssim;", "\xE2\x89\xB2" }, // U+2272
    { "lfisht;", "\xE2\xA5\xBC" }, // U+297C
    { "lfloor;", "\xE2\x8C\x8A" }, // U+230A
    { "lfr;", "\xF0\x9D\x94\xA9" }, // U+1D529
    { "lg;", "\xE2\x89\xB6" }, // U+2276
    { "lgE;", "\xE2\xAA\x91" }, // U+2A91
    { "lhard;", "\xE2\x86\xBD" }, // U+21BD
    { "lharu;", "\xE2\x86\xBC" }, // U+21BC
    { "lharul;", "\xE2\xA5\xAA" }, // U+296A
    { "lhblk;", "\xE2\x96\x84" }, // U+2584
    { "ljcy;", "\xD1\x99" }, // U+0459
    { "ll;", "\xE2\x89\xAA" }, // U+226A
    { "llarr;", "\xE2\x87\x87" }, // U+21C7
    { "llcorner;", "\xE2\x8C\x9E" }, // U+231E
    { "llhard;", "\xE2\xA5\xAB" }, // U+296B
    { "lltri;", "\xE2\x97\xBA" }, // U+25FA
    { "lmidot;", "\xC5\x80" }, // U+0140
    { "lmoust;", "\xE2\x8E\xB0" }, // U+23B0
    { "lmoustache;", "\xE2\x8E\xB0" }, // U+23B0
    { "lnE;", "\xE2\x89\xA8" }, // U+2268
    { "lnap;", "\xE2\xAA\x89" }, // U+2A89
    { "lnapprox;", "\xE2\xAA\x89" }, // U+2A89
    { "lne;", "\xE2\xAA\x87" }, // U+2A87
    { "lneq;", "\xE2\xAA\x87" }, // U+2A87
    { "lneqq;", "\xE2\x89\xA8" }, // U+2268
    { "lnsim;", "\xE2\x8B\xA6" }, // U+22E6
    { "loang;", "\xE2\x9F\xAC" }, // U+27EC
    { "loarr;", "\xE2\x87\xBD" }, // U+21FD
    { "lobrk;", "\xE2\x9F\xA6" }, // U+27E6
    { "longleftarrow;", "\xE2\x9F\xB5" }, // U+27F5
    { "longleftrightarrow;", "\xE2\x9F\xB7" }, // U+27F7
    { "longmapsto;", "\xE2\x9F\xBC" }, // U+27FC
    { "longrightarrow;", "\xE2\x9F\xB6" }, // U+27F6
    { "looparrowleft;", "\xE2\x86\xAB" }, // U+21AB
    { "looparrowright;", "\xE2\x86\xAC" }, // U+21AC
    { "lopar;", "\xE2\xA6\x85" }, // U+2985
    { "lopf;", "\xF0\x9D\x95\x9D" }, // U+1D55D
    { "loplus;", "\xE2\xA8\xAD" }, // U+2A2D
    { "lotimes;", "\xE2\xA8\xB4" }, // U+2A34
    { "lowast;", "\xE2\x88\x97" }, // U+2217
    { "lowbar;", "\x5F" }, // U+005F
    { "loz;", "\xE2\x97\x8A" }, // U+25CA
    { "lozenge;", "\xE2\x97\x8A" }, // U+25CA
    { "lozf;", "\xE2\xA7\xAB" }, // U+29EB
    { "lpar;", "\x28" }, // U+0028
    { "lparlt;", "\xE2\xA6\x93" }, // U+2993
    { "lrarr;", "\xE2\x87\x86" }, // U+21C6
    { "lrcorner;", "\xE2\x8C\x9F" }, // U+231F
    { "lrhar;", "\xE2\x87\x8B" }, // U+21CB
    { "lrhard;", "\xE2\xA5\xAD" }, // U+296D
    { "lrm;", "\xE2\x80\x8E" }, // U+200E LEFT-TO-RIGHT MARK
    { "lrtri;", "\xE2\x8A\xBF" }, // U+22BF
    { "lsaquo;", "\xE2\x80\xB9" }, // U+2039
    { "lscr;", "\xF0\x9D\x93\x81" }, // U+1D4C1
    { "lsh;", "\xE2\x86\xB0" }, // U+21B0
    { "lsim;", "\xE2\x89\xB2" }, // U+2272
    { "lsime;", "\xE2\xAA\x8D" }, // U+2A8D
    { "lsimg;", "\xE2\xAA\x8F" }, // U+2A8F
    { "lsqb;", "\x5B" }, // U+005B
    { "lsquo;", "\xE2\x80\x98" }, // U+2018
    { "lsquor;", "\xE2\x80\x9A" }, // U+201A
    { "lstrok;", "\xC5\x82" }, // U+0142
    { "lt;", "\x3C" }, // U+003C
    { "ltcc;", "\xE2\xAA\xA6" }, // U+2AA6
    { "ltcir;", "\xE2\xA9\xB9" }, // U+2A79
    { "ltdot;", "\xE2\x8B\x96" }, // U+22D6
    { "lthree;", "\xE2\x8B\x8B" }, // U+22CB
    { "ltimes;", "\xE2\x8B\x89" }, // U+22C9
    { "ltlarr;", "\xE2\xA5\xB6" }, // U+2976
    { "ltquest;", "\xE2\xA9\xBB" }, // U+2A7B
    { "ltrPar;", "\xE2\xA6\x96" }, // U+2996
    { "ltri;", "\xE2\x97\x83" }, // U+25C3
    { "ltrie;", "\xE2\x8A\xB4" }, // U+22B4
    { "ltrif;", "\xE2\x97\x82" }, // U+25C2
    { "lurdshar;", "\xE2\xA5\x8A" }, // U+294A
    { "luruhar;", "\xE2\xA5\xA6" }, // U+2966
    { "lvertneqq;", "\xE2\x89\xA8\xEF\xB8\x80" }, // U+2268 U+FE00
    { "lvnE;", "\xE2\x89\xA8\xEF\xB8\x80" }, // U+2268 U+FE00
};

static constexpr UCSTBL const ucstbl_m[] = {
    { "mDDot;", "\xE2\x88\xBA" }, // U+223A
    { "macr;", "\xC2\xAF" }, // U+00AF
    { "male;", "\xE2\x99\x82" }, // U+2642
    { "malt;", "\xE2\x9C\xA0" }, // U+2720
    { "maltese;", "\xE2\x9C\xA0" }, // U+2720
    { "map;", "\xE2\x86\xA6" }, // U+21A6
    { "mapsto;", "\xE2\x86\xA6" }, // U+21A6
    { "mapstodown;", "\xE2\x86\xA7" }, // U+21A7
    { "mapstoleft;", "\xE2\x86\xA4" }, // U+21A4
    { "mapstoup;", "\xE2\x86\xA5" }, // U+21A5
    { "marker;", "\xE2\x96\xAE" }, // U+25AE
    { "mcomma;", "\xE2\xA8\xA9" }, // U+2A29
    { "mcy;", "\xD0\xBC" }, // U+043C
    { "mdash;", "\xE2\x80\x94" }, // U+2014
    { "measuredangle;", "\xE2\x88\xA1" }, // U+2221
    { "mfr;", "\xF0\x9D\x94\xAA" }, // U+1D52A
    { "mho;", "\xE2\x84\xA7" }, // U+2127
    { "micro;", "\xC2\xB5" }, // U+00B5
    { "mid;", "\xE2\x88\xA3" }, // U+2223
    { "midast;", "\x2A" }, // U+002A
    { "midcir;", "\xE2\xAB\xB0" }, // U+2AF0
    { "middot;", "\xC2\xB7" }, // U+00B7
    { "minus;", "\xE2\x88\x92" }, // U+2212
    { "minusb;", "\xE2\x8A\x9F" }, // U+229F
    { "minusd;", "\xE2\x88\xB8" }, // U+2238
    { "minusdu;", "\xE2\xA8\xAA" }, // U+2A2A
    { "mlcp;", "\xE2\xAB\x9B" }, // U+2ADB
    { "mldr;", "\xE2\x80\xA6" }, // U+2026
    { "mnplus;", "\xE2\x88\x93" }, // U+2213
    { "models;", "\xE2\x8A\xA7" }, // U+22A7
    { "mopf;", "\xF0\x9D\x95\x9E" }, // U+1D55E
    { "mp;", "\xE2\x88\x93" }, // U+2213
    { "mscr;", "\xF0\x9D\x93\x82" }, // U+1D4C2
    { "mstpos;", "\xE2\x88\xBE" }, // U+223E
    { "mu;", "\xCE\xBC" }, // U+03BC
    { "multimap;", "\xE2\x8A\xB8" }, // U+22B8
    { "mumap;", "\xE2\x8A\xB8" }, // U+22B8
};

static constexpr UCSTBL const ucstbl_n[] = {
    { "nGg;", "\xE2\x8B\x99\xCC\xB8" }, // U+22D9 U+0338
    { "nGt;", "\xE2\x89\xAB\xE2\x83\x92" }, // U+226B U+20D2
    { "nGtv;", "\xE2\x89\xAB\xCC\xB8" }, // U+226B U+0338
    { "nLeftarrow;", "\xE2\x87\x8D" }, // U+21CD
    { "nLeftrightarrow;", "\xE2\x87\x8E" }, // U+21CE
    { "nLl;", "\xE2\x8B\x98\xCC\xB8" }, // U+22D8 U+0338
    { "nLt;", "\xE2\x89\xAA\xE2\x83\x92" }, // U+226A U+20D2
    { "nLtv;", "\xE2\x89\xAA\xCC\xB8" }, // U+226A U+0338
    { "nRightarrow;", "\xE2\x87\x8F" }, // U+21CF
    { "nVDash;", "\xE2\x8A\xAF" }, // U+22AF
    { "nVdash;", "\xE2\x8A\xAE" }, // U+22AE
    { "nabla;", "\xE2\x88\x87" }, // U+2207
    { "nacute;", "\xC5\x84" }, // U+0144
    { "nang;", "\xE2\x88\xA0\xE2\x83\x92" }, // U+2220 U+20D2
    { "nap;", "\xE2\x89\x89" }, // U+2249
    { "napE;", "\xE2\xA9\xB0\xCC\xB8" }, // U+2A70 U+0338
    { "napid;", "\xE2\x89\x8B\xCC\xB8" }, // U+224B U+0338
    { "napos;", "\xC5\x89" }, // U+0149
    { "napprox;", "\xE2\x89\x89" }, // U+2249
    { "natur;", "\xE2\x99\xAE" }, // U+266E
    { "natural;", "\xE2\x99\xAE" }, // U+266E
    { "naturals;", "\xE2\x84\x95" }, // U+2115
    { "nbsp;", "\xC2\xA0" }, // U+00A0
    { "nbump;", "\xE2\x89\x8E\xCC\xB8" }, // U+224E U+0338
    { "nbumpe;", "\xE2\x89\x8F\xCC\xB8" }, // U+224F U+0338
    { "ncap;", "\xE2\xA9\x83" }, // U+2A43
    { "ncaron;", "\xC5\x88" }, // U+0148
    { "ncedil;", "\xC5\x86" }, // U+0146
    { "ncong;", "\xE2\x89\x87" }, // U+2247
    { "ncongdot;", "\xE2\xA9\xAD\xCC\xB8" }, // U+2A6D U+0338
    { "ncup;", "\xE2\xA9\x82" }, // U+2A42
    { "ncy;", "\xD0\xBD" }, // U+043D
    { "ndash;", "\xE2\x80\x93" }, // U+2013
    { "ne;", "\xE2\x89\xA0" }, // U+2260
    { "neArr;", "\xE2\x87\x97" }, // U+21D7
    { "nearhk;", "\xE2\xA4\xA4" }, // U+2924
    { "nearr;", "\xE2\x86\x97" }, // U+2197
    { "nearrow;", "\xE2\x86\x97" }, // U+2197
    { "nedot;", "\xE2\x89\x90\xCC\xB8" }, // U+2250 U+0338
    { "nequiv;", "\xE2\x89\xA2" }, // U+2262
    { "nesear;", "\xE2\xA4\xA8" }, // U+2928
    { "nesim;", "\xE2\x89\x82\xCC\xB8" }, // U+2242 U+0338
    { "nexist;", "\xE2\x88\x84" }, // U+2204
    { "nexists;", "\xE2\x88\x84" }, // U+2204
    { "nfr;", "\xF0\x9D\x94\xAB" }, // U+1D52B
    { "ngE;", "\xE2\x89\xA7\xCC\xB8" }, // U+2267 U+0338
    { "nge;", "\xE2\x89\xB1" }, // U+2271
    { "ngeq;", "\xE2\x89\xB1" }, // U+2271
    { "ngeqq;", "\xE2\x89\xA7\xCC\xB8" }, // U+2267 U+0338
    { "ngeqslant;", "\xE2\xA9\xBE\xCC\xB8" }, // U+2A7E U+0338
    { "nges;", "\xE2\xA9\xBE\xCC\xB8" }, // U+2A7E U+0338
    { "ngsim;", "\xE2\x89\xB5" }, // U+2275
    { "ngt;", "\xE2\x89\xAF" }, // U+226F
    { "ngtr;", "\xE2\x89\xAF" }, // U+226F
    { "nhArr;", "\xE2\x87\x8E" }, // U+21CE
    { "nharr;", "\xE2\x86\xAE" }, // U+21AE
    { "nhpar;", "\xE2\xAB\xB2" }, // U+2AF2
    { "ni;", "\xE2\x88\x8B" }, // U+220B
    { "nis;", "\xE2\x8B\xBC" }, // U+22FC
    { "nisd;", "\xE2\x8B\xBA" }, // U+22FA
    { "niv;", "\xE2\x88\x8B" }, // U+220B
    { "njcy;", "\xD1\x9A" }, // U+045A
    { "nlArr;", "\xE2\x87\x8D" }, // U+21CD
    { "nlE;", "\xE2\x89\xA6\xCC\xB8" }, // U+2266 U+0338
    { "nlarr;", "\xE2\x86\x9A" }, // U+219A
    { "nldr;", "\xE2\x80\xA5" }, // U+2025
    { "nle;", "\xE2\x89\xB0" }, // U+2270
    { "nleftarrow;", "\xE2\x86\x9A" }, // U+219A
    { "nleftrightarrow;", "\xE2\x86\xAE" }, // U+21AE
    { "nleq;", "\xE2\x89\xB0" }, // U+2270
    { "nleqq;", "\xE2\x89\xA6\xCC\xB8" }, // U+2266 U+0338
    { "nleqslant;", "\xE2\xA9\xBD\xCC\xB8" }, // U+2A7D U+0338
    { "nles;", "\xE2\xA9\xBD\xCC\xB8" }, // U+2A7D U+0338
    { "nless;", "\xE2\x89\xAE" }, // U+226E
    { "nlsim;", "\xE2\x89\xB4" }, // U+2274
    { "nlt;", "\xE2\x89\xAE" }, // U+226E
    { "nltri;", "\xE2\x8B\xAA" }, // U+22EA
    { "nltrie;", "\xE2\x8B\xAC" }, // U+22EC
    { "nmid;", "\xE2\x88\xA4" }, // U+2224
    { "nopf;", "\xF0\x9D\x95\x9F" }, // U+1D55F
    { "not;", "\xC2\xAC" }, // U+00AC
    { "notin;", "\xE2\x88\x89" }, // U+2209
    { "notinE;", "\xE2\x8B\xB9\xCC\xB8" }, // U+22F9 U+0338
    { "notindot;", "\xE2\x8B\xB5\xCC\xB8" }, // U+22F5 U+0338
    { "notinva;", "\xE2\x88\x89" }, // U+2209
    { "notinvb;", "\xE2\x8B\xB7" }, // U+22F7
    { "notinvc;", "\xE2\x8B\xB6" }, // U+22F6
    { "notni;", "\xE2\x88\x8C" }, // U+220C
    { "notniva;", "\xE2\x88\x8C" }, // U+220C
    { "notnivb;", "\xE2\x8B\xBE" }, // U+22FE
    { "notnivc;", "\xE2\x8B\xBD" }, // U+22FD
    { "npar;", "\xE2\x88\xA6" }, // U+2226
    { "nparallel;", "\xE2\x88\xA6" }, // U+2226
    { "nparsl;", "\xE2\xAB\xBD\xE2\x83\xA5" }, // U+2AFD U+20E5
    { "npart;", "\xE2\x88\x82\xCC\xB8" }, // U+2202 U+0338
    { "npolint;", "\xE2\xA8\x94" }, // U+2A14
    { "npr;", "\xE2\x8A\x80" }, // U+2280
    { "nprcue;", "\xE2\x8B\xA0" }, // U+22E0
    { "npre;", "\xE2\xAA\xAF\xCC\xB8" }, // U+2AAF U+0338
    { "nprec;", "\xE2\x8A\x80" }, // U+2280
    { "npreceq;", "\xE2\xAA\xAF\xCC\xB8" }, // U+2AAF U+0338
    { "nrArr;", "\xE2\x87\x8F" }, // U+21CF
    { "nrarr;", "\xE2\x86\x9B" }, // U+219B
    { "nrarrc;", "\xE2\xA4\xB3\xCC\xB8" }, // U+2933 U+0338
    { "nrarrw;", "\xE2\x86\x9D\xCC\xB8" }, // U+219D U+0338
    { "nrightarrow;", "\xE2\x86\x9B" }, // U+219B
    { "nrtri;", "\xE2\x8B\xAB" }, // U+22EB
    { "nrtrie;", "\xE2\x8B\xAD" }, // U+22ED
    { "nsc;", "\xE2\x8A\x81" }, // U+2281
    { "nsccue;", "\xE2\x8B\xA1" }, // U+22E1
    { "nsce;", "\xE2\xAA\xB0\xCC\xB8" }, // U+2AB0 U+0338
    { "nscr;", "\xF0\x9D\x93\x83" }, // U+1D4C3
    { "nshortmid;", "\xE2\x88\xA4" }, // U+2224
    { "nshortparallel;", "\xE2\x88\xA6" }, // U+2226
    { "nsim;", "\xE2\x89\x81" }, // U+2241
    { "nsime;", "\xE2\x89\x84" }, // U+2244
    { "nsimeq;", "\xE2\x89\x84" }, // U+2244
    { "nsmid;", "\xE2\x88\xA4" }, // U+2224
    { "nspar;", "\xE2\x88\xA6" }, // U+2226
    { "nsqsube;", "\xE2\x8B\xA2" }, // U+22E2
    { "nsqsupe;", "\xE2\x8B\xA3" }, // U+22E3
    { "nsub;", "\xE2\x8A\x84" }, // U+2284
    { "nsubE;", "\xE2\xAB\x85\xCC\xB8" }, // U+2AC5 U+0338
    { "nsube;", "\xE2\x8A\x88" }, // U+2288
    { "nsubset;", "\xE2\x8A\x82\xE2\x83\x92" }, // U+2282 U+20D2
    { "nsubseteq;", "\xE2\x8A\x88" }, // U+2288
    { "nsubseteqq;", "\xE2\xAB\x85\xCC\xB8" }, // U+2AC5 U+0338
    { "nsucc;", "\xE2\x8A\x81" }, // U+2281
    { "nsucceq;", "\xE2\xAA\xB0\xCC\xB8" }, // U+2AB0 U+0338
    { "nsup;", "\xE2\x8A\x85" }, // U+2285
    { "nsupE;", "\xE2\xAB\x86\xCC\xB8" }, // U+2AC6 U+0338
    { "nsupe;", "\xE2\x8A\x89" }, // U+2289
    { "nsupset;", "\xE2\x8A\x83\xE2\x83\x92" }, // U+2283 U+20D2
    { "nsupseteq;", "\xE2\x8A\x89" }, // U+2289
    { "nsupseteqq;", "\xE2\xAB\x86\xCC\xB8" }, // U+2AC6 U+0338
    { "ntgl;", "\xE2\x89\xB9" }, // U+2279
    { "ntilde;", "\xC3\xB1" }, // U+00F1
    { "ntlg;", "\xE2\x89\xB8" }, // U+2278
    { "ntriangleleft;", "\xE2\x8B\xAA" }, // U+22EA
    { "ntrianglelefteq;", "\xE2\x8B\xAC" }, // U+22EC
    { "ntriangleright;", "\xE2\x8B\xAB" }, // U+22EB
    { "ntrianglerighteq;", "\xE2\x8B\xAD" }, // U+22ED
    { "nu;", "\xCE\xBD" }, // U+03BD
    { "num;", "\x23" }, // U+0023
    { "numero;", "\xE2\x84\x96" }, // U+2116
    { "numsp;", "\xE2\x80\x87" }, // U+2007
    { "nvDash;", "\xE2\x8A\xAD" }, // U+22AD
    { "nvHarr;", "\xE2\xA4\x84" }, // U+2904
    { "nvap;", "\xE2\x89\x8D\xE2\x83\x92" }, // U+224D U+20D2
    { "nvdash;", "\xE2\x8A\xAC" }, // U+22AC
    { "nvge;", "\xE2\x89\xA5\xE2\x83\x92" }, // U+2265 U+20D2
    { "nvgt;", "\x3E\xE2\x83\x92" }, // U+003E U+20D2
    { "nvinfin;", "\xE2\xA7\x9E" }, // U+29DE
    { "nvlArr;", "\xE2\xA4\x82" }, // U+2902
    { "nvle;", "\xE2\x89\xA4\xE2\x83\x92" }, // U+2264 U+20D2
    { "nvlt;", "\x3C\xE2\x83\x92" }, // U+003C U+20D2
    { "nvltrie;", "\xE2\x8A\xB4\xE2\x83\x92" }, // U+22B4 U+20D2
    { "nvrArr;", "\xE2\xA4\x83" }, // U+2903
    { "nvrtrie;", "\xE2\x8A\xB5\xE2\x83\x92" }, // U+22B5 U+20D2
    { "nvsim;", "\xE2\x88\xBC\xE2\x83\x92" }, // U+223C U+20D2
    { "nwArr;", "\xE2\x87\x96" }, // U+21D6
    { "nwarhk;", "\xE2\xA4\xA3" }, // U+2923
    { "nwarr;", "\xE2\x86\x96" }, // U+2196
    { "nwarrow;", "\xE2\x86\x96" }, // U+2196
    { "nwnear;", "\xE2\xA4\xA7" }, // U+2927
};

static constexpr UCSTBL const ucstbl_o[] = {
    { "oS;", "\xE2\x93\x88" }, // U+24C8
    { "oacute;", "\xC3\xB3" }, // U+00F3
    { "oast;", "\xE2\x8A\x9B" }, // U+229B
    { "ocir;", "\xE2\x8A\x9A" }, // U+229A
    { "ocirc;", "\xC3\xB4" }, // U+00F4
    { "ocy;", "\xD0\xBE" }, // U+043E
    { "odash;", "\xE2\x8A\x9D" }, // U+229D
    { "odblac;", "\xC5\x91" }, // U+0151
    { "odiv;", "\xE2\xA8\xB8" }, // U+2A38
    { "odot;", "\xE2\x8A\x99" }, // U+2299
    { "odsold;", "\xE2\xA6\xBC" }, // U+29BC
    { "oelig;", "\xC5\x93" }, // U+0153
    { "ofcir;", "\xE2\xA6\xBF" }, // U+29BF
    { "ofr;", "\xF0\x9D\x94\xAC" }, // U+1D52C
    { "ogon;", "\xCB\x9B" }, // U+02DB
    { "ograve;", "\xC3\xB2" }, // U+00F2
    { "ogt;", "\xE2\xA7\x81" }, // U+29C1
    { "ohbar;", "\xE2\xA6\xB5" }, // U+29B5
    { "ohm;", "\xCE\xA9" }, // U+03A9
    { "oint;", "\xE2\x88\xAE" }, // U+222E
    { "olarr;", "\xE2\x86\xBA" }, // U+21BA
    { "olcir;", "\xE2\xA6\xBE" }, // U+29BE
    { "olcross;", "\xE2\xA6\xBB" }, // U+29BB
    { "oline;", "\xE2\x80\xBE" }, // U+203E
    { "olt;", "\xE2\xA7\x80" }, // U+29C0
    { "omacr;", "\xC5\x8D" }, // U+014D
    { "omega;", "\xCF\x89" }, // U+03C9
    { "omicron;", "\xCE\xBF" }, // U+03BF
    { "omid;", "\xE2\xA6\xB6" }, // U+29B6
    { "ominus;", "\xE2\x8A\x96" }, // U+2296
    { "oopf;", "\xF0\x9D\x95\xA0" }, // U+1D560
    { "opar;", "\xE2\xA6\xB7" }, // U+29B7
    { "operp;", "\xE2\xA6\xB9" }, // U+29B9
    { "oplus;", "\xE2\x8A\x95" }, // U+2295
    { "or;", "\xE2\x88\xA8" }, // U+2228
    { "orarr;", "\xE2\x86\xBB" }, // U+21BB
    { "ord;", "\xE2\xA9\x9D" }, // U+2A5D
    { "order;", "\xE2\x84\xB4" }, // U+2134
    { "orderof;", "\xE2\x84\xB4" }, // U+2134
    { "ordf;", "\xC2\xAA" }, // U+00AA
    { "ordm;", "\xC2\xBA" }, // U+00BA
    { "origof;", "\xE2\x8A\xB6" }, // U+22B6
    { "oror;", "\xE2\xA9\x96" }, // U+2A56
    { "orslope;", "\xE2\xA9\x97" }, // U+2A57
    { "orv;", "\xE2\xA9\x9B" }, // U+2A5B
    { "oscr;", "\xE2\x84\xB4" }, // U+2134
    { "oslash;", "\xC3\xB8" }, // U+00F8
    { "osol;", "\xE2\x8A\x98" }, // U+2298
    { "otilde;", "\xC3\xB5" }, // U+00F5
    { "otimes;", "\xE2\x8A\x97" }, // U+2297
    { "otimesas;", "\xE2\xA8\xB6" }, // U+2A36
    { "ouml;", "\xC3\xB6" }, // U+00F6
    { "ovbar;", "\xE2\x8C\xBD" }, // U+233D
};

static constexpr UCSTBL const ucstbl_p[] = {
    { "par;", "\xE2\x88\xA5" }, // U+2225
    { "para;", "\xC2\xB6" }, // U+00B6
    { "parallel;", "\xE2\x88\xA5" }, // U+2225
    { "parsim;", "\xE2\xAB\xB3" }, // U+2AF3
    { "parsl;", "\xE2\xAB\xBD" }, // U+2AFD
    { "part;", "\xE2\x88\x82" }, // U+2202
    { "pcy;", "\xD0\xBF" }, // U+043F
    { "percnt;", "\x25" }, // U+0025
    { "period;", "\x2E" }, // U+002E
    { "permil;", "\xE2\x80\xB0" }, // U+2030
    { "perp;", "\xE2\x8A\xA5" }, // U+22A5
    { "pertenk;", "\xE2\x80\xB1" }, // U+2031
    { "pfr;", "\xF0\x9D\x94\xAD" }, // U+1D52D
    { "phi;", "\xCF\x86" }, // U+03C6
    { "phiv;", "\xCF\x95" }, // U+03D5
    { "phmmat;", "\xE2\x84\xB3" }, // U+2133
    { "phone;", "\xE2\x98\x8E" }, // U+260E
    { "pi;", "\xCF\x80" }, // U+03C0
    { "pitchfork;", "\xE2\x8B\x94" }, // U+22D4
    { "piv;", "\xCF\x96" }, // U+03D6
    { "planck;", "\xE2\x84\x8F" }, // U+210F
    { "planckh;", "\xE2\x84\x8E" }, // U+210E
    { "plankv;", "\xE2\x84\x8F" }, // U+210F
    { "plus;", "\x2B" }, // U+002B
    { "plusacir;", "\xE2\xA8\xA3" }, // U+2A23
    { "plusb;", "\xE2\x8A\x9E" }, // U+229E
    { "pluscir;", "\xE2\xA8\xA2" }, // U+2A22
    { "plusdo;", "\xE2\x88\x94" }, // U+2214
    { "plusdu;", "\xE2\xA8\xA5" }, // U+2A25
    { "pluse;", "\xE2\xA9\xB2" }, // U+2A72
    { "plusmn;", "\xC2\xB1" }, // U+00B1
    { "plussim;", "\xE2\xA8\xA6" }, // U+2A26
    { "plustwo;", "\xE2\xA8\xA7" }, // U+2A27
    { "pm;", "\xC2\xB1" }, // U+00B1
    { "pointint;", "\xE2\xA8\x95" }, // U+2A15
    { "popf;", "\xF0\x9D\x95\xA1" }, // U+1D561
    { "pound;", "\xC2\xA3" }, // U+00A3
    { "pr;", "\xE2\x89\xBA" }, // U+227A
    { "prE;", "\xE2\xAA\xB3" }, // U+2AB3
    { "prap;", "\xE2\xAA\xB7" }, // U+2AB7
    { "prcue;", "\xE2\x89\xBC" }, // U+227C
    { "pre;", "\xE2\xAA\xAF" }, // U+2AAF
    { "prec;", "\xE2\x89\xBA" }, // U+227A
    { "precapprox;", "\xE2\xAA\xB7" }, // U+2AB7
    { "preccurlyeq;", "\xE2\x89\xBC" }, // U+227C
    { "preceq;", "\xE2\xAA\xAF" }, // U+2AAF
    { "precnapprox;", "\xE2\xAA\xB9" }, // U+2AB9
    { "precneqq;", "\xE2\xAA\xB5" }, // U+2AB5
    { "precnsim;", "\xE2\x8B\xA8" }, // U+22E8
    { "precsim;", "\xE2\x89\xBE" }, // U+227E
    { "prime;", "\xE2\x80\xB2" }, // U+2032
    { "primes;", "\xE2\x84\x99" }, // U+2119
    { "prnE;", "\xE2\xAA\xB5" }, // U+2AB5
    { "prnap;", "\xE2\xAA\xB9" }, // U+2AB9
    { "prnsim;", "\xE2\x8B\xA8" }, // U+22E8
    { "prod;", "\xE2\x88\x8F" }, // U+220F
    { "profalar;", "\xE2\x8C\xAE" }, // U+232E
    { "profline;", "\xE2\x8C\x92" }, // U+2312
    { "profsurf;", "\xE2\x8C\x93" }, // U+2313
    { "prop;", "\xE2\x88\x9D" }, // U+221D
    { "propto;", "\xE2\x88\x9D" }, // U+221D
    { "prsim;", "\xE2\x89\xBE" }, // U+227E
    { "prurel;", "\xE2\x8A\xB0" }, // U+22B0
    { "pscr;", "\xF0\x9D\x93\x85" }, // U+1D4C5
    { "psi;", "\xCF\x88" }, // U+03C8
    { "puncsp;", "\xE2\x80\x88" }, // U+2008
};

static constexpr UCSTBL const ucstbl_q[] = {
    { "qfr;", "\xF0\x9D\x94\xAE" }, // U+1D52E
    { "qint;", "\xE2\xA8\x8C" }, // U+2A0C
    { "qopf;", "\xF0\x9D\x95\xA2" }, // U+1D562
    { "qprime;", "\xE2\x81\x97" }, // U+2057
    { "qscr;", "\xF0\x9D\x93\x86" }, // U+1D4C6
    { "quaternions;", "\xE2\x84\x8D" }, // U+210D
    { "quatint;", "\xE2\xA8\x96" }, // U+2A16
    { "quest;", "\x3F" }, // U+003F
    { "questeq;", "\xE2\x89\x9F" }, // U+225F
    { "quot;", "\x22" }, // U+0022
};

static constexpr UCSTBL const ucstbl_r[] = {
    { "rAarr;", "\xE2\x87\x9B" }, // U+21DB
    { "rArr;", "\xE2\x87\x92" }, // U+21D2
    { "rAtail;", "\xE2\xA4\x9C" }, // U+291C
    { "rBarr;", "\xE2\xA4\x8F" }, // U+290F
    { "rHar;", "\xE2\xA5\xA4" }, // U+2964
    { "race;", "\xE2\x88\xBD\xCC\xB1" }, // U+223D U+0331
    { "racute;", "\xC5\x95" }, // U+0155
    { "radic;", "\xE2\x88\x9A" }, // U+221A
    { "raemptyv;", "\xE2\xA6\xB3" }, // U+29B3
    { "rang;", "\xE2\x9F\xA9" }, // U+27E9
    { "rangd;", "\xE2\xA6\x92" }, // U+2992
    { "range;", "\xE2\xA6\xA5" }, // U+29A5
    { "rangle;", "\xE2\x9F\xA9" }, // U+27E9
    { "raquo;", "\xC2\xBB" }, // U+00BB
    { "rarr;", "\xE2\x86\x92" }, // U+2192
    { "rarrap;", "\xE2\xA5\xB5" }, // U+2975
    { "rarrb;", "\xE2\x87\xA5" }, // U+21E5
    { "rarrbfs;", "\xE2\xA4\xA0" }, // U+2920
    { "rarrc;", "\xE2\xA4\xB3" }, // U+2933
    { "rarrfs;", "\xE2\xA4\x9E" }, // U+291E
    { "rarrhk;", "\xE2\x86\xAA" }, // U+21AA
    { "rarrlp;", "\xE2\x86\xAC" }, // U+21AC
    { "rarrpl;", "\xE2\xA5\x85" }, // U+2945
    { "rarrsim;", "\xE2\xA5\xB4" }, // U+2974
    { "rarrtl;", "\xE2\x86\xA3" }, // U+21A3
    { "rarrw;", "\xE2\x86\x9D" }, // U+219D
    { "ratail;", "\xE2\xA4\x9A" }, // U+291A
    { "ratio;", "\xE2\x88\xB6" }, // U+2236
    { "rationals;", "\xE2\x84\x9A" }, // U+211A
    { "rbarr;", "\xE2\xA4\x8D" }, // U+290D
    { "rbbrk;", "\xE2\x9D\xB3" }, // U+2773
    { "rbrace;", "\x7D" }, // U+007D
    { "rbrack;", "\x5D" }, // U+005D
    { "rbrke;", "\xE2\xA6\x8C" }, // U+298C
    { "rbrksld;", "\xE2\xA6\x8E" }, // U+298E
    { "rbrkslu;", "\xE2\xA6\x90" }, // U+2990
    { "rcaron;", "\xC5\x99" }, // U+0159
    { "rcedil;", "\xC5\x97" }, // U+0157
    { "rceil;", "\xE2\x8C\x89" }, // U+2309
    { "rcub;", "\x7D" }, // U+007D
    { "rcy;", "\xD1\x80" }, // U+0440
    { "rdca;", "\xE2\xA4\xB7" }, // U+2937
    { "rdldhar;", "\xE2\xA5\xA9" }, // U+2969
    { "rdquo;", "\xE2\x80\x9D" }, // U+201D
    { "rdquor;", "\xE2\x80\x9D" }, // U+201D
    { "rdsh;", "\xE2\x86\xB3" }, // U+21B3
    { "real;", "\xE2\x84\x9C" }, // U+211C
    { "realine;", "\xE2\x84\x9B" }, // U+211B
    { "realpart;", "\xE2\x84\x9C" }, // U+211C
    { "reals;", "\xE2\x84\x9D" }, // U+211D
    { "rect;", "\xE2\x96\xAD" }, // U+25AD
    { "reg;", "\xC2\xAE" }, // U+00AE
    { "rfisht;", "\xE2\xA5\xBD" }, // U+297D
    { "rfloor;", "\xE2\x8C\x8B" }, // U+230B
    { "rfr;", "\xF0\x9D\x94\xAF" }, // U+1D52F
    { "rhard;", "\xE2\x87\x81" }, // U+21C1
    { "rharu;", "\xE2\x87\x80" }, // U+21C0
    { "rharul;", "\xE2\xA5\xAC" }, // U+296C
    { "rho;", "\xCF\x81" }, // U+03C1
    { "rhov;", "\xCF\xB1" }, // U+03F1
    { "rightarrow;", "\xE2\x86\x92" }, // U+2192
    { "rightarrowtail;", "\xE2\x86\xA3" }, // U+21A3
    { "rightharpoondown;", "\xE2\x87\x81" }, // U+21C1
    { "rightharpoonup;", "\xE2\x87\x80" }, // U+21C0
    { "rightleftarrows;", "\xE2\x87\x84" }, // U+21C4
    { "rightleftharpoons;", "\xE2\x87\x8C" }, // U+21CC
    { "rightrightarrows;", "\xE2\x87\x89" }, // U+21C9
    { "rightsquigarrow;", "\xE2\x86\x9D" }, // U+219D
    { "rightthreetimes;", "\xE2\x8B\x8C" }, // U+22CC
    { "ring;", "\xCB\x9A" }, // U+02DA
    { "risingdotseq;", "\xE2\x89\x93" }, // U+2253
    { "rlarr;", "\xE2\x87\x84" }, // U+21C4
    { "rlhar;", "\xE2\x87\x8C" }, // U+21CC
    { "rlm;", "\xE2\x80\x8F" }, // U+200F RIGHT-TO-LEFT MARK
    { "rmoust;", "\xE2\x8E\xB1" }, // U+23B1
    { "rmoustache;", "\xE2\x8E\xB1" }, // U+23B1
    { "rnmid;", "\xE2\xAB\xAE" }, // U+2AEE
    { "roang;", "\xE2\x9F\xAD" }, // U+27ED
    { "roarr;", "\xE2\x87\xBE" }, // U+21FE
    { "robrk;", "\xE2\x9F\xA7" }, // U+27E7
    { "ropar;", "\xE2\xA6\x86" }, // U+2986
    { "ropf;", "\xF0\x9D\x95\xA3" }, // U+1D563
    { "roplus;", "\xE2\xA8\xAE" }, // U+2A2E
    { "rotimes;", "\xE2\xA8\xB5" }, // U+2A35
    { "rpar;", "\x29" }, // U+0029
    { "rpargt;", "\xE2\xA6\x94" }, // U+2994
    { "rppolint;", "\xE2\xA8\x92" }, // U+2A12
    { "rrarr;", "\xE2\x87\x89" }, // U+21C9
    { "rsaquo;", "\xE2\x80\xBA" }, // U+203A
    { "rscr;", "\xF0\x9D\x93\x87" }, // U+1D4C7
    { "rsh;", "\xE2\x86\xB1" }, // U+21B1
    { "rsqb;", "\x5D" }, // U+005D
    { "rsquo;", "\xE2\x80\x99" }, // U+2019
    { "rsquor;", "\xE2\x80\x99" }, // U+2019
    { "rthree;", "\xE2\x8B\x8C" }, // U+22CC
    { "rtimes;", "\xE2\x8B\x8A" }, // U+22CA
    { "rtri;", "\xE2\x96\xB9" }, // U+25B9
    { "rtrie;", "\xE2\x8A\xB5" }, // U+22B5
    { "rtrif;", "\xE2\x96\xB8" }, // U+25B8
    { "rtriltri;", "\xE2\xA7\x8E" }, // U+29CE
    { "ruluhar;", "\xE2\xA5\xA8" }, // U+2968
    { "rx;", "\xE2\x84\x9E" }, // U+211E
};

static constexpr UCSTBL const ucstbl_s[] = {
    { "sacute;", "\xC5\x9B" }, // U+015B
    { "sbquo;", "\xE2\x80\x9A" }, // U+201A
    { "sc;", "\xE2\x89\xBB" }, // U+227B
    { "scE;", "\xE2\xAA\xB4" }, // U+2AB4
    { "scap;", "\xE2\xAA\xB8" }, // U+2AB8
    { "scaron;", "\xC5\xA1" }, // U+0161
    { "sccue;", "\xE2\x89\xBD" }, // U+227D
    { "sce;", "\xE2\xAA\xB0" }, // U+2AB0
    { "scedil;", "\xC5\x9F" }, // U+015F
    { "scirc;", "\xC5\x9D" }, // U+015D
    { "scnE;", "\xE2\xAA\xB6" }, // U+2AB6
    { "scnap;", "\xE2\xAA\xBA" }, // U+2ABA
    { "scnsim;", "\xE2\x8B\xA9" }, // U+22E9
    { "scpolint;", "\xE2\xA8\x93" }, // U+2A13
    { "scsim;", "\xE2\x89\xBF" }, // U+227F
    { "scy;", "\xD1\x81" }, // U+0441
    { "sdot;", "\xE2\x8B\x85" }, // U+22C5
    { "sdotb;", "\xE2\x8A\xA1" }, // U+22A1
    { "sdote;", "\xE2\xA9\xA6" }, // U+2A66
    { "seArr;", "\xE2\x87\x98" }, // U+21D8
    { "searhk;", "\xE2\xA4\xA5" }, // U+2925
    { "searr;", "\xE2\x86\x98" }, // U+2198
    { "searrow;", "\xE2\x86\x98" }, // U+2198
    { "sect;", "\xC2\xA7" }, // U+00A7
    { "semi;", "\x3B" }, // U+003B
    { "seswar;", "\xE2\xA4\xA9" }, // U+2929
    { "setminus;", "\xE2\x88\x96" }, // U+2216
    { "setmn;", "\xE2\x88\x96" }, // U+2216
    { "sext;", "\xE2\x9C\xB6" }, // U+2736
    { "sfr;", "\xF0\x9D\x94\xB0" }, // U+1D530
    { "sfrown;", "\xE2\x8C\xA2" }, // U+2322
    { "sharp;", "\xE2\x99\xAF" }, // U+266F
    { "shchcy;", "\xD1\x89" }, // U+0449
    { "shcy;", "\xD1\x88" }, // U+0448
    { "shortmid;", "\xE2\x88\xA3" }, // U+2223
    { "shortparallel;", "\xE2\x88\xA5" }, // U+2225
    { "shy;", "\xC2\xAD" }, // U+00AD
    { "sigma;", "\xCF\x83" }, // U+03C3
    { "sigmaf;", "\xCF\x82" }, // U+03C2
    { "sigmav;", "\xCF\x82" }, // U+03C2
    { "sim;", "\xE2\x88\xBC" }, // U+223C
    { "simdot;", "\xE2\xA9\xAA" }, // U+2A6A
    { "sime;", "\xE2\x89\x83" }, // U+2243
    { "simeq;", "\xE2\x89\x83" }, // U+2243
    { "simg;", "\xE2\xAA\x9E" }, // U+2A9E
    { "simgE;", "\xE2\xAA\xA0" }, // U+2AA0
    { "siml;", "\xE2\xAA\x9D" }, // U+2A9D
    { "simlE;", "\xE2\xAA\x9F" }, // U+2A9F
    { "simne;", "\xE2\x89\x86" }, // U+2246
    { "simplus;", "\xE2\xA8\xA4" }, // U+2A24
    { "simrarr;", "\xE2\xA5\xB2" }, // U+2972
    { "slarr;", "\xE2\x86\x90" }, // U+2190
    { "smallsetminus;", "\xE2\x88\x96" }, // U+2216
    { "smashp;", "\xE2\xA8\xB3" }, // U+2A33
    { "smeparsl;", "\xE2\xA7\xA4" }, // U+29E4
    { "smid;", "\xE2\x88\xA3" }, // U+2223
    { "smile;", "\xE2\x8C\xA3" }, // U+2323
    { "smt;", "\xE2\xAA\xAA" }, // U+2AAA
    { "smte;", "\xE2\xAA\xAC" }, // U+2AAC
    { "smtes;", "\xE2\xAA\xAC\xEF\xB8\x80" }, // U+2AAC U+FE00
    { "softcy;", "\xD1\x8C" }, // U+044C
    { "sol;", "\x2F" }, // U+002F
    { "solb;", "\xE2\xA7\x84" }, // U+29C4
    { "solbar;", "\xE2\x8C\xBF" }, // U+233F
    { "sopf;", "\xF0\x9D\x95\xA4" }, // U+1D564
    { "spades;", "\xE2\x99\xA0" }, // U+2660
    { "spadesuit;", "\xE2\x99\xA0" }, // U+2660
    { "spar;", "\xE2\x88\xA5" }, // U+2225
    { "sqcap;", "\xE2\x8A\x93" }, // U+2293
    { "sqcaps;", "\xE2\x8A\x93\xEF\xB8\x80" }, // U+2293 U+FE00
    { "sqcup;", "\xE2\x8A\x94" }, // U+2294
    { "sqcups;", "\xE2\x8A\x94\xEF\xB8\x80" }, // U+2294 U+FE00
    { "sqsub;", "\xE2\x8A\x8F" }, // U+228F
    { "sqsube;", "\xE2\x8A\x91" }, // U+2291
    { "sqsubset;", "\xE2\x8A\x8F" }, // U+228F
    { "sqsubseteq;", "\xE2\x8A\x91" }, // U+2291
    { "sqsup;", "\xE2\x8A\x90" }, // U+2290
    { "sqsupe;", "\xE2\x8A\x92" }, // U+2292
    { "sqsupset;", "\xE2\x8A\x90" }, // U+2290
    { "sqsupseteq;", "\xE2\x8A\x92" }, // U+2292
    { "squ;", "\xE2\x96\xA1" }, // U+25A1
    { "square;", "\xE2\x96\xA1" }, // U+25A1
    { "squarf;", "\xE2\x96\xAA" }, // U+25AA
    { "squf;", "\xE2\x96\xAA" }, // U+25AA
    { "srarr;", "\xE2\x86\x92" }, // U+2192
    { "sscr;", "\xF0\x9D\x93\x88" }, // U+1D4C8
    { "ssetmn;", "\xE2\x88\x96" }, // U+2216
    { "ssmile;", "\xE2\x8C\xA3" }, // U+2323
    { "sstarf;", "\xE2\x8B\x86" }, // U+22C6
    { "star;", "\xE2\x98\x86" }, // U+2606
    { "starf;", "\xE2\x98\x85" }, // U+2605
    { "straightepsilon;", "\xCF\xB5" }, // U+03F5
    { "straightphi;", "\xCF\x95" }, // U+03D5
    { "strns;", "\xC2\xAF" }, // U+00AF
    { "sub;", "\xE2\x8A\x82" }, // U+2282
    { "subE;", "\xE2\xAB\x85" }, // U+2AC5
    { "subdot;", "\xE2\xAA\xBD" }, // U+2ABD
    { "sube;", "\xE2\x8A\x86" }, // U+2286
    { "subedot;", "\xE2\xAB\x83" }, // U+2AC3
    { "submult;", "\xE2\xAB\x81" }, // U+2AC1
    { "subnE;", "\xE2\xAB\x8B" }, // U+2ACB
    { "subne;", "\xE2\x8A\x8A" }, // U+228A
    { "subplus;", "\xE2\xAA\xBF" }, // U+2ABF
    { "subrarr;", "\xE2\xA5\xB9" }, // U+2979
    { "subset;", "\xE2\x8A\x82" }, // U+2282
    { "subseteq;", "\xE2\x8A\x86" }, // U+2286
    { "subseteqq;", "\xE2\xAB\x85" }, // U+2AC5
    { "subsetneq;", "\xE2\x8A\x8A" }, // U+228A
    { "subsetneqq;", "\xE2\xAB\x8B" }, // U+2ACB
    { "subsim;", "\xE2\xAB\x87" }, // U+2AC7
    { "subsub;", "\xE2\xAB\x95" }, // U+2AD5
    { "subsup;", "\xE2\xAB\x93" }, // U+2AD3
    { "succ;", "\xE2\x89\xBB" }, // U+227B
    { "succapprox;", "\xE2\xAA\xB8" }, // U+2AB8
    { "succcurlyeq;", "\xE2\x89\xBD" }, // U+227D
    { "succeq;", "\xE2\xAA\xB0" }, // U+2AB0
    { "succnapprox;", "\xE2\xAA\xBA" }, // U+2ABA
    { "succneqq;", "\xE2\xAA\xB6" }, // U+2AB6
    { "succnsim;", "\xE2\x8B\xA9" }, // U+22E9
    { "succsim;", "\xE2\x89\xBF" }, // U+227F
    { "sum;", "\xE2\x88\x91" }, // U+2211
    { "sung;", "\xE2\x99\xAA" }, // U+266A
    { "sup1;", "\xC2\xB9" }, // U+00B9
    { "sup2;", "\xC2\xB2" }, // U+00B2
    { "sup3;", "\xC2\xB3" }, // U+00B3
    { "sup;", "\xE2\x8A\x83" }, // U+2283
    { "supE;", "\xE2\xAB\x86" }, // U+2AC6
    { "supdot;", "\xE2\xAA\xBE" }, // U+2ABE
    { "supdsub;", "\xE2\xAB\x98" }, // U+2AD8
    { "supe;", "\xE2\x8A\x87" }, // U+2287
    { "supedot;", "\xE2\xAB\x84" }, // U+2AC4
    { "suphsol;", "\xE2\x9F\x89" }, // U+27C9
    { "suphsub;", "\xE2\xAB\x97" }, // U+2AD7
    { "suplarr;", "\xE2\xA5\xBB" }, // U+297B
    { "supmult;", "\xE2\xAB\x82" }, // U+2AC2
    { "supnE;", "\xE2\xAB\x8C" }, // U+2ACC
    { "supne;", "\xE2\x8A\x8B" }, // U+228B
    { "supplus;", "\xE2\xAB\x80" }, // U+2AC0
    { "supset;", "\xE2\x8A\x83" }, // U+2283
    { "supseteq;", "\xE2\x8A\x87" }, // U+2287
    { "supseteqq;", "\xE2\xAB\x86" }, // U+2AC6
    { "supsetneq;", "\xE2\x8A\x8B" }, // U+228B
    { "supsetneqq;", "\xE2\xAB\x8C" }, // U+2ACC
    { "supsim;", "\xE2\xAB\x88" }, // U+2AC8
    { "supsub;", "\xE2\xAB\x94" }, // U+2AD4
    { "supsup;", "\xE2\xAB\x96" }, // U+2AD6
    { "swArr;", "\xE2\x87\x99" }, // U+21D9
    { "swarhk;", "\xE2\xA4\xA6" }, // U+2926
    { "swarr;", "\xE2\x86\x99" }, // U+2199
    { "swarrow;", "\xE2\x86\x99" }, // U+2199
    { "swnwar;", "\xE2\xA4\xAA" }, // U+292A
    { "szlig;", "\xC3\x9F" }, // U+00DF
};

static constexpr UCSTBL const ucstbl_t[] = {
    { "target;", "\xE2\x8C\x96" }, // U+2316
    { "tau;", "\xCF\x84" }, // U+03C4
    { "tbrk;", "\xE2\x8E\xB4" }, // U+23B4
    { "tcaron;", "\xC5\xA5" }, // U+0165
    { "tcedil;", "\xC5\xA3" }, // U+0163
    { "tcy;", "\xD1\x82" }, // U+0442
    { "tdot;", "\xE2\x83\x9B" }, // U+20DB
    { "telrec;", "\xE2\x8C\x95" }, // U+2315
    { "tfr;", "\xF0\x9D\x94\xB1" }, // U+1D531
    { "there4;", "\xE2\x88\xB4" }, // U+2234
    { "therefore;", "\xE2\x88\xB4" }, // U+2234
    { "theta;", "\xCE\xB8" }, // U+03B8
    { "thetasym;", "\xCF\x91" }, // U+03D1
    { "thetav;", "\xCF\x91" }, // U+03D1
    { "thickapprox;", "\xE2\x89\x88" }, // U+2248
    { "thicksim;", "\xE2\x88\xBC" }, // U+223C
    { "thinsp;", "\xE2\x80\x89" }, // U+2009
    { "thkap;", "\xE2\x89\x88" }, // U+2248
    { "thksim;", "\xE2\x88\xBC" }, // U+223C
    { "thorn;", "\xC3\xBE" }, // U+00FE
    { "tilde;", "\xCB\x9C" }, // U+02DC
    { "times;", "\xC3\x97" }, // U+00D7
    { "timesb;", "\xE2\x8A\xA0" }, // U+22A0
    { "timesbar;", "\xE2\xA8\xB1" }, // U+2A31
    { "timesd;", "\xE2\xA8\xB0" }, // U+2A30
    { "tint;", "\xE2\x88\xAD" }, // U+222D
    { "toea;", "\xE2\xA4\xA8" }, // U+2928
    { "top;", "\xE2\x8A\xA4" }, // U+22A4
    { "topbot;", "\xE2\x8C\xB6" }, // U+2336
    { "topcir;", "\xE2\xAB\xB1" }, // U+2AF1
    { "topf;", "\xF0\x9D\x95\xA5" }, // U+1D565
    { "topfork;", "\xE2\xAB\x9A" }, // U+2ADA
    { "tosa;", "\xE2\xA4\xA9" }, // U+2929
    { "tprime;", "\xE2\x80\xB4" }, // U+2034
    { "trade;", "\xE2\x84\xA2" }, // U+2122
    { "triangle;", "\xE2\x96\xB5" }, // U+25B5
    { "triangledown;", "\xE2\x96\xBF" }, // U+25BF
    { "triangleleft;", "\xE2\x97\x83" }, // U+25C3
    { "trianglelefteq;", "\xE2\x8A\xB4" }, // U+22B4
    { "triangleq;", "\xE2\x89\x9C" }, // U+225C
    { "triangleright;", "\xE2\x96\xB9" }, // U+25B9
    { "trianglerighteq;", "\xE2\x8A\xB5" }, // U+22B5
    { "tridot;", "\xE2\x97\xAC" }, // U+25EC
    { "trie;", "\xE2\x89\x9C" }, // U+225C
    { "triminus;", "\xE2\xA8\xBA" }, // U+2A3A
    { "triplus;", "\xE2\xA8\xB9" }, // U+2A39
    { "trisb;", "\xE2\xA7\x8D" }, // U+29CD
    { "tritime;", "\xE2\xA8\xBB" }, // U+2A3B
    { "trpezium;", "\xE2\x8F\xA2" }, // U+23E2
    { "tscr;", "\xF0\x9D\x93\x89" }, // U+1D4C9
    { "tscy;", "\xD1\x86" }, // U+0446
    { "tshcy;", "\xD1\x9B" }, // U+045B
    { "tstrok;", "\xC5\xA7" }, // U+0167
    { "twixt;", "\xE2\x89\xAC" }, // U+226C
    { "twoheadleftarrow;", "\xE2\x86\x9E" }, // U+219E
    { "twoheadrightarrow;", "\xE2\x86\xA0" }, // U+21A0
};

static constexpr UCSTBL const ucstbl_u[] = {
    { "uArr;", "\xE2\x87\x91" }, // U+21D1
    { "uHar;", "\xE2\xA5\xA3" }, // U+2963
    { "uacute;", "\xC3\xBA" }, // U+00FA
    { "uarr;", "\xE2\x86\x91" }, // U+2191
    { "ubrcy;", "\xD1\x9E" }, // U+045E
    { "ubreve;", "\xC5\xAD" }, // U+016D
    { "ucirc;", "\xC3\xBB" }, // U+00FB
    { "ucy;", "\xD1\x83" }, // U+0443
    { "udarr;", "\xE2\x87\x85" }, // U+21C5
    { "udblac;", "\xC5\xB1" }, // U+0171
    { "udhar;", "\xE2\xA5\xAE" }, // U+296E
    { "ufisht;", "\xE2\xA5\xBE" }, // U+297E
    { "ufr;", "\xF0\x9D\x94\xB2" }, // U+1D532
    { "ugrave;", "\xC3\xB9" }, // U+00F9
    { "uharl;", "\xE2\x86\xBF" }, // U+21BF
    { "uharr;", "\xE2\x86\xBE" }, // U+21BE
    { "uhblk;", "\xE2\x96\x80" }, // U+2580
    { "ulcorn;", "\xE2\x8C\x9C" }, // U+231C
    { "ulcorner;", "\xE2\x8C\x9C" }, // U+231C
    { "ulcrop;", "\xE2\x8C\x8F" }, // U+230F
    { "ultri;", "\xE2\x97\xB8" }, // U+25F8
    { "umacr;", "\xC5\xAB" }, // U+016B
    { "uml;", "\xC2\xA8" }, // U+00A8
    { "uogon;", "\xC5\xB3" }, // U+0173
    { "uopf;", "\xF0\x9D\x95\xA6" }, // U+1D566
    { "uparrow;", "\xE2\x86\x91" }, // U+2191
    { "updownarrow;", "\xE2\x86\x95" }, // U+2195
    { "upharpoonleft;", "\xE2\x86\xBF" }, // U+21BF
    { "upharpoonright;", "\xE2\x86\xBE" }, // U+21BE
    { "uplus;", "\xE2\x8A\x8E" }, // U+228E
    { "upsi;", "\xCF\x85" }, // U+03C5
    { "upsih;", "\xCF\x92" }, // U+03D2
    { "upsilon;", "\xCF\x85" }, // U+03C5
    { "upuparrows;", "\xE2\x87\x88" }, // U+21C8
    { "urcorn;", "\xE2\x8C\x9D" }, // U+231D
    { "urcorner;", "\xE2\x8C\x9D" }, // U+231D
    { "urcrop;", "\xE2\x8C\x8E" }, // U+230E
    { "uring;", "\xC5\xAF" }, // U+016F
    { "urtri;", "\xE2\x97\xB9" }, // U+25F9
    { "uscr;", "\xF0\x9D\x93\x8A" }, // U+1D4CA
    { "utdot;", "\xE2\x8B\xB0" }, // U+22F0
    { "utilde;", "\xC5\xA9" }, // U+0169
    { "utri;", "\xE2\x96\xB5" }, // U+25B5
    { "utrif;", "\xE2\x96\xB4" }, // U+25B4
    { "uuarr;", "\xE2\x87\x88" }, // U+21C8
    { "uuml;", "\xC3\xBC" }, // U+00FC
    { "uwangle;", "\xE2\xA6\xA7" }, // U+29A7
};

static constexpr UCSTBL const ucstbl_v[] = {
    { "vArr;", "\xE2\x87\x95" }, // U+21D5
    { "vBar;", "\xE2\xAB\xA8" }, // U+2AE8
    { "vBarv;", "\xE2\xAB\xA9" }, // U+2AE9
    { "vDash;", "\xE2\x8A\xA8" }, // U+22A8
    { "vangrt;", "\xE2\xA6\x9C" }, // U+299C
    { "varepsilon;", "\xCF\xB5" }, // U+03F5
    { "varkappa;", "\xCF\xB0" }, // U+03F0
    { "varnothing;", "\xE2\x88\x85" }, // U+2205
    { "varphi;", "\xCF\x95" }, // U+03D5
    { "varpi;", "\xCF\x96" }, // U+03D6
    { "varpropto;", "\xE2\x88\x9D" }, // U+221D
    { "varr;", "\xE2\x86\x95" }, // U+2195
    { "varrho;", "\xCF\xB1" }, // U+03F1
    { "varsigma;", "\xCF\x82" }, // U+03C2
    { "varsubsetneq;", "\xE2\x8A\x8A\xEF\xB8\x80" }, // U+228A U+FE00
    { "varsubsetneqq;", "\xE2\xAB\x8B\xEF\xB8\x80" }, // U+2ACB U+FE00
    { "varsupsetneq;", "\xE2\x8A\x8B\xEF\xB8\x80" }, // U+228B U+FE00
    { "varsupsetneqq;", "\xE2\xAB\x8C\xEF\xB8\x80" }, // U+2ACC U+FE00
    { "vartheta;", "\xCF\x91" }, // U+03D1
    { "vartriangleleft;", "\xE2\x8A\xB2" }, // U+22B2
    { "vartriangleright;", "\xE2\x8A\xB3" }, // U+22B3
    { "vcy;", "\xD0\xB2" }, // U+0432
    { "vdash;", "\xE2\x8A\xA2" }, // U+22A2
    { "vee;", "\xE2\x88\xA8" }, // U+2228
    { "veebar;", "\xE2\x8A\xBB" }, // U+22BB
    { "veeeq;", "\xE2\x89\x9A" }, // U+225A
    { "vellip;", "\xE2\x8B\xAE" }, // U+22EE
    { "verbar;", "\x7C" }, // U+007C
    { "vert;", "\x7C" }, // U+007C
    { "vfr;", "\xF0\x9D\x94\xB3" }, // U+1D533
    { "vltri;", "\xE2\x8A\xB2" }, // U+22B2
    { "vnsub;", "\xE2\x8A\x82\xE2\x83\x92" }, // U+2282 U+20D2
    { "vnsup;", "\xE2\x8A\x83\xE2\x83\x92" }, // U+2283 U+20D2
    { "vopf;", "\xF0\x9D\x95\xA7" }, // U+1D567
    { "vprop;", "\xE2\x88\x9D" }, // U+221D
    { "vrtri;", "\xE2\x8A\xB3" }, // U+22B3
    { "vscr;", "\xF0\x9D\x93\x8B" }, // U+1D4CB
    { "vsubnE;", "\xE2\xAB\x8B\xEF\xB8\x80" }, // U+2ACB U+FE00
    { "vsubne;", "\xE2\x8A\x8A\xEF\xB8\x80" }, // U+228A U+FE00
    { "vsupnE;", "\xE2\xAB\x8C\xEF\xB8\x80" }, // U+2ACC U+FE00
    { "vsupne;", "\xE2\x8A\x8B\xEF\xB8\x80" }, // U+228B U+FE00
    { "vzigzag;", "\xE2\xA6\x9A" }, // U+299A
};

static constexpr UCSTBL const ucstbl_w[] = {
    { "wcirc;", "\xC5\xB5" }, // U+0175
    { "wedbar;", "\xE2\xA9\x9F" }, // U+2A5F
    { "wedge;", "\xE2\x88\xA7" }, // U+2227
    { "wedgeq;", "\xE2\x89\x99" }, // U+2259
    { "weierp;", "\xE2\x84\x98" }, // U+2118
    { "wfr;", "\xF0\x9D\x94\xB4" }, // U+1D534
    { "wopf;", "\xF0\x9D\x95\xA8" }, // U+1D568
    { "wp;", "\xE2\x84\x98" }, // U+2118
    { "wr;", "\xE2\x89\x80" }, // U+2240
    { "wreath;", "\xE2\x89\x80" }, // U+2240
    { "wscr;", "\xF0\x9D\x93\x8C" }, // U+1D4CC
};

static constexpr UCSTBL const ucstbl_x[] = {
    { "xcap;", "\xE2\x8B\x82" }, // U+22C2
    { "xcirc;", "\xE2\x97\xAF" }, // U+25EF
    { "xcup;", "\xE2\x8B\x83" }, // U+22C3
    { "xdtri;", "\xE2\x96\xBD" }, // U+25BD
    { "xfr;", "\xF0\x9D\x94\xB5" }, // U+1D535
    { "xhArr;", "\xE2\x9F\xBA" }, // U+27FA
    { "xharr;", "\xE2\x9F\xB7" }, // U+27F7
    { "xi;", "\xCE\xBE" }, // U+03BE
    { "xlArr;", "\xE2\x9F\xB8" }, // U+27F8
    { "xlarr;", "\xE2\x9F\xB5" }, // U+27F5
    { "xmap;", "\xE2\x9F\xBC" }, // U+27FC
    { "xnis;", "\xE2\x8B\xBB" }, // U+22FB
    { "xodot;", "\xE2\xA8\x80" }, // U+2A00
    { "xopf;", "\xF0\x9D\x95\xA9" }, // U+1D569
    { "xoplus;", "\xE2\xA8\x81" }, // U+2A01
    { "xotime;", "\xE2\xA8\x82" }, // U+2A02
    { "xrArr;", "\xE2\x9F\xB9" }, // U+27F9
    { "xrarr;", "\xE2\x9F\xB6" }, // U+27F6
    { "xscr;", "\xF0\x9D\x93\x8D" }, // U+1D4CD
    { "xsqcup;", "\xE2\xA8\x86" }, // U+2A06
    { "xuplus;", "\xE2\xA8\x84" }, // U+2A04
    { "xutri;", "\xE2\x96\xB3" }, // U+25B3
    { "xvee;", "\xE2\x8B\x81" }, // U+22C1
    { "xwedge;", "\xE2\x8B\x80" }, // U+22C0
};

static constexpr UCSTBL const ucstbl_y[] = {
    { "yacute;", "\xC3\xBD" }, // U+00FD
    { "yacy;", "\xD1\x8F" }, // U+044F
    { "ycirc;", "\xC5\xB7" }, // U+0177
    { "ycy;", "\xD1\x8B" }, // U+044B
    { "yen;", "\xC2\xA5" }, // U+00A5
    { "yfr;", "\xF0\x9D\x94\xB6" }, // U+1D536
    { "yicy;", "\xD1\x97" }, // U+0457
    { "yopf;", "\xF0\x9D\x95\xAA" }, // U+1D56A
    { "yscr;", "\xF0\x9D\x93\x8E" }, // U+1D4CE
    { "yucy;", "\xD1\x8E" }, // U+044E
    { "yuml;", "\xC3\xBF" }, // U+00FF
};

static constexpr UCSTBL const ucstbl_z[] = {
    { "zacute;", "\xC5\xBA" }, // U+017A
    { "zcaron;", "\xC5\xBE" }, // U+017E
    { "zcy;", "\xD0\xB7" }, // U+0437
    { "zdot;", "\xC5\xBC" }, // U+017C
    { "zeetrf;", "\xE2\x84\xA8" }, // U+2128
    { "zeta;", "\xCE\xB6" }, // U+03B6
    { "zfr;", "\xF0\x9D\x94\xB7" }, // U+1D537
    { "zhcy;", "\xD0\xB6" }, // U+0436
    { "zigrarr;", "\xE2\x87\x9D" }, // U+21DD
    { "zopf;", "\xF0\x9D\x95\xAB" }, // U+1D56B
    { "zscr;", "\xF0\x9D\x93\x8F" }, // U+1D4CF
    { "zwj;", "\xE2\x80\x8D" }, // U+200D ZERO WIDTH JOINER
    { "zwnj;", "\xE2\x80\x8C" }, // U+200C ZERO WIDTH NON-JOINER
};

static constexpr std::array<JDLIB::span<const UCSTBL>, 26> const ucstbl_lower = {
    ucstbl_a, ucstbl_b, ucstbl_c, ucstbl_d, ucstbl_e,
    ucstbl_f, ucstbl_g, ucstbl_h, ucstbl_i, ucstbl_j,
    ucstbl_k, ucstbl_l, ucstbl_m, ucstbl_n, ucstbl_o,
    ucstbl_p, ucstbl_q, ucstbl_r, ucstbl_s, ucstbl_t,
    ucstbl_u, ucstbl_v, ucstbl_w, ucstbl_x, ucstbl_y,
    ucstbl_z
};

static constexpr std::array<JDLIB::span<const UCSTBL>, 26> const ucstbl_upper = {
    ucstbl_A, ucstbl_B, ucstbl_C, ucstbl_D, ucstbl_E,
    ucstbl_F, ucstbl_G, ucstbl_H, ucstbl_I, ucstbl_J,
    ucstbl_K, ucstbl_L, ucstbl_M, ucstbl_N, ucstbl_O,
    ucstbl_P, ucstbl_Q, ucstbl_R, ucstbl_S, ucstbl_T,
    ucstbl_U, ucstbl_V, ucstbl_W, ucstbl_X, ucstbl_Y,
    ucstbl_Z,
};


enum
{
    UCS_ZWSP    = 0x200B,
    UCS_ZWNJ    = 0x200C,
    UCS_ZWJ     = 0x200D,
    UCS_LRM     = 0x200E,
    UCS_RLM     = 0x200F,
    CP_LINE_SEPARATOR = 8232,
    UCS_REPLACE = 0xFFFD,
};


#endif
