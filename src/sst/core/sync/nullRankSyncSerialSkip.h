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

#ifndef SST_CORE_SYNC_NULLRANKSYNCSERIALSKIP_H
#define SST_CORE_SYNC_NULLRANKSYNCSERIALSKIP_H

#include "sst/core/sst_types.h"
#include "sst/core/sync/syncManager.h"
#include "sst/core/threadsafe.h"

#include <map>

#ifdef SST_CONFIG_HAVE_MPI
DISABLE_WARN_MISSING_OVERRIDE
#include <mpi.h>
REENABLE_WARNING
#define UNUSED_WO_MPI(x) x
#else
#define UNUSED_WO_MPI(x) UNUSED(x)
#endif


namespace SST {

class SyncQueue;
class TimeConverter;

class NullMessageSentBuffer
{
public:
    NullMessageSentBuffer() : resize(false), buffer(nullptr) {}
    ~NullMessageSentBuffer() { delete[] buffer; } // buffer deleted elsewhere

    char* getBuffer() { return buffer; }

    void setBuffer(char* buf) { buffer = buf; }

    bool isResize() const { return resize; }
    void setResize() { resize = true; }

    MPI_Request* getRequest() { return &request; }

private:
    bool        resize;
    char*       buffer;
    MPI_Request request;
};


class NullRankSyncSerialSkip : public RankSync
{
public:
    /** Create a new Sync object which fires with a specified period */
    NullRankSyncSerialSkip(RankInfo num_ranks, TimeConverter* minPartTC);
    virtual ~NullRankSyncSerialSkip();

    /** Register a Link which this Sync Object is responsible for */
    ActivityQueue* registerLink(
        const RankInfo& to_rank, const RankInfo& from_rank, const std::string& name, Link* link,
        SimTime_t latency) override;

    /** Cause an exchange of Untimed Data to occur */
    void exchangeLinkUntimedData(int thread, std::atomic<int>& msg_count) override;
    /** Finish link configuration */
    void finalizeLinkConfigurations() override;
    /** Prepare for the complete() stage */
    void prepareForComplete() override;

    void execute(int UNUSED(thread)) override {}

    uint64_t getDataSize() const override;

    SimTime_t getSafeTime() const { return safe_time; }

    SimTime_t calculateGuaranteeTime(int rank);

    void calculateSafeTime();

    void initialize();

    void sendData(int to_rank);

    void receiveData(bool blocking);

    void testSendComplete();

private:
    MPI_Request* requests;

    struct comm_pair
    {
        SyncQueue* squeue; // SyncQueue
        char*      rbuf;   // receive buffer
        uint32_t   local_size;
        uint32_t   remote_size;
        SimTime_t  delay;         // minimum delay over all incoming channels
        SimTime_t guarantee_time; // no message will ever arrive on any incoming link with a receive time less than this
    };

    typedef std::map<int, comm_pair>         comm_map_t;
    typedef std::map<std::string, uintptr_t> link_map_t;
    typedef std::map<int, int>               request_map_t;

    // TimeConverter* period;
    comm_map_t                       comm_map;
    link_map_t                       link_map;
    request_map_t                    request_map;
    std::list<NullMessageSentBuffer> output_buffers;

    SimTime_t safe_time;

    double deserializeTime;
    bool   local_done;

    Core::ThreadSafe::Spinlock lock;
};

class NullMessageEvent : public Action
{
public:
    NullMessageEvent(NullRankSyncSerialSkip* skip, int rank) : skip(skip), rank(rank) {}

    void execute(void) override
    {
        // std::cout << "NullMessageEvent: " << rank << std::endl;
        skip->sendData(rank);
    }

private:
    NullRankSyncSerialSkip* skip;
    int                     rank;
};

} // namespace SST

#endif // SST_CORE_SYNC_RANKSYNCSERIALSKIP_H
