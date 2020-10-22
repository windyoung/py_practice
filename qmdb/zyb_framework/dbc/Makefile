##CHECKED_VERSION=FR3|FILE_PATH=BASEDIR/dbc/
include ../Makefile.incl

clean:
	rm -f *.o  libdbc.$(TRUESUFFIX)

ALL_OBJS=T$(ODBC_FILETAG)DBQuery.o T$(ODBC_FILETAG)DBCSQuery.o TOraDBQuery.o TDBFactory.o TQmdbInterfaceAdapter.o

all_objs: $(ALL_OBJS)

all:	$(ALL_OBJS)
	$(AR) libdbc.$(TRUESUFFIX)  $(ALL_OBJS)  $(ORACLELIBS) 
	echo $(VERID) >> libdbc.$(TRUESUFFIX)

install: all
	cp -rf *.$(TRUESUFFIX)  $(PROJECT_DIR)/lib

