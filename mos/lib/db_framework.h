/*====================================================================
 * Generic database framework.
 *
 *--------------------------------------------------------------------
 * Usage example:
 *--------------------------------------------------------------------


    #define DB_RECORD_NUM 1
    DB_NEW(db, db_record_t, DB_RECORD_NUM);

    DB_TOP(db);

    rxLen = radioRecv( &(DB_REC(db)), DB_REC_SIZE(db));

    for(...; !DB_EOF(dbname); DB_NEXT(db) ) {...}

// typedef struct {
//     uint8_t data[RADIO_MAX_PACKET];
// } __attribute__((packed)) max_msg_buffer_t;

// #define MAX_MSG_TYPE  max_msg_buffer_t

// typedef struct 
// {
//     msg_timestamp_t timestamp;
//     int recLen;     //received packet length
//     rssi_t rssi;
//     MAX_MSG_TYPE msg;    
// } __attribute__((packed)) 
// db_record_t;

// #define DB_RECORD_NUM 1
// DB_NEW(db, db_record_t, DB_RECORD_NUM);


==================================================================*/
  
#ifndef _db_framework_h_
#define _db_framework_h_

//======================================================================
// Data types and helper macros
//======================================================================

#define DB_REC_SIZE(dbname) dbname##__db_rec_size
#define DB_REC_NUM(dbname) dbname##__db_size

//======================================================================
// API
//======================================================================

#define DB_NEW(dbname, db_record_t, num_records)    \
    struct {                                        \
        int curIdx;                                 \
        db_record_t data[num_records];              \
    } dbname                                        \
    = { .curIdx=0 };                                \
    enum { DB_REC_NUM(dbname) = num_records };               \
    enum { DB_REC_SIZE(dbname) = sizeof(db_record_t) };   \

#define DB_TOP(dbname)  dbname.curIdx=0;

#define DB_NEXT(dbname)     \
    dbname.curIdx++;        \
    if(dbname.curIdx > DB_LAST_IDX ) dbname.curIdx=0;  \

#define DB_REC(dbname)  (dbname.data[dbname.curIdx])

#define DB_EOF(dbname)  (dbname.curIdx >= DB_REC_NUM)


#endif // _db_framework_h_
