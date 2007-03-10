<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xml:lang="ja">

<xsl:include href="main.xsl" />

<xsl:template match="/document">
    <ul class="disc">
        <xsl:apply-templates select="command" mode="link" />
    </ul>

    <ul class="decimal">
        <xsl:apply-templates />
    </ul>
</xsl:template>

<xsl:template match="command" mode="link">
    <li>
        <a href="#{generate-id()}"><xsl:value-of select="@header" /></a>
    </li>
</xsl:template>

<xsl:template match="command">
    <li>
    <h3>
        <a name="{generate-id()}"><xsl:value-of select="@header" /></a>
    </h3>
    
    <p><xsl:apply-templates /></p>
    </li>
</xsl:template>

<xsl:template match="name">
    <span class="red"><xsl:value-of select="." /></span><br />
</xsl:template>

<xsl:template match="exec">
    <span class="green"><xsl:value-of select="." /></span>
</xsl:template>

</xsl:stylesheet>
