#To be included by makefile.def.
copyfile: $(TARGET)
        copy ..\setup\saenaru*.inf $(**D)
        copy ..\resource\saenaru*.dic $(**D)
