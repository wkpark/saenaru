#
# Maintainer Buildtool for nmake
#
# $Saenaru: saenaru/Makefile,v 1.1 2003/12/26 22:03:07 perky Exp $
#

RELVERSION=	031226
SETUPEXE=	setup\Saenaru-$(RELVERSION).exe
IMEBUILDDIR=	src\objfre\i386
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
