<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xml:lang="ja">

<xsl:include href="main.xsl" />

<xsl:template match="/document">
    <p>スレ一覧の右クリックメニューに自前のコマンドを登録できる。</p>
    <p>JDを一度起動するとキャッシュディレクトリにできる usrcmd.txt にコマンドを登録する。</p>
    <p>usrcmd.txtの形式は</p>
    <p><span class="red">コマンド名</span><br /><span class="green">実行するコマンド</span></p>
    <p>であり、実行するコマンドでは以下のような置換文字列を指定することが出来る。</p>

    <table id="usrcmd" summary="ユーザーコマンド">
    <xsl:apply-templates select="command" />
    </table>

    <p>なお空白行と # から始まるコメント行は無視される。ユーザコマンドの具体的な使いかたは下の例や <a href="usrcmd_collection.xml">ユーザーコマンド設定集</a>を参照すること。</p>

    <p>(usrcmd.txtの例) </p>
    <p class="nowrap"><span class="red">googleで検索</span><br /><span class="green">$VIEW http://www.google.co.jp/search?hl=ja&amp;q=$TEXTU&amp;btnG=Google+%E6%A4%9C%E7%B4%A2&amp;lr=</span></p>
    <p class="nowrap"><span class="red">infoseek国語辞典</span><br /><span class="green">$VIEW http://jiten.www.infoseek.co.jp/Kokugo?qt=$TEXTE&amp;sm=1&amp;pg=result_k.html&amp;col=KO</span></p>

</xsl:template>

<xsl:template match="command">
    <tr>
    <td class="green">
        <xsl:choose>
        <xsl:when test="cmd!=''"><xsl:value-of select="cmd" /></xsl:when>
        <xsl:otherwise>&#160;</xsl:otherwise>
        </xsl:choose>
    </td>
    <td>
        <xsl:choose>
        <xsl:when test="description!=''"><xsl:value-of select="description" /></xsl:when>
        <xsl:otherwise>&#160;</xsl:otherwise>
        </xsl:choose>
    </td>
    </tr>
</xsl:template>

</xsl:stylesheet>
