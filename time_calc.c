/* File: time_calc.c
 * Author: M.Underwood E.Ford K.McGregor
 * Date: 2026/04/13
 * Description: This file contains functions for calculating time for the Solar Planetarium simulation.
 */

#include "time_calc.h"

time_t current_time; 

//define months abbreviated to 3 characters to save screen space 
const char* months[12] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};



/* function: formatDate
 * description: formats the time into MONTH/DAY/YEAR 
 * @param: date 
 * @param: buffer 
 * @return: void
 * */
void formatDate(time_t date, char* buffer) { 
    struct tm *tm_info = localtime(&date); 
	//taking current local date and time from date which is set up in timeCalculation below.
    sprintf(buffer, "%s %02d %d",months[tm_info->tm_mon], tm_info->tm_mday,tm_info->tm_year + 1900); 
}

/* function: timeCalculation
 * description: calculates the dates for display from current date 
 * @param: current time
 * @param: pointer to current date 
 * @param: pointer to days in future 
 * @param: pointer to days in past 
 * @param: days to add to current date 
 * @param: days to subtract from current date 
 * @return: void
 * */
void timeCalculation(time_t current_time, char* current_str, char* plus_str, char* minus_str,int plus_days, int minus_days) {
    time_t plus = current_time + (plus_days * SECONDS_IN_DAY); 
    time_t minus = current_time - (minus_days * SECONDS_IN_DAY);

	//run the date formatting for MONTH/DAY/YEAR
    formatDate(current_time, current_str);
    formatDate(plus, plus_str);
    formatDate(minus, minus_str);
} 

