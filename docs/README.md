# オンラインマニュアルのメンテナンスについて

**※この項目は草案の段階です。**

- [概要](#概要)
- [ポリシー](#ポリシー)
- [内容の更新について](#内容の更新について)
- [JDimリリース時の作業について](#jdimリリース時の作業について)


## 概要
JDimのマニュアルは更新作業を簡素化するためMarkdownで記述し、
[GitHub pages][gh-pages]の機能([jekyll][jekyll])を利用してHTMLに変換する方法をとっています。

また、この文書は https://jd4linux.osdn.jp/maintenance_of_manual.html のコピーを起点としています。


## ポリシー
一部のWebブラウザだけではなく、多くの環境で閲覧出来る事を目標にしています。

#### Markdownの利用
軽量マークアップ言語 [Markdown][gh-markdown] を使ってマニュアルを記述します。

HTMLのタグによるマークアップはなるべく使わないようにします。

#### スタイルについて
スタイルはGitHub pagesで用意されているjekyllテーマを使用します。


## 内容の更新について
1. gitリポジトリ `docs/` ディレクトリ以下にある `*.md` ファイルを修正します。
2. ローカルでjekyllを動かして修正したページの表示を確認してください。
   jekyllの導入や実行は [Quickstart (jekyll)][jekyll-quickstart] を参照してください。<br>
   デフォルトの設定では `http://localhost:4000/JDim/` にサイトが公開されます。
3. Pull requestを提出します。([CONTRIBUTING.md][contributing]を参照)
4. マージされたらWebブラウザでアクセスして確認します。https://jdimproved.github.io/JDim/

#### 新規にファイルを追加する
他のファイルを参考に`docs/manual/`ディレクトリの中に作成して下さい。

#### 更新履歴を書く
これ以降の`YYYY`は2011などの年数に読み替えて下さい。

`docs/manual/YYYY.md`のunreleasedの中に一行ずつ簡潔に書きます。
また、関連のIssueやPull requestへのリンクがあると参照しやすいです。例:
```markdown
- Always include crypt.h header for crypt function ←PRのタイトル
  ([#1](https://github.com/JDimproved/JDim/pull/1)) ←PRの番号とURL
  ([f50eb12365](https://github.com/JDimproved/JDim/commit/f50eb12365)) ←マージコミットのハッシュとリンク
```

#### 年が変わる場合
更新履歴は年ごとに分けているので、年が変わる場合は以下の作業をします。

1. 前年のフォーマットを参考に`docs/manual/YYYY.md`を作成する。
2. 前年のunreleasedを新年のファイルへ移動する。
2. `docs/manual/history.md`のリストに以下を追加する。
   ```markdown
   - [YYYY年]({{ site.baseurl }}/YYYY/)
   ```


## JDimリリース時の作業について
JDimがリリースされた時には以下の作業を行います。

1. `docs/manual/YYYY.md`にある先頭の見出しをコピーします。例:
   ```markdown
   <a name="1.0.0-unreleased"></a>
   ### [1.0.0-unreleased](https://github.com/JDimproved/JDim/compare/JDim-v0.1.0...master) (unreleased)

   <a name="1.0.0-unreleased"></a>
   ### [1.0.0-unreleased](https://github.com/JDimproved/JDim/compare/JDim-v0.1.0...master) (unreleased)
   ...
   ```
2. バージョン番号、日付、リンクを修正します。例:
   ```markdown
   <a name="2.0.0-unreleased"></a>
   ### [2.0.0-unreleased](https://github.com/JDimproved/JDim/compare/JDim-v1.0.0...master) (unreleased)

   <a name="1.0.0-20190430"></a>
   ### [1.0.0-20190430](https://github.com/JDimproved/JDim/compare/JDim-v0.1.0...JDim-v1.0.0) (2019-04-30)
   ...
   ```
3. リリースの見出しとリンクを追加します。例:
   ```markdown
   <a name="2.0.0-unreleased"></a>
   ### [2.0.0-unreleased](https://github.com/JDimproved/JDim/compare/JDim-v1.0.0...master) (unreleased)

   <a name="JDim-v1.0.0"></a>
   ### [**JDim-v1.0.0** Release](https://github.com/JDimproved/JDim/releases/tag/JDim-v1.0.0) (2019-05-01)

   <a name="1.0.0-20190430"></a>
   ### [1.0.0-20190430](https://github.com/JDimproved/JDim/compare/JDim-v0.1.0...JDim-v1.0.0) (2019-04-30)
   ...
   ```


[gh-pages]: https://pages.github.com/
[jekyll]: https://jekyllrb.com/
[gh-markdown]: https://guides.github.com/features/mastering-markdown/
[jekyll-quickstart]: https://jekyllrb.com/docs/
[contributing]: https://github.com/JDimproved/JDim/tree/master/CONTRIBUTING.md
