/**
 * @file sys_arch.cc
 * @brief lwIP system abstraction implementation (NO_SYS=1)
 * @note The whole of lwIP's OS dependency on this port: the millisecond
 *       clock and the assert halt. No threads, mailboxes, semaphores or
 *       mutexes exist, because NO_SYS=1 compiles all of that out of lwIP.
 *
 *       Everything is extern "C" because every caller is a lwIP .c file.
 *
 * @author Bex Saw
 */

#include "arch/sys_arch.h"

#include "arch/cc.h"  // lwip_port_assert_failed
#include "delay.h"    // EoT::Utils::get_ms_ticks()

extern "C"
{
    uint32_t sys_now(void)
    {
        return EoT::Utils::get_ms_ticks();
    }

    uint32_t sys_jiffies(void)
    {
        return EoT::Utils::get_ms_ticks();
    }

    void lwip_port_assert_failed(const char* message)
    {
        (void)message;

        // Mask interrupts so SysTick cannot keep the superloop looking
        // alive, then park. Guarded because this file also builds for the
        // native preset (same #ifdef __arm__ split as dcache.cc); on the
        // host the spin alone is halt enough.
#ifdef __arm__
        __asm volatile("cpsid i" ::: "memory");
#endif

        while (true)
        {
        }
    }
}
