<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet
    version="2.0"
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
    xmlns:fn="http://www.w3.org/2005/xpath-functions"
    xmlns:regexp="http://exslt.org/regular-expressions"
>
<xsl:output
    method="text"
    encoding="UTF-8"
    media-type="text/plain"
/>
<xsl:include href="main.xsl" />

<xsl:template match="/document">
    <xsl:text>[ </xsl:text><xsl:value-of select="@header" /><xsl:text> ]</xsl:text>
    <xsl:text>&#x0A;&#x0A;</xsl:text>

    <xsl:apply-templates select="*" />
</xsl:template>


</xsl:stylesheet>
