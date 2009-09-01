!include objects.mk

data/data.h : "$(data2c)" $(DATA_FILES)
	"$(data2c)" data\\data.h $(DATA_FILES)

build : data/data.h

clean :
	del data\\data.h

rebuild : clean build

