---
title: バックアップ、アンインストールについて
layout: default
---

&gt; [Top](../) &gt; {{ page.title }}

## {{ page.title }}

- [バックアップ方法](#backup)
- [アンインストール方法( rpm の場合 )](#uninstall-rpm)
- [アンインストール方法( 手動の場合 )](#uninstall-manual)


<a name="backup"></a>
### バックアップ方法
キャッシュのバックアップはキャッシュディレクトリ(デフォルトでは `~/.jd` )
以下を保存するだけでよい。


<a name="uninstall-rpm"></a>
### アンインストール方法( rpm の場合 )
```
$ rpm -e jd
$ rm -rf <キャッシュディレクトリ>
```


<a name="uninstall-manual"></a>
### アンインストール方法( 手動の場合 )
```
$ rm (インストールパス)/jd
$ rm -rf <キャッシュディレクトリ>
```
