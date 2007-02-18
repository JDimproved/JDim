<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xml:lang="ja">

<xsl:include href="main.xsl" />

<xsl:template match="/document">
    <ul class="disc">
        <xsl:apply-templates select="qa" mode="link" />
    </ul>
    <xsl:apply-templates />
</xsl:template>

<xsl:template match="qa" mode="link">
    <li>
        <a href="#{generate-id()}"><xsl:value-of select="question" /></a>
    </li>
</xsl:template>

<xsl:template match="qa">
    <div class="qa">
    <p>
    <span class="red">Q. </span>
    <a name="{generate-id()}"><xsl:value-of select="question" /></a>
    </p>
    <p><span class="blue">A. </span><xsl:value-of select="answer" /></p>
    <div class="ofset"><xsl:apply-templates select="other" /></div>
    </div>
</xsl:template>

</xsl:stylesheet>
