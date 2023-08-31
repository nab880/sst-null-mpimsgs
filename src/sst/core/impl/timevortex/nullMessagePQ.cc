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

#include "sst/core/impl/timevortex/nullMessagePQ.h"

#include "sst/core/clock.h"
#include "sst/core/output.h"
#include "sst/core/sync/nullRankSyncSerialSkip.h"

namespace SST {
namespace IMPL {

NullMessagePQ::NullMessagePQ(Params& params) : TimeVortexPQ(params), null_skip(nullptr)
{
    // std::cout << "NullMessagePQ::NullMessagePQ()" << std::endl;
}

NullMessagePQ::~NullMessagePQ()
{


    // Activities in TimeVortexPQ all need to be deleted
    while ( !data.empty() ) {
        Activity* it = data.top();
        delete it;
        data.pop();
    }
}

void
NullMessagePQ::setNullSkip(NullRankSyncSerialSkip* skip)
{
    null_skip = skip;
}

Activity*
NullMessagePQ::pop()
{
    // uint32_t my_rank = Simulation_impl::getSimulation()->getRank().rank;

    assert(null_skip);

    null_skip->receiveData(false);

    null_skip->calculateSafeTime();

    Activity* ret_val = data.top();

    while ( ret_val->getDeliveryTime() > null_skip->getSafeTime() ) {
        // std::cout << ret_val->getDeliveryTime() << " " << null_skip->getSafeTime() << std::endl;
        null_skip->receiveData(true);
        null_skip->calculateSafeTime();
        ret_val = data.top();
    }
    null_skip->testSendComplete();

    data.pop();
    current_depth--;

    return ret_val;
    /*
    if ( TS ) slock.lock();
    if ( data.empty() ) return nullptr;
    Activity* ret_val = data.top();
    data.pop();
    current_depth--;
    if ( TS ) slock.unlock();
    return ret_val;
    */
}

} // namespace IMPL
} // namespace SST
