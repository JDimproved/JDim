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

<xsl:variable name="LASTYEAR" select="document('../xml/history.xml')//history/year[1]" />
<xsl:variable name="ISSUE">&#169;2006-<xsl:value-of select="$LASTYEAR" /></xsl:variable>


<xsl:template match="group">
  <xsl:if test="position() &gt; 1">
    <xsl:text>&#x0A;</xsl:text>
  </xsl:if>
  <xsl:text>&#x0A;* </xsl:text>
  <xsl:value-of select="@header" />
  <xsl:apply-templates select="node()" />
</xsl:template>


<xsl:template match="subgroup">
  <xsl:text>&#x0A;</xsl:text>
  <xsl:if test="@header">
    <xsl:text>&#x0A;  </xsl:text>
    <xsl:value-of select="@header" />
  </xsl:if>
  <xsl:apply-templates select="node()" />
</xsl:template>


<xsl:template match="issue">
  <xsl:value-of select="$ISSUE" />
  <xsl:text> </xsl:text>
</xsl:template>


<xsl:template match="sentence">
  <xsl:text>&#x0A;&#x0A;  </xsl:text>
  <xsl:apply-templates select="node()" />
</xsl:template>


<xsl:template match="list">
  <xsl:text>&#x0A;</xsl:text>
  <xsl:choose>
    <xsl:when test="@type='alpha'">
      <xsl:apply-templates select="item" mode="alpha" />
    </xsl:when>
    <xsl:when test="@type='circle'">
      <xsl:apply-templates select="item" mode="circle" />
    </xsl:when>
    <xsl:when test="@type='decimal'">
      <xsl:apply-templates select="item" mode="decimal" />
    </xsl:when>
    <xsl:otherwise>
      <xsl:apply-templates select="item" mode="none" />
    </xsl:otherwise>
  </xsl:choose>
</xsl:template>

<xsl:template match="item" mode="alpha">
  <xsl:text>&#x0A;    </xsl:text>
  <xsl:number format="a" />
  <xsl:text>. </xsl:text>
  <xsl:apply-templates select="node()" />
</xsl:template>

<xsl:template match="item" mode="circle">
  <xsl:text>&#x0A;  ・</xsl:text>
  <xsl:apply-templates select="node()" />
</xsl:template>

<xsl:template match="item" mode="decimal">
  <xsl:text>&#x0A;    </xsl:text>
  <xsl:number format="0" />
  <xsl:text>. </xsl:text>
  <xsl:apply-templates select="node()" />
</xsl:template>

<xsl:template match="item" mode="none">
  <xsl:text>&#x0A;    </xsl:text>
  <xsl:apply-templates select="node()" />
</xsl:template>


<xsl:template match="descriptions">
  <xsl:text>&#x0A;</xsl:text>
  <xsl:if test="@caption">
    <xsl:text>&#x0A;  </xsl:text>
    <xsl:value-of select="@caption" />
    <xsl:text>&#x0A;</xsl:text>
  </xsl:if>
  <xsl:apply-templates select="item" mode="descriptions" />
</xsl:template>

<xsl:template match="item" mode="descriptions">
  <xsl:if test="position() &gt; 1">
    <xsl:text>&#x0A;</xsl:text>
  </xsl:if>
  <xsl:text>&#x0A;    </xsl:text>
  <xsl:value-of select="@header" />
  <xsl:text>&#x0A;&#x0A;       </xsl:text>
  <xsl:apply-templates select="node()" />
</xsl:template>


<xsl:template match="shell">
  <xsl:text>&#x0A;&#x0A;</xsl:text>
  <xsl:choose>
    <xsl:when test="local-name(..)='item'">
      <xsl:text>       </xsl:text>
      <xsl:value-of select="text()" />
      <xsl:if test="position()=last()">
        <xsl:text>&#x0A;</xsl:text>
      </xsl:if>
    </xsl:when>
    <xsl:otherwise>
      <xsl:text>    </xsl:text>
      <xsl:value-of select="text()" />
    </xsl:otherwise>
  </xsl:choose>
</xsl:template>


<xsl:template match="text[@link]">
    <xsl:text>&quot;</xsl:text>
    <xsl:choose>
      <xsl:when test="starts-with( @link, 'http:' ) or starts-with( @link, 'https:' )">
        <xsl:value-of select="text()" />
        <xsl:text> [</xsl:text>
        <xsl:value-of select="@link" />
        <xsl:text>]</xsl:text>
      </xsl:when>
      <xsl:otherwise>
        <xsl:text>http://jd4linux.sourceforge.jp/manual/</xsl:text>
      </xsl:otherwise>
    </xsl:choose>
    <xsl:text>&quot;</xsl:text>
</xsl:template>


<xsl:template match="text()">
    <!-- SAXONとFirefox3で"replace()"の実装が異なる -->
    <xsl:choose>
      <xsl:when test="function-available( 'fn:replace' )">
        <xsl:value-of select="fn:replace( ., '^\s+|\n|\s{2,}|\s+$', '', 's' )" />
      </xsl:when>
      <xsl:otherwise><!--xsl:when test="function-available( 'regexp:replace' )"-->
        <xsl:value-of use-when="function-available( 'regexp:replace', 2 )"
                      select="regexp:replace( ., '^\s+|\n|\s{2,}|\s+$', 'g', '' )" />
      </xsl:otherwise>
    </xsl:choose>
</xsl:template>


</xsl:stylesheet>
