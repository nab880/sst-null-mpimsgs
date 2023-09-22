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

#include "sst_config.h"
#include "sst/core/sync/exchangeManager.h"
#include "sst/core/sync/nullRankSyncSerialSkip.h"
#include "sst/core/sync/mpiMessages.h"
#include "sst/core/event.h"
#include "sst/core/exit.h"
#include "sst/core/impl/timevortex/nullMessagePQ.h"
#include "sst/core/link.h"
#include "sst/core/profile.h"
#include "sst/core/serialization/serializer.h"
#include "sst/core/simulation_impl.h"
#include "sst/core/stopAction.h"
#include "sst/core/sync/syncQueue.h"
#include "sst/core/timeConverter.h"
#include "sst/core/warnmacros.h"

// #include <limits>


#if SST_EVENT_PROFILING
#define SST_EVENT_PROFILE_START auto event_profile_start = std::chrono::high_resolution_clock::now();

#define SST_EVENT_PROFILE_STOP                                                                                  \
    auto event_profile_stop = std::chrono::high_resolution_clock::now();                                        \
    auto event_profile_count =                                                                                  \
        std::chrono::duration_cast<std::chrono::nanoseconds>(event_profile_stop - event_profile_start).count(); \
    sim->incrementSerialCounters(event_profile_count);
#else
#define SST_EVENT_PROFILE_START
#define SST_EVENT_PROFILE_STOP
#endif


namespace SST {

// Static Data Members

NullRankSyncSerialSkip::NullRankSyncSerialSkip(RankInfo num_ranks, TimeConverter* UNUSED(minPartTC)) :
    RankSync(num_ranks),
    // delay(std::numeric_limits<SimTime_t>::max()),
    safe_time(0),
    deserializeTime(0.0),
    local_done(false)

{
    // std::cout << "NullRankSyncSerialSkip" << std::endl;
    max_period = Simulation_impl::getSimulation()->getMinPartTC();
}

NullRankSyncSerialSkip::~NullRankSyncSerialSkip()
{
    for ( comm_map_t::iterator i = comm_map.begin(); i != comm_map.end(); ++i ) {
        delete i->second.squeue;
    }
    comm_map.clear();

    if ( deserializeTime > 0.0 )
        Output::getDefaultObject().verbose(
            CALL_INFO, 1, 0, "RankSyncSerialSkip deserializeWait:  %lg sec\n", deserializeTime);

    delete[] requests;
}

ActivityQueue*
NullRankSyncSerialSkip::registerLink(
    const RankInfo& to_rank, const RankInfo& UNUSED(from_rank), const std::string& name, Link* link, SimTime_t latency)
{
    std::lock_guard<Core::ThreadSafe::Spinlock> slock(lock);
    // std::cout << "RegisterLink TO: " << to_rank.rank << " FROM: " << from_rank.rank << " Name: " << name <<
    // std::endl;
    SyncQueue*                                  queue;
    if ( comm_map.count(to_rank.rank) == 0 ) {
        queue = comm_map[to_rank.rank].squeue = new SyncQueue();
        comm_map[to_rank.rank].rbuf           = new char[4096];
        comm_map[to_rank.rank].local_size     = 4096;
        comm_map[to_rank.rank].remote_size    = 4096;
        comm_map[to_rank.rank].guarantee_time = 0;
        comm_map[to_rank.rank].delay          = latency;
    }
    else {
        queue = comm_map[to_rank.rank].squeue;
        if ( latency < comm_map[to_rank.rank].delay ) {
            comm_map[to_rank.rank].delay = latency;
        }
    }

    // std::cout << "Delay of " << latency << " add for rank " << to_rank.rank << std::endl;
    link_maps[to_rank.rank][name] = reinterpret_cast<uintptr_t>(link);
#ifdef __SST_DEBUG_EVENT_TRACKING__
    link->setSendingComponentInfo("SYNC", "SYNC", "");
#endif
    return queue;
}

void
NullRankSyncSerialSkip::finalizeLinkConfigurations()
{
    // NOTE:: Hopefully this is called / verify this is called after all registerLinks
    // std::cout << "Calling initializeSendReceiveBuffers" << std::endl;

    auto timeVortex = Simulation_impl::getSimulation()->getTimeVortex();

    // this skip should only be used for NULL Message
    IMPL::NullMessagePQ* nmpq = dynamic_cast<IMPL::NullMessagePQ*>(timeVortex);
    assert(nmpq);
    nmpq->setNullSkip(this);

    initialize();
}

void
NullRankSyncSerialSkip::prepareForComplete()
{
    // std::cout << "NullRankSyncSerialSkip::prepareForComplete" << std::endl;
    while ( output_buffers.size() > 0 ) {
        testSendComplete();
        receiveData(false);
    }
}

uint64_t
NullRankSyncSerialSkip::getDataSize() const
{
    size_t count = 0;
    for ( comm_map_t::const_iterator it = comm_map.begin(); it != comm_map.end(); ++it ) {
        count += (it->second.squeue->getDataSize() + it->second.local_size);
    }
    return count;
}

void
NullRankSyncSerialSkip::calculateSafeTime()
{

    // DEBUG
    // uint32_t  my_rank       = Simulation_impl::getSimulation()->getRank().rank;
    // SimTime_t current_cycle = Simulation_impl::getSimulation()->getCurrentSimCycle();
    // END_DEBUG


    SimTime_t new_safe_time = std::numeric_limits<SimTime_t>::max();
    for ( comm_map_t::const_iterator i = comm_map.begin(); i != comm_map.end(); ++i ) {
        new_safe_time = std::min(new_safe_time, i->second.guarantee_time);
    }

    // DEBUG
    // if ( safe_time != new_safe_time ) {
    //    std::cout << current_cycle << ":" << my_rank << ": safe time updated to " << new_safe_time << std::endl;
    //}
    // END_DEBUG

    safe_time = new_safe_time;
}

SimTime_t
NullRankSyncSerialSkip::calculateGuaranteeTime(int rank)
{
    SimTime_t next_activity_time = Simulation_impl::getLocalMinimumNextActivityTime();
    return std::min(next_activity_time, safe_time) + comm_map[rank].delay;
}

void
NullRankSyncSerialSkip::testSendComplete()
{
    auto iter = output_buffers.begin();
    while ( iter != output_buffers.end() ) {
        MPI_Status status;
        int        flag = 0;
        MPI_Test(iter->getRequest(), &flag, &status);
        auto current = iter;
        ++iter;
        if ( flag ) {
            output_buffers.erase(current);
        }
    }
}

void
NullRankSyncSerialSkip::sendData(int to_rank)
{
#ifdef SST_CONFIG_HAVE_MPI
    // DEBUG
    uint32_t  my_rank       = Simulation_impl::getSimulation()->getRank().rank;
    // END DEBUG
    //  NOTE not DEBUG, need for future event scheduling
    SimTime_t current_cycle = Simulation_impl::getSimulation()->getCurrentSimCycle();


    NullMessageSentBuffer output_buffer;
    output_buffers.push_back(output_buffer);
    // last element
    auto iter = output_buffers.rbegin();

    iter->setBuffer(comm_map[to_rank].squeue->getDataNewBuffer());

    // Cast to Header so we can get/fill in data
    SyncQueue::Header* hdr = reinterpret_cast<SyncQueue::Header*>(iter->getBuffer());

    // fill in my new guarantee time (which may be from a null message)
    hdr->guarantee_time = calculateGuaranteeTime(to_rank);
    int tag             = 1;

    mpiMessage *message = exchangeManager->sendMessage(iter->getBuffer(), &comm_map[to_rank], to_rank);
    message->sendMessage();
    delete message;
    NullMessageEvent* ev        = new NullMessageEvent(this, to_rank);
    SimTime_t         next_send = current_cycle + comm_map[to_rank].delay;

    Simulation_impl::getSimulation()->insertActivity(next_send, ev);
    // std::cout << current_cycle << ":" << my_rank << ":scheduling next data sent time to " << to_rank << " at time "
    // << next_send << std::endl;

#endif
}

void
NullRankSyncSerialSkip::receiveData(bool blocking)
{
#ifdef SST_CONFIG_HAVE_MPI

    uint32_t  my_rank       = Simulation_impl::getSimulation()->getRank().rank;
    SimTime_t current_cycle = Simulation_impl::getSimulation()->getCurrentSimCycle();

    if ( comm_map.size() == 0 ) return;
    if ( Simulation_impl::getSimulation()->endSim == true ) {
        return;
    }
    bool stop = false;

    Exit* exit = Simulation_impl::getSimulation()->getSimulation()->getExit();
    if ( exit->getRefCount() == 0 && !local_done ) {
        local_done = true;
        int val, sum;
        MPI_Iallreduce(&val, &sum, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD, &requests[comm_map.size()]);
    }

    int num_requests = local_done ? comm_map.size() + 1 : comm_map.size();

    do {
        MPI_Status status;
        int        messages_received = 0;
        int        index             = 0;

        if ( blocking ) {
            MPI_Waitany(num_requests, requests, &index, &status);
            messages_received = 1;
            stop              = true;
        }
        else {
            // std::cout << my_rank << ": test any with size " << comm_map.size() << std::endl;
            MPI_Testany(num_requests, requests, &index, &messages_received, &status);
        }

        if ( index == (int)comm_map.size() ) {
            auto* sim = Simulation_impl::getSimulation();
            sim->insertActivity(sim->getCurrentSimCycle(), new StopAction());
            return;
        }
        if ( messages_received ) {
            // std::cout << my_rank << ": messages received!" << std::endl;
            int   from_rank = request_map[index];
            char* buffer    = comm_map[from_rank].rbuf;

            SyncQueue::Header* hdr            = reinterpret_cast<SyncQueue::Header*>(buffer);
            unsigned int       size           = hdr->buffer_size;
            int                mode           = hdr->mode;
            SimTime_t          guarantee_time = hdr->guarantee_time;
            assert(comm_map[from_rank].guarantee_time <= guarantee_time);
            // set new guarantee time from this rank
            // std::cout << current_cycle << ":" << my_rank << ": received guarantee time of " << guarantee_time
            //          << " from " << from_rank << std::endl;

            comm_map[from_rank].guarantee_time = guarantee_time;
	    mpiMessage *message = exchangeManager->completeRequest(comm_map[from_rank].rbuf, &comm_map[from_rank], from_rank);
	    message->recvMessage();
            auto deserialStart = SST::Core::Profile::now();

	    message->serialize();
            SST::Core::Serialization::serializer ser;
            std::vector<Activity*> activities;
            activities.clear();
            ser & activities;


            deserializeTime += SST::Core::Profile::getElapsed(deserialStart);

            for ( unsigned int j = 0; j < activities.size(); j++ ) {

                Event*    ev    = static_cast<Event*>(activities[j]);
                SimTime_t delay = ev->getDeliveryTime() - current_cycle;
                getDeliveryLink(ev)->send(delay, ev);
            }

            activities.clear();

            // requue the next async receive
            message->queueIrecv(&requests[index]);
	    delete message;
        }
        else {
            stop = true;
        }

    } while ( !stop );
    // std::cout << my_rank << ": done with received messages" << std::endl;
#endif
}

void
NullRankSyncSerialSkip::initialize()
{
#ifdef SST_CONFIG_HAVE_MPI
    // uint32_t my_rank = Simulation_impl::getSimulation()->getRank().rank;

    requests = new MPI_Request[comm_map.size() + 1];
    int idx  = 0;
    for ( comm_map_t::iterator i = comm_map.begin(); i != comm_map.end(); ++i, idx++ ) {
        MPI_Irecv(i->second.rbuf, i->second.local_size, MPI_BYTE, i->first, 1, MPI_COMM_WORLD, &requests[idx]);
        // std::cout << my_rank << ": scheduled recv from rank " << i->first << " at index " << idx << std::endl;
        request_map[idx] = i->first;

        NullMessageEvent* ev = new NullMessageEvent(this, i->first);
        Simulation_impl::getSimulation()->insertActivity(0, ev);
    }
#endif
}

void
NullRankSyncSerialSkip::exchangeLinkUntimedData(int UNUSED_WO_MPI(thread), std::atomic<int>& UNUSED_WO_MPI(msg_count))
{}

} // namespace SST
