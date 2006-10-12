#
# Maintainer Buildtool for nmake
#
# $Saenaru: saenaru/Makefile,v 1.2 2003/12/27 15:20:42 perky Exp $
#

RELVERSION=	061013
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
