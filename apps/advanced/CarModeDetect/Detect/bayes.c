#define FALSE    0
#define TRUE     1
#define NOT_SET -1

enum State_e {
    NONE = -1,

    WAS_MOVING,
    PAST_ACCEL,
    PAST_BRAKE,
    ACCEL,
    MOVING,
    LSD,
    MSD,
    SSD,
    DIFF,

    TOTAL_STATES
} PACKED;

typedef enum State_e State_e;

State_e fullBNDependencies[TOTAL_STATES][4];
volatile uint8_t fullBNNumDependencies[TOTAL_STATES];
float fullBNProbabilities[TOTAL_STATES][16];

void initFullBN(void)
{
    int i;
    for (i = 0; i < sizeof(fullBNDependencies) / sizeof(int); ++i) {
        ((int*)fullBNDependencies)[i] = NONE;
    }
    fullBNNumDependencies[WAS_MOVING] = 0;

    fullBNNumDependencies[PAST_ACCEL] = 0;

    fullBNNumDependencies[PAST_BRAKE] = 0;

    fullBNNumDependencies[ACCEL] = 0;

    fullBNNumDependencies[MOVING] = 4;
    fullBNDependencies[MOVING][0] = PAST_ACCEL;
    fullBNDependencies[MOVING][1] = WAS_MOVING;
    fullBNDependencies[MOVING][3] = PAST_BRAKE;
    fullBNDependencies[MOVING][4] = ACCEL;

    fullBNNumDependencies[LSD] = 1;
    fullBNDependencies[LSD][0] = MOVING;

    fullBNNumDependencies[MSD] = 1;
    fullBNDependencies[MSD][0] = MOVING;

    fullBNNumDependencies[SSD] = 1;
    fullBNDependencies[SSD][0] = MOVING;

    fullBNNumDependencies[DIFF] = 2;
    fullBNDependencies[DIFF][0] = MOVING;
    fullBNDependencies[DIFF][0] = WAS_MOVING;


    for (i = 0; i < sizeof(fullBNProbabilities) / sizeof(float); ++i) {
        ((float*)fullBNProbabilities)[i] = 0.0;
    }
    fullBNProbabilities[WAS_MOVING][0] = 0.5;
    fullBNProbabilities[PAST_ACCEL][0] = 0.2;
    fullBNProbabilities[PAST_BRAKE][0] = 0.2;
    fullBNProbabilities[ACCEL][0] = 0.2;

    fullBNProbabilities[MOVING][0] = 0.01;
    fullBNProbabilities[MOVING][1] = 0.98;
    fullBNProbabilities[MOVING][2] = 0.1;
    fullBNProbabilities[MOVING][3] = 0.9;

    fullBNProbabilities[MOVING][4] = 0.94;
    fullBNProbabilities[MOVING][5] = 0.99;
    fullBNProbabilities[MOVING][6] = 0.85;
    fullBNProbabilities[MOVING][7] = 0.99;

    fullBNProbabilities[MOVING][8] = 0.10;
    fullBNProbabilities[MOVING][9] = 0.99;
    fullBNProbabilities[MOVING][10] = 0.95;
    fullBNProbabilities[MOVING][11] = 0.9;

    fullBNProbabilities[MOVING][12] = 0.97;
    fullBNProbabilities[MOVING][13] = 0.995;
    fullBNProbabilities[MOVING][14] = 0.99;
    fullBNProbabilities[MOVING][15] = 0.999;

    fullBNProbabilities[LSD][0] = 0.0;
    fullBNProbabilities[LSD][1] = 0.7;

    fullBNProbabilities[MSD][0] = 0.1;
    fullBNProbabilities[MSD][1] = 0.8;

    fullBNProbabilities[SSD][0] = 0.5;
    fullBNProbabilities[SSD][1] = 0.99;

    fullBNProbabilities[DIFF][0] = 0.01;
    fullBNProbabilities[DIFF][1] = 0.6;
    fullBNProbabilities[DIFF][2] = 0.67;
    fullBNProbabilities[DIFF][3] = 0.67;
}

float getProbabilityForState(int8_t states[TOTAL_STATES], State_e state)
{
    // PRINTF("states = %c %c %c\n",
    //         states[0] == TRUE ? 'T' : 'F',
    //         states[1] == TRUE ? 'T' : 'F',
    //         states[2] == TRUE ? 'T' : 'F');
    int i;
    uint16_t index = 0;
    uint16_t bit = 1 << (fullBNNumDependencies[state] - 1);
//    PRINTF("  state=%d, num deps=%d\n", state, fullBNNumDependencies[state]);
    for (i = 0; i < fullBNNumDependencies[state]; ++i) {
        State_e dependency = fullBNDependencies[state][i];
//        PRINTF("  dependency=%d\n", (int) dependency);
        if (states[dependency] == TRUE) {
            index += bit;
        }
        bit >>= 1;
    }
//    PRINTF("%d: index=%d\n", state, index);
    float result = fullBNProbabilities[state][index];
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
        // optimization: don't care if the state has no deps
        if (fullBNNumDependencies[i] != 0) {
            result *= getProbabilityForState(states, (State_e) i);
        }
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

float fullBNConditionalProbability(State_e stateTrue, int8_t states[TOTAL_STATES])
{
    float stateProbability = sumOverProbabilities(states);

//    PRINTF("\nstate prob=%d%%\n\n", (int)(stateProbability * 100));
    states[stateTrue] = TRUE;
    float extendedStateProbability = sumOverProbabilities(states);

//    PRINTF("\nextended state prob=%d%%\n\n", (int)(extendedStateProbability * 100));

    return extendedStateProbability / stateProbability;
}
