#include "dcache.h"

#ifdef __arm__
#include "stm32h723xx.h"
#endif

// This process of clean cache an everything happen at the CPU level
// This is what we intereact with the CPU level function to clean, invalidate, flush all, flush range

namespace EoT::Utils
{

bool DCache::clean()
{

#ifdef __arm__
    SCB_CleanDCache();
#endif

    return true;
}

bool DCache::invalidate()
{

#ifdef __arm__
    SCB_InvalidateDCache();
#endif

    return true;
}

bool DCache::flush_all()
{

#ifdef __arm__
    SCB_CleanInvalidateDCache();
#endif

    return true;
}

bool DCache::flush_range(uint32_t start_addr, uint32_t end_addr)
{

#ifdef __arm__
    SCB_CleanInvalidateDCache_by_Addr(
        reinterpret_cast<uint32_t*>(start_addr),
        static_cast<int32_t>(end_addr - start_addr));
#endif

    return true;
}

}  // namespace EoT::Utils
