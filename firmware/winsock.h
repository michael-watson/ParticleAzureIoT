 

include/winsock.h Microsoft Invisible Computing 


/* Copyright (c) Microsoft Corporation. All rights reserved. */
/* WINSOCK.H--definitions to be used with the WINSOCK.DLL
 *
 * This header file corresponds to version 1.1 of the Windows Sockets
 * specification.
 *
 * This file includes parts which are Copyright (c) 1982-1986 Regents
 * of the University of California.  All rights reserved.  The
 * Berkeley Software License Agreement specifies the terms and
 * conditions for redistribution.
 *
 * Change log:
 *
 * Fri Apr 23 16:31:01 1993  Mark Towfiq  (towfiq@Microdyne.COM)
 *      New version from David Treadwell which adds extern "C" around
 *      __WSAFDIsSet() and removes "const" from buf param of
 *      WSAAsyncGetHostByAddr().  Added change log.
 *
 * Sat May 15 10:55:00 1993 David Treadwell (davidtr@microsoft.com)
 *  Fix the IN_CLASSC macro to account for class-D multicasts.
 *  Add AF_IPX == AF_NS.
 *
 * Wed Sep 14 12:34:00 1994 Alessandro Forin (sandrof@microsoft.com)
 *  Reduced for MMLITE use (no MS extensions).
 *  Upgrade to BSD4.4 pending.
 */

#ifndef _WINSOCKAPI_
#define _WINSOCKAPI_

//#include <mmmach.h>     /* get UINT16 etc */


#ifdef i386
#define PASCAL __stdcall
#define WINAPI __stdcall
#else
#define PASCAL
#define WINAPI
#endif

/*
 * Basic system type definitions, taken from the BSD file sys/types.h.
 */
typedef unsigned char   u_char;
typedef unsigned short  u_short;
typedef unsigned int    u_int;
typedef unsigned long   u_long;

/*
 * The new type to be used in all
 * instances which refer to sockets.
 */
typedef u_int           SOCKET;

/*
 * Select uses arrays of SOCKETs.  These macros manipulate such
 * arrays.  FD_SETSIZE may be defined by the user before including
 * this file, but the default here should be >= 64.
 *
 * CAVEAT IMPLEMENTOR and USER: THESE MACROS AND TYPES MUST BE
 * INCLUDED IN WINSOCK.H EXACTLY AS SHOWN HERE.
 */
#ifndef FD_SETSIZE
#define FD_SETSIZE      64
#endif /* FD_SETSIZE */

typedef struct fd_set {
        u_short fd_count;               /* how many are SET? */
        SOCKET  fd_array[FD_SETSIZE];   /* an array of SOCKETs */
} fd_set;

#ifdef __cplusplus
extern "C" {
#endif

extern int PASCAL __WSAFDIsSet(SOCKET, fd_set *);

#ifdef __cplusplus
}
#endif

#define FD_CLR(fd, set) do { \
    u_int __i; \
    for (__i = 0; __i < ((fd_set *)(set))->fd_count ; __i++) { \
        if (((fd_set *)(set))->fd_array[__i] == fd) { \
            while (__i < ((fd_set *)(set))->fd_count-1) { \
                ((fd_set *)(set))->fd_array[__i] = \
                    ((fd_set *)(set))->fd_array[__i+1]; \
                __i++; \
            } \
            ((fd_set *)(set))->fd_count--; \
            break; \
        } \
    } \
} while(set == 0)

#define FD_SET(fd, set) do { \
    if (((fd_set *)(set))->fd_count < FD_SETSIZE) \
        ((fd_set *)(set))->fd_array[((fd_set *)(set))->fd_count++]=(fd);\
} while(set == 0)

#define FD_ZERO(set) (((fd_set *)(set))->fd_count=0)

#define FD_ISSET(fd, set) __WSAFDIsSet((SOCKET)(fd), (fd_set *)(set))

/*
 * Structure used in select() call, taken from the BSD file sys/time.h.
 */
struct timeval {
        long    tv_sec;         /* seconds */
        long    tv_usec;        /* and microseconds */
};

/*
 * Operations on timevals.
 *
 * NB: timercmp does not work for >= or <=.
 */
#define timerisset(tvp)         ((tvp)->tv_sec || (tvp)->tv_usec)
#define timercmp(tvp, uvp, cmp) \
        ((tvp)->tv_sec cmp (uvp)->tv_sec || \
         (tvp)->tv_sec == (uvp)->tv_sec && (tvp)->tv_usec cmp (uvp)->tv_usec)
#define timerclear(tvp)         (tvp)->tv_sec = (tvp)->tv_usec = 0

/*
 * Commands for ioctlsocket(),  taken from the BSD file fcntl.h.
 *
 *
 * Ioctl's have the command encoded in the lower word,
 * and the size of any in or out parameters in the upper
 * word.  The high 2 bits of the upper word are used
 * to encode the in/out status of the parameter; for now
 * we restrict parameters to at most 128 bytes.
 */
#define IOCPARM_MASK    0x7ff            /* parameters must be < 2k bytes */
#define IOCGROUP(x)     (((x) >> 8) & 0xff)
#define IOC_VOID        0x20000000      /* no parameters */
#define IOC_OUT         0x40000000      /* copy out parameters */
#define IOC_IN          0x80000000      /* copy in parameters */
#define IOC_INOUT       (IOC_IN|IOC_OUT)
#define _IOC(inout,group,num,len) \
        (inout | ((len & IOCPARM_MASK) << 16) | ((group) << 8) | (num))
#define _IO(g,n)        _IOC(IOC_VOID,  (g), (n), 0)
#define _IOR(g,n,t)     _IOC(IOC_OUT,   (g), (n), sizeof(t))
#define _IOW(g,n,t)     _IOC(IOC_IN,    (g), (n), sizeof(t))
/* this should be _IORW, but stdio got there first */
#define _IOWR(g,n,t)    _IOC(IOC_INOUT, (g), (n), sizeof(t))

/* instead of ioctl.h */
#define IOCPARM_MASK    0x7ff            /* parameters must be < 2k bytes */
#define IOCGROUP(x)     (((x) >> 8) & 0xff)
#define IOC_VOID        0x20000000      /* no parameters */
#define IOC_OUT         0x40000000      /* copy out parameters */
#define IOC_IN          0x80000000      /* copy in parameters */
#define _IOC(inout,group,num,len) \
        (inout | ((len & IOCPARM_MASK) << 16) | ((group) << 8) | (num))
#define _IO(g,n)        _IOC(IOC_VOID,  (g), (n), 0)
#define _IOR(g,n,t)     _IOC(IOC_OUT,   (g), (n), sizeof(t))
#define _IOW(g,n,t)     _IOC(IOC_IN,    (g), (n), sizeof(t))
/* this should be _IORW, but stdio got there first */


/* Socket I/O Controls */
#define SIOCSHIWAT  _IOW('s',  0, u_long)  /* set high watermark */
#define SIOCGHIWAT  _IOR('s',  1, u_long)  /* get high watermark */
#define SIOCSLOWAT  _IOW('s',  2, u_long)  /* set low watermark */
#define SIOCGLOWAT  _IOR('s',  3, u_long)  /* get low watermark */
#define SIOCATMARK  _IOR('s',  7, u_long)  /* at oob mark? */

#define SIOCSPGRP       _IOW('s',  8, int)              /* set process group */
#define SIOCGPGRP       _IOR('s',  9, int)              /* get process group */

#define SIOCSIFADDR     _IOW('i', 12, struct ifreq)     /* set ifnet address */
#define OSIOCGIFADDR    _IOWR('i',13, struct ifreq)     /* get ifnet address */
#define SIOCGIFADDR     _IOWR('i',33, struct ifreq)     /* get ifnet address */
#define SIOCSIFDSTADDR  _IOW('i', 14, struct ifreq)     /* set p-p address */
#define OSIOCGIFDSTADDR _IOWR('i',15, struct ifreq)     /* get p-p address */
#define SIOCGIFDSTADDR  _IOWR('i',34, struct ifreq)     /* get p-p address */
#define SIOCSIFFLAGS    _IOW('i', 16, struct ifreq)     /* set ifnet flags */
#define SIOCGIFFLAGS    _IOWR('i',17, struct ifreq)     /* get ifnet flags */
#define OSIOCGIFBRDADDR _IOWR('i',18, struct ifreq)     /* get brdcast addr */
#define SIOCGIFBRDADDR  _IOWR('i',35, struct ifreq)     /* get brdcast addr */
#define SIOCSIFBRDADDR  _IOW('i',19, struct ifreq)      /* set brdcast addr */
#define OSIOCGIFCONF    _IOWR('i',20, struct ifconf)    /* get ifnet list */
#define SIOCGIFCONF     _IOWR('i',36, struct ifconf)    /* get ifnet list */
#define OSIOCGIFNETMASK _IOWR('i',21, struct ifreq)     /* get net addr mask */
#define SIOCGIFNETMASK  _IOWR('i',37, struct ifreq)     /* get net addr mask */
#define SIOCSIFNETMASK  _IOW('i',22, struct ifreq)      /* set net addr mask */
#define SIOCGIFMETRIC   _IOWR('i',23, struct ifreq)     /* get IF metric */
#define SIOCSIFMETRIC   _IOW('i',24, struct ifreq)      /* set IF metric */
#define SIOCDIFADDR     _IOW('i',25, struct ifreq)      /* delete IF addr */
#define SIOCAIFADDR     _IOW('i',26, struct ifaliasreq) /* add/chg IF alias */

#define SIOCADDMULTI    _IOW('i', 49, struct ifreq)     /* add m'cast addr */
#define SIOCDELMULTI    _IOW('i', 50, struct ifreq)     /* del m'cast addr */

#define MMLITE_EXT 1
#ifdef MMLITE_EXT
#define SIOCSIFDEBUG    _IOW('i',27, struct ifreq)      /* set IF debug opt */
#define SIOCGIFSTATS    _IOWR('i',42, struct if_data)   /* get statistics */
#define SIOCIDHCP       _IOW('i',43, struct ifreq)      /* initiate DHCP */
#define SIOCGDHCPINFO   _IOWR('i',44, struct DhcpInfo)  /* get DHCP info */
#define SIOCGDHCPOPTION _IOWR('i',45, struct DhcpOption)  /* get DHCP option */
#define SIOCGDHCPFILE _IOWR('i',46, char[255])  /* get DHCP boot filename */
#define SIOCGDHCPINFOPTR _IOWR('i',47, struct ifreq)/* get pointer to DHCP info */
#endif

#define FIONREAD        _IOR('f', 127, int)     /* get # bytes to read */
#define FIONBIO         _IOW('f', 126, int)     /* set/clear non-blocking i/o */
#define FIOASYNC        _IOW('f', 125, int)     /* set/clear async i/o */

/* end ioctl.h */

/*
 * Structures returned by network data base library, taken from the
 * BSD file netdb.h.  All addresses are supplied in host order, and
 * returned in network order (suitable for use in system calls).
 */

struct  hostent {
        char    * h_name;           /* official name of host */
        char    * * h_aliases;  /* alias list */
        short   h_addrtype;             /* host address type */
        short   h_length;               /* length of address */
        char    * * h_addr_list; /* list of addresses */
#define h_addr  h_addr_list[0]          /* address, for backward compat */
};

/*
 * It is assumed here that a network number
 * fits in 32 bits.
 */
struct  netent {
        char    * n_name;           /* official name of net */
        char    * * n_aliases;  /* alias list */
        int     n_addrtype;             /* net address type */
        u_long  n_net;                  /* network # */
};

struct  servent {
        char    * s_name;           /* official service name */
        char    * * s_aliases;  /* alias list */
        int     s_port;                 /* port # */
        char    * s_proto;          /* protocol to use */
};

struct  protoent {
        char    * p_name;           /* official protocol name */
        char    * * p_aliases;  /* alias list */
        int     p_proto;                /* protocol # */
};

/*
 * Constants and structures defined by the internet system,
 * Per RFC 790, September 1981, taken from the BSD file netinet/in.h.
 */

/*
 * Protocols
 */
#define IPPROTO_IP              0               /* dummy for IP */
#define IPPROTO_ICMP            1               /* control message protocol */
#define IPPROTO_TCP             6               /* tcp */
#define IPPROTO_PUP             12              /* pup */
#define IPPROTO_UDP             17              /* user datagram protocol */
#define IPPROTO_IDP             22              /* xns idp */
#define IPPROTO_ND              77              /* UNOFFICIAL net disk proto */

#define IPPROTO_RAW             255             /* raw IP packet */
#define IPPROTO_MAX             256

/*
 * Port/socket numbers: network standard functions
 */
#define IPPORT_ECHO             7
#define IPPORT_DISCARD          9
#define IPPORT_SYSTAT           11
#define IPPORT_DAYTIME          13
#define IPPORT_NETSTAT          15
#define IPPORT_FTP              21
#define IPPORT_TELNET           23
#define IPPORT_SMTP             25
#define IPPORT_TIMESERVER       37
#define IPPORT_NAMESERVER       42
#define IPPORT_WHOIS            43
#define IPPORT_MTP              57

/*
 * Port/socket numbers: host specific functions
 */
#define IPPORT_TFTP             69
#define IPPORT_RJE              77
#define IPPORT_FINGER           79
#define IPPORT_TTYLINK          87
#define IPPORT_SUPDUP           95

/*
 * UNIX TCP sockets
 */
#define IPPORT_EXECSERVER       512
#define IPPORT_LOGINSERVER      513
#define IPPORT_CMDSERVER        514
#define IPPORT_EFSSERVER        520

/*
 * UNIX UDP sockets
 */
#define IPPORT_BIFFUDP          512
#define IPPORT_WHOSERVER        513
#define IPPORT_ROUTESERVER      520
                                        /* 520+1 also used */

/*
 * Ports < IPPORT_RESERVED are reserved for
 * privileged processes (e.g. root).
 */
#define IPPORT_RESERVED         1024

/*
 * Link numbers
 */
#define IMPLINK_IP              155
#define IMPLINK_LOWEXPER        156
#define IMPLINK_HIGHEXPER       158

/*
 * Internet address (old style... should be updated)
 */
struct in_addr {
    UINT32 s_addr;
#ifdef obsolete
        union {
                struct { u_char s_b1,s_b2,s_b3,s_b4; } S_un_b;
                struct { u_short s_w1,s_w2; } S_un_w;
                u_long S_addr;
        } S_un;
#define s_addr  S_un.S_addr
                                /* can be used for most tcp & ip code */
#define s_host  S_un.S_un_b.s_b2
                                /* host on imp */
#define s_net   S_un.S_un_b.s_b1
                                /* network */
#define s_imp   S_un.S_un_w.s_w2
                                /* imp */
#define s_impno S_un.S_un_b.s_b4
                                /* imp # */
#define s_lh    S_un.S_un_b.s_b3
                                /* logical host */
#endif
};

/*
 * Definitions of bits in internet address integers.
 * On subnets, the decomposition of addresses to host and net parts
 * is done according to subnet mask, not the masks here.
 */
#define IN_CLASSA(i)            (((INT32)(i) & 0x80000000) == 0)
#define IN_CLASSA_NET           0xff000000
#define IN_CLASSA_NSHIFT        24
#define IN_CLASSA_HOST          0x00ffffff
#define IN_CLASSA_MAX           128

#define IN_CLASSB(i)            (((INT32)(i) & 0xc0000000) == 0x80000000)
#define IN_CLASSB_NET           0xffff0000
#define IN_CLASSB_NSHIFT        16
#define IN_CLASSB_HOST          0x0000ffff
#define IN_CLASSB_MAX           65536

#define IN_CLASSC(i)            (((INT32)(i) & 0xe0000000) == 0xc0000000)
#define IN_CLASSC_NET           0xffffff00
#define IN_CLASSC_NSHIFT        8
#define IN_CLASSC_HOST          0x000000ff

#define IN_CLASSD(i)       (((INT32)(i) & 0xf0000000) == 0xe0000000)
#define IN_CLASSD_NET      0xf0000000  /* These ones aren't really */
#define IN_CLASSD_NSHIFT   28          /* net and host fields, but */
#define IN_CLASSD_HOST     0x0fffffff  /* routing needn't know.    */
#define IN_MULTICAST(i)    IN_CLASSD(i)

#define INADDR_ANY              (UINT32)0x00000000
#define INADDR_LOOPBACK         0x7f000001
#define INADDR_BROADCAST        (UINT32)0xffffffff    
#define INADDR_NONE             0xffffffff

/*
 * Socket address, internet style.
 */
struct sockaddr_in {
    UINT8 sin_len;
    UINT8 sin_family;
    UINT16 sin_port;
    struct in_addr sin_addr;
    BYTE sin_zero[8];
#ifdef obsolete
        short   sin_family;
        u_short sin_port;
        struct  in_addr sin_addr;
        char    sin_zero[8];
#endif
};

#define WSADESCRIPTION_LEN      256
#define WSASYS_STATUS_LEN       128

typedef struct WSAData {
        UINT16                  wVersion;
        UINT16                  wHighVersion;
        char                    szDescription[WSADESCRIPTION_LEN+1];
        char                    szSystemStatus[WSASYS_STATUS_LEN+1];
        unsigned short          iMaxSockets;
        unsigned short          iMaxUdpDg;
        char *                  lpVendorInfo;
} WSADATA;

typedef WSADATA *LPWSADATA;

/*
 * Options for use with [gs]etsockopt at the IP level.
 */
#define IP_OPTIONS      1               /* set/get IP per-packet options */

/*
 * Definitions related to sockets: types, address families, options,
 * taken from the BSD file sys/socket.h.
 */

/*
 * This is used instead of -1, since the
 * SOCKET type is unsigned.
 */
#define INVALID_SOCKET  (SOCKET)(~0)
#define SOCKET_ERROR            (-1)

/*
 * Types
 */
#define SOCK_STREAM     1               /* stream socket */
#define SOCK_DGRAM      2               /* datagram socket */
#define SOCK_RAW        3               /* raw-protocol interface */
#define SOCK_RDM        4               /* reliably-delivered message */
#define SOCK_SEQPACKET  5               /* sequenced packet stream */

/*
 * Option flags per-socket.
 */
#define SO_DEBUG        0x0001          /* turn on debugging info recording */
#define SO_ACCEPTCONN   0x0002          /* socket has had listen() */
#define SO_REUSEADDR    0x0004          /* allow local address reuse */
#define SO_KEEPALIVE    0x0008          /* keep connections alive */
#define SO_DONTROUTE    0x0010          /* just use interface addresses */
#define SO_BROADCAST    0x0020          /* permit sending of broadcast msgs */
#define SO_USELOOPBACK  0x0040          /* bypass hardware when possible */
#define SO_LINGER       0x0080          /* linger on close if data present */
#define SO_OOBINLINE    0x0100          /* leave received OOB data in line */

#define SO_DONTLINGER   (u_int)(~SO_LINGER)

/*
 * Additional options.
 */
#define SO_SNDBUF       0x1001          /* send buffer size */
#define SO_RCVBUF       0x1002          /* receive buffer size */
#define SO_SNDLOWAT     0x1003          /* send low-water mark */
#define SO_RCVLOWAT     0x1004          /* receive low-water mark */
#define SO_SNDTIMEO     0x1005          /* send timeout */
#define SO_RCVTIMEO     0x1006          /* receive timeout */
#define SO_ERROR        0x1007          /* get error status and clear */
#define SO_TYPE         0x1008          /* get socket type */

/*
 * TCP options.
 */
#define TCP_NODELAY     0x0001

/*
 * Address families.
 */
#define AF_UNSPEC       0               /* unspecified */
#define AF_UNIX         1               /* local to host (pipes, portals) */
#define AF_INET         2               /* internetwork: UDP, TCP, etc. */
#define AF_IMPLINK      3               /* arpanet imp addresses */
#define AF_PUP          4               /* pup protocols: e.g. BSP */
#define AF_CHAOS        5               /* mit CHAOS protocols */
#define AF_NS           6               /* XEROX NS protocols */
#define AF_ISO          7               /* ISO protocols */
#define AF_OSI          AF_ISO          /* OSI is ISO */
#define AF_ECMA         8               /* european computer manufacturers */
#define AF_DATAKIT      9               /* datakit protocols */
#define AF_CCITT        10              /* CCITT protocols, X.25 etc */
#define AF_SNA          11              /* IBM SNA */
#define AF_DECnet       12              /* DECnet */
#define AF_DLI          13              /* Direct data link interface */
#define AF_LAT          14              /* LAT */
#define AF_HYLINK       15              /* NSC Hyperchannel */
#define AF_APPLETALK    16              /* AppleTalk */
#define AF_ROUTE       17  /* Internal Routing Protocol */
#define AF_LINK        18  /* Link layer interface */
#define pseudo_AF_XTP  19  /* eXpress Transfer Protocol (no AF) */
#define AF_COIP        20  /* connection-oriented IP, aka ST II */
#define AF_CNT         21  /* Computer Network Technology */
#define pseudo_AF_RTIP 22  /* Help Identify RTIP packets */
#define AF_IPX         23  /* Novell Internet Protocol */
#define AF_SIP         24  /* Simple Internet Protocol */
#define pseudo_AF_PIP  25  /* Help Identify PIP packets */

#define AF_MAX         26

#ifndef __netif_h__ /* defined in netif.xml */
/*
 * Structure used by kernel to store most
 * addresses.
 */
struct sockaddr {
        u_char  sa_len;
        u_char  sa_family;
        char    sa_data[14];            /* up to 14 bytes of direct address */
};
#endif

/*
 * Structure used by kernel to pass protocol
 * information in raw sockets.
 */
struct sockproto {
        u_short sp_family;              /* address family */
        u_short sp_protocol;            /* protocol */
};

/*
 * Protocol families, same as address families for now.
 */
#define PF_UNSPEC       AF_UNSPEC
#define PF_UNIX         AF_UNIX
#define PF_INET         AF_INET
#define PF_IMPLINK      AF_IMPLINK
#define PF_PUP          AF_PUP
#define PF_CHAOS        AF_CHAOS
#define PF_NS           AF_NS
#define PF_ISO          AF_ISO
#define PF_OSI          AF_OSI
#define PF_ECMA         AF_ECMA
#define PF_DATAKIT      AF_DATAKIT
#define PF_CCITT        AF_CCITT
#define PF_SNA          AF_SNA
#define PF_DECnet       AF_DECnet
#define PF_DLI          AF_DLI
#define PF_LAT          AF_LAT
#define PF_HYLINK       AF_HYLINK
#define PF_APPLETALK    AF_APPLETALK
#define PF_ROUTE     AF_ROUTE
#define PF_LINK      AF_LINK
#define PF_XTP       pseudo_AF_XTP   /* really just proto family, no AF */
#define PF_COIP      AF_COIP
#define PF_CNT       AF_CNT
#define PF_SIP       AF_SIP
#define PF_IPX       AF_IPX          /* same format as AF_NS */
#define PF_RTIP      pseudo_AF_FTIP  /* same format as AF_INET */
#define PF_PIP       pseudo_AF_PIP

#define PF_MAX          AF_MAX

/*
 * Structure used for manipulating linger option.
 */
struct  linger {
        u_short l_onoff;                /* option on/off */
        u_short l_linger;               /* linger time */
};

/*
 * Level number for (get/set)sockopt() to apply to socket itself.
 */
#define SOL_SOCKET      0xffff          /* options for socket level */

/*
 * Maximum queue length specifiable by listen.
 */
#define SOMAXCONN       5

#define MSG_OOB         0x1             /* process out-of-band data */
#define MSG_PEEK        0x2             /* peek at incoming message */
#define MSG_DONTROUTE   0x4             /* send without using routing tables */

#define MSG_MAXIOVLEN   16

/*
 * Define constant based on rfc883, used by gethostbyxxxx() calls.
 */
#define MAXGETHOSTSTRUCT        1024

/*
 * Define flags to be used with the WSAAsyncSelect() call.
 */
#define FD_READ         0x01
#define FD_WRITE        0x02
#define FD_OOB          0x04
#define FD_ACCEPT       0x08
#define FD_CONNECT      0x10
#define FD_CLOSE        0x20

#ifndef WSABASEERR
/*
 * All Windows Sockets error constants are biased by WSABASEERR from
 * the "normal".
 */
#define WSABASEERR              10000
/*
 * Windows Sockets definitions of regular Microsoft C error constants
 */
#define WSAEINTR                (WSABASEERR+4)
#define WSAEBADF                (WSABASEERR+9)
#define WSAEACCES               (WSABASEERR+13)
#define WSAEFAULT               (WSABASEERR+14)
#define WSAEINVAL               (WSABASEERR+22)
#define WSAEMFILE               (WSABASEERR+24)

/*
 * Windows Sockets definitions of regular Berkeley error constants
 */
#define WSAEWOULDBLOCK          (WSABASEERR+35)
#define WSAEINPROGRESS          (WSABASEERR+36)
#define WSAEALREADY             (WSABASEERR+37)
#define WSAENOTSOCK             (WSABASEERR+38)
#define WSAEDESTADDRREQ         (WSABASEERR+39)
#define WSAEMSGSIZE             (WSABASEERR+40)
#define WSAEPROTOTYPE           (WSABASEERR+41)
#define WSAENOPROTOOPT          (WSABASEERR+42)
#define WSAEPROTONOSUPPORT      (WSABASEERR+43)
#define WSAESOCKTNOSUPPORT      (WSABASEERR+44)
#define WSAEOPNOTSUPP           (WSABASEERR+45)
#define WSAEPFNOSUPPORT         (WSABASEERR+46)
#define WSAEAFNOSUPPORT         (WSABASEERR+47)
#define WSAEADDRINUSE           (WSABASEERR+48)
#define WSAEADDRNOTAVAIL        (WSABASEERR+49)
#define WSAENETDOWN             (WSABASEERR+50)
#define WSAENETUNREACH          (WSABASEERR+51)
#define WSAENETRESET            (WSABASEERR+52)
#define WSAECONNABORTED         (WSABASEERR+53)
#define WSAECONNRESET           (WSABASEERR+54)
#define WSAENOBUFS              (WSABASEERR+55)
#define WSAEISCONN              (WSABASEERR+56)
#define WSAENOTCONN             (WSABASEERR+57)
#define WSAESHUTDOWN            (WSABASEERR+58)
#define WSAETOOMANYREFS         (WSABASEERR+59)
#define WSAETIMEDOUT            (WSABASEERR+60)
#define WSAECONNREFUSED         (WSABASEERR+61)
#define WSAELOOP                (WSABASEERR+62)
#define WSAENAMETOOLONG         (WSABASEERR+63)
#define WSAEHOSTDOWN            (WSABASEERR+64)
#define WSAEHOSTUNREACH         (WSABASEERR+65)
#define WSAENOTEMPTY            (WSABASEERR+66)
#define WSAEPROCLIM             (WSABASEERR+67)
#define WSAEUSERS               (WSABASEERR+68)
#define WSAEDQUOT               (WSABASEERR+69)
#define WSAESTALE               (WSABASEERR+70)
#define WSAEREMOTE              (WSABASEERR+71)

/*
 * Extended Windows Sockets error constant definitions
 */
#define WSASYSNOTREADY          (WSABASEERR+91)
#define WSAVERNOTSUPPORTED      (WSABASEERR+92)
#define WSANOTINITIALISED       (WSABASEERR+93)

/*
 * Error return codes from gethostbyname() and gethostbyaddr()
 * (when using the resolver). Note that these errors are
 * retrieved via WSAGetLastError() and must therefore follow
 * the rules for avoiding clashes with error numbers from
 * specific implementations or language run-time systems.
 * For this reason the codes are based at WSABASEERR+1001.
 * Note also that [WSA]NO_ADDRESS is defined only for
 * compatibility purposes.
 */

#define h_errno         WSAGetLastError()

/* Authoritative Answer: Host not found */
#define WSAHOST_NOT_FOUND       (WSABASEERR+1001)
#define HOST_NOT_FOUND          WSAHOST_NOT_FOUND

/* Non-Authoritative: Host not found, or SERVERFAIL */
#define WSATRY_AGAIN            (WSABASEERR+1002)
#define TRY_AGAIN               WSATRY_AGAIN

/* Non recoverable errors, FORMERR, REFUSED, NOTIMP */
#define WSANO_RECOVERY          (WSABASEERR+1003)
#define NO_RECOVERY             WSANO_RECOVERY

/* Valid name, no data record of requested type */
#define WSANO_DATA              (WSABASEERR+1004)
#define NO_DATA                 WSANO_DATA

/* no address, look for MX record */
#define WSANO_ADDRESS           WSANO_DATA
#define NO_ADDRESS              WSANO_ADDRESS

#endif /* WSABASEERR */


/*
 * Windows Sockets errors redefined as regular Berkeley error constants
 */
#define EWOULDBLOCK             WSAEWOULDBLOCK
#define EINPROGRESS             WSAEINPROGRESS
#define EALREADY                WSAEALREADY
#define ENOTSOCK                WSAENOTSOCK
#define EDESTADDRREQ            WSAEDESTADDRREQ
#define EMSGSIZE                WSAEMSGSIZE
#define EPROTOTYPE              WSAEPROTOTYPE
#define ENOPROTOOPT             WSAENOPROTOOPT
#define EPROTONOSUPPORT         WSAEPROTONOSUPPORT
#define ESOCKTNOSUPPORT         WSAESOCKTNOSUPPORT
#define EOPNOTSUPP              WSAEOPNOTSUPP
#define EPFNOSUPPORT            WSAEPFNOSUPPORT
#define EAFNOSUPPORT            WSAEAFNOSUPPORT
#define EADDRINUSE              WSAEADDRINUSE
#define EADDRNOTAVAIL           WSAEADDRNOTAVAIL
#define ENETDOWN                WSAENETDOWN
#define ENETUNREACH             WSAENETUNREACH
#define ENETRESET               WSAENETRESET
#define ECONNABORTED            WSAECONNABORTED
#define ECONNRESET              WSAECONNRESET
#define ENOBUFS                 WSAENOBUFS
#define EISCONN                 WSAEISCONN
#define ENOTCONN                WSAENOTCONN
#define ESHUTDOWN               WSAESHUTDOWN
#define ETOOMANYREFS            WSAETOOMANYREFS
#define ETIMEDOUT               WSAETIMEDOUT
#define ECONNREFUSED            WSAECONNREFUSED
#define ELOOP                   WSAELOOP
#define ENAMETOOLONG            WSAENAMETOOLONG
#define EHOSTDOWN               WSAEHOSTDOWN
#define EHOSTUNREACH            WSAEHOSTUNREACH
#define ENOTEMPTY               WSAENOTEMPTY
#define EPROCLIM                WSAEPROCLIM
#define EUSERS                  WSAEUSERS
#define EDQUOT                  WSAEDQUOT
#define ESTALE                  WSAESTALE
#define EREMOTE                 WSAEREMOTE

/* Socket function prototypes */

#ifdef __cplusplus
extern "C" {
#endif

SOCKET PASCAL accept (SOCKET s, struct sockaddr *addr,
                          int *addrlen);

int PASCAL bind (SOCKET s, const struct sockaddr *addr, int namelen);

int PASCAL closesocket (SOCKET s);

int PASCAL connect (SOCKET s, const struct sockaddr *name, int namelen);

int PASCAL ioctlsocket (SOCKET s, long cmd, u_long *argp);

int PASCAL getpeername (SOCKET s, struct sockaddr *name,
                            int * namelen);

int PASCAL getsockname (SOCKET s, struct sockaddr *name,
                            int * namelen);

int PASCAL getsockopt (SOCKET s, int level, int optname,
                           char * optval, int *optlen);

u_long PASCAL htonl (u_long hostlong);

u_short PASCAL htons (u_short hostshort);

UINT32 PASCAL inet_addr (const char * cp);

char * PASCAL inet_ntoa (struct in_addr in);
#ifdef _UNICODE
wchar_t * WINAPI _tinet_ntoa (struct in_addr in);
UINT32 WINAPI _tinet_addr (const wchar_t * cp);
#else
#define _tinet_ntoa(_in_) inet_ntoa(_in_)
#define _tinet_addr(_s_) inet_addr(_s_)
#endif

int PASCAL listen (SOCKET s, int backlog);

u_long PASCAL ntohl (u_long netlong);

u_short PASCAL ntohs (u_short netshort);

int PASCAL recv (SOCKET s, char * buf, int len, int flags);

int PASCAL recvfrom (SOCKET s, char * buf, int len, int flags,
                         struct sockaddr *from, int * fromlen);

int PASCAL select (int nfds, fd_set *readfds, fd_set *writefds,
                       fd_set *exceptfds, const struct timeval *timeout);

int PASCAL send (SOCKET s, const char * buf, int len, int flags);

int PASCAL sendto (SOCKET s, const char * buf, int len, int flags,
                       const struct sockaddr *to, int tolen);

int PASCAL setsockopt (SOCKET s, int level, int optname,
                           const char * optval, int optlen);

int PASCAL shutdown (SOCKET s, int how);

SOCKET PASCAL socket (int af, int type, int protocol);

/* Database function prototypes */

struct hostent * PASCAL gethostbyaddr(const char * addr,
                                              int len, int type);

struct hostent * PASCAL gethostbyname(const char * name);

int PASCAL gethostname (char * name, int namelen);
UINT32 gethostaddr(void);

struct servent * PASCAL getservbyport(int port, const char * proto);

struct servent * PASCAL getservbyname(const char * name,
                                              const char * proto);

struct netent * PASCAL getnetbyaddr(u_long net, int type);

struct netent * PASCAL getnetbyname (const char *name);

struct netent * PASCAL getnetent (void);

struct protoent * PASCAL getprotobynumber(int proto);

struct protoent * PASCAL getprotobyname(const char * name);

/* Microsoft Windows Extension function prototypes */

int PASCAL WSAStartup(UINT16 wVersionRequired, LPWSADATA lpWSAData);

int PASCAL WSACleanup(void);

int PASCAL WSAGetLastError(void);

#ifdef __cplusplus
}
#endif

/* Microsoft Windows Extended data types */
typedef struct sockaddr SOCKADDR;
typedef struct sockaddr *PSOCKADDR;
typedef struct sockaddr *LPSOCKADDR;

typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr_in *PSOCKADDR_IN;
typedef struct sockaddr_in *LPSOCKADDR_IN;

typedef struct linger LINGER;
typedef struct linger *PLINGER;
typedef struct linger *LPLINGER;

typedef struct in_addr IN_ADDR;
typedef struct in_addr *PIN_ADDR;
typedef struct in_addr *LPIN_ADDR;

typedef struct fd_set FD_SET;
typedef struct fd_set *PFD_SET;
typedef struct fd_set *LPFD_SET;

typedef struct hostent HOSTENT;
typedef struct hostent *PHOSTENT;
typedef struct hostent *LPHOSTENT;

typedef struct servent SERVENT;
typedef struct servent *PSERVENT;
typedef struct servent *LPSERVENT;

typedef struct protoent PROTOENT;
typedef struct protoent *PPROTOENT;
typedef struct protoent *LPPROTOENT;

typedef struct timeval TIMEVAL;
typedef struct timeval *PTIMEVAL;
typedef struct timeval *LPTIMEVAL;


#endif  /* _WINSOCKAPI_ */


 

�2004 Microsoft Corporation. All rights reserved. Terms of Use Privacy Statement Accessibility End User License Agreement 

 

 

 

 

 

 

 

 

 

 

 

 
