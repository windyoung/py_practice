include ../Makefile.incl

LOCALINC = -I${ZSMARTMQ_HOME}/include $(LOCALINC_BASE) 

ALL_OBJS= TZSmartMQProducer.o TZSmartMQConsumer.o TZSmartMQMsgListener.o TMQMsgBufferMgr.o

all:$(ALL_OBJS)
	$(AR) libzsmartmq.$(TRUESUFFIX)  $(ALL_OBJS)
	echo $(VERID) >> libzsmartmq.$(TRUESUFFIX)

install: all
	cp -rf *.$(TRUESUFFIX)  $(PROJECT_DIR)/lib

clean:
	rm -f *.o  libzsmartmq.$(TRUESUFFIX)