<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xml:lang="ja">

<xsl:include href="main.xsl" />

<xsl:template match="/document">
    <ul class="disc">
        <xsl:apply-templates select="h3" mode="link" />
    </ul>
    <xsl:apply-templates />
</xsl:template>

<xsl:template match="h3" mode="link">
    <li>
        <a href="#{generate-id()}"><xsl:value-of select="." /></a>
    </li>
</xsl:template>

<xsl:template match="h3">
    <h3 class="green">
        <a name="{generate-id()}"><xsl:value-of select="." /></a>
    </h3>
</xsl:template>


</xsl:stylesheet>
