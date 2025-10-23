#ifndef _LWIPOPTS_H
#define _LWIPOPTS_H

// Required for Pico W builds
#define NO_SYS                      0
#define LWIP_SOCKET                 0
#define LWIP_NETCONN                0

// Enable DHCP if needed (harmless even if unused)
#define LWIP_DHCP                   1

// Needed for threadsafe background mode
#define SYS_LIGHTWEIGHT_PROT        1
#define LWIP_NETIF_API              1

#define LWIP_STATS                  0
#define LWIP_STATS_DISPLAY          0
#define LWIP_PROVIDE_ERRNO          1

#endif /* _LWIPOPTS_H */
