#ifndef __SPACEMESH_APP_MENU_H__
#define __SPACEMESH_APP_MENU_H__

#include <os_io_seproxyhal.h>

#if defined(TARGET_NANOS)
extern const ux_menu_entry_t menu_main[4];
#elif defined(TARGET_NANOX)
extern const ux_flow_step_t* const ux_idle_flow [];
#endif

#endif // __SPACEMESH_APP_MENU_H__
