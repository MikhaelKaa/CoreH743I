/**
 * @file memory_man.h
 * @brief Memory utilities
 * @author Mikhael Kaa (Михаил Каа)
 * @date 05.09.2025
 */

#ifndef _MEM_MAN_
#define _MEM_MAN_

#include "dev_interface.h"

// uCMD handler for mem_dump.
int ucmd_mem(int argc, char **argv);

// Set memory interface
int mem_set_interface(interface_t* iface);

#endif /* _MEM_MAN_ */