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

#ifndef SST_CORE_IMPL_TIMEVORTEX_TIMEVORTEXPQ_H
#define SST_CORE_IMPL_TIMEVORTEX_TIMEVORTEXPQ_H

#include "sst/core/eli/elementinfo.h"
#include "sst/core/timeVortex.h"

#include <functional>
#include <queue>
#include <vector>

namespace SST {

class Output;

namespace IMPL {

/**
 * Primary Event Queue
 */
template <bool TS>
class TimeVortexPQBase : public TimeVortex
{

public:
    // TimeVortexPQ();
    TimeVortexPQBase(Params& params);
    ~TimeVortexPQBase();

    bool      empty() override;
    int       size() override;
    void      insert(Activity* activity) override;
    Activity* pop() override;
    Activity* front() override;

    /** Print the state of the TimeVortex */
    void print(Output& out) const override;

    uint64_t getCurrentDepth() const override { return current_depth; }
    uint64_t getMaxDepth() const override { return max_depth; }

protected:
    typedef std::priority_queue<Activity*, std::vector<Activity*>, Activity::greater<true, true, true>> dataType_t;

    // Data
    dataType_t data;
    uint64_t   insertOrder;

    // Stats about usage
    uint64_t max_depth;

    // Need current depth to be atomic if we are thread safe
    typename std::conditional<TS, std::atomic<uint64_t>, uint64_t>::type current_depth;

    CACHE_ALIGNED(SST::Core::ThreadSafe::Spinlock, slock);
};

class TimeVortexPQ : public TimeVortexPQBase<false>
{
public:
    SST_ELI_REGISTER_DERIVED(
        TimeVortex,
        TimeVortexPQ,
        "sst",
        "timevortex.priority_queue",
        SST_ELI_ELEMENT_VERSION(1,0,0),
        "TimeVortex based on std::priority_queue.")


    TimeVortexPQ(Params& params) : TimeVortexPQBase<false>(params) {}
    ~TimeVortexPQ() {}
    SST_ELI_EXPORT(TimeVortexPQ)
};

class TimeVortexPQ_ts : public TimeVortexPQBase<true>
{
public:
    SST_ELI_REGISTER_DERIVED(
        TimeVortex,
        TimeVortexPQ_ts,
        "sst",
        "timevortex.priority_queue.ts",
        SST_ELI_ELEMENT_VERSION(1,0,0),
        "Thread safe verion of TimeVortex based on std::priority_queue.  Do not reference this element directly, just specify sst.timevortex.priority_queue and this version will be selected when it is needed based on other parameters.")


    TimeVortexPQ_ts(Params& params) : TimeVortexPQBase<true>(params) {}
    ~TimeVortexPQ_ts() {}
    SST_ELI_EXPORT(TimeVortexPQ_ts)
};


template <bool TS>
TimeVortexPQBase<TS>::TimeVortexPQBase(Params& UNUSED(params)) :
    TimeVortex(),
    insertOrder(0),
    max_depth(0),
    current_depth(0)
{}

template <bool TS>
TimeVortexPQBase<TS>::~TimeVortexPQBase()
{
    // Activities in TimeVortexPQ all need to be deleted
    while ( !data.empty() ) {
        Activity* it = data.top();
        delete it;
        data.pop();
    }
}

template <bool TS>
bool
TimeVortexPQBase<TS>::empty()
{
    if ( TS ) slock.lock();
    auto ret = data.empty();
    if ( TS ) slock.unlock();
    return ret;
}

template <bool TS>
int
TimeVortexPQBase<TS>::size()
{
    if ( TS ) slock.lock();
    auto ret = data.size();
    if ( TS ) slock.unlock();
    return ret;
}

template <bool TS>
void
TimeVortexPQBase<TS>::insert(Activity* activity)
{
    if ( TS ) slock.lock();
    activity->setQueueOrder(insertOrder++);
    data.push(activity);
    current_depth++;
    if ( current_depth > max_depth ) { max_depth = current_depth; }
    if ( TS ) slock.unlock();
}

template <bool TS>
Activity*
TimeVortexPQBase<TS>::pop()
{
    if ( TS ) slock.lock();
    if ( data.empty() ) return nullptr;
    Activity* ret_val = data.top();
    data.pop();
    current_depth--;
    if ( TS ) slock.unlock();
    return ret_val;
}

template <bool TS>
Activity*
TimeVortexPQBase<TS>::front()
{
    if ( TS ) slock.lock();
    auto ret = data.top();
    if ( TS ) slock.unlock();
    return ret;
}

template <bool TS>
void
TimeVortexPQBase<TS>::print(Output& out) const
{
    out.output("TimeVortex state:\n");

    //  STL's priority_queue does not support iteration.
    //
    //    dataType_t::iterator it;
    //    for ( it = data.begin(); it != data.end(); it++ ) {
    //        (*it)->print("  ", out);
    //    }
}

} // namespace IMPL




} // namespace SST

#endif // SST_CORE_IMPL_TIMEVORTEX_TIMEVORTEXPQ_H
