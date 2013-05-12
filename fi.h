#pragma once

// This file is sadness in source code form. It is a forced include that 
// dodges all kinds of terrible and allows LibVNCServer to somewhat compile
// in VS2005. I am not proud of this file. 

#include "stdint.h"

#define in_addr_t unsigned long
#ifndef INET_ADDRSTRLEN
#define socklen_t int
#endif 

#define SHUT_RDWR 2

#include <string.h>

#ifndef LIBVNCSERVER_PACKAGE_NAME 
#define LIBVNCSERVER_PACKAGE_NAME  "LibVNCServer" 
#endif

/* Define to the full name and version of this package. */
#ifndef LIBVNCSERVER_PACKAGE_STRING 
#define LIBVNCSERVER_PACKAGE_STRING  "LibVNCServer 0.9.7" 
#endif

/* Define to the one symbol short name of this package. */
#ifndef LIBVNCSERVER_PACKAGE_TARNAME 
#define LIBVNCSERVER_PACKAGE_TARNAME  "libvncserver" 
#endif

/* Define to the version of this package. */
#ifndef LIBVNCSERVER_PACKAGE_VERSION 
#define LIBVNCSERVER_PACKAGE_VERSION  "0.9.7" 
#endif

#define snprintf _snprintf

int getpid();

#ifndef __cplusplus
typedef int bool;
#define true 1
#define false 0
#endif
