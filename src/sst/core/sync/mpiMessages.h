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

#ifndef SST_CORE_MESSAGES
#define SST_CORE_MESSAGES


#include "sst_config.h"
#include "sst/core/event.h"
#include "sst/core/exit.h"
#include "sst/core/link.h"
#include "sst/core/profile.h"
#include "sst/core/serialization/serializer.h"
#include "sst/core/simulation_impl.h"
#include "sst/core/stopAction.h"
#include "sst/core/sync/syncQueue.h"
#include "sst/core/timeConverter.h"
#include "sst/core/warnmacros.h"
#include "sst/core/activityQueue.h"
#include "sst/core/threadsafe.h"
#include "sst/core/sync/nullRankSyncSerialSkip.h"
#define SHORT_MODE 0
#define RESIZE_MODE 1
#define LONG_MODE 2
#define SHORT_INIT_TAG 1
#define RESIZE_INIT_TAG 2
#define LONG_INIT_TAG 3
#define MAX_MSG 16000
#include "sst/core/sync/mpiMessage.h"
#include "sst/core/sync/mpiShortMessage.h"
#include "sst/core/sync/mpiResizeMessage.h"
#include "sst/core/sync/mpiLongMessage.h"
#ifdef SST_CONFIG_HAVE_MPI
DISABLE_WARN_MISSING_OVERRIDE
#include <mpi.h>
REENABLE_WARNING
#define UNUSED_WO_MPI(x) x
#else
#define UNUSED_WO_MPI(x) UNUSED(x)
#endif


#endif // SST_CORE_MESSAGES
