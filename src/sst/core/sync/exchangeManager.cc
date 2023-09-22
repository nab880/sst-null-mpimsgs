#include <iostream>
#include "sst/core/sync/exchangeManager.h"
namespace SST {    
    
    ExchangeManager::ExchangeManager()
    {
    }
    mpiMessage* 
    ExchangeManager::sendMessage(char * buff, NullRankSyncSerialSkip::comm_pair *comm_element, uint32_t target_rank)
    {
      SyncQueue::Header* hdr = reinterpret_cast<SyncQueue::Header*>(buff);
      mpiMessage *msg;
      if(mpiResizeMessage::checkSendMessage(comm_element->remote_size, hdr->buffer_size)){
	msg = new mpiResizeMessage(hdr, comm_element, target_rank, buff);
      }
      else if (mpiLongMessage::checkSendMessage(comm_element->remote_size, hdr->buffer_size)) { //long send
	msg = new mpiLongMessage(hdr, comm_element, target_rank, buff);
      } 
      else if (mpiShortMessage::checkSendMessage(comm_element->remote_size, hdr->buffer_size)) {
	msg = new mpiShortMessage(hdr, comm_element, target_rank, buff);

      }
      else
      {
	std::cout<<"ERROR \n";
	exit(0);
      }
      return msg;
    }
    
    mpiMessage*
    ExchangeManager::completeRequest(char * buff, NullRankSyncSerialSkip::comm_pair *comm_element, uint32_t target_rank)
    {
      mpiMessage *msg; 
      SyncQueue::Header* hdr = reinterpret_cast<SyncQueue::Header*>(buff);
      if (mpiShortMessage::checkRecvMessage(hdr)) 
      { 
      	  msg = new mpiShortMessage(hdr, comm_element, target_rank, buff);
      } else if (mpiResizeMessage::checkRecvMessage(hdr)) {
	  msg = new mpiResizeMessage(hdr, comm_element, target_rank, buff);
      } else if(mpiLongMessage::checkRecvMessage(hdr)) { 
       	  msg = new mpiLongMessage(hdr, comm_element, target_rank, buff);
      } else {
	std::cout<<"ERROR \n";
	exit(0);
      }
      return msg;	
    }
    
}
