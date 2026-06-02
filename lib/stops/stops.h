// stop related constants and variables
#define STOPCOUNT 3

#define STOP0 2
#define STOP1 4
#define STOP2 6

#define STOP0LED 3
#define STOP1LED 5
#define STOP2LED 7

#define STOP_OFF 0
#define STOP_ON_0 1
#define STOP_ON_1 2
#define STOP_ON_2 3
#define STOP_ON_MIN1 -1
#define STOP_ON_MIN2 -2

void setupStopLines();
void setupStops();

void scanStops();

// function declarations for the ISRs for the stop lines
void stop0ISR();
void stop1ISR();
void stop2ISR();
void stop3ISR();
void stop4ISR();
void stop5ISR();
void stop6ISR();
void stop7ISR();
void stop8ISR();
void stopISR (int stopNum, int stopShift);