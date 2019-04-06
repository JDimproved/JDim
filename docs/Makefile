JAVA = /usr/bin/java
SAXON = saxon9he.jar
XSLT = xml/main.xsl
SOURCES = $(wildcard xml/*.xml)
HISTORIES = xml/history.xml $(sort $(wildcard xml/20[0-9][0-9].xml))
ATTACH = ChangeLog INSTALL README
OUT =
PLACE = $(shell /bin/ls -Fd $(OUT) 2>/dev/null)


.SUFFIXES: .xsl .xml .html

.PHONY: all clean manual attach

all: manual attach

clean:
	-rm -rf $(addprefix $(PLACE), *.html *.css image/ $(ATTACH))


# マニュアル
manual: $(SOURCES:.xml=.html)
	@cp -ru xml/*.css xml/image $(PLACE)

$(HISTORIES:.xml=.html): XSLT = xml/history.xsl

$(SOURCES): xml/main.xsl

$(HISTORIES): xml/history.xsl

xml/history.xsl: xml/main.xsl

.xml.html:
	$(JAVA) -jar $(SAXON) -xsl:$(XSLT) -s:$< -o:$(addprefix $(PLACE), $(notdir $@))


# リリース時に付属するファイル
attach: $(ATTACH)

ChangeLog: $(word $(words $(HISTORIES)), $(HISTORIES)) attach/changelog.xsl
	$(JAVA) -jar $(SAXON) -xsl:attach/changelog.xsl -s:$< -o:$(addprefix $(PLACE), $(notdir $@))

INSTALL: xml/make.xml attach/install.xsl
	$(JAVA) -jar $(SAXON) -xsl:attach/install.xsl -s:$< -o:$(addprefix $(PLACE), $(notdir $@))

README: xml/about.xml attach/readme.xsl
	$(JAVA) -jar $(SAXON) -xsl:attach/readme.xsl -s:$< -o:$(addprefix $(PLACE), $(notdir $@))
