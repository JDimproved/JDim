<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xml:lang="ja">

<xsl:include href="main.xsl" />

<xsl:template match="/document">
    <xsl:apply-templates select="release">
    <xsl:sort select="@date" order="ascending" />
    </xsl:apply-templates>
</xsl:template>

<xsl:template match="release">
    <p><xsl:attribute name="class">
    <xsl:choose>
        <xsl:when test="position()=last()">age</xsl:when>
        <xsl:otherwise>sage</xsl:otherwise>
    </xsl:choose>
    </xsl:attribute>
    <span class="number"><xsl:number format="1" /></span>
    <span class="underline">バージョン</span>:<span class="nanashi"><xsl:value-of select="@version" /></span>
    <span><xsl:attribute name="class">
    <xsl:choose>
        <xsl:when test="position()=last()">red</xsl:when>
        <xsl:otherwise></xsl:otherwise>
    </xsl:choose>
    </xsl:attribute>
    [<xsl:choose>
        <xsl:when test="position()=last()"><a name="new">new</a></xsl:when>
        <xsl:otherwise>sage</xsl:otherwise>
    </xsl:choose>]</span>:<xsl:value-of select="@date" /> 00:00:00
    </p>

    <div class="basictext">
        <xsl:apply-templates name="basictext" />
    </div>
</xsl:template>

</xsl:stylesheet>
