#ifndef CO_MQTT_PACKET_H_INCLUDED
#define CO_MQTT_PACKET_H_INCLUDED

#include <coldforce/core/co_list.h>
#include <coldforce/core/co_string_list.h>
#include <coldforce/core/co_byte_array.h>

#include <coldforce/mqtt/co_mqtt.h>

CO_EXTERN_C_BEGIN

//---------------------------------------------------------------------------//
// mqtt packet
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//

#define CO_MQTT_PACKET_HEADER_MIN_SIZE         2

typedef struct
{
    uint8_t type;
    bool dup;
    uint8_t qos;
    bool retain;
    uint32_t remaining_length;

} co_mqtt_fixed_header_t;

typedef struct
{
    uint8_t qos;
    bool retain;

    char* topic;

    struct co_mqtt_will_payload_t
    {
        uint16_t length;
        uint8_t* data;

    } payload;

    co_list_t* properties;

} co_mqtt_will_t;

typedef struct
{
    struct co_mqtt_connect_vaiable_header_t
    {
        struct co_mqtt_connect_protocol_t
        {
            char* name;
            uint8_t version;

        } protocol;

        struct co_mqtt_connect_flags_t
        {
            bool user_name;
            bool password;
            bool will_retain;
            uint8_t will_qos;
            bool will;
            bool clean_start;

        } flags;

        uint16_t keep_alive_timer;

    } variable_header;
    
    struct co_mqtt_connect_payload_t
    {
        co_mqtt_will_t will;

        char* client_id;
        char* user_name;
        char* password;

    } payload;

} co_mqtt_packet_connect_t;

typedef struct
{
    struct co_mqtt_connack_variable_header_t
    {
        bool session_present;
        uint8_t reason_code;

    } variable_header;

} co_mqtt_packet_connack_t;

typedef struct
{
    struct co_mqtt_publish_variable_header_t
    {
        char* topic_name;
        uint16_t id;

    } variable_header;
    
    struct co_mqtt_publish_payload_t
    {
        uint32_t length;
        uint8_t* data;

    } payload;

} co_mqtt_packet_publish_t;

typedef struct
{
    struct co_mqtt_puback_variable_header_t
    {
        uint16_t id;
        uint8_t reason_code;

    } variable_header;

} co_mqtt_packet_puback_t;

typedef struct
{
    struct co_mqtt_pubrec_variable_header_t
    {
        uint16_t id;
        uint8_t reason_code;

    } variable_header;

} co_mqtt_packet_pubrec_t;

typedef struct
{
    struct co_mqtt_pubrel_variable_header_t
    {
        uint16_t id;
        uint8_t reason_code;
    
    } variable_header;

} co_mqtt_packet_pubrel_t;

typedef struct
{
    struct co_mqtt_pubcomp_variable_header_t
    {
        uint16_t id;
        uint8_t reason_code;

    } variable_header;

} co_mqtt_packet_pubcomp_t;

typedef struct
{
    uint8_t retain_handling;
    bool retain_as_published;
    bool no_local;
    uint8_t qos;

} co_mqtt_subscribe_options_t;

typedef struct
{
    char* str;
    co_mqtt_subscribe_options_t options;

} co_mqtt_topic_filter_t;

typedef struct
{
    struct co_mqtt_subscribe_variable_header_t
    {
        uint16_t id;

    } variable_header;

    struct co_mqtt_subscribe_payload_t
    {
        co_list_t* topic_filters;

    } payload;

} co_mqtt_packet_subscribe_t;

typedef struct
{
    struct co_mqtt_suback_variable_header_t
    {
        uint16_t id;

    } variable_header;

    struct co_mqtt_suback_payload_t
    {
        size_t reason_code_count;
        uint8_t* reason_codes;

    } payload;

} co_mqtt_packet_suback_t;

typedef struct
{
    struct co_mqtt_unsubscribe_variable_header_t
    {
        uint16_t id;

    } variable_header;

    struct co_mqtt_unsubscribe_payload_t
    {
        co_string_list_t* topic_filters;

    } payload;

} co_mqtt_packet_unsubscribe_t;

typedef struct
{
    struct co_mqtt_unsuback_variable_header_t
    {
        uint16_t id;
    
    } variable_header;
    
    struct co_mqtt_unsuback_payload_t
    {
        size_t reason_code_count;
        uint8_t* reason_codes;

    } payload;

} co_mqtt_packet_unsuback_t;

typedef struct
{
    struct co_mqtt_disconnect_variable_header_t
    {
        uint8_t reason_code;

    } variable_header;

} co_mqtt_packet_disconnect_t;

typedef struct
{
    struct co_mqtt_auth_variable_header_t
    {
        uint8_t reason_code;

    } variable_header;

} co_mqtt_packet_auth_t;

typedef struct co_mqtt_packet_t
{
    co_mqtt_fixed_header_t header;

    union co_mqtt_packet_body_un
    {
        co_mqtt_packet_connect_t connect;
        co_mqtt_packet_connack_t connack;
        co_mqtt_packet_publish_t publish;
        co_mqtt_packet_puback_t puback;
        co_mqtt_packet_pubrec_t pubrec;
        co_mqtt_packet_pubrel_t pubrel;
        co_mqtt_packet_pubcomp_t pubcomp;
        co_mqtt_packet_subscribe_t subscribe;
        co_mqtt_packet_suback_t suback;
        co_mqtt_packet_unsubscribe_t unsubscribe;
        co_mqtt_packet_unsuback_t unsuback;
        co_mqtt_packet_disconnect_t disconnect;
        co_mqtt_packet_auth_t auth;

    } body;

    co_list_t* properties;

} co_mqtt_packet_t;

enum CO_MQTT_QOS_EN
{
    CO_MQTT_QOS_0 = 0,
    CO_MQTT_QOS_1 = 1,
    CO_MQTT_QOS_2 = 2
};

enum CO_MQTT_PACKET_TYPE_EN
{
    CO_MQTT_PACKET_TYPE_UNKNOWN        = 0,
    CO_MQTT_PACKET_TYPE_CONNECT        = 1,
    CO_MQTT_PACKET_TYPE_CONNACK        = 2,
    CO_MQTT_PACKET_TYPE_PUBLISH        = 3,
    CO_MQTT_PACKET_TYPE_PUBACK         = 4,
    CO_MQTT_PACKET_TYPE_PUBREC         = 5,
    CO_MQTT_PACKET_TYPE_PUBREL         = 6,
    CO_MQTT_PACKET_TYPE_PUBCOMP        = 7,
    CO_MQTT_PACKET_TYPE_SUBSCRIBE      = 8,
    CO_MQTT_PACKET_TYPE_SUBACK         = 9,
    CO_MQTT_PACKET_TYPE_UNSUBSCRIBE    = 10,
    CO_MQTT_PACKET_TYPE_UNSUBACK       = 11,
    CO_MQTT_PACKET_TYPE_PINGREQ        = 12,
    CO_MQTT_PACKET_TYPE_PINGRESP       = 13,
    CO_MQTT_PACKET_TYPE_DISCONNECT     = 14,
    CO_MQTT_PACKET_TYPE_AUTH           = 15
};

enum CO_MQTT_REASON_CODE_EN
{
    CO_MQTT_REASON_CODE_SUCCESS                          = 0x00,
    CO_MQTT_REASON_CODE_NORMAL_DISCONNECTION             = 0x00,
    CO_MQTT_REASON_CODE_GRANTED_QOS_0                    = 0x00,
    CO_MQTT_REASON_CODE_GRANTED_QOS_1                    = 0x01,
    CO_MQTT_REASON_CODE_GRANTED_QOS_2                    = 0x02,
    CO_MQTT_REASON_CODE_DISCONNECT_WITH_WILL_MESSAGE     = 0x04,
    CO_MQTT_REASON_CODE_NO_MATCHING_SUBSCRIBERS          = 0x10,
    CO_MQTT_REASON_CODE_NO_SUBSCRIPTION_EXISTED          = 0x11,
    CO_MQTT_REASON_CODE_CONTINUE_AUTHENTICATION          = 0x18,
    CO_MQTT_REASON_CODE_RE_AUTHENTICATE                  = 0x19
};

//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
// private
//---------------------------------------------------------------------------//

void
co_mqtt_serialize_variable_byte_integer(
    uint32_t value,
    co_byte_array_t* buffer
);

bool
co_mqtt_deserialize_variable_byte_integer(
    const uint8_t* data,
    size_t* index,
    size_t data_size,
    uint32_t* variable_byte_integer
);

void
co_mqtt_serialize_utf8_string(
    const char* str,
    co_byte_array_t* buffer
);

bool
co_mqtt_deserialize_utf8_string(
    const uint8_t* data,
    size_t* index,
    size_t data_size,
    char** str
);

void
co_mqtt_serialize_utf8_string_pair(
    const char* str1,
    const char* str2,
    co_byte_array_t* buffer
);

bool
co_mqtt_deserialize_utf8_string_pair(
    const uint8_t* data,
    size_t* index,
    size_t data_size,
    char** str1,
    char** str2
);

void
co_mqtt_serialize_binary_data(
    const void* data,
    uint16_t data_size,
    co_byte_array_t* buffer
);

bool
co_mqtt_deserialize_binary_data(
    const uint8_t* data,
    size_t* index,
    size_t data_size,
    uint8_t** binary_ptr,
    uint16_t* binary_size 
);

void
co_mqtt_serialize_u8(
    uint8_t value,
    co_byte_array_t* buffer
);

uint8_t
co_mqtt_deserialize_u8(
    const uint8_t* data,
    size_t* index
);

void
co_mqtt_serialize_u16(
    uint16_t value,
    co_byte_array_t* buffer
);

uint16_t
co_mqtt_deserialize_u16(
    const uint8_t* data,
    size_t* index
);

void
co_mqtt_serialize_u32(
    uint32_t value,
    co_byte_array_t* buffer
);

uint32_t
co_mqtt_deserialize_u32(
    const uint8_t* data,
    size_t* index
);

void
co_mqtt_packet_serialize(
    co_mqtt_packet_t* packet,
    co_byte_array_t* buffer
);

int
co_mqtt_packet_deserialize(
    co_mqtt_packet_t* packet,
    const uint8_t* data,
    const size_t data_size,
    size_t* index
);

//---------------------------------------------------------------------------//
// public
//---------------------------------------------------------------------------//

CO_MQTT_API
co_mqtt_packet_t*
co_mqtt_packet_create(
    void
);

CO_MQTT_API
void
co_mqtt_packet_destroy(
    co_mqtt_packet_t* packet
);

CO_MQTT_API
co_mqtt_packet_t*
co_mqtt_packet_create_connack(
    bool session_present,
    uint8_t reason_code
);

CO_MQTT_API
co_mqtt_packet_t*
co_mqtt_packet_create_suback(
    uint16_t id,
    const uint8_t* reason_codes,
    size_t reason_code_count
);

CO_MQTT_API
co_mqtt_packet_t*
co_mqtt_packet_create_unsuback(
    uint16_t id,
    const uint8_t* reason_codes,
    size_t reason_code_count
);

//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//

CO_MQTT_API
const char*
co_mqtt_packet_get_user_name(
    const co_mqtt_packet_t* packet
);

CO_MQTT_API
const char*
co_mqtt_packet_get_password(
    const co_mqtt_packet_t* packet
);

//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//

CO_EXTERN_C_END

#endif // CO_MQTT_PACKET_H_INCLUDED
