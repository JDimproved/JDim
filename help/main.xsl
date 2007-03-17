<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xml:lang="ja">


<!-- 基本 -->
<xsl:template match="/">
    <html lang="ja">
    <head>
    <title><xsl:value-of select="document/@header" /></title>
    <link rel="stylesheet" type="text/css" href="main.css" />
    </head>
    <body>
    
    <p><a href="./help.xml">トップに戻る</a></p>

    <h1 id="header">
    <img src="jd.png" width="96" height="96" alt="JDロゴ" />
    <span id="text"><xsl:value-of select="document/@header" /></span>
    </h1>

    <p id="version">
    Version:
    <xsl:choose>
    <xsl:when test="document('history.xml')//prerelease">
        <xsl:value-of select="document('history.xml')//prerelease/@version" />
    </xsl:when>
    <xsl:otherwise>
        <xsl:value-of select="document('history.xml')//release[last()]/@version" />
    </xsl:otherwise>
    </xsl:choose>
    </p>

    <xsl:apply-templates select="document" />
    </body>
    </html>
</xsl:template>


<!-- HTMLなどの要素 -->
<xsl:template match="document">
    <xsl:if test="block/@header">
        <ul class="disc">
            <xsl:apply-templates select="block" mode="link" />
        </ul>
    </xsl:if>

    <xsl:apply-templates select="a|array|block|br|div|h1|h2|h3|hr|li|p|pre|q|span|table|td|th|tr|ul" />
</xsl:template>

<xsl:template match="a|array|block|br|div|h1|h2|h3|hr|li|p|pre|q|span|table|td|th|tr|ul">
    <xsl:copy>
        <xsl:for-each select="@*"><xsl:copy /></xsl:for-each>
        <xsl:apply-templates />
    </xsl:copy>
</xsl:template>


<!-- ブロック -->
<xsl:template match="block" mode="link">
    <li>
        <a href="#{generate-id()}"><xsl:value-of select="@header" /></a>
    </li>
</xsl:template>

<xsl:template match="block">
    <div>
    <xsl:copy-of select="@*" />
    <xsl:if test="@header">
        <h3 class="green"><a name="{generate-id()}"><xsl:value-of select="@header" /></a></h3>
    </xsl:if>
    <xsl:apply-templates />
    </div>
</xsl:template>


<!-- テーブル -->
<xsl:template match="array">
    <xsl:if test="@header">
        <h3 class="green"><xsl:value-of select="@header" /></h3>
    </xsl:if>
    <table><xsl:apply-templates select="item" /></table>
</xsl:template>

<xsl:template match="item">
    <tr>
    <td>
        <xsl:copy-of select="key/@*" />
        <xsl:value-of select="key" />
    </td>
    <td>
        <xsl:copy-of select="value/@*" />
        <xsl:value-of select="value" />
    </td>
    </tr>
</xsl:template>

</xsl:stylesheet>
