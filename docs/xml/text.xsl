<?xml version="1.0"?>
<!DOCTYPE stylesheet [
<!ENTITY space "<xsl:text> </xsl:text>">
<!ENTITY cr "<xsl:text>
</xsl:text>">
<!ENTITY tab "<xsl:text>	</xsl:text>">
]>
<xsl:stylesheet version="1.0"
	xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
	xmlns="http://www.w3.org/1999/xhtml">


<!-- FIX ME - <br> is translated but then immediatly filtered out again.
 |   Furthermore, indention of list entries etc. is not done at all. We should
 |   fix this somehow, but how?
 +-->


<xsl:strip-space elements="*"/>
<xsl:output
	method="text"
	indent="no"
	encoding="iso-8859-1"/>

<!-- Keys -->
<xsl:key name="sub_ref" match="sub" use="@name"/>
<xsl:key name="section_ref" match="section" use="@title"/>



<!-- Faq Templates -->
<xsl:template name="TOC">
	<xsl:for-each select="section">
		<xsl:number level="multiple"
					count="section"
					format="1. "/>
		<xsl:value-of select="@title"/><xsl:text>&#xA;</xsl:text>
		<xsl:for-each select="sub">
			<xsl:number level="multiple"
						count="section|sub"
						format="1."
						value="count(ancestor::section/preceding-sibling::section)+1"/>
			<xsl:number format="1. "/>
			<xsl:apply-templates select="header"/><xsl:text>&#xA;</xsl:text>
		</xsl:for-each>
		<xsl:text>&#xA;</xsl:text>
	</xsl:for-each>
</xsl:template>

<xsl:template match="faqs">
	<xsl:value-of select="@title"/>
	<xsl:text> F.A.Q. (frequently asked questions)&#xA;</xsl:text>
	<xsl:text>last changed: </xsl:text>
	<xsl:value-of select="@changed"/>
	<xsl:text>&#xA;&#xA;</xsl:text>
	<xsl:text>The latest version of this document can be found at http://pentagram.sourceforge.net/faq.php&#xA;</xsl:text>
	<xsl:text>&#xA;&#xA;</xsl:text>

	<!-- BEGIN TOC -->
	<xsl:call-template name="TOC"/>
	<!-- END TOC -->

	<!-- BEGIN CONTENT -->
	<xsl:apply-templates select="section"/>
	<!-- END CONTENT -->
</xsl:template>

<!-- Readme Templates -->
<xsl:template match="readme">
	<xsl:value-of select="@title"/>
	<xsl:text> Documentation&#xA;</xsl:text>
	<xsl:text>last changed: </xsl:text>
	<xsl:value-of select="@changed"/>
	<xsl:text>&#xA;&#xA;</xsl:text>
	<xsl:text>The latest version of this document can be found at http://pentagram.sourceforge.net/docs.php&#xA;</xsl:text>
	<xsl:text>&#xA;&#xA;</xsl:text>

	<!-- BEGIN TOC -->
	<xsl:call-template name="TOC"/>
	<!-- END TOC -->

	<!-- BEGIN CONTENT -->
	<xsl:apply-templates select="section"/>
	<!-- END CONTENT -->
</xsl:template>


<!-- section Template -->
<xsl:template match="section">
	<xsl:text>&#xA;</xsl:text>
	<xsl:text>--------------------------------------------------------------------------------&#xA;</xsl:text>
	<xsl:text>&#xA;</xsl:text>
	<xsl:number format="1. "/>
	<xsl:value-of select="@title"/>
	<xsl:text>&#xA;</xsl:text>
	<xsl:apply-templates select="sub"/>
</xsl:template>


<!-- Entry Template -->
<xsl:template match="sub">
	<xsl:variable name = "num_idx">
		<xsl:number level="multiple"
					count="section|sub"
					format="1."
					value="count(ancestor::section/preceding-sibling::section)+1"/>
		<xsl:number format="1. "/>
	</xsl:variable>
	<xsl:value-of select="$num_idx"/><xsl:apply-templates select="header"/>
	<xsl:text>&#xA;</xsl:text>
	<xsl:apply-templates select="body"/>
	<xsl:text>&#xA;&#xA;</xsl:text>
</xsl:template>


<xsl:template match="header">
	<!-- In order to do proper formatting, we have to apply a little trick -
	 |   we first store the result tree fragment (RTF) in a variable, then
	 |   we can apply normalize-space to this variable. Nifty, hu? ;)
	 +-->
	<xsl:variable name = "spaced_text">
		<xsl:apply-templates/>
	</xsl:variable>
	<xsl:value-of select="normalize-space($spaced_text)"/>
</xsl:template>


<xsl:template match="body">
	<xsl:apply-templates/>
</xsl:template>


<!--=========================-->
<!-- Internal Link Templates -->
<!--=========================-->
<xsl:template match="ref">
	<xsl:value-of select="count(key('sub_ref',@target)/parent::section/preceding-sibling::section)+1"/>
	<xsl:text>.</xsl:text>
	<xsl:value-of select="count(key('sub_ref',@target)/preceding-sibling::sub)+1"/>
	<xsl:text>.</xsl:text>
</xsl:template>

<xsl:template match="ref1">
	<xsl:text>&#xA;</xsl:text>
	<xsl:value-of select="count(key('sub_ref',@target)/parent::section/preceding-sibling::section)+1"/>
	<xsl:text>.</xsl:text>
	<xsl:value-of select="count(key('sub_ref',@target)/preceding-sibling::sub)+1"/>
	<xsl:text>. </xsl:text>
	<xsl:apply-templates select="key('sub_ref',@target)/child::header"/>
</xsl:template>


<xsl:template match="section_ref">
	<xsl:text>&#xA;</xsl:text>
	<xsl:value-of select="count(key('section_ref',@target)/preceding-sibling::section)+1"/>
	<xsl:text>. </xsl:text>
	<xsl:apply-templates select="key('section_ref',@target)/@title"/>
</xsl:template>

<!-- External Link Template -->
<xsl:template match="extref">
	<xsl:choose>
		<xsl:when test="count(child::node())>0">
				<xsl:value-of select="."/>
		</xsl:when>
		<xsl:when test="@doc='faq'">
			<xsl:text>FAQ.txt</xsl:text>
		</xsl:when>
		<xsl:when test="@doc='readme'">
			<xsl:text>ReadMe.txt</xsl:text>
		</xsl:when>
		<xsl:otherwise>
			<xsl:value-of select="@target"/>
		</xsl:otherwise>
	</xsl:choose>
</xsl:template>


<!--================-->
<!-- Misc Templates -->
<!--================-->

<xsl:template match="para">
	<!-- Same trick as in the header template -->
	<xsl:variable name = "data">
		<xsl:apply-templates/>
	</xsl:variable>
	<xsl:value-of select="normalize-space($data)"/>
	<xsl:text>&#xA;</xsl:text>
</xsl:template>


<xsl:template match="cite">
	<xsl:if test="position()!=1">
		<xsl:text>&#xA;</xsl:text>
	</xsl:if>
	<xsl:value-of select="@name"/><xsl:text>:&#xA;</xsl:text>
	<!-- Same trick as in the header template -->
	<xsl:variable name = "data">
		<xsl:apply-templates/>
	</xsl:variable>
	<xsl:value-of select="normalize-space($data)"/>
	<xsl:text>&#xA;</xsl:text>
</xsl:template>


<xsl:template match="ol">
	<xsl:for-each select="li">
		<xsl:text></xsl:text>
		<xsl:number format="1. "/>
		<xsl:variable name = "data">
			<xsl:apply-templates/>
		</xsl:variable>
		<xsl:value-of select="normalize-space($data)"/>
		<xsl:text>&#xA;</xsl:text>
	</xsl:for-each>
	<xsl:text>&#xA;</xsl:text>
</xsl:template>

<xsl:template match="ul">
	<xsl:for-each select="li">
		<xsl:text>* </xsl:text>
		<xsl:variable name = "data">
			<xsl:apply-templates/>
		</xsl:variable>
		<xsl:value-of select="normalize-space($data)"/>
		<xsl:text>&#xA;</xsl:text>
	</xsl:for-each>
	<xsl:text>&#xA;</xsl:text>
</xsl:template>


<xsl:template match="br">
	<xsl:text>&#xA;</xsl:text>
</xsl:template>


<xsl:template match="key">'<xsl:value-of select="."/>'</xsl:template>

<xsl:template match="Pent">
	<xsl:text>Pentagram</xsl:text>
</xsl:template>

<!--=======================-->
<!-- Key Command Templates -->
<!--=======================-->
<xsl:template match="keytable">
	<table border="0" cellpadding="0" cellspacing="0" width="0">
		<tr>
			<th colspan="3" align="left">
				<xsl:text>&#xA;</xsl:text>
				<xsl:value-of select="@title"/>
				<xsl:text>&#xA;</xsl:text>
			</th>
		</tr>
		<xsl:apply-templates select="keydesc"/>
	</table>
</xsl:template>


<xsl:template match="keydesc">
	<tr>
		<td nowrap="nowrap" valign="top">
			<font color="maroon"><xsl:value-of select="@name"/></font>
		</td>
		<td><xsl:text> : </xsl:text></td>
		<td><xsl:value-of select="."/></td>
		<xsl:text>&#xA;</xsl:text>
	</tr>
</xsl:template>


<!--========================-->
<!-- Config Table Templates -->
<!--========================-->
<xsl:template match="configdesc">
	<xsl:text>&#xA;</xsl:text>
	<table border="0" cellpadding="0" cellspacing="0">
		<xsl:apply-templates select="configsec"/>
	</table>
	<xsl:text>&#xA;</xsl:text>
</xsl:template>

<xsl:strip-space elements="configtag"/>

<xsl:template match="configsec">
	<xsl:param name="indent"></xsl:param>
	<xsl:value-of select="$indent"/>[<xsl:value-of select="@name"/>]&cr;
	<xsl:apply-templates select="configtag"/>
</xsl:template>

<xsl:template match="configtag">
	<xsl:param name="indent"></xsl:param>
			<xsl:apply-templates select="comment"/>
			<xsl:value-of select="@name"/><xsl:text>=</xsl:text><xsl:value-of select="normalize-space(text())"/>
			&cr;
</xsl:template>


<xsl:template match="comment">
	<xsl:apply-templates/>
	&cr;
</xsl:template>

<!-- Clone template. Allows one to use any XHTML in the source file -->
<!--
<xsl:template match="@*|node()">
	<xsl:copy>
		<xsl:apply-templates select="@*|node()"/>
	</xsl:copy>
</xsl:template>
-->

</xsl:stylesheet>
