/**
 * @file
 * @brief utils library all modules should share all
 */

#ifndef ST_H
#define ST_H
#include <mutex>
enum modState {
    ModOK = 0,
    ModUninitialized,
    ModError
};
#endif // ST_H
