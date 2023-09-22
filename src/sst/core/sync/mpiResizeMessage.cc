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


namespace SST {
        mpiResizeMessage::mpiResizeMessage(SyncQueue::Header *h, NullRankSyncSerialSkip::comm_pair *comm, uint32_t rank, char * buff)
	{
		hdr = h;
		comm_element = comm;
		myreqs = new MPI_Request;
		target_rank = rank;
		mybuff = buff;
	}
        mpiResizeMessage::~mpiResizeMessage()
	{
		delete myreqs;
	}
    	void mpiResizeMessage::sendMessage()
    	{
          hdr->mode = RESIZE_MODE;
	  sendHeader();
	  comm_element->remote_size = hdr->buffer_size;
          MPI_Isend(
	   mybuff, hdr->buffer_size, MPI_BYTE, target_rank, RESIZE_INIT_TAG, MPI_COMM_WORLD, myreqs);
    	}
	void mpiResizeMessage::recvMessage() {
		unsigned int size = hdr->buffer_size;
		delete[] comm_element->rbuf;
                comm_element->rbuf = new char[size];
                comm_element->local_size = size;
		MPI_Recv(comm_element->rbuf, comm_element->local_size, MPI_BYTE, target_rank, RESIZE_INIT_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE); // blocking recv, wait for completion
	}


}

