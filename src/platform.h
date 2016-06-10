#ifndef __EP_PLATFORM_H__
#define __EP_PLATFORM_H__

#ifdef __APPLE__
#include <AvailabilityMacros.h>
#endif

#ifdef __linux__
#define EP_HAVE_EPOLL 1
#endif

#if (defined(__APPLE__) && defined(MAC_OS_X_VERSION_10_6)) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined (__NetBSD__)
#define EP_HAVE_KQUEUE 1
#endif

#endif //!__EP_PLATFORM_H__
