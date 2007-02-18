<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xml:lang="ja">

<xsl:include href="main.xsl" />

<xsl:template match="/document">
    <ul class="disc">
        <xsl:apply-templates select="command" mode="link" />
    </ul>

    <ul class="decimal">
        <xsl:apply-templates select="command" />
    </ul>
</xsl:template>

<xsl:template match="command" mode="link">
    <li>
        <a href="#{generate-id()}"><xsl:value-of select="@title" /></a>
    </li>
</xsl:template>

<xsl:template match="command">
    <li>
    <h3>
        <a name="{generate-id()}"><xsl:value-of select="@title" /></a>
    </h3>
    <xsl:apply-templates select="before" />
    <p>
    <span class="red"><xsl:value-of select="name" /></span><br />
    <span class="green"><xsl:value-of select="exec" /></span>
    </p>
    <xsl:apply-templates select="after" />
    </li>
</xsl:template>

<xsl:template match="before">
    <xsl:apply-templates />
</xsl:template>

<xsl:template match="after">
    <xsl:apply-templates />
</xsl:template>

</xsl:stylesheet>
