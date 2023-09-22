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
        mpiLongMessage::mpiLongMessage(SyncQueue::Header *h, NullRankSyncSerialSkip::comm_pair *comm, uint32_t rank, char * buff)
	{
		hdr = h;
		comm_element = comm;
		target_rank = rank;
		mybuff = buff;
		newbuff = NULL;
		myreqs = NULL;
		mystatus = NULL;
	}
        mpiLongMessage::~mpiLongMessage()
	{
	     if(myreqs != NULL)
		delete myreqs;
	     if(newbuff != NULL)
		delete newbuff;
	     if(mystatus != NULL)
		delete mystatus;
	}
    	void mpiLongMessage::sendMessage()
    	{
		numMessages = hdr->buffer_size/MAX_MSG;
		myreqs = new MPI_Request[numMessages];
      		uint32_t tag = LONG_INIT_TAG;
		hdr->mode = LONG_MODE;
		sendHeader();
		for (uint32_t i = 0; i < numMessages; i++)
		{
			MPI_Isend(mybuff + i*MAX_MSG, MAX_MSG, MPI_BYTE, target_rank, tag, MPI_COMM_WORLD, &myreqs[i]);
			tag++;
      		}
    	}
	void mpiLongMessage::recvMessage() {
		numMessages = comm_element->local_size/MAX_MSG;
		myreqs = new MPI_Request[numMessages];
		mystatus = new MPI_Status[numMessages];
		newbuff = new char[MAX_MSG*numMessages];
		uint32_t tag = LONG_INIT_TAG;
      		for(uint32_t i = 0; i < numMessages; i++) {
			MPI_Irecv(newbuff + i*MAX_MSG, MAX_MSG, MPI_BYTE, target_rank, tag, MPI_COMM_WORLD, &myreqs[i]);
			tag++;
      		}
		MPI_Waitall(numMessages, myreqs, mystatus);//wait for recvs to complete
	}


}

