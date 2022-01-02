---
title: JDimについて
layout: default
---

&gt; [Top](../) &gt; {{ page.title }}

## {{ page.title }}

- [概要](#abstract)
- [著作権](#copyright)
- [ライセンス](#license)
- [連絡先](#contact)
- [動作プラットフォーム](#platform)


<a name="abstract"></a>
### 概要
JDim (JD improved)はGTK+(gtkmm)を使用した[５ちゃんねる][]型マルチスレッドBBSを閲覧するためのブラウザです。
JDimはGPLv2の下で公開されている [JD][] からforkしたソフトウェアであり、
ルック・アンド・フィールや環境設定はJDと[互換性][]があります。

**注意: JDim本体は5ch.netのAPIに対応しておりません。**
ご不便をおかけして申し訳ありませんが、5ch.netにアクセスする場合はWebブラウザなどをご使用ください。


<a name="copyright"></a>
### 著作権
© 2017-2019 [yama-natuki][]  
© 2019-2022 [JDimproved project][repository]

パッチやファイルを取り込んだ場合、それらのコピーライトは「JDimproved project」に統一します。

##### fork元
© 2006-2015 [JD project][JD]


<a name="license"></a>
### ライセンス
[GNU General Public License, version 2][gpl2]


<a name="contact"></a>
### 連絡先
バグ報告その他は[Linux板@５ちゃんねる][linux]のJD/JDimスレ、または[JDimのリポジトリ][repository]にて行ってください。


<a name="platform"></a>
### 動作プラットフォーム
LinuxなどのUnixライクなOS(FreeBSD,OpenBSD,Nexenta,MacOSXでも動作報告例があります)。

メンテナンスの都合によりWindows(MinGW)版のサポートは[終了][#445]しました。


[５ちゃんねる]: https://5ch.net
[JD]: https://ja.osdn.net/projects/jd4linux/ "JD for Linux プロジェクト日本語トップページ"
[yama-natuki]: https://github.com/yama-natuki
[repository]: https://github.com/JDimproved/JDim
[gpl2]: https://ja.osdn.net/projects/opensource/wiki/licenses%2FGNU_General_Public_License
[linux]: https://mao.5ch.net/linux/
[#445]: https://github.com/JDimproved/JDim/issues/445

[互換性]: {{ site.baseurl }}/start/#compatibility "起動について \| JDim"
