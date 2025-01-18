<!-- SPDX-License-Identifier: FSFAP -->
# JDimへコントリビュートする

はじめに、ガイドを読む時間を取っていただきありがとうございます。 :+1:  
私達はJDimを開発・保守してくためにパッチを歓迎しています！ :revolving_hearts:

JDimへのコントリビュート方法を案内します。

* :beginner: まずはJDimproved/JDimをforkしてリポジトリを用意し、ローカルにcloneしてJDimをビルドしてみましょう。
  → [README.md][readme-md]
* :mag: [Issues][issues]や[Pull requests][pull-requests]を確認すると興味関心のあるトピックが見つかるかもしれません。
* :earth_asia: [Linux板@５ちゃんねる][linux-5ch]のJD/JDimスレで質問やバグ報告をすることもできます。
* :heart_decoration: JDimはボランティアによって開発・保守されています。
  そのため速やかに返信することが難しい場合がありますが予めご了承ください。


## :question: 質問をしたい
Discussionsを開いて質問をします。 → [New discussion][new-discussion]

使い方や設定方法の情報は[オンラインマニュアル][manual]や
2ch/5chのスレッド・過去ログにありますのでそちらも参照してください。


## :beetle: バグを報告したい
Issueを開いてバグを報告します。 → [Issue: 不具合(バグ)の報告][new-bug-report]

以下の情報は原因特定の手がかりになりますのでご報告くださいますようお願いいたします。

* 動作環境(クリップボードへのコピーを利用してください)
* やりたいこと・期待する動作
* バグが現れるまでの手順・操作
* 実際の結果・エラーメッセージ

バグなのかはっきりしない場合はDiscussionsで質問してみてください。 → [New discussion][new-discussion]


## :muscle: 機能のリクエストをしたい
Issueを開いて欲しい機能の要望を出しフィードバックを集めます。 → [Issue: 機能の変更][new-feature-request]  
可能ならコードを書き始めてください。実証用でもコードがあると具体的なフィードバックが集まりやすいです。


## :mailbox_with_mail: Pull requestを提出する

Pull requestは`master`ブランチに対してお願いいたします。

* 文書やソースコードのタイプミス修正、バグ修正、文書の改善、機能の改善などは直接PRを受け付けています。
* ユーザーインタフェースの変更や互換性に影響が出る修正は最初にissueを開いて意見・要望をお伝えいただければ幸いです。
* 影響が大きい変更は[RFCプロセス][rfcs]が必要になる場合があります。
* オンラインマニュアルの編集については [docs/README.md][docs-readme] を参照してください。
* テストケースを追加して動作をチェックすることもできます。詳細は [test/README.md][test-readme] を参照してください。

#### :pencil: C++ソースコードを修正するときの注意

* C++17の機能を使う。迷ったときは[C++ Core Guidelines][isocpp]を参考にする。
* CIのビルドで失敗する機能は使わなくてもビルドできるようにする。(下記参照)
* コーディングスタイルは周囲のコードになるべく合わせる。
* ソースコードを修正したときはビルド可能なことチェックする。
* 修正前よりコンパイル時警告を増やさないように気をつける。
* ファイルの作成、ファイル間で内容のコピーを行うとき[ライセンスに注意する点](#notice-about-license)があります。(下記参照)

C++17で追加された標準ライブラリのうちg++ 10またはclang++ 11が[サポート][support]していないものに注意

| JDimの動作環境に合わない標準ライブラリ | ヘッダー | gcc | clang |
| --- | --- | ---:| ---:|
| [Standardization of Parallelism TS][cpp17exe] | `<execution>` | 9 | n/a |
| [Hardware interference size][cpp17his]  | | 12 | 19 |
| [Polymorphic memory resources][cpp17pmr] | `<memory_resources>` | 9 | 16 |
| [Mathematical special functions][cpp17math] | | 7 | n/a |
| [Elementary string conversions][cpp17conv] (floating-point support) | `<charconv>` | 11 | n/a |
| DR17: [`std::hash<std::filesystem::path>`][cpp17fspathhash] | | 11.4  | 17 |

[readme-md]: https://github.com/JDimproved/JDim/tree/master/README.md
[issues]: https://github.com/JDimproved/JDim/issues
[pull-requests]: https://github.com/JDimproved/JDim/pulls
[linux-5ch]: https://mao.5ch.net/linux/
[new-discussion]: https://github.com/JDimproved/JDim/discussions/new
[new-bug-report]: https://github.com/JDimproved/JDim/issues/new?assignees=&labels=bug&template=bug-report.md&title=
[new-feature-request]: https://github.com/JDimproved/JDim/issues/new?assignees=&labels=feature&template=feature-request.md&title=
[manual]: https://jdimproved.github.io/JDim/
[rfcs]: https://github.com/JDimproved/rfcs
[docs-readme]: https://github.com/JDimproved/JDim/tree/master/docs/README.md
[test-readme]: https://github.com/JDimproved/JDim/tree/master/test/README.md
[isocpp]: https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines
[support]: https://en.cppreference.com/w/cpp/compiler_support/17
[cpp17exe]: https://en.cppreference.com/w/cpp/header/execution
[cpp17his]: https://en.cppreference.com/w/cpp/thread/hardware_destructive_interference_size
[cpp17pmr]: https://en.cppreference.com/w/cpp/header/memory_resource
[cpp17math]: https://en.cppreference.com/w/cpp/numeric/special_functions
[cpp17conv]: https://en.cppreference.com/w/cpp/header/charconv
[cpp17fspathhash]: https://en.cppreference.com/w/cpp/filesystem/path/hash

#### :chains: Unity buildの注意
ビルドツールMesonは複数のソースファイルを1つに結合してコンパイルする
[Unity build][meson-unity-build] という機能があります(デフォルトは無効)。
機能を有効に設定してソースファイルを結合するとマクロや変数などの名前が衝突してコンパイルに失敗することがあります。

ビルドの問題を見つけたときはバグ報告や修正のPull requestをいただけると幸いです。

* 名前が衝突したときは名称を変更したり入れ子の名前空間の中に入れるなどで衝突を回避できます。
  ```cpp
  // 入れ子の名前空間の例
  namespace Outer::priv { constexpr int kBufferSize = 1024; }

  using namespace Outer;
  use_value( priv::kBufferSize );
  ```
* `#define`マクロによる意図しない定義変更(上書き)に注意してください。
  コンパイラーがマクロの警告を出したときはマクロ以外の方法が使えないかチェックしてみてください。

##### Unity buildの使い方
setupサブコマンドで`-Dunity=on`を指定します。
ビルドオプション`-Dunity_size=N`で1つに結合するファイル数を変更できます。
```sh
meson setup builddir -Dunity=on -Dunity_size=10
ninja -C builddir
```

[meson-unity-build]: https://mesonbuild.com/Unity-builds.html


<a name="notice-about-license"></a>
#### ソースコードのファイルを新しく作成するときは
ファイルの冒頭に GPL-2.0-or-later を表示するSPDX形式のコメントを追加してください。
```cpp
// SPDX-License-Identifier: GPL-2.0-or-later
```

- <https://spdx.org/licenses/GPL-2.0-or-later.html>
- <https://spdx.github.io/spdx-spec/v2.3/using-SPDX-short-identifiers-in-source-files/>

##### ソースコード以外のファイルを新しく作成するときは
ソースコードと同様にファイルの冒頭にライセンスを表示するSPDXのコメントを追加してください。
コメントが書けないファイル形式のときはコミットメッセージにファイル名とライセンスを書いて指定できます
（未指定のときはGPL-2.0-or-later）。

JDimのプログラムに影響しないドキュメントやメタデータなどのファイルはGPLと互換性のある寛容なライセンスを使用しても問題ありません。


#### ファイルの内容を移動、コピーするときの取り扱い
GPL-2.0-or-later のファイルに GPL2 の内容をコピーしないよう注意してください。

コピー元 | コピー先 | 可否
--- | --- | ---
GPL2 | GPL-2.0-or-later | :x: ライセンスがGPL2に変わるためコピーは避ける &#8251;
GPL-2.0-or-later | GPL2 | :heavy_check_mark: コピーしてよい

&#8251; ライセンス変更に承諾した貢献者が書いた修正であり既存の改変ではなく新しく追加したコードブロックであるならコピーしてもよい

ファイルのライセンスついては [RFC 0013: ライセンス GPL-2.0-or-later を導入する][rfc0013] も参照してください。

[rfc0013]: https://github.com/JDimproved/rfcs/tree/master/docs/0013-introduce-license-gpl-2.0-or-later.md
