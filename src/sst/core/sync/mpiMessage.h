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

#ifndef SST_CORE_MESSAGE
#define SST_CORE_MESSAGE

#include "sst/core/activityQueue.h"
#include "sst/core/threadsafe.h"
#include "sst/core/sync/syncQueue.h"
#include "sst/core/sync/mpiMessage.h"
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
class mpiMessage
{
public:
	void sendMessage();
	void recvMessage();
	void sendHeader();
	void queueIrecv(MPI_Request *req);
	void serialize();
protected:
	MPI_Request* myreqs;
	MPI_Status* mystatus;
	char * mybuff;
	NullRankSyncSerialSkip::comm_pair *comm_element;
	SyncQueue::Header *hdr;
	uint32_t target_rank;
};
}

#endif // SST_CORE_MESSAGE
