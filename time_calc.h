#ifndef _TIME_CALC_H_
#define _TIME_CALC_H_

#include <time.h>
#include <stdio.h>



#define SECONDS_IN_DAY 86400 //60*60*24

// Expose month names (optional: make extern to avoid duplication)
extern const char* months[12];

// Function declarations
void formatDate(time_t current_date, char* buffer);

void timeCalculation(
time_t current_time,
    char* current_str,
    char* plus_str,
    char* minus_str,
    int plus_days,
    int minus_days
);

void simulatedCalculation (char *simulated_str, int simulated_value); // replace with our own code ?

#endif
