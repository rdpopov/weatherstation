// module template
#ifndef MD_MOD_H
#define MD_MOD_H
#include "st.h"
// includes
// interface
// variables
static modState MD_state; // module state
// functuions

// ------------------------
// base interface 
// ------------------------

modState MD_init() { 
    MD_state = ModOK;
    return MD_state;
}

modState MD_update(){ 
    return ModOK;
}

int MD_get(){ 
    return 0;
}

modState MD_display(){ 
#ifdef DISPLAY_MOD_H
    // do the display part
#endif
    Serial.printf("MD:%d,",0);
    return ModOK;
}

modState MD_log(){ 
#ifdef LOG_MOD_H
    // do the log part
#endif
    return ModOK;
}

#endif // MD_MOD_H
