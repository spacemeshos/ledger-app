#include "common.h"
#include "assert.h"

// In DEBUG we need to keep going
// because rendering on display takes multiple SEPROXYHAL
// exchanges until it renders the display
void assert(
    int cond,
    const char* msgStr
#ifdef RESET_ON_CRASH
    MARK_UNUSED
#endif
)
{
    if (cond) return; // everything holds
#ifdef RESET_ON_CRASH
    io_seproxyhal_se_reset();
#else
    ui_displayPaginatedText("Assertion failed", msgStr, NULL);
    THROW(ERR_ASSERT);
#endif
}
