
# 導入方法

## 事前準備

　一度だけやればいい。

### Redhad系
`  dnf install gtkmm24-devel gnutls-devel libSM-devel libtool automake git `

### Debin系
`  sudo apt-get install libgtkmm-2.4-dev libgnutls28-dev libsm-dev libtool automake git `

## インストール

```
    git clone -b test --depth 1 https://github.com/yama-natuki/JD.git jd  
    cd jd  
    ./autogen.sh  
    ./configure  
    make
```

　src/にjdが出来上がり。 src/jd をそのまま実行するもよし、 /usr/bin/ にコピーするもよし。


# 参考
　詳しいインストールの方法は [本家のwiki](https://ja.osdn.net/projects/jd4linux/wiki/OS%2f%E3%83%87%E3%82%A3%E3%82%B9%E3%83%88%E3%83%AA%E3%83%93%E3%83%A5%E3%83%BC%E3%82%B7%E3%83%A7%E3%83%B3%E5%88%A5%E3%82%A4%E3%83%B3%E3%82%B9%E3%83%88%E3%83%BC%E3%83%AB%E6%96%B9%E6%B3%95) を参照。


