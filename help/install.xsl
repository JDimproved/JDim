<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xml:lang="ja">

<xsl:include href="main.xsl" />

<xsl:template match="/document">
    <p>サポートBBSの「ディストロ別インストール情報」スレに投稿された書き込みなどを転載しています。</p>
    <p>尚、ここに書かれている事は記載された時点での情報です。<br />
    JDまたはディストリビュータ側の変更により内容が異なる場合があります。</p>
    <ul class="disc">
        <xsl:apply-templates select="dist" mode="link" />
    </ul>
    <xsl:apply-templates />
</xsl:template>

<xsl:template match="dist" mode="link">
    <li>
        <a href="#{generate-id()}"><xsl:value-of select="@name" /></a>
    </li>
</xsl:template>

<xsl:template match="dist">
    <div class="border">
        <h3 class="blue"><a name="{generate-id()}"><xsl:value-of select="@name" /></a></h3>
        <xsl:apply-templates />
    </div>
</xsl:template>

<xsl:template match="other|res">
    <pre>
    <xsl:if test="name()='res'">
        <xsl:attribute name="class">res</xsl:attribute>
    </xsl:if>
    <xsl:value-of select="substring-after( ., '&#x0A;' )" />
    </pre>
</xsl:template>

</xsl:stylesheet>
