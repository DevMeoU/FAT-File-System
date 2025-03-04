#ifndef TEST_H
#define TEST_H

#include <stdio.h>
#include <assert.h>
#include <time.h>

/**
 * @brief Test IP Driver
 * 
 * @return 0 if success, -1 otherwise
 */
int test_ip_driver(void);


/**
 * @brief Measure function execution time
 * 
 * @param args Function to measure
 * @return Time in milliseconds
 */
int measure_function(int (*args)(void));

#endif // TEST_H

