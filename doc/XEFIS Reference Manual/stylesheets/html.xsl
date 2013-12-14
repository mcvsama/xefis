<?xml version='1.0'?>
<xsl:stylesheet version='1.0' xmlns:xsl='http://www.w3.org/1999/XSL/Transform' xmlns:doc='xefis-doc' xmlns='http://www.w3.org/1999/xhtml' xmlns:h='http://www.w3.org/1999/xhtml' exclude-result-prefixes="h">
	<xsl:output method="xml" version="1.0" encoding="UTF-8" indent="yes" media-type="application/xhtml+xml"/>

	<xsl:template match='/'>
		<html lang='en-us' xml:lang='en-us'>
			<head>
				<title><xsl:value-of select='//doc:title'/></title>
				<link rel='stylesheet' type='text/css' href='stylesheets/webstyle.css'/>
			</head>
			<body>
				<header>
					<h1><a href='index.xhtml'>XEFIS Reference Manual</a></h1>
				</header>
				<xsl:apply-templates/>
				<footer>
					<p><a href='http://xefis.mulabs.org/'>XEFIS.mulabs.org</a></p>
				</footer>
			</body>
		</html>
	</xsl:template>

	<xsl:template match='doc:noxslt'>
	</xsl:template>

	<xsl:template match='doc:file-list'>
		<ul class='files'>
			<xsl:apply-templates mode='file-list'/>
		</ul>
	</xsl:template>

	<xsl:template match='doc:file' mode='file-list'>
		<xsl:apply-templates select='document(@name)/*' mode='file-list'>
			<xsl:with-param name='filename' select='@name'/>
		</xsl:apply-templates>
	</xsl:template>

	<xsl:template match='doc:page' mode='file-list'>
		<xsl:param name='filename'/>
		<li>
			<a href='{$filename}'>
				<xsl:apply-templates select='doc:meta' mode='file-list'/>
			</a>
		</li>
	</xsl:template>

	<xsl:template match='doc:module' mode='file-list'>
		<xsl:param name='filename'/>
		<li>
			<a href='{$filename}'>
				<xsl:apply-templates select='doc:meta' mode='file-list'/>
			</a>
		</li>
	</xsl:template>

	<xsl:template match='doc:page'>
		<section>
			<xsl:apply-templates select='doc:meta' mode='page'/>
			<xsl:apply-templates/>
		</section>
	</xsl:template>

	<xsl:template match='doc:module'>
		<section>
			<xsl:apply-templates select='doc:meta' mode='module'/>
			<xsl:apply-templates/>
		</section>
	</xsl:template>

	<xsl:template match='doc:meta'>
	</xsl:template>

	<xsl:template match='doc:meta' mode='file-list'>
		<p class='name'><xsl:apply-templates select='doc:name' mode='module'/></p>
		<p class='title'><xsl:apply-templates select='doc:title' mode='module'/></p>
	</xsl:template>

	<xsl:template match='doc:meta' mode='page'>
		<h1><xsl:apply-templates select='doc:title'/></h1>
	</xsl:template>

	<xsl:template match='doc:meta' mode='module'>
		<section>
			<h1>
				<xsl:apply-templates select='doc:title' mode='module'/>
				<xsl:apply-templates select='doc:name' mode='module'/>
			</h1>
		</section>
	</xsl:template>

	<xsl:template match='doc:title' mode='module'>
		<xsl:apply-templates/>
	</xsl:template>

	<xsl:template match='doc:name' mode='module'>
		<span class='module-name'>
			<xsl:apply-templates/>
		</span>
	</xsl:template>

	<xsl:template match='doc:toc'>
		<section>
			<h1>Table of contents</h1>
			<ol class='toc'>
				<xsl:apply-templates select='../h:section' mode='toc'/>
			</ol>
		</section>
	</xsl:template>

	<xsl:template match='h:section' mode='toc'>
		<li>
			<p>
				<a>
					<xsl:attribute name='href'>
						<xsl:choose>
							<xsl:when test='@id != ""'>
								<xsl:value-of select='concat("#", @id)'/>
							</xsl:when>
							<xsl:otherwise>
								<xsl:value-of select='concat("#", generate-id(.))'/>
							</xsl:otherwise>
						</xsl:choose>
					</xsl:attribute>
					<xsl:value-of select='h:h1'/>
				</a>
			</p>
			<xsl:if test='count(h:section) > 0'>
				<ol>
					<xsl:apply-templates select='h:section' mode='toc'/>
				</ol>
			</xsl:if>
		</li>
	</xsl:template>

	<xsl:template match='h:section'>
		<section id='{generate-id(.)}'>
			<xsl:copy-of select='@*'/>
			<xsl:apply-templates/>
		</section>
	</xsl:template>

	<xsl:template match='doc:settings'>
		<table class='items settings'>
			<thead>
				<tr class='main'>
					<th scope='col' colspan='3'>Settings</th>
				</tr>
				<tr>
					<th scope='col'>Setting</th>
					<th scope='col'>Type</th>
					<th scope='col'>Description</th>
				</tr>
			</thead>
			<tbody>
				<xsl:apply-templates mode='settings'/>
			</tbody>
		</table>
	</xsl:template>

	<xsl:template match='doc:properties'>
		<table class='items properties'>
			<thead>
				<tr class='main'>
					<th scope='col' colspan='3'>Properties</th>
				</tr>
				<tr>
					<th scope='col'>Property</th>
					<th scope='col'>Type</th>
					<th scope='col'>Description</th>
				</tr>
			</thead>
			<tbody>
				<xsl:apply-templates mode='properties'/>
			</tbody>
		</table>
	</xsl:template>

	<xsl:template match='doc:group' mode='settings'>
		<tr>
			<th class='group' scope='col' colspan='3'><xsl:value-of select='@title'/></th>
		</tr>
		<xsl:apply-templates mode='settings'/>
	</xsl:template>

	<xsl:template match='doc:group' mode='properties'>
		<tr>
			<th class='group' scope='col' colspan='3'><xsl:value-of select='@title'/></th>
		</tr>
		<xsl:apply-templates mode='properties'/>
	</xsl:template>

	<xsl:template match='doc:item' mode='settings'>
		<tr>
			<td class='setting'><xsl:value-of select='@name'/></td>
			<td class='type'><xsl:value-of select='@type'/></td>
			<td class='description'><xsl:apply-templates/></td>
		</tr>
	</xsl:template>

	<xsl:template match='doc:item' mode='properties'>
		<tr>
			<td class='property'><xsl:value-of select='@name'/></td>
			<td class='type'><xsl:value-of select='@type'/></td>
			<td class='description'><xsl:apply-templates/></td>
		</tr>
	</xsl:template>

	<xsl:template match='doc:fma'>
		<span class='fma-mode'>
			<xsl:apply-templates/>
		</span>
	</xsl:template>

	<xsl:template match='doc:btn'>
		<span class='mcp-button'>
			<xsl:apply-templates/>
		</span>
	</xsl:template>

	<xsl:template match='doc:todo'>
		<span class='todo'>TODO</span>
	</xsl:template>

	<xsl:template match='*'>
		<xsl:copy>
			<xsl:copy-of select='@*'/>
			<xsl:apply-templates/>
		</xsl:copy>
	</xsl:template>

</xsl:stylesheet>
