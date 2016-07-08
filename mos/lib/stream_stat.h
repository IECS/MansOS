/*====================================================================
 * Generic streaming data statistics framework
 *
 *--------------------------------------------------------------------
 * Usage example:
 *--------------------------------------------------------------------


==================================================================*/
  
#ifndef _stream_stat_h_
#define _stream_stat_h_

//======================================================================
// Data types and helper macros
//======================================================================

// Declare a structure for stream data statistical processing, with counter type.
#define STREAM_STAT_DECLARE_EXT(name_t, sum_type, counter_type) \
    typedef struct              \
    {                           \
        counter_type num;       \
        sum_type sum;           \
        sum_type sum_squares;   \
    } name_t;

//======================================================================
// API
//======================================================================


// Declare a structure for stream data statistical processing
#define STREAM_STAT_DECLARE(name_t, sum_type) STREAM_STAT_DECLARE_EXT(name_t, sum_type, uint16_t)

// Processing incoming data
#define STREAM_STAT_INIT(experiment) { experiment.num=0; experiment.sum = 0; experiment.sum_squares=0; }

#define STREAM_STAT_ADD(experiment, val) {experiment.num++; experiment.sum += (val); experiment.sum_squares += (val*val);}

// Processing results
#define STREAM_STAT_MEAN(experiment) ((experiment).sum / (experiment).num)

#define STREAM_STAT_DEVIATION_SQUARED(experiment) ( \
    ((experiment).sum_squares / (experiment).num) -   \
    ((experiment).sum / (experiment).num) * ((experiment).sum / (experiment).num) )


#endif // _stream_stat_h_
