/**
 * @file cc.h
 * @brief lwIP compiler/architecture abstraction for arm-none-eabi-gcc on
 *        Cortex-M7
 * @note lwIP includes this from lwip/arch.h into every one of its own C
 *       translation units, so it must be valid C: macros and C
 *       declarations only, no enum class, no namespace. It is also reached
 *       from C++ (any of our headers that include a lwip/*.h), which is
 *       why the one function declaration is wrapped in extern "C" -- the
 *       definition lives in sys_arch.cc and lwIP calls it by its C name.
 *
 *       Three things are answered here:
 *
 *       Integer types: none defined. lwIP 2.2 derives u8_t/u16_t/u32_t
 *       from <stdint.h> itself; redefining them is the classic way to
 *       break a port.
 *
 *       Byte order: Cortex-M7 is little-endian in every STM32 configuration.
 *       lwIP uses BYTE_ORDER to decide whether htons/htonl swap.
 *
 *       Structure packing: lwIP overlays protocol structs directly on
 *       received frames, so the compiler must not pad them. GCC spells
 *       that __attribute__((packed)) on the struct.
 *
 *       Diagnostics: there is no stdio in this build, so LWIP_PLATFORM_DIAG
 *       swallows its printf-style argument list (still parsed, so format
 *       strings stay type-checked, nothing emitted). LWIP_PLATFORM_ASSERT
 *       halts via lwip_port_assert_failed: a tripped lwIP assert means its
 *       internal state is already inconsistent and unwinding back into it
 *       would only corrupt more.
 *
 * @author Bex Saw
 */

#pragma once

#include <stddef.h>
#include <stdint.h>

#ifndef BYTE_ORDER
#define BYTE_ORDER LITTLE_ENDIAN
#endif

#define PACK_STRUCT_BEGIN
#define PACK_STRUCT_STRUCT __attribute__((packed))
#define PACK_STRUCT_END
#define PACK_STRUCT_FIELD(x) x

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @brief Halt handler for a failed LWIP_ASSERT. Does not return
     * @param message The assert text, readable off the stack in a debugger
     */
    void lwip_port_assert_failed(const char* message);

#ifdef __cplusplus
}
#endif

#define LWIP_PLATFORM_DIAG(x) \
    do                        \
    {                         \
    } while (0)

#define LWIP_PLATFORM_ASSERT(x) lwip_port_assert_failed(x)
