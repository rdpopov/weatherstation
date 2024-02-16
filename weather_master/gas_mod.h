#ifndef GAS_MOD_H
#define GAS_MOD_H
#include "st.h"
// includes
#include "MQ7.h"

static modState GAS_state = ModUninitialized; // module state
static MQ7 mq7(36); // connected pin
static float GAS_ppm = -1.0;

/**
 * @brief Gas sensor init method 
 *
 * @return Module state
 */
modState GAS_init() {
    GAS_state = ModOK;
    mq7.calibrate();
    return GAS_state;
}

/**
 * @brief Get reading from gas sensor
 *
 * @return Module state
 */
modState GAS_update(){ 
    GAS_ppm = mq7.readPpm();
    return ModOK;
}

/**
 * @brief Log GAS sensor value
 *
 * @return Module state
 */
modState GAS_log(){ 
#ifdef LOG_MOD_H
    LOG_write_float("CO/ppm", GAS_ppm);
#endif
    return ModOK;
}

#endif // MD_MOD_H
