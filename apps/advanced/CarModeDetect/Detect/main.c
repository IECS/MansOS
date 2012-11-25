#include <stdmansos.h>
//#include "user_button.h"
#include <math.h>

#include "bayes.c"

//
// Sample 3D Accelerometer on Zolertia Z1 platform, print output to serial
//
typedef struct Packet_s {
    int16_t accX;
    int16_t accY;
    int16_t accZ;
} Packet_t;

#define MAIN_LOOP_LENGTH 320 // ticks; a little less than 10 ms

#define START_DELAY    1000

#define NUM_SAMPLES 100

int16_t xvector[NUM_SAMPLES], yvector[NUM_SAMPLES], zvector[NUM_SAMPLES];
uint16_t vectorPos;
//int16_t prevxvector[NUM_SAMPLES], prevyvector[NUM_SAMPLES], prevzvector[NUM_SAMPLES];
float prevavgxvector[NUM_SAMPLES / 10], prevavgyvector[NUM_SAMPLES / 10], prevavgzvector[NUM_SAMPLES / 10];
int8_t past[3];

bool bayesStandingMode = true;

// 1g acceleration
#define ONE_G 232

#define THRESH_LOW    (ONE_G / 25)
#define THRESH_MEDIUM (2 * ONE_G / 25)
#define THRESH_HIGH   (ONE_G / 5)

// # ground vector (length ~ 1g)
float groundVector[3] = {108.277044855, 13.5725593668, 210.928759894};

// # fwd vector 3, scaled to length = 1
float fwdVector[3] = {0.9093539895608515, 0.07775100794703511, -0.408693164162288};

static float vlength(float vector[3]) {
    return sqrtf(vector[0] * vector[0] + vector[1] * vector[1] + vector[2] * vector[2]);
}

#if 0
static void normalize(float vector[3]) {
    float invlength = 1 / vlength(vector);
    vector[0] *= invlength;
    vector[1] *= invlength;
    vector[2] *= invlength;
}
#endif

static void normalizeQuick(float vector[3], float length) {
    float invlength = 1 / length;
    vector[0] *= invlength;
    vector[1] *= invlength;
    vector[2] *= invlength;
}

// vectors must be normalized to length=1 !
static float cosangle(float vector1[3], float vector2[3])
{
    return vector1[0] * vector2[0] + vector1[1] * vector2[1] + vector1[2] * vector2[2];
}

// two vectors are in the same direction?
static bool sameDirection(float vector1[3], float vector2[3])
{
    // sqrt(2) / 2
    return cosangle(vector1, vector2) >= 0.707106;
}

// two vectors are in the inverse direction?
static bool inverseDirection(float vector1[3], float vector2[3])
{
    // -sqrt(2) / 2
    return cosangle(vector1, vector2) <= -0.707106;
}

// ------------------------------------------------------

static float average(int16_t vector[NUM_SAMPLES])
{
    int32_t sum = 0;
    uint16_t i;
    for (i = 0; i < NUM_SAMPLES; ++i) {
        sum += vector[i];
    }
    return sum * 0.01;
}

static float average32(uint32_t vector[NUM_SAMPLES])
{
    uint32_t sum = 0;
    uint16_t i;
    for (i = 0; i < NUM_SAMPLES; ++i) {
        sum += vector[i];
    }
    return sum * 0.01;
}

static float stdev(int16_t vector[NUM_SAMPLES])
{
    static uint32_t squares[NUM_SAMPLES];
    uint16_t i;
    for (i = 0; i < NUM_SAMPLES; ++i) {
        squares[i] = ((int32_t)vector[i]) * vector[i];
    }
    float avg = average(vector);
    float avgSq = average32(squares);
    return sqrtf(avgSq - avg * avg);
}

// static float calcDiff(int16_t vector1[], int16_t vector2[])
// {
//     const uint16_t alen = NUM_SAMPLES / 10;
//     float v1[alen];
//     float v2[alen];
//     int32_t i, sum1, sum2;
//     for (i = 0, sum1 = 0, sum2 = 0; i < NUM_SAMPLES; ++i) {
//         sum1 += vector1[i];
//         sum2 += vector2[i];
//         if (i % 10 == 9) {
//             v1[i / 10] = sum1 * 0.1;
//             v2[i / 10] = sum2 * 0.1;
//             sum1 = sum2 = 0;
//         }
//     }

//     float result = 0;
//     for (i = 0; i < alen; ++i) {
//         float t = v1[i] - v2[i];
//         result += t * t;
//     }
//     return result * 0.1;
// }

static float calcDiff(int16_t vector[NUM_SAMPLES], float prevvector[NUM_SAMPLES / 10])
{
    float v[NUM_SAMPLES / 10];
    int32_t i, sum;
    for (i = 0, sum = 0; i < NUM_SAMPLES; ++i) {
        sum += vector[i];
        if (i % 10 == 9) {
            v[i / 10] = sum * 0.1;
            sum = 0;
        }
    }

    float result = 0;
    for (i = 0; i < NUM_SAMPLES / 10; ++i) {
          float t = v[i] - prevvector[i];
          prevvector[i] = v[i];
          result += t * t;
    }
    return result;
}

static void clearGlobalVectors(void)
{
    // memcpy(prevxvector, xvector, sizeof(prevxvector));
    // memcpy(prevyvector, yvector, sizeof(prevyvector));
    // memcpy(prevzvector, zvector, sizeof(prevzvector));
    memset(xvector, 0, sizeof(xvector));
    memset(yvector, 0, sizeof(yvector));
    memset(zvector, 0, sizeof(zvector));
}

// ------------------------------------------------------

static void calcNewProb(float sdev, bool wasMoving, bool pastBack,
                        bool pastFwd, bool nowFwd, bool diffFromPrevious)
{
    static int8_t fullBNVariables[TOTAL_STATES];
    fullBNVariables[MOVING] = NOT_SET;
    fullBNVariables[WAS_MOVING] = wasMoving;
    fullBNVariables[PAST_BRAKE] = pastBack;
    fullBNVariables[PAST_ACCEL] = pastFwd;
    fullBNVariables[ACCEL] = nowFwd;
    fullBNVariables[SSD] = sdev >= THRESH_LOW;
    fullBNVariables[MSD] = sdev >= THRESH_MEDIUM;
    fullBNVariables[LSD] = sdev >= THRESH_HIGH;
    fullBNVariables[DIFF] = diffFromPrevious;

    float probability = fullBNConditionalProbability(MOVING, fullBNVariables);

//    PRINTF("probability=%u%%\n\n", (uint16_t)(probability * 100));

    bayesStandingMode = probability < 0.5;

    if (bayesStandingMode) redLedOff();
    else redLedOn();

    clearGlobalVectors();
}

static void addMeasurement(Packet_t *packet)
{
    xvector[vectorPos] = packet->accX;
    yvector[vectorPos] = packet->accY;
    zvector[vectorPos] = packet->accZ;
    vectorPos++;
    if (vectorPos < NUM_SAMPLES) return;

    vectorPos = 0;

    float s = stdev(xvector) + stdev(yvector) + stdev(zvector);
//    PRINTF("stdev = %u\n", (uint16_t) s);
    float accelVector[3] = {
        average(xvector) - groundVector[0],
        average(yvector) - groundVector[1],
        average(zvector) - groundVector[2]
    };

    // PRINTF("accel=(%d %d %d)\n", (int)accelVector[0], (int)accelVector[1], (int)accelVector[2]);

    bool nowFwd, nowBack;
    float accLen = vlength(accelVector);
    if (accLen < ONE_G / 10) {
        nowBack = nowFwd = false;
    } else {
        normalizeQuick(accelVector, accLen);
        nowFwd = sameDirection(fwdVector, accelVector);
        nowBack = inverseDirection(fwdVector, accelVector);
    }
    int now = nowFwd ? 1 : (nowBack ? -1 : 0);

    bool diffFromPrevious;
    static bool notFirstTime;
    if (notFirstTime) {
        // float difference = (calcDiff(xvector, prevxvector) 
        //     + calcDiff(yvector, prevyvector)           
        //     + calcDiff(zvector, prevzvector)) / 3.0;
        float difference = calcDiff(xvector, prevavgxvector) 
                + calcDiff(yvector, prevavgyvector)           
                + calcDiff(zvector, prevavgzvector);
        // PRINTF("difference=%d\n", (int)difference);
        diffFromPrevious = (difference >= 30 * (ONE_G / 10));
    } else {
        notFirstTime = true;
        diffFromPrevious = false;
    }

    bool pastBack = false;
    bool pastFwd = false;
    if (past[0] + past[1] + past[2] >= 1) {
        pastFwd = true;
    } else if (past[0] + past[1] + past[2] <= -1) {
        pastBack = true;
    }
    past[0] = past[1];
    past[1] = past[2];
    past[2] = now;

    bool wasMoving = !bayesStandingMode;

#if 0
    PRINTF("SSD       \t%d\n",  (int)(s >= THRESH_LOW));
    PRINTF("MSD       \t%d\n",  (int)(s >= THRESH_MEDIUM));
    PRINTF("LSD       \t%d\n" , (int)(s > THRESH_HIGH));
    PRINTF("Accelerating\t%d\n", (int)(nowFwd));
    PRINTF("PastAccel \t%d\n", (int)(pastFwd));
    PRINTF("PastBrake \t%d\n", (int)(pastBack));
    PRINTF("WasMoving \t%d\n", (int)(wasMoving));
    PRINTF("Diff      \t%d\n", (int)(diffFromPrevious));
#endif
    
    calcNewProb(s, wasMoving, pastBack, pastFwd, nowFwd, diffFromPrevious);
}

//-------------------------------------------
//      Entry point for the application
//-------------------------------------------
void appMain(void)
{
    PRINTF("Starting...\n");

    initFullBN();

    accelOn();

    mdelay(START_DELAY);

    redLedOn();

    for (;;) {
        uint16_t now = ALARM_TIMER_VALUE();
        uint16_t endTime = now + MAIN_LOOP_LENGTH;

        Packet_t packet;
        packet.accX = accelReadX();
        packet.accY = accelReadY();
        packet.accZ = accelReadZ();

        // PRINTF("read %d %d %d\n", packet.accX, packet.accY, packet.accZ);

        uint16_t start = ALARM_TIMER_VALUE();
        MEMORY_BARRIER();

        addMeasurement(&packet);

        MEMORY_BARRIER();
        uint16_t end = ALARM_TIMER_VALUE();
        
        if (vectorPos == 0) {
            PRINTF("time to calc = %u ticks\n", end - start);
        }

        while (timeAfter16(endTime, ALARM_TIMER_VALUE()));
    }
}
