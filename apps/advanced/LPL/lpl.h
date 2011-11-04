#ifndef MANSOS_LPL_H
#define MANSOS_LPL_H

/**
 * Low Power Listening Send States
 */
typedef enum {
    S_LPL_NOT_SENDING,    // DEFAULT
    S_LPL_FIRST_MESSAGE,  // 1. Sending the first message
    S_LPL_SENDING,        // 2. Sending all other messages
    S_LPL_CLEAN_UP,       // 3. Clean up the transmission
} lpl_sendstate_t;


/**
 * This is a measured value of the time in ms the radio is actually on
 * We round this up to err on the side of better performance ratios
 * This includes the acknowledgement wait period and backoffs,
 * which can typically be much longer than the transmission.
 * 
 * Measured by Tony O'Donovan
 */
// #ifndef DUTY_ON_TIME
// #define DUTY_ON_TIME 11 
// #endif

/**
 * The maximum number of CCA checks performed on each wakeup.
 * If there are too few, the receiver may wake up between messages
 * and not detect the transmitter.
 *
 * The on-time had to increase from the original version to allow multiple
 * transmitters to co-exist.  This is due to using ack's, which then requires us
 * to extend the backoff period.  In networks that transmit frequently, possibly
 * with multiple transmitters, this power scheme makes sense.  
 *
 * In networks that transmit very infrequently or without multiple transmitters,
 * it makes more sense to go with no acks and no backoffs and make the
 * receive check as short as possible.
 */
#ifndef MAX_LPL_CCA_CHECKS
#define MAX_LPL_CCA_CHECKS 400
#endif

/**
 * The minimum number of samples that must be taken in CC2420DutyCycleP
 * that show the channel is not clear before a detection event is issued
 */
#ifndef MIN_SAMPLES_BEFORE_DETECT
#define MIN_SAMPLES_BEFORE_DETECT 3
#endif

#endif

