# -*- Makefile -*-
#
#

sst_core_sources += \
	sync/exchangeManager.cc \
	sync/exchangeManager.h \
	sync/nullRankSyncSerialSkip.h \
	sync/nullRankSyncSerialSkip.cc \
	sync/rankSyncParallelSkip.h \
	sync/rankSyncParallelSkip.cc \
	sync/rankSyncSerialSkip.h \
	sync/rankSyncSerialSkip.cc \
	sync/syncManager.h \
	sync/syncManager.cc \
	sync/syncQueue.h \
	sync/syncQueue.cc \
	sync/threadSyncDirectSkip.h \
	sync/threadSyncDirectSkip.cc \
	sync/threadSyncSimpleSkip.h \
	sync/threadSyncSimpleSkip.cc \
	sync/threadSyncQueue.h \
	sync/mpiMessage.cc \
	sync/mpiMessage.h \
	sync/mpiShortMessage.cc \
	sync/mpiShortMessage.h \
	sync/mpiResizeMessage.cc \
	sync/mpiResizeMessage.h \
	sync/mpiLongMessage.cc \
	sync/mpiLongMessage.h \
	sync/mpiMessages.h
