##CHECKED_VERSION=FR3|FILE_PATH=BASEDIR/dbc/
include ../Makefile.incl

clean:
	rm -f *.o  libjsoncpp.$(TRUESUFFIX)

ALL_OBJS=jsoncpp.o 

all_objs: $(ALL_OBJS) $(DEBUG_NEW_FLAG)

all: $(ALL_OBJS)
	$(AR) libjsoncpp.$(TRUESUFFIX)  $(ALL_OBJS) $(DEBUG_NEW_FLAG) 
	echo $(VERID) >> libjsoncpp.$(TRUESUFFIX)

install: all
	cp -rf *.$(TRUESUFFIX)  $(PROJECT_DIR)/lib