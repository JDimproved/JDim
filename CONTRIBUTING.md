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

* C++20の機能を使う。迷ったときは[C++ Core Guidelines][isocpp]を参考にする。
* CIのビルドで失敗する機能は使わなくてもビルドできるようにする。(下記参照)
* コーディングスタイルは周囲のコードになるべく合わせる。
* ソースコードを修正したときはビルド可能なことチェックする。
* 修正前よりコンパイル時警告を増やさないように気をつける。
* ファイルの作成、ファイル間で内容のコピーを行うとき[ライセンスに注意する点](#notice-about-license)があります。(下記参照)

C++17で追加された標準ライブラリのうちg++ 11またはclang++ 14が[サポート][support]していないものに注意

| JDimの動作環境に合わない C++17 標準ライブラリ | ヘッダー | gcc | clang |
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

C++20で追加されたコア言語機能や標準ライブラリのうちg++ 11またはclang++ 14が[サポート][support20]していないものに注意

[support20]: https://en.cppreference.com/w/cpp/compiler_support/20

**JDimの動作環境では使用できない主な C++20 コア言語機能**

Feature (Core) | Standard | Paper(s) | GCC | Clang
--- | --- | --- | --- | ---
Modules | C++20 | P1103 etc. | 16* | 21*
Immediate functions (consteval) | C++20 |  P1073 | 11* | 17*
Coroutines | C++20 |  P0912 etc. | 10* | 17*

<details>
<summary>完全な禁止リストはこちら</summary>

この禁止リストは GCC ≥11 / Clang ≥14 で安定して利用できない、または実装が不完全な C++20 機能を対象としています。
表はより高いコンパイラバージョンを要求する機能ほど上位に来るよう並べています。

Feature (Core) | Standard | Paper(s) | GCC | Clang
--- | --- | --- | --- | ---
Simplifying implicit lambda capture  |  C++20  |  P0588  | 8 | —
Modules  |  C++20  |  P1103 P1703 P1766 P1779 P1811 P1815 P1857 P1874 P1979 P2115 P2615 P2788  | 16* | 21*
Inconsistencies with constant template parameters  |  C++20  |   P1907  | 11* | 18*
Class template argument deduction for alias templates  |  C++20  |   P1814  | 10 | 19*
Converting from T* to bool should be considered narrowing  |  C++20  |   P1957  | 11* | 18
Immediate functions (consteval)  |  C++20  |   P1073  | 11* | 17*
Coroutines  |  C++20  |   P0912 LWG3393  | 10* | 17*
Class template argument deduction for aggregates  |  C++20  |   P1816 P2082  | 11 | 17
Wording for lambdas in unevaluated contexts  |  C++20  |   P0315  | 9 | 17*
Structured binding extensions  |  C++20  |   P1091 P1381  | 10 | 16*
Fixing functionality gaps in constraints  |  C++20  |   P0857  | 10 | 16
Conditionally trivial special member functions  |  C++20  |   P0848  | 10 | 16
Parenthesized initialization of aggregates  |  C++20  |   P0960 P1975  | 10 | 16
Down with typename!   |  C++20  |   P0634  | 9 | 16
\_\_VA\_OPT\_\_  |  C++20  |  P0306 P1042  | 12* | 9

</details>

**JDimの動作環境では使用できない主な C++20 標準ライブラリ**

Feature (Library) | Standard | Paper(s) | GCC | Clang
--- | --- | --- | --- | ---
Extending <chrono> to calendars and time zones | C++20 | P0355 | 14* | 19*
std::format() | C++20 | P0645 | 13 | 17
constexpr std::string | C++20 | P0980 etc. | 12 | 15
constexpr std::vector | C++20 | P1004 | 12 | 15
The One Ranges Proposal (std::ranges) | C++20 |  P0896 | 10 | 15*

<details>
<summary>完全な禁止リストはこちら</summary>

この禁止リストは GCC ≥11 / Clang ≥14 で安定して利用できない、または実装が不完全な C++20 機能を対象としています。
表はより高いコンパイラバージョンを要求する機能ほど上位に来るよう並べています。

Feature (Library) | Standard | Paper(s) | GCC | Clang
--- | --- | --- | --- | ---
Atomic compare-and-exchange with padding bits  |  C++20  |  P0528  | 13 | —
Atomic std::shared_ptr and std::weak_ptr  |  C++20  |  P0718  | 12 | —
Layout-compatibility and pointer-interconvertibility traits  |  C++20  |  P0466  | 12 | —
Standard library header units  |  C++20  |  P1502  | 11 | —
Extending <chrono> to calendars and time zones  |  C++20  |  P0355  | 14* | 19*
Integration of chrono with text formatting  |  C++20  |  P1361  | 13 | 21
Reviewing deprecated facilities of C++17 for C++20  |  C++20  |  P0619  | 12 | 20*
std::stop_token and std::jthread  |  C++20  |   P0660  | 10 | 20*
std::format()  |  C++20  |  P0645  | 13 | 17
std::atomic_ref::wait(), std::atomic_ref::notify_one() and std::atomic_ref::notify_all()  |  C++20  |   P1643  | 11 | 19
Output chrono::days with 'd' suffix  |  C++20  |  P1650  | 13 | 16
C++ Synchronized Buffered Ostream (std::basic_osyncstream)  |  C++20  |   P0053  | 11 | 18
std::atomic_ref  |  C++20  |   P0019  | 10 | 19
The Mothership has Landed: Adding <=> to the Library  |  C++20  |   P1614  | 10 | 19
Smart pointer creation with default initialization (e.g. std::make\_unique\_for\_overwrite())  |  C++20  |  P1020 P1973  | 12 | 16
Floating Point Atomic  |  C++20  |   P0020  | 10 | 18
Efficient access to std::basic_stringbuf's buffer  |  C++20  |   P0408  | 11 | 17
Library support for operator<=> (<compare>)  |  C++20  |   P0768  | 10 | 17*
Extending std::make\_shared() to support arrays  |  C++20  |  P0674  | 12 | 15
constexpr std::string  |  C++20  |  P0426 P1032 P0980  | 12 | 15
constexpr std::vector  |  C++20  |  P1004  | 12 | 15
std::source_location  |  C++20  |   P1208  | 11 | 16
ranges::basic_istream_view::iterator should not be copyable  |  C++20  |   P1638  | 11 | 16
Target Vectorization Policies from Parallelism V2 TS to C++20 (std::execution::unseq)  |  C++20  |   P1001  | 9 | 17*
std::assume_aligned()  |  C++20  |   P1007  | 11 | 15
Input range adaptors  |  C++20  |   P1035  | 10 | 16
Ranges adaptors for non-copyable iterators  |  C++20  |   P1862  | 10 | 16
ranges::elements_view needs its own sentinel  |  C++20  |   P1994  | 10 | 16
constexpr for std::complex  |  C++20  |   P0415  | 9 | 16*
[[nodiscard]] in the library  |  C++20  |   P0600  | 9 | 16*
Library support for char8_t  |  C++20  |   P0482  | 9 | 16*
The One Ranges Proposal  |  C++20  |   P0896  | 10 | 15*
Utility functions to implement uses-allocator construction  |  C++20  |   P0591  | 9 | 16
Make stateful allocator propagation more consistent for operator+(basic_string)  |  C++20  |   P1165  | 10 | 15
pmr::polymorphic_allocator<> as a vocabulary type  |  C++20  |   P0339  | 9 | 16
char8_t backward compatibility remediation  |  C++20  |   P1423  | 10 | 15

</details>

コンパイラーのC++20コア言語機能と標準ライブラリのサポート状況は <https://cppstat.dev/> を参考にしました。

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
