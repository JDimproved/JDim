# JDim テストガイド

JDimは [Google Test][google_test] を使って機能をテストします。
テストはmakeコマンドでビルド、実行が行えます。

makeのかわりに [Meson][mesonbuild] を利用することもできます。([下記参照](#meson-test))

## テストプログラムのビルドに必要なもの
- JDimソースコードの各namespaceで生成されるオブジェクトファイル(\*.o)や静的ライブラリ(\*.a)
- googletestのライブラリ、またはソースコード (セットアップを参照)


## セットアップ
(a)ディストリビューションのパッケージをインストールするか、
(b)ソースコード(googletestリポジトリ)を利用してください。

#### (a) ディストリビューションのパッケージを利用する場合
ディストロのパッケージ管理ツールを使ってgoogletestのパッケージをインストールします。

##### Debian系
1. `libgtest-dev`をインストールします。

   ```sh
   sudo apt install libgtest-dev
   ```

2. ./configureを実行してmakeコマンドでJDimをビルドします。

   ```sh
   autoreconf -i
   ./configure
   make
   ```

#### (b) ソースコード(googletestリポジトリ)を利用する場合
1. GitHubの"[google/googletest][google_test]"リポジトリからmasterブランチをクローンします。
   他のgoogletestリポジトリでも可能なはずです。

   ```sh
   git clone -b master --depth 1 https://github.com/google/googletest.git /path/to/googletest
   ```

2. configureスクリプトの引数 **GTEST_SRCDIR** にリポジトリのフルパスを指定してください。
   makeコマンドでJDimをビルドします。

   ```sh
   autoreconf -i
   ./configure GTEST_SRCDIR=/path/to/googletest
   make
   ```

#### セットアップの注意
互換性のためディストロのパッケージよりソースコードが優先されます。
パッケージを利用するときは **GTEST_SRCDIR** を設定しないでください。


## テストのビルドと実行

makeの **test** サブコマンドでテストコードのビルドと実行を行います。
結果表示など詳細はGoogle Testを解説しているwebページを参照してください。

```sh
make test
```

ライブラリとソースコードどちらも見つからないときは `make test` (及び `make check`)は失敗します。


## テストを追加する

テストコードのファイル名は gtest\_\*.cpp の形式にする必要があります。
ファイル名は小文字で `gtest_サブディレクトリ名_ソースファイル名.cpp` を推奨します。
テストの記述方法はテストコードを見るかwebを参照してください。

例
* `src/jdlib/miscutil.cpp` → `test/gtest_jdlib_miscutil.cpp`
* `src/core.cpp` → `test/gtest_core.cpp`

追加したテストコードのファイルを **test/Makefile.am** の `gtest_jdim_SOURCES` にリストします。
(行末のバックスラッシュ`\`に注意)


## 制限

以下は今のところサポートしておりません。
* JDim全体ではなくテスト対象のソースコードだけをビルドする
* `src/main.cpp` をテストする (ファイル名やエントリーポイントが衝突する)


[google_test]: https://github.com/google/googletest

---

<a name="meson-test"></a>
## Mesonを利用してテストする

Mesonのインストール方法は[GitHub][#556]を参照してください。

### セットアップ

[事前準備][readme-prepare]に加えて`googletest`をインストールします。
(a)ディストリビューションのパッケージをインストールするか、
(b)[meson wraptool][meson-wraptool]を利用してください。

#### (a) ディストリビューションのパッケージを利用する場合
ディストロのパッケージ管理ツールを使ってgoogletestのパッケージをインストールします。

##### Debian系
```sh
sudo apt install libgtest-dev
```

#### (b) Meson wraptool を利用する場合
[Wrap DB][wrapdb]を利用してローカルの`subprojects`ディレクトリにインストールします。
googletestのソースコードは構成時にダウンロードされます。
```sh
mkdir subprojects
meson wrap install gtest
```

### テストのビルドと実行
mesonの **test** サブコマンドでテストプログラムのビルドと実行を行います。
結果表示など詳細はMesonの[リファレンス][meson-reference]を参照してください。
```sh
meson builddir
meson test -C builddir
```

[mesonbuild]: https://mesonbuild.com/
[#556]: https://github.com/JDimproved/JDim/discussions/556
[readme-prepare]: https://github.com/JDimproved/JDim/blob/master/README.md#%E4%BA%8B%E5%89%8D%E6%BA%96%E5%82%99
[meson-wraptool]: https://mesonbuild.com/Using-wraptool.html
[wrapdb]: https://wrapdb.mesonbuild.com/gtest
[meson-reference]: https://mesonbuild.com/Unit-tests.html#other-test-options
