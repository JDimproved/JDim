<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xml:lang="ja">

<xsl:include href="main.xsl" />

<xsl:template match="/document">
    <p>サポートBBSの「ディストロ別インストール情報」スレに投稿された書き込みなどを転載しています。</p>
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
        <xsl:apply-templates select="other" />
        <xsl:apply-templates select="res" />
    </div>
</xsl:template>

<xsl:template match="other">
    <xsl:apply-templates />
</xsl:template>

<xsl:template match="res">
    <p><xsl:attribute name="class">
    <xsl:choose>
        <xsl:when test="contains( mail, 'sage' )">sage</xsl:when>
        <xsl:otherwise>age</xsl:otherwise>
    </xsl:choose>
    </xsl:attribute>
    <span class="number"><xsl:value-of select="number" /></span>
    <span class="underline">名前</span>:<span class="nanashi"><xsl:value-of select="name" /></span>
    <span class="mail"> [<xsl:value-of select="mail" />]</span>:<xsl:value-of select="date" />
    </p>

    <div class="basictext">
        <xsl:apply-templates name="basictext" select="basictext" />
    </div>
</xsl:template>

</xsl:stylesheet>
