---
title: バックアップ、アンインストールについて
layout: default
---

&gt; [Top](../) &gt; {{ page.title }}

## {{ page.title }}

- [バックアップ方法](#backup)
- [アンインストール方法( 手動の場合 )](#uninstall-manual)


<a name="backup"></a>
### バックアップ方法
キャッシュのバックアップはキャッシュディレクトリ(デフォルトでは `$XDG_CACHE_HOME/jdim` )
以下を保存するだけでよい。


<a name="uninstall-manual"></a>
### アンインストール方法( 手動の場合 )
```
$ rm (インストールパス)/jdim
$ rm -rf <キャッシュディレクトリ>
```
