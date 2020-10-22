##CHECKED_VERSION=FR3|FILE_PATH=BASEDIR/helper/
include ../Makefile.incl

clean:
	rm -f *.o  libhelper.$(TRUESUFFIX)

ALL_OBJS=BillLog.o \
  Common.o \
	DataStruct.o \
  DiameterParser.o \
  ReadIni.o \
	SOHelper.o \
	TDataConvertFunc.o \
	TDateTimeFunc.o \
	TException.o \
	TFileOper.o \
	TFileScanOper.o \
	trie.o \
	TSem.o \
	TShm.o \
	TSplit.o \
	TStrFunc.o \
	TMathFunc.o \
	wgetopt.o \
	tinyxml.o \
	tinyxmlerror.o \
	tinyxmlparser.o \
	Mutex.o \
	TThreadBase.o \
	md5.o\
  Aes.o\
  Aes_Encryptor.o


all_objs:	$(ALL_OBJS) $(DEBUG_NEW_FLAG)

all:	all_objs
	$(AR) libhelper.$(TRUESUFFIX)  $(ALL_OBJS) $(DEBUG_NEW_FLAG)
	echo $(VERID) >> libhelper.$(TRUESUFFIX)

install: all
	cp -rf *.$(TRUESUFFIX)  $(PROJECT_DIR)/lib
