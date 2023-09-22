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


#include "sst/core/sync/mpiMessages.h"
#include <vector>


namespace SST {
	void mpiMessage::serialize(){
	
	}
	void mpiMessage::sendMessage() {
	 return;
	}
	void mpiMessage::recvMessage() {
	 return;
	}
	void 
	mpiMessage::sendHeader() {
		MPI_Request req;
		MPI_Isend(mybuff, sizeof(SyncQueue::Header), MPI_BYTE, target_rank, SHORT_INIT_TAG, MPI_COMM_WORLD, &req);
	}
	void 
	mpiMessage::queueIrecv(MPI_Request *req)
	{
		MPI_Irecv(
                comm_element->rbuf, comm_element->local_size, MPI_BYTE, target_rank, 1, MPI_COMM_WORLD, req);	
	}
	
}

