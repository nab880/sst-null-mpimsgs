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

#ifndef SST_CORE_IMPL_TIMEVORTEX_NULL_TIMEVORTEXPQ_H
#define SST_CORE_IMPL_TIMEVORTEX_NULL_TIMEVORTEXPQ_H

#include "sst/core/eli/elementinfo.h"
#include "timeVortexPQ.h"


#include <functional>
#include <queue>
#include <vector>

namespace SST {

class Output;
class NullRankSyncSerialSkip;

namespace IMPL {



/**
 * Primary Event Queue
 */
class NullMessagePQ : public TimeVortexPQ
{

public:
    SST_ELI_REGISTER_DERIVED(
        TimeVortex,
        NullMessagePQ,
        "sst",
        "timevortex.null_message_priority_queue",
        SST_ELI_ELEMENT_VERSION(1,0,0),
        "Null message TimeVortex based on std::priority_queue.")


    // TimeVortexPQ();
    NullMessagePQ(Params& params);
    ~NullMessagePQ();
    SST_ELI_EXPORT(NullMessagePQ)

    void setNullSkip(NullRankSyncSerialSkip* skip);


    // bool      empty() override;
    //int       size() override;
    //void      insert(Activity* activity) override;
    Activity* pop() override;


    /** Print the state of the TimeVortex */
    //void print(Output& out) const override;

    //uint64_t getCurrentDepth() const override { return current_depth; }
    //uint64_t getMaxDepth() const override { return max_depth; }
private:
    NullRankSyncSerialSkip* null_skip;
/*
private:
    typedef std::priority_queue<Activity*, std::vector<Activity*>, Activity::greater<true, true, true>> dataType_t;

    // Data
    dataType_t data;
    uint64_t   insertOrder;

    // Stats about usage
    uint64_t max_depth;

    // Need current depth to be atomic if we are thread safe
    typename std::conditional<TS, std::atomic<uint64_t>, uint64_t>::type current_depth;

    CACHE_ALIGNED(SST::Core::ThreadSafe::Spinlock, slock);
*/
};


} // namespace IMPL
} // namespace SST

#endif // SST_CORE_IMPL_TIMEVORTEX_TIMEVORTEXPQ_H
