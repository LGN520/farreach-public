#ifndef __LINUX_COMPILER_H
#if 0
/* Disable this check - it no longer makes sense with so many backports
 * due to spectre mitigation
 */
#ifndef HAVE_LINUX_COMPILER_TYPES_H
#error "Please don't include <linux/compiler-gcc.h> directly, include <linux/compiler.h> instead."
#endif
#endif
#endif

#include_next <linux/compiler-gcc.h>

#ifndef __packed
#define __packed __attribute__((packed))
#endif

#ifndef __always_unused
#define __always_unused __attribute__((unused))
#endif
