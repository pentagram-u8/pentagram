!include objects.mk

data/data.h : "$(OutDir)\\data2c.exe" $(DATA_FILES)
	"$(OutDir)\\data2c.exe" data\\data.h $(DATA_FILES)

build : data/data.h

clean :
	del data\\data.h

rebuild : clean build

