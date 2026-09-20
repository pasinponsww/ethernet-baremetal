/**
 * @file lwipopts.h
 * @brief lwIP compile-time configuration for the EoT bare-metal port
 * @note lwIP pulls this in from lwip/opt.h. Every option left undefined
 *       falls back to the default in lwip/opt.h, so only deviations from
 *       those defaults belong here.
 *
 *       Why macros and not an enum or a class: lwIP is C and tests these
 *       symbols in its own sources with the preprocessor (#if LWIP_TCP,
 *       #if NO_SYS, MEMP_NUM_* inside array declarators). They have to be
 *       preprocessor symbols. What can be done is group them, which the
 *       sections below do; the grouping is documented once, here.
 *
 *       System: NO_SYS=1. No RTOS, no threads, no protection primitives.
 *       lwIP is a passive library driven entirely from NetIf::poll() on
 *       the superloop, so only the raw/callback API exists; the sequential
 *       (netconn) and BSD socket APIs are compiled out, and nothing in this
 *       tree may call a blocking lwIP function.
 *
 *       Memory: lwIP's own pool allocator, never newlib malloc (the
 *       linker script reserves 0x200 bytes of heap). MEM_SIZE is the heap
 *       PBUF_RAM (transmit) pbufs are carved from. PBUF_POOL_BUFSIZE must
 *       hold a whole untagged 1518-byte frame because NetIf hands lwIP
 *       one linear buffer per received frame and never builds a chain;
 *       1524 is 1518 rounded up to MEM_ALIGNMENT. The MEMP_NUM_* counts
 *       size the fixed pools: TCP_PCB must cover every TcpServer<T>'s
 *       kMaxConnections plus its listener, and SYS_TIMEOUT must leave
 *       headroom because running out makes lwIP silently stop keeping
 *       time.
 *
 *       Protocols: IPv4 only. ICMP so the board answers ping (the first
 *       useful smoke test). TCP and UDP for the packet/ transports. DHCP
 *       gated on the link flag NetIf drives, so discovery does not start
 *       before the PHY reports link.
 *
 *       TCP: MSS is the 1500-byte MTU minus the 40-byte IPv4+TCP header.
 *       Send buffer and window are four segments each.
 *
 *       netif: hostname for the DHCP client-id option, link/status
 *       callbacks so DHCP restarts on a replugged cable, and a small ARP
 *       table with the outgoing entry cached.
 *
 *       Checksums: all in software. Nothing sets up the MAC's checksum
 *       offload in the DMA descriptors yet; flip the GEN_/CHECK_ pairs off
 *       only once descriptor-level offload is wired up in the driver.
 *
 *       Diagnostics: stats and debug off (flash and cycles), asserts on (a
 *       tripped one is a real bug). There is no stdio in this build; see
 *       arch/cc.h for where LWIP_PLATFORM_DIAG and LWIP_PLATFORM_ASSERT go.
 *
 * @author Bex Saw
 */

#pragma once

/*                                                                */
/*                      System / threading                        */
/*                                                                */

#define NO_SYS 1
#define SYS_LIGHTWEIGHT_PROT 0
#define LWIP_NETCONN 0
#define LWIP_SOCKET 0

/*                                                                */
/*                      Memory                                    */
/*                                                                */

#define MEM_ALIGNMENT 4
#define MEM_LIBC_MALLOC 0
#define MEMP_MEM_MALLOC 0
#define MEM_SIZE (16 * 1024)

#define PBUF_POOL_SIZE 16
#define PBUF_POOL_BUFSIZE 1524

#define MEMP_NUM_PBUF 16
#define MEMP_NUM_UDP_PCB 6
#define MEMP_NUM_TCP_PCB 8
#define MEMP_NUM_TCP_PCB_LISTEN 4
#define MEMP_NUM_TCP_SEG 16
#define MEMP_NUM_REASSDATA 4
#define MEMP_NUM_ARP_QUEUE 8
#define MEMP_NUM_SYS_TIMEOUT 10

/*                                                                */
/*                      Protocol layers                           */
/*                                                                */

#define LWIP_IPV4 1
#define LWIP_IPV6 0

#define LWIP_ETHERNET 1
#define LWIP_ARP 1
#define LWIP_ICMP 1
#define LWIP_UDP 1
#define LWIP_TCP 1
#define LWIP_RAW 1
#define LWIP_DHCP 1
#define LWIP_AUTOIP 0
#define LWIP_IGMP 0
#define LWIP_DNS 0

#define LWIP_DHCP_CHECK_LINK_UP 1

/*                                                                */
/*                      TCP tuning                                */
/*                                                                */

#define TCP_MSS 1460
#define TCP_SND_BUF (4 * TCP_MSS)
#define TCP_WND (4 * TCP_MSS)
#define TCP_SND_QUEUELEN ((4 * TCP_SND_BUF) / TCP_MSS)

/*                                                                */
/*                      netif                                     */
/*                                                                */

#define LWIP_NETIF_HOSTNAME 1
#define LWIP_NETIF_LINK_CALLBACK 1
#define LWIP_NETIF_STATUS_CALLBACK 1
#define ETHARP_TABLE_SIZE 10
#define LWIP_NETIF_TX_SINGLE_PBUF 1

/*                                                                */
/*                      Checksums                                 */
/*                                                                */

#define CHECKSUM_GEN_IP 1
#define CHECKSUM_GEN_UDP 1
#define CHECKSUM_GEN_TCP 1
#define CHECKSUM_GEN_ICMP 1
#define CHECKSUM_CHECK_IP 1
#define CHECKSUM_CHECK_UDP 1
#define CHECKSUM_CHECK_TCP 1
#define CHECKSUM_CHECK_ICMP 1

/*                                                                */
/*                      Diagnostics                               */
/*                                                                */

#define LWIP_STATS 0
#define LWIP_STATS_DISPLAY 0
#define LWIP_DEBUG 0
#define LWIP_NOASSERT 0
