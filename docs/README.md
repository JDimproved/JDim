<!-- SPDX-License-Identifier: FSFAP -->

# オンラインマニュアルのメンテナンスについて

- [概要](#概要)
- [ポリシー](#ポリシー)
- [内容の更新について](#内容の更新について)
- [JDimリリース時の作業について](#jdimリリース時の作業について)


## 概要
JDimのマニュアルは更新作業を簡素化するためMarkdownで記述し、
[GitHub pages][gh-pages]の機能([jekyll][jekyll])を利用してHTMLに変換する方法をとっています。

この文書は JD project のマニュアルのメンテナンス文書を参考にしています。


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
```

`api.github.com`からPull requestのデータを取得して変更履歴を作るコマンド (curl, jq, sed を使った例)
```sh
# マージされた最新100件のPRから変更履歴を作る
generate_changelogs () {
  API='https://api.github.com/repos/JDimproved/JDim/pulls?state=closed&base=master&per_page=100'
  QUERY='.[] | select(.merged_at != null) | .html_url, .title'
  curl "$API" | jq -r "$QUERY" | sed -e '1~2s%^.\+/\(.\+\)$%- ([#\1](&))%' -e '2~2s/^ */  /'
}
generate_changelogs
```
```sh
# 特定のPRから変更履歴を作る
generate_changelog () {
  API="https://api.github.com/repos/JDimproved/JDim/pulls/$1"
  curl "$API" | jq -r '.html_url, .title' | sed -e '1~2s%^.\+/\(.\+\)$%- ([#\1](&))%' -e '2~2s/^ */  /'
}
generate_changelog 1
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
   <a name="0.2.0-unreleased"></a>
   ### [0.2.0-unreleased](https://github.com/JDimproved/JDim/compare/JDim-v0.1.0...master) (unreleased)

   <a name="0.2.0-unreleased"></a>
   ### [0.2.0-unreleased](https://github.com/JDimproved/JDim/compare/JDim-v0.1.0...master) (unreleased)
   ...
   ```
2. バージョン番号、日付、リンクを修正します。例:
   ```markdown
   <a name="0.3.0-unreleased"></a>
   ### [0.3.0-unreleased](https://github.com/JDimproved/JDim/compare/JDim-v0.2.0...master) (unreleased)

   <a name="0.2.0-20190720"></a>
   ### [0.2.0-20190720](https://github.com/JDimproved/JDim/compare/JDim-v0.1.0...JDim-v0.2.0) (2019-07-20)
   ...
   ```
3. リリースの見出しとリンクを追加します。例:
   ```markdown
   <a name="0.3.0-unreleased"></a>
   ### [0.3.0-unreleased](https://github.com/JDimproved/JDim/compare/JDim-v0.2.0...master) (unreleased)

   <a name="JDim-v0.2.0"></a>
   ### [**JDim-v0.2.0** Release](https://github.com/JDimproved/JDim/releases/tag/JDim-v0.2.0) (2019-07-20)

   <a name="0.2.0-20190720"></a>
   ### [0.2.0-20190720](https://github.com/JDimproved/JDim/compare/JDim-v0.1.0...JDim-v0.2.0) (2019-07-20)
   ...
   ```
4. 前のバージョンのマニュアルを更新します。<br>
   リンク集のファイル `link-YYYYMMDD` をコピーして日付やバージョン番号、URLなどを改めます。
   そして `index.md` の「前のバージョンのマニュアル (GitHubリンク)」に新しいページへのリンクを追加します。


[gh-pages]: https://pages.github.com/
[jekyll]: https://jekyllrb.com/
[gh-markdown]: https://guides.github.com/features/mastering-markdown/
[jekyll-quickstart]: https://jekyllrb.com/docs/
[contributing]: https://github.com/JDimproved/JDim/tree/master/CONTRIBUTING.md
