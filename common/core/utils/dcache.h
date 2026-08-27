/**
 * @brief A simple data cache implementation.
 * @author Bex Sawetrattanathumrong 
 * @note This is a simple utils file that help support the EthDma model 
 *       When you want to flush, clean, invalidate certain thing this is where it's being handled
 */

#pragma once

#include <cstddef>
#include <cstdint>

namespace EoT::Utils
{

class DCache
{
public:
    /**
    * @brief Cleans the data cache by writing back any modified cache lines to main memory.
    * @return true if the operation was successful, false otherwise.
    */
    bool clean();

    /**
    * @brief Invalidates the data cache by removing all cache lines from the cache.
    * @return true if the operation was successful, false otherwise.
    */
    bool invalidate();

    /**
    * @brief Flushes all data from the cache to main memory.
    * @note This operation will write back all modified cache lines to main memory and then invalidate the cache.
    * @return true if the operation was successful, false otherwise.
    */
    bool flush_all();

    /**
    * @brief Flushes a range of data from the cache to main memory.
    * @param start_addr The starting address of the range to flush.
    * @param end_addr The ending address of the range to flush.
    * @return true if the operation was successful, false otherwise.
    */
    bool flush_range(uint32_t start_addr, uint32_t end_addr);
};
}  // namespace EoT::Utils
