

#include "time_calc.h"

time_t current_time; 

//Define months (time.h has months setup, however they aren't abbreviated to 3 letters to save space).
const char* months[12] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

void formatDate(time_t date, char* buffer) { //function that converts the time into MONTH/DAY/YEAR
    struct tm *tm_info = localtime(&date); //for Edie/Kip. What struct_tm means/references https://en.cppreference.com/w/c/chrono/tm
	//so it's taking current local date and time from date which is set up in timeCalculation below.
    sprintf(buffer, "%s %02d %d",months[tm_info->tm_mon], tm_info->tm_mday,tm_info->tm_year + 1900); 
}

void timeCalculation(time_t current_time, char* current_str, char* plus_str, char* minus_str,int plus_days, int minus_days) {
    time_t plus = current_time + (plus_days * SECONDS_IN_DAY); 
    time_t minus = current_time - (minus_days * SECONDS_IN_DAY);

	//run the date formatting for MONTH/DAY/YEAR
    formatDate(current_time, current_str);
    formatDate(plus, plus_str);
    formatDate(minus, minus_str);
} 

