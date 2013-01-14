#include <stdmansos.h>

//
// See http://en.wikipedia.org/wiki/Bayesian_network for details
//


#define MAIN_LOOP_LENGTH 1000

#define FALSE    0
#define TRUE     1
#define NOT_SET -1

enum State_e {
    NONE = -1,

    RAIN = 0,
    SPRINKLER = 1,
    GRASS_WET = 2,

    TOTAL_STATES
} PACKED;

typedef enum State_e State_e;

State_e simpleBNDependencies[TOTAL_STATES][2];
volatile uint8_t simpleBNNumDependencies[TOTAL_STATES];
float simpleBNProbabilities[TOTAL_STATES][4];

void initSimpleBN(void)
{
    int i;
    for (i = 0; i < sizeof(simpleBNDependencies) / sizeof(int); ++i) {
        ((int*)simpleBNDependencies)[i] = NONE;
    }
    simpleBNNumDependencies[RAIN] = 0;
    simpleBNNumDependencies[SPRINKLER] = 1;
    simpleBNDependencies[SPRINKLER][0] = RAIN;
    simpleBNNumDependencies[GRASS_WET] = 2;
    simpleBNDependencies[GRASS_WET][0] = SPRINKLER;
    simpleBNDependencies[GRASS_WET][1] = RAIN;


    for (i = 0; i < sizeof(simpleBNProbabilities) / sizeof(float); ++i) {
        ((float*)simpleBNProbabilities)[i] = 0.0;
    }
    simpleBNProbabilities[RAIN][0] = 0.2;
    simpleBNProbabilities[SPRINKLER][0] = 0.4;
    simpleBNProbabilities[SPRINKLER][1] = 0.01;
    simpleBNProbabilities[GRASS_WET][0] = 0.0;
    simpleBNProbabilities[GRASS_WET][1] = 0.8;
    simpleBNProbabilities[GRASS_WET][2] = 0.9;
    simpleBNProbabilities[GRASS_WET][3] = 0.99;
}

float getProbabilityForState(int8_t states[TOTAL_STATES], State_e state)
{
    // PRINTF("states = %c %c %c\n",
    //         states[0] == TRUE ? 'T' : 'F',
    //         states[1] == TRUE ? 'T' : 'F',
    //         states[2] == TRUE ? 'T' : 'F');
    int i;
    uint16_t index = 0;
    uint16_t bit = 1 << (simpleBNNumDependencies[state] - 1);
//    PRINTF("  state=%d, num deps=%d\n", state, simpleBNNumDependencies[state]);
    for (i = 0; i < simpleBNNumDependencies[state]; ++i) {
        State_e dependency = simpleBNDependencies[state][i];
//        PRINTF("  dependency=%d\n", (int) dependency);
        if (states[dependency] == TRUE) {
            index += bit;
        }
        bit >>= 1;
    }
//    PRINTF("%d: index=%d\n", state, index);
    float result = simpleBNProbabilities[state][index];
    if (states[state] == FALSE) {
        result = 1.0 - result;
    }
//    PRINTF("    result=%d\n", (int)(result * 100));
    return result;
}

float calculateProbability(int8_t states[TOTAL_STATES])
{
    float result = 1;
    uint16_t i;
    for (i = 0; i < TOTAL_STATES; ++i) {
        result *= getProbabilityForState(states, (State_e) i);
    }
    return result;
}

float sumOverProbabilities(int8_t states[TOTAL_STATES])
{
    uint16_t i, v;
    int8_t stateCopy[TOTAL_STATES];
    uint8_t numFreeStates = 0;
    for (i = 0; i < TOTAL_STATES; ++i) {
        if (states[i] == NOT_SET) numFreeStates++;
        stateCopy[i] = states[i];
    }
    uint16_t numVariants = 1u << numFreeStates;
    float result = 0;
    for (v = 0; v < numVariants; ++v) {
        uint16_t numFree;
        for (i = 0, numFree = 0; i < TOTAL_STATES; ++i) {
            if (states[i] == NOT_SET) {
                if (v & (1 << numFree)) {
                    stateCopy[i] = FALSE;
                } else {
                    stateCopy[i] = TRUE;
                }
                numFree++;
            }
        }
        // state copy array all filled; calculate the probability of this
        // specific variant
        result += calculateProbability(stateCopy);
    }
    return result;
}

float simpleBNConditionalProbability(State_e stateTrue, int8_t states[TOTAL_STATES])
{
    float stateProbability = sumOverProbabilities(states);

//    PRINTF("\nstate prob=%d%%\n\n", (int)(stateProbability * 100));
    states[stateTrue] = TRUE;
    float extendedStateProbability = sumOverProbabilities(states);

//    PRINTF("\nextended state prob=%d%%\n\n", (int)(extendedStateProbability * 100));

    return extendedStateProbability / stateProbability;
}

//-------------------------------------------
//      Entry point for the application
//-------------------------------------------
void appMain(void)
{
    initSimpleBN();

    for (;;) {
        uint32_t now = getTimeMs();
        uint32_t endTime = now + MAIN_LOOP_LENGTH;

        uint16_t start = ALARM_TIMER_VALUE();
        MEMORY_BARRIER();

        int8_t simpleBNVariables[TOTAL_STATES];
        simpleBNVariables[GRASS_WET] = TRUE;
        simpleBNVariables[SPRINKLER] = NOT_SET;
        simpleBNVariables[RAIN] = NOT_SET;
        float prob = simpleBNConditionalProbability(RAIN, simpleBNVariables);

        MEMORY_BARRIER();
        uint16_t end = ALARM_TIMER_VALUE();

        PRINTF("prob=%d%%, time to calc = %u ticks\n", (int)(prob * 100), end - start);

        while (timeAfter32(endTime, getTimeMs()));
    }
}
