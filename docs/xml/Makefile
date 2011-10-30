CLASSPATH = xml-apis.jar:xercesImpl.jar:xalan.jar
JAVA = java
INPUT = readme.xml faq.xml
CLASS = org.apache.xalan.xslt.Process

HTML_OBJS := $(patsubst %.xml,%.html,$(wildcard *.xml))
TEXT_OBJS := $(patsubst %.xml,%.txt,$(wildcard *.xml))
PHP_OBJS := $(patsubst %.xml,%.dat,$(wildcard *.xml))

all: html text php duplicate phpfix

# Rules for transforming the XML source
.xml.html: html.xsl
	$(JAVA) -classpath $(CLASSPATH) $(CLASS) -IN $< -XSL html.xsl -OUT $@
.xml.txt: text.xsl
	$(JAVA) -classpath $(CLASSPATH) $(CLASS) -IN $< -XSL text.xsl -OUT $@
.xml.dat: php.xsl
	$(JAVA) -classpath $(CLASSPATH) $(CLASS) -IN $< -XSL php.xsl -OUT $@
.SUFFIXES: .xml .html .txt .dat

# rule for copying faq.txt, readme.txt to FAQ and README
duplicate: faq.txt readme.txt
	cp faq.txt FAQ
	cp readme.txt README

# rule for stripping the unneded <test> tag from the PHP files
phpfix: readme.dat faq.dat
	gsed -i -e 's@<test xmlns="http://www.w3.org/1999/xhtml">@@; s@</test>@@' *.dat

# remove all generated files
clean:
	-rm -f *.html *.txt *.dat FAQ README

html: $(HTML_OBJS)
text: $(TEXT_OBJS)
php: $(PHP_OBJS)

.PHONY: all html text php clean
