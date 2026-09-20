/**
 * @file sys_arch.h
 * @brief lwIP system abstraction for the NO_SYS=1 bare-metal port
 * @note With NO_SYS=1 lwIP never includes this header itself (lwip/sys.h
 *       only pulls in arch/sys_arch.h on the RTOS path) and needs no
 *       mailbox, semaphore or mutex types from us. The single obligation
 *       left is a monotonic millisecond clock, sys_now(), which lwIP's
 *       timeout wheel reads from sys_check_timeouts().
 *
 *       Only sys_arch.cc includes this, so it is a C++ header (<cstdint>).
 *       The declarations are still extern "C" because the callers are
 *       lwIP's C files (timeouts.c, etharp.c), which look the symbols up
 *       by their unmangled names; without C linkage the link fails.
 *
 * @author Bex Saw
 */

#pragma once

#include <cstdint>

extern "C"
{
    /**
     * @brief Milliseconds since boot, as lwIP's timeout wheel sees time
     * @return Free-running 32-bit millisecond counter
     * @note Backed by EoT::Utils::get_ms_ticks(), the 1 kHz SysTick. Wrap
     *       at ~49.7 days is expected: lwIP only compares timestamps as
     *       unsigned differences.
     */
    uint32_t sys_now(void);

    /**
     * @brief Free-running tick counter, same source as sys_now()
     * @note lwIP allows a finer counter here but never requires one. Used
     *       only for ARP/DHCP transaction-id entropy.
     */
    uint32_t sys_jiffies(void);
}
