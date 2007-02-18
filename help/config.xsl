<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xml:lang="ja">

<xsl:include href="main.xsl" />

<xsl:template match="/document">
    <p>キャッシュディレクトリは環境変数 JD_CACHE で指定する。例えば</p>

    <p class="example">JD_CACHE=~/.jdcache ./jd</p>

    <p>の様にする。指定が無ければ ~/.jd を使用する。</p>

    <p>
    設定メニューにない設定項目は、設定ファイル(~/.jd/jd.conf)を直接編集して設定する。<br />
    ある設定をデフォルト値に戻したい場合は設定ファイルの対象列を削除してからJDを起動する。<br />
    デフォルト値は src/config/globalconf.cpp の CONFIG::init_config()　関数に書いてある。
    </p>

    <p>(注意) </p>

    <p>
    以下の設定はJD終了時に上書きされるのでJDを終了してから書き換えること。<br />
    設定が無い場合は一度JDを起動してから終了させると現れる。
    </p>

    <xsl:apply-templates select="config" />

    <p>
    キー割り当てを変えるにはキャッシュフォルダにある key.conf を編集する。<br />
    マウスボタン割り当てを変えるにはキャッシュフォルダにある button.conf を編集する。<br />
    割り当てをデフォルトに戻したいときは各設定ファイルを削除してからJDを起動する。
    </p>

    <p>
    マウスジェスチャの設定を変えるにはキャッシュフォルダにある mouse.conf を編集する。<br />
    最大ストロークは5で、判定開始半径は設定ファイル(~/.jd/jd.conf)の mouse_radius の値を変える
    </p>

<pre>     ８
     ↑
４ ←  → ６
     ↓
     ２
</pre>
    <p>( 例 ) ↑→↓← = 8624</p>
</xsl:template>

<xsl:template match="config">
    <h3 class="green"><xsl:value-of select="@name" /></h3>
    <table>
        <xsl:attribute name="summary"><xsl:value-of select="@name" /></xsl:attribute>
        <xsl:apply-templates select="key" />
    </table>
</xsl:template>

<xsl:template match="key">
    <tr>
    <td class="blue">
        <xsl:value-of select="name" />
    </td>
    <td>
        <xsl:value-of select="description" />
    </td>
    </tr>
</xsl:template>

</xsl:stylesheet>
