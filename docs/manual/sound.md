---
title: 効果音の再生について
layout: default
---

&gt; [Top](../) &gt; [その他]({{ site.baseurl }}/info/) &gt; {{ page.title }}

## {{ page.title }}


### 使い方
ALSAによる効果音再生機能を有効にするには「configure」実行時に「**\-\-with-alsa**」オプションを付けてから make する。
さらにキャッシュディレクトリ( `$XDG_CACHE_HOME/jdim` )に **sound/** ディレクトリを作り、以下のwavファイルをコピーする。

| ファイル名 | 説明 |
| --- | --- |
| new.wav | 新規スレを開いた |
| res.wav | 更新がある |
| no.wav | 更新がない |
| err.wav | エラー |
