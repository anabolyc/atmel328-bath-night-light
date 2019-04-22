#define PIN_WHI  6
#define PIN_RED 10
#define PIN_BLU  5
#define PIN_GRN 11

#define PIN_ON0 12
#define PIN_ON1 13

#define PIN_SNH A0

#define ON  LOW
#define OFF HIGH
#define ON_DELAY 1000

enum state {
    BEFORE_ON,
    WHITE_ON,
    POWER_GONE,
    RAINBOW_ON
};