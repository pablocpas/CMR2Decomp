#ifndef _CMR2_LAYOUT_CHECKS_H
#define _CMR2_LAYOUT_CHECKS_H

// These conditions describe the original Win32 ABI. Native pointer-bearing
// runtime records grow on 64-bit builds; on-disk formats still need explicit
// separation in the port, even when this original-layout check is inactive.
#ifndef CMR2_LAYOUT_CHECK
#if defined(_M_IX86) || defined(__i386__)
#define CMR2_LAYOUT_CHECK(name, condition) typedef char name[condition ? 1 : -1]
#else
#define CMR2_LAYOUT_CHECK(name, condition)
#endif
#endif

#endif
