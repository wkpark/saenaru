#
# Maintainer Buildtool for nmake
#
# $Saenaru: saenaru/Makefile,v 1.3 2006/10/12 22:13:53 wkpark Exp $
#

RELVERSION=	1.0
SETUPEXE=	setup\Saenaru-$(RELVERSION).exe
IMEBUILDDIR=	src\objfre_wxp_x86\i386
NSISDIR=	C:\Program Files\NSIS

all: $(SETUPEXE)

clean:
	del $(SETUPEXE)
	cd $(IMEBUILDDIR) && del /F /Q *.*

$(SETUPEXE): $(IMEBUILDDIR)\saenaru.ime
	cd setup
	"$(NSISDIR)\makensis.exe" saenaru.nsi

$(IMEBUILDDIR)\saenaru.ime:
	cd src && build -c

# ex: ts=8 sts=8 sw=8 noet
