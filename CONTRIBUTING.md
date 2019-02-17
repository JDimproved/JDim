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
Issueを開いて質問をします。 → [New issue][new-issue]

使い方や設定方法の情報は[オンラインマニュアル (JD)][manual]や
2ch/5chのスレッド・過去ログにありますのでそちらも参照してください。


## :beetle: バグを報告したい
Issueを開いてバグを報告します。 → [New issue][new-issue]

以下の情報は原因特定の手がかりになりますのでご報告くださいますようお願いいたします。

* 動作環境(クリップボードへのコピーを利用してください)
* やりたいこと・期待する動作
* バグが現れるまでの手順・操作
* 実際の結果・エラーメッセージ


## :muscle: 機能のリクエストをしたい
Issueを開いて欲しい機能の要望を出しフィードバックを集めます。 → [New issue][new-issue]  
可能ならコードを書き始めてください。実証用でもコードがあると具体的なフィードバックが集まりやすいです。


## :mailbox_with_mail: Pull requestを提出する

Pull requestは`master`ブランチに対してお願いいたします。

* 文書やソースコードのタイプミス修正、バグ修正、文書の改善、機能の改善などは直接PRを受け付けています。
* ユーザーインタフェースの変更や互換性に影響が出る修正は最初にissueを開いて意見・要望をお伝えいただければ幸いです。

#### :pencil: C++ソースコードを修正するときの注意

* C++11の機能を使う。迷ったときは[C++ Core Guidelines][isocpp]を参考にする。
* コーディングスタイルは周囲のコードになるべく合わせる。
* ソースコードを修正したときはビルド可能なことチェックする。
* 修正前よりコンパイル時警告を増やさないように気をつける。


[readme-md]: https://github.com/JDimproved/JDim/tree/master/README.md
[issues]: https://github.com/JDimproved/JDim/issues
[pull-requests]: https://github.com/JDimproved/JDim/pulls
[linux-5ch]: https://mao.5ch.net/linux/
[new-issue]: https://github.com/JDimproved/JDim/issues/new
[manual]: https://jd4linux.osdn.jp/manual/289/
[isocpp]: https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines
