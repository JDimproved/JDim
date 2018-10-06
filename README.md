
# 導入方法

## 事前準備

　一度だけやればいい。

### Redhat系
`  dnf install gtkmm24-devel gnutls-devel libSM-devel libtool automake git `

### Debin系
`   sudo apt-get build-dep jd`

　開発環境が入っていない場合は、

`  sudo apt-get install build-essential automake autoconf-archive git`

### Ubuntu 18.04
　開発環境が入っていない場合は、

`    sudo apt-get install build-essential automake autoconf-archive git libtool`

　必要なライブラリを入れる。(抜けがあるかも)

`    sudo apt-get install libgtkmm-2.4-dev libmigemo1 libasound2-data libltdl-dev libasound2-dev libgnutls28-dev libgcrypt20-dev`

## インストール

```
    git clone -b test --depth 1 https://github.com/yama-natuki/JD.git jd  
    cd jd  
    autoreconf -i  
    ./configure  
    make
```

　実行するには直接 src/jd を起動するか手動で /usr/bin あたりに src/jd を cp する。


# 参考
　詳しいインストールの方法は [本家のwiki](https://ja.osdn.net/projects/jd4linux/wiki/OS%2f%E3%83%87%E3%82%A3%E3%82%B9%E3%83%88%E3%83%AA%E3%83%93%E3%83%A5%E3%83%BC%E3%82%B7%E3%83%A7%E3%83%B3%E5%88%A5%E3%82%A4%E3%83%B3%E3%82%B9%E3%83%88%E3%83%BC%E3%83%AB%E6%96%B9%E6%B3%95) を参照。


## Tips

### ●buildの高速化
　make するときに -j job数(並列処理の数) を指定すれば高速にコンパイルできます。  
使用するCPUのコア数と相談して決めてください。

###  ●to_string’ is not a member of ‘std’なエラーが出た場合
　このエラーが出た場合は、 automake のマクロが入っていないか、
gcc のバージョンが古い可能性があります。

automakeのマクロはubuntuでは autoconf-archive というパッケージ名です。

またマクロを入れなくても、

`   make CXXFLAGS+="-std=c++11" `

でも同様の効果があります。

　もしこれで駄目な場合はgccのversionが古すぎるので、
gccのバージョンアップをするか、ディストリをバージョンアップしてください。

### ●configureチェック中に `AX_CXX_COMPILE_STDCXX_11(noext, mandatory)` に関連したエラーがでた場合
ubuntuでは `autoconf-archive` をインストールして `autoreconf -i` からやり直してみてください。
パッケージが見つからないまたはエラーが消えない場合は以下の手順を試してみてください。

1. `configure.ac` の `AX_CXX_COMPILE_STDCXX_11([noext], [mandatory])` の行を削除する。
2. `autoreconf -i` で `configure` を作りconfigureチェックをやり直す。
3. `make CXXFLAGS+="-std=c++11"` でビルドする。
