#include <Arduino.h>

// keyboard related constants and variables
#define KEY_PINMODE OUTPUT
#define GROUPLINE_PINMODE INPUT_PULLUP

#define KEYCOUNT 64
#define GROUPLINE0 53
#define GROUPLINE1 52
#define GROUPLINE2 51
#define GROUPLINE3 50
#define GROUPLINE4 49 
#define GROUPLINE5 48
#define GROUPLINE6 47
#define GROUPLINE7 46
#define KEY0 41
#define KEY1 44
#define KEY2 42
#define KEY3 45
#define KEY4 43
#define KEY5 39
#define KEY6 38
#define KEY7 40

void setupKeylines();
void setupGroupLines();

// scan loop declarations
void scanKeyboard();