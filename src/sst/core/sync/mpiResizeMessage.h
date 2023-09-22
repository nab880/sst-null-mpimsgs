// Copyright 2009-2023 NTESS. Under the terms
// of Contract DE-NA0003525 with NTESS, the U.S.
// Government retains certain rights in this software.
//
// Copyright (c) 2009-2023, NTESS
// All rights reserved.
//
// This file is part of the SST software package. For license
// information, see the LICENSE file in the top level directory of the
// distribution.

#ifndef SST_CORE_RESIZE_MESSAGE
#define SST_CORE_RESIZE_MESSAGE

#include "sst/core/activityQueue.h"
#include "sst/core/threadsafe.h"
#include "sst/core/sync/syncQueue.h"
#include <vector>

#ifdef SST_CONFIG_HAVE_MPI
DISABLE_WARN_MISSING_OVERRIDE
#include <mpi.h>
REENABLE_WARNING
#define UNUSED_WO_MPI(x) x
#else
#define UNUSED_WO_MPI(x) UNUSED(x)
#endif

namespace SST {
class mpiResizeMessage:public mpiMessage
{
public:
        mpiResizeMessage(SyncQueue::Header *h, NullRankSyncSerialSkip::comm_pair *comm, uint32_t rank, char * buff);
	~mpiResizeMessage();
	static bool checkSendMessage(uint32_t remote_size, uint32_t local_size){
		return (remote_size > local_size && local_size < MAX_MSG);
	}
	static bool checkRecvMessage(SyncQueue::Header *hdr){
		return (hdr->mode == RESIZE_MODE); 
	}
	void sendMessage();
	void recvMessage();


};
}

#endif // SST_CORE_RESIZE_MESSAGE
