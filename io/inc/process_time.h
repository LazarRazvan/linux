#ifndef PROCESS_TIME_H
#define PROCESS_TIME_H

// Timer init
void process_time_init(void);

// Register timer
int process_time_register(void);

// Start timer
int process_time_start(int);

// Stop timer
int process_time_end(int);

// Release timer
int process_time_release(int);

#endif	// PROCESS_TIME_H

