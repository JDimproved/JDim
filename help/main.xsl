<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xml:lang="ja">


<!-- 基本 -->
<xsl:template match="/">
    <html lang="ja">
    <head>
    <title><xsl:value-of select="document/@title" /></title>
    <link rel="stylesheet" type="text/css" href="main.css" />
    </head>
    <body>
    <p><a href="./help.xml">トップに戻る</a></p>
    <h1 id="title">
    <img src="jd.png" width="96" height="96" alt="JDロゴ" />
    <span id="text"><xsl:value-of select="document/@title" /></span>
    </h1>
    <p id="modified">更新:<xsl:value-of select="document/@modified" /></p>

    <xsl:apply-templates select="document" />
    </body>
    </html>
</xsl:template>


<!-- HTML -->
<xsl:template match="document">
    <xsl:apply-templates select="a|br|div|h1|h2|h3|hr|li|p|pre|span|table|td|th|tr|ul" />
</xsl:template>

<xsl:template match="a|br|div|h1|h2|h3|hr|li|p|pre|span|table|td|th|tr|ul">
    <xsl:copy>
        <xsl:for-each select="@*"><xsl:copy /></xsl:for-each>
        <xsl:apply-templates />
    </xsl:copy>
</xsl:template>


<!-- テキスト -->
<xsl:template match="basictext">
    <div>
        <xsl:for-each select="@*"><xsl:copy /></xsl:for-each>
        <xsl:apply-templates name="basictext" />
    </div>
</xsl:template>

<xsl:template match="text()">
    <xsl:call-template name="basictext">
        <xsl:with-param name="line">
            <xsl:value-of select="." />
        </xsl:with-param>
    </xsl:call-template>
</xsl:template>

<xsl:template name="basictext">
    <xsl:param name="line" />
    <xsl:choose>
        <xsl:when test="contains( $line, '&#x0A;' )">
            <xsl:variable name="before">
                <xsl:value-of select="substring-before( $line, '&#x0A;' )" />
            </xsl:variable>
            <xsl:value-of select="$before" />
            <xsl:if test="string-length( $before )!=0" ><br /></xsl:if>
            <xsl:call-template name="basictext">
                <xsl:with-param name="line"><xsl:value-of select="substring-after( $line, '&#x0A;' )" /></xsl:with-param>
            </xsl:call-template>
        </xsl:when>
        <xsl:otherwise>
            <xsl:value-of select="$line" />
        </xsl:otherwise>
    </xsl:choose>
</xsl:template>

</xsl:stylesheet>
