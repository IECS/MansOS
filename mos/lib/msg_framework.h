/*====================================================================
 * Generic message framework.
 *
 *--------------------------------------------------------------------
 * Usage example:
 *--------------------------------------------------------------------

    //--------------------------- Making message buffers
 
    // Declare the payload type
    typedef struct { ... } my_payload_t;

    // Declare the new message type "msg_name_t". The payload will be encapsulated
    MSG_DECLARE(msg_name_t, my_payload_t);

    // Define a buffer for the message and the payload.
    MSG_DEFINE(msg_name_t, msg);

    // Access the payload fields, assign values.
    msg.payload.myValue = 17;

    //--------------------------- Sending
     
    // Calculate the checksum for the message and send it over the radio
    MSG_DO_CHECKSUM(msg);
    err = MSG_RADIO_SEND(msg);

    //--------------------------- Receiving

    // Create a buffer for receiving messages. Choose the max size for payload.
    MSG_DEFINE_BUFFER(radioBuffer, payload_ptr, max_payload_size);

    // Receive and check whether this is a message for you
    err = MSG_RADIO_RECV(radioBuffer);

    if( ! MSG_SIGNATURE_OK(radioBuffer) ) { return; }

    MSG_CHECKSUM_OK(radioBuffer, isOK);
    if( ! isOK ) { return; }

    //--------------------------- Send and wait for a reply

    // Send and wait for ACK:
    MSG_RADIO_SEND_FOR_ACK( msg, flag_ACK_OK);

    // Wait for acknowledgement after message sent. 
    MSG_WAIT_FOR_ACK( flag_ACK_OK );

 *--------------------------------------------------------------------
 * Additional options are as follows:
 *--------------------------------------------------------------------

    // Define my message and a pointer variable to it.
    MSG_DEFINE_WITH_PTR(msg_name_t, msg, msg_p);

    // Initialize the signature field for the message 
    // Note: this is also done at MSG_DEFINE()
    MSG_SET_SIGNATURE(msg);

    // Custom signature can be defined before the framework include.
    #define MSG_DEFAULT_SIGNATURE 0x1234
    #include "msg_framework.h"

==================================================================*/
  
#ifndef _msg_framework_h_
#define _msg_framework_h_

//======================================================================
// Data types and helper macros
//======================================================================
#include "stdint.h"

// Timestamp for messages
typedef uint32_t msg_timestamp_t;

// Common reasons or requests for actions messages are sent
// User can use these in their payload as fields.
typedef uint8_t msg_action_t;
enum {
    MSG_ACT_CLEAR = 0x00,
    MSG_ACT_SET   = 0x01,
    MSG_ACT_ACK   = 0x02,
    MSG_ACT_DONE  = 0x03,

    MSG_ACT_START   = 0x04,
    MSG_ACT_STOP    = 0x05,
    MSG_ACT_PAUSE   = 0x06,
    MSG_ACT_RESTART = 0x07,

    MSG_ACT_CONFIGURE = 0x08,
    MSG_ACT_STATUS  = 0x09,
    MSG_ACT_IDLE    = 0x0A,

} msg_action_e;

// Msg Action code to text
#define MSG_ACT_NAME( act )  ( \
    (act)==MSG_ACT_CLEAR    ? "CLEAR"   :   \
    (act)==MSG_ACT_SET      ? "SET"     :   \
    (act)==MSG_ACT_ACK      ? "ACK"     :   \
    (act)==MSG_ACT_DONE     ? "DONE"    :   \
    (act)==MSG_ACT_START    ? "START"   :   \
    (act)==MSG_ACT_STOP     ? "STOP"    :   \
    (act)==MSG_ACT_PAUSE    ? "PAUSE"   :   \
    (act)==MSG_ACT_RESTART  ? "RESTART" :   \
    (act)==MSG_ACT_CONFIGURE ? "CONFIGURE"   :   \
    (act)==MSG_ACT_STATUS   ? "STATUS"  :   \
    (act)==MSG_ACT_IDLE     ? "IDLE"    :   \
    "?")


// You can define the signature before this include. Use with caution.
#ifndef MSG_DEFAULT_SIGNATURE
#define MSG_DEFAULT_SIGNATURE 0xBE47
#endif

typedef uint16_t msg_signature_t;
typedef uint8_t msg_checksum_t;
typedef uint8_t msg_id_t;

typedef uint16_t msg_address_t;  // FIXME: Not implemented
enum { MSG_ADDR_ALL = 0 };        // Broadcast address

#define MSG_NAME2TYPE(msg_type_name) msg_type_name##__t
#define MSG_NAME2SIG(msg_type_name) msg_type_name##__sig
#define MSG_NAME2SIZE(msg_type_name) msg_type_name##__sizeof
#define MSG_NAME2ID(name) name##__id

#define MSG_PAYLOAD_SIZE(msg_type_name) msg_type_name##_pld_sizeof

#define MSG_NAME2PAYLOAD_T(name) name##__payload_t
#define MSG_NAME2MSG_T(name) name##__msg_t

#define MSG_CHECKSUM_SKIP_SIZE \
    ( sizeof(msg_signature_t) + sizeof(msg_checksum_t) )

#define MSG_CHECKSUM_ADD_SIZE(msg) \
    ( MSG_NAME2SIZE(msg) - MSG_PAYLOAD_SIZE(msg) - MSG_CHECKSUM_SKIP_SIZE )
    // ( offsetof( MSG_NAME2MSG_T(msg), payload ) )

// Message declaration helper prototype. 
// The extra_field_defs may be empty (optional) or must be terminated with ;
#define MSG_DECLARE_TAG(msg_type_name, payload_type, msg_signature, extra_field_defs)    \
    typedef struct {    \
        msg_signature_t signature;  \
        msg_checksum_t checksum;    \
        extra_field_defs;           \
        payload_type payload;       \
    } __attribute__((packed))       \
    MSG_NAME2TYPE(msg_type_name);   \
    enum { MSG_NAME2SIZE(msg_type_name) = sizeof( MSG_NAME2TYPE(msg_type_name) ) }; \
    enum { MSG_PAYLOAD_SIZE(msg_type_name) = sizeof(payload_type) }; \
    enum { MSG_NAME2SIG(msg_type_name) = msg_signature }; \

#define MSG_DEFINE_ATTRIBUTES(name_t, name) \
    enum { MSG_NAME2SIZE(name) = MSG_NAME2SIZE(name_t) }; \
    enum { MSG_PAYLOAD_SIZE(name) = MSG_PAYLOAD_SIZE(name_t) }; \
    enum { MSG_NAME2SIG(name) = MSG_NAME2SIG(name_t) }; \


//======================================================================
// Message framework API:
//======================================================================

//======================================================================
// Declare and define messages.
//======================================================================
// Declare = make type. Define = make buffer of the type

// Message declaration
#define MSG_DECLARE(msg_type_name, payload_type)    \
    MSG_DECLARE_TAG(msg_type_name, payload_type, MSG_DEFAULT_SIGNATURE , );

// Message declaration with signature
#define MSG_DECLARE_SIG(msg_type_name, payload_type, msg_signature) \
    MSG_DECLARE_TAG(msg_type_name, payload_type, msg_signature, );

// Define the message as a buffer variable.
// It is assigned a signature to distinguish from other messages in the air.
// Create a memory buffer for the message with the payload within.
// The payload can be accessed by "msg_name.payload"
#define MSG_DEFINE(msg_type_name, msg_name) \
    MSG_NAME2TYPE(msg_type_name) msg_name = { .signature = MSG_NAME2SIG(msg_type_name) }; \
    MSG_DEFINE_ATTRIBUTES(msg_type_name, msg_name); \

// Define pointer. Can follow up this with =&my_msg; at the tail.
#define MSG_DEFINE_P(msg_type_name, msg_name_p) \
    MSG_DEFINE_ATTRIBUTES(msg_type_name, msg_name_p); \
    MSG_NAME2TYPE(msg_type_name) * msg_name_p

// Define message and a pointer to it.
#define MSG_DEFINE_WITH_PTR(msg_type_name, msg_var_name, msg_name_p) \
    MSG_DEFINE(msg_type_name, msg_var_name);  \
    MSG_DEFINE_P(msg_type_name, msg_name_p) = &(msg_var_name); \

// A simple convenience function for message creation, all in one solution.
#define MSG_NEW(msg_name, payload_type) \
    MSG_DECLARE(MSG_NAME2MSG_T(msg_name), payload_type); \
    MSG_DEFINE(MSG_NAME2MSG_T(msg_name), msg_name); \


// Define buffer for receiving messages with enough space for the payload
// "payload_ptr" will return the pointer to the payload in created buffer
// "buffer_msg_name" can be used with SEND... and RECV... functions.
#define MSG_DEFINE_BUFFER( buffer_msg_name, payload_ptr, max_payload_size) \
    typedef struct {                         \
        uint8_t data[max_payload_size];      \
    } __attribute__((packed))                \
        MSG_NAME2PAYLOAD_T(buffer_msg_name); \
    MSG_DECLARE(MSG_NAME2MSG_T(buffer_msg_name), MSG_NAME2PAYLOAD_T(buffer_msg_name));   \
    MSG_DEFINE(MSG_NAME2MSG_T(buffer_msg_name), buffer_msg_name);  \
    uint8_t * payload_ptr = (uint8_t*) &(buffer_msg_name.payload); \


// TODO?
// Alias one message with another name of a different type. 
// This helps with proper checksum calculation and processing.
#define MSG_ALIAS(msg_name, payload_type, src_msg_buffer) \
    MSG_DECLARE(MSG_NAME2MSG_T(msg_name), payload_type); \
    MSG_DEFINE_ATTRIBUTES(MSG_NAME2MSG_T(msg_name), msg_var_name); \
    MSG_NAME2TYPE(msg_type_name) msg_var_name = { .signature = MSG_NAME2SIG(msg_type_name) }; \

//======================================================================
//  Messages with ID. 
// The id is the message type that helps when receiving to select 
// the appropriate buffer or parser for decoding the payload
//======================================================================

#define MSG_DECLARE_WITH_ID(msg_type_name, payload_type, msg_id)    \
    MSG_DECLARE_TAG(msg_type_name, payload_type, \
                    MSG_DEFAULT_SIGNATURE, msg_id_t id); \
    enum { MSG_NAME2ID(msg_type_name) = msg_id }; \

#define MSG_DEFINE_WITH_ID(msg_type_name, msg_var_name) \
    MSG_NAME2TYPE(msg_type_name) msg_var_name = { \
        .signature = MSG_NAME2SIG(msg_type_name), \
        .id = MSG_NAME2ID(msg_type_name) \
    }; \
    MSG_DEFINE_ATTRIBUTES(msg_type_name, msg_var_name); \
    enum { MSG_NAME2ID(msg_var_name) = MSG_NAME2ID(msg_type_name) }; \


#define MSG_NEW_WITH_ID(msg_name, payload_type, msg_id) \
    MSG_DECLARE_WITH_ID(MSG_NAME2MSG_T(msg_name), payload_type, msg_id); \
    MSG_DEFINE_WITH_ID(MSG_NAME2MSG_T(msg_name), msg_name); \

#define MSG_DEFINE_BUFFER_WITH_ID( buffer_msg_name, payload_ptr, max_payload_size ) \
    typedef struct { uint8_t data[max_payload_size]; } MSG_NAME2PAYLOAD_T(buffer_msg_name);   \
    MSG_DECLARE_WITH_ID(MSG_NAME2MSG_T(buffer_msg_name), MSG_NAME2PAYLOAD_T(buffer_msg_name), 0);   \
    MSG_DEFINE_WITH_ID(MSG_NAME2MSG_T(buffer_msg_name), buffer_msg_name);  \
    uint8_t * payload_ptr = (uint8_t*) &(buffer_msg_name.payload); \


//======================================================================
// Checksum calculation and verification
//======================================================================

// Helper function to calculate the checksum for a message
// Note: the msg must be defined with this framework!
#define MSG_CHECKSUM_FUN(msg_p, sum, size)  \
    { \
        uint8_t *p = (uint8_t*)(msg_p);     \
        int i = size;                       \
        for((sum) = 0; i>0; i--) (sum) += (*p++);  \
    } \

// Calculate and update the checksum for the message
#define MSG_DO_CHECKSUM(msg)        \
     MSG_CHECKSUM_FUN( &msg + MSG_CHECKSUM_SKIP_SIZE, msg.checksum,  \
        MSG_CHECKSUM_ADD_SIZE(msg) + MSG_PAYLOAD_SIZE(msg) );  


// #define MSG_DO_CHECKSUM_P(msg_p)    
//      MSG_CHECKSUM_FUN(msg_p + MSG_CHECKSUM_SKIP_SIZE, msg_p->checksum,  
//         MSG_NAME2SIZE(msg_p) - MSG_CHECKSUM_SKIP_SIZE );


// Check the checksum.
// If not OK, exec the command (break, return,...);
#define MSG_CHECK(msg, commandIfBad)  \
    { \
        msg_checksum_t mchksum=0; \
        MSG_CHECKSUM_FUN( &msg + MSG_CHECKSUM_SKIP_SIZE,  msg.checksum,  \
            MSG_CHECKSUM_ADD_SIZE(msg) + MSG_PAYLOAD_SIZE(msg) );  \
        if( mchksum != msg.checksum ) commandIfBad;   \
    } \

        // MSG_NAME2SIZE(msg) - MSG_CHECKSUM_SKIP_SIZE );   


// Check the msg checksum, but for different payload type.
// This is useful for receive buffers that we do not know the payload types.
// If not OK, exec the command (break, return,...);
#define MSG_CHECK_FOR_PAYLOAD(msg, payload_t, commandIfBad)  \
    { \
        msg_checksum_t mchksum=0; \
        MSG_CHECKSUM_FUN( &msg + MSG_CHECKSUM_SKIP_SIZE,  msg.checksum,  \
            MSG_CHECKSUM_ADD_SIZE(msg) + sizeof(payload_t) );  \
        if( mchksum != msg.checksum ) commandIfBad;   \
    } \


//======================================================================
// Sending and receiving messages
//======================================================================

// Send a message over the radio
#define MSG_RADIO_SEND(msg) radioSend(&(msg), sizeof(msg));

#define MSG_RADIO_SEND_P(msg_p) radioSend((msg_p), MSG_NAME2SIZE(msg_p));


// Receive a message to the msg buffer from the radio
#define MSG_RADIO_RECV(msg) radioRecv(&(msg), sizeof(msg));

#define MSG_RADIO_RECV_P(msg_p) radioRecv((msg_p), MSG_NAME2SIZE(msg_p));


// Set message signature (this is done also when defined)
#define MSG_SET_SIGNATURE(var_name) \
    var_name.signature = MSG_NAME2SIG(msg_type_name); \

// Compare message signature to the defined. Evaluates to true if there is a match.
#define MSG_SIGNATURE_OK(msg) \
    ( msg.signature == MSG_NAME2SIG(msg) )

#define MSG_SIGNATURE_OK_P(msg_p) \
    ( (msg_p)->signature == MSG_NAME2SIG(msg_p) )

//======================================================================
//  API: ACK and retries
//======================================================================
// Constants for retries and wiat times in ms. 
// These could be redefined prior to macro calls or this include
#ifndef MSG_WAIT_DELAY_MS
#define MSG_WAIT_DELAY_MS 10
#endif
#ifndef MSG_WAIT_RETRY_COUNT
#define MSG_WAIT_RETRY_COUNT 100
#endif
#ifndef MSG_RESEND_COUNT
#define MSG_RESEND_COUNT 3
#endif

// Acknowledgements: wait for global flag to be set (e.g. after ACK received)
// Will wait for MSG_WAIT_DELAY_MS, then check the flag. 
// if the flag is true, then will exit. Else will retry waiting MSG_WAIT_RETRY_COUNT times.
// Note: the "flag" must be set to false before sending the request that is expected to be ACKed.
#define MSG_WAIT_FOR_ACK( flag_ACK_OK ) \
    {   \
        int msg_wait_retry_cnt = MSG_WAIT_RETRY_COUNT;  \
        for(; msg_wait_retry_cnt>=0 && !flag_ACK_OK; msg_wait_retry_cnt--){ \
            mdelay(MSG_WAIT_DELAY_MS);  \
        }   \
    }   \

// Send a message and wait for ACK. If the ACK was received, the flag is set to true.
// The flag must be defined as global, visible variable. 
// Note: you need to write the ACK receive code elsewhere yourself, and set 
// the flag true once the ACK has been received. 
// Note: the checksum calculation must be done before this call.
#define MSG_RADIO_SEND_FOR_ACK( msg, flag_ACK_OK ) \
    {   \
        flag_ACK_OK=false;  \
        int msg_resend_cnt_i = MSG_RESEND_COUNT;    \
        for(; msg_resend_cnt_i>=0; msg_resend_cnt_i--){ \
            MSG_RADIO_SEND( msg );  \
            MSG_WAIT_FOR_ACK( fl_AngleSet )    \
            if( flag_ACK_OK ) break;    \
        }   \
    }   \

// Copy new payload data to message and send
#define MSG_COPY_AND_SEND( msg, src_data_p )  \
    memcpy(&(msg.payload), (src_data_p), MSG_PAYLOAD_SIZE(msg));  \
    MSG_DO_CHECKSUM( msg );  \
    MSG_RADIO_SEND( msg );  \


//======================================================================
//  Extra features API
//======================================================================
// Messages with subtype ID, with address, etc.

// typedef uint8_t msg_address_t;
// typedef uint8_t msg_lentype_t;
// typedef struct {
//     msg_address_t target;
//     msg_lentype_t len;
// } xxxx_t;

// Clone a payload pointer from the message. May be of different type
#define MSG_NEW_PAYLOAD_PTR(msg, payload_t, new_payload_p)  \
    payload_t * new_payload_p = (payload_t*) &(msg.payload);


//======================================================================
//  Predefined message types
//======================================================================
#define MSG_TEXT_SIZE_MAX 32

typedef struct 
{
    char text[MSG_TEXT_SIZE_MAX];
}
msg_text_data_t;

#endif // _msg_framework_h_
