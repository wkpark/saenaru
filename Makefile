#
# Maintainer Buildtool for nmake
#
# $Saenaru$
#

RELVERSION=	031226
SETUPEXE=	Saenaru-$(RELVERSION).exe
IMEBUILDDIR=	src\objfre\i386
NSISDIR=	"C:\Program Files\NSIS"

all: $(SETUPEXE)

clean:
	del $(SETUPEXE)
	cd $(IMEBUILDDIR) && del /F /Q *.*

$(SETUPEXE): $(IMEBUILDDIR)\saenaru.ime
	echo "MERONG"

$(IMEBUILDDIR)\saenaru.ime:
	cd src && build -c

# ex: ts=8 sts=8 sw=8 noet
