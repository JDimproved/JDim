# JDimへコントリビュートする

はじめに、ガイドを読む時間を取っていただきありがとうございます。:+1:  
私達はJDimを開発・保守してくためにパッチを歓迎しています！:revolving_hearts:

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

C++17で追加された標準ライブラリのうちg++ 8またはclang++ 7が[サポート][support]していないものに注意

| JDimの動作環境に合わない標準ライブラリ | ヘッダー | gcc | clang |
| --- | --- | ---:| ---:|
| [Standardization of Parallelism TS][cpp17exe] | `<execution>` | 9 | n/a |
| [Hardware interference size][cpp17his]  | | 12 | n/a |
| ( [File system library][cpp17fs] ) | `<filesystem>` | 8 | 7 |
| [Polymorphic memory resources][cpp17pmr] | `<memory_resources>` | 9 | 16 |
| [Mathematical special functions][cpp17math] | | 7 | n/a |
| Splicing [Maps][cpp17maps] and [Sets][cpp17sets] | | 7 | 8 |
| [Elementary string conversions][cpp17conv] (floating-point support) | `<charconv>` | 11 | n/a |
| `std::shared_ptr` and `std::weak_ptr` with array support | | 7 | 11 |
| DR: [`std::hash<std::filesystem::path>`][cpp17fspathhash] | | 11.4  | n/a |

* [gcc-8][gcc8fs] と [clang-7][clang7fs] の `<filesystem>` はライブラリが分かれている  
  使うときは別途リンクが必要なことをREADMEに注意書きすること

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
[clang7fs]: https://releases.llvm.org/7.1.0/projects/libcxx/docs/UsingLibcxx.html
[cpp17exe]: https://en.cppreference.com/w/cpp/header/execution
[cpp17his]: https://en.cppreference.com/w/cpp/thread/hardware_destructive_interference_size
[gcc8fs]: https://stackoverflow.com/questions/53201991/how-to-use-stdfilesystem-on-gcc-8
[cpp17fs]: https://en.cppreference.com/w/cpp/filesystem
[cpp17pmr]: https://en.cppreference.com/w/cpp/header/memory_resource
[cpp17math]: https://en.cppreference.com/w/cpp/numeric/special_functions
[cpp17maps]: https://en.cppreference.com/w/cpp/container/map/merge
[cpp17sets]: https://en.cppreference.com/w/cpp/container/set/merge
[cpp17conv]: https://en.cppreference.com/w/cpp/header/charconv
[cpp17fspathhash]: https://en.cppreference.com/w/cpp/filesystem/path/hash
