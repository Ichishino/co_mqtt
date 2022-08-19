#ifndef CO_MQTT_PROPERTY_H_INCLUDED
#define CO_MQTT_PROPERTY_H_INCLUDED

#include <coldforce/core/co_list.h>
#include <coldforce/core/co_byte_array.h>

#include <coldforce/mqtt/co_mqtt.h>

CO_EXTERN_C_BEGIN

//---------------------------------------------------------------------------//
// mqtt property
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//

typedef struct co_mqtt_property_t
{
    uint8_t id;

    union co_mqtt_property_data_un
    {
        uint8_t u8;
        uint16_t u16;
        uint32_t u32;
        char* utf8_str;

        struct co_mqtt_property_data_utf8_pair_t
        {
            char* name;
            char* value;

        } utf8_pair;

        struct co_mqtt_property_data_binary_t
        {
            uint16_t length;
            uint8_t* data;

        } binary;

    } data;

} co_mqtt_property_t;

enum CO_MQTT_PROPERTY_ID_EN
{
    CO_MQTT_PROPERTY_ID_PAYLOAD_FORMAT_INDICATOR = 0x01,            // Byte
    CO_MQTT_PROPERTY_ID_MESSAGE_EXPIRY_INTERVAL = 0x02,             // Four Byte Integer
    CO_MQTT_PROPERTY_ID_CONTENT_TYPE = 0x03,                        // UTF-8 Encoded String

    CO_MQTT_PROPERTY_ID_RESPONSE_TOPIC = 0x08,                      // UTF-8 Encoded String
    CO_MQTT_PROPERTY_ID_CORRELATION_DATA = 0x09,                    // Binary Data

    CO_MQTT_PROPERTY_ID_SUBSCRIPTION_IDENTIFIER = 0x0b,             // Variable Byte Integer

    CO_MQTT_PROPERTY_ID_SESSION_EXPIRY_INTERVAL = 0x11,             // Four Byte Integer
    CO_MQTT_PROPERTY_ID_ASSIGNED_CLIENT_IDENTIFIER = 0x12,          // UTF-8 Encoded String
    CO_MQTT_PROPERTY_ID_SERVER_KEEP_ALIVE = 0x13,                   // Two Byte Integer

    CO_MQTT_PROPERTY_ID_AUTHENTICATION_METHOD = 0x15,               // UTF-8 Encoded String
    CO_MQTT_PROPERTY_ID_AUTHENTICATION_DATA = 0x16,                 // Binary Data
    CO_MQTT_PROPERTY_ID_REQUEST_PROBLEM_INFORMATION = 0x17,         // Byte
    CO_MQTT_PROPERTY_ID_WILL_DELAY_INTERVAL = 0x18,                 // Four Byte Integer
    CO_MQTT_PROPERTY_ID_REQUEST_RESPONSE_INFORMATION = 0x19,        // Byte
    CO_MQTT_PROPERTY_ID_RESPONSE_INFORMATION = 0x1a,                // UTF-8 Encoded String

    CO_MQTT_PROPERTY_ID_SERVER_REFERENCE = 0x1c,                    // UTF-8 Encoded String

    CO_MQTT_PROPERTY_ID_REASON_STRING = 0x1f,                       // UTF-8 Encoded String

    CO_MQTT_PROPERTY_ID_RECEIVE_MAXIMUM = 0x21,                     // Two Byte Integer
    CO_MQTT_PROPERTY_ID_TOPIC_ALIAS_MAXIMUM = 0x22,                 // Two Byte Integer
    CO_MQTT_PROPERTY_ID_TOPIC_ALIAS = 0x23,                         // Two Byte Integer
    CO_MQTT_PROPERTY_ID_MAXIMUM_QOS = 0x24,                         // Byte
    CO_MQTT_PROPERTY_ID_RETAIN_AVAILABLE = 0x25,                    // Byte
    CO_MQTT_PROPERTY_ID_USER_PROPERTY = 0x26,                       // UTF-8 String Pair
    CO_MQTT_PROPERTY_ID_MAXIMUM_PACKET_SIZE = 0x27,                 // Four Byte Integer
    CO_MQTT_PROPERTY_ID_WILDCARD_SUBSCRIPTION_AVAILABLE = 0x28,     // Byte
    CO_MQTT_PROPERTY_ID_SUBSCRIPTION_IDENTIFIER_AVAILABLE = 0x29,   // Byte
    CO_MQTT_PROPERTY_ID_SHARED_SUBSCRIPTION_AVAILABLE = 0x2a        // Byte
};

//---------------------------------------------------------------------------//
// private
//---------------------------------------------------------------------------//

void
co_mqtt_serialize_properties(
    const co_list_t* properties,
    co_byte_array_t* buffer
);

bool
co_mqtt_deserialize_properties(
    const uint8_t* data,
    size_t* index,
    size_t data_size,
    co_list_t* properties
);

void
co_mqtt_property_destroy(
    co_mqtt_property_t* property
);

//---------------------------------------------------------------------------//
// public
//---------------------------------------------------------------------------//

CO_MQTT_API
co_mqtt_property_t*
co_mqtt_property_create_u8(
    uint8_t id,
    uint8_t value
);

CO_MQTT_API
co_mqtt_property_t*
co_mqtt_property_create_u16(
    uint8_t id,
    uint16_t value
);

CO_MQTT_API
co_mqtt_property_t*
co_mqtt_property_create_u32(
    uint8_t id,
    uint32_t value
);

CO_MQTT_API
co_mqtt_property_t*
co_mqtt_property_create_utf8(
    uint8_t id,
    const char* str
);

CO_MQTT_API
co_mqtt_property_t*
co_mqtt_property_create_binary(
    uint8_t id,
    const void* data,
    uint16_t data_size
);

CO_MQTT_API
co_mqtt_property_t*
co_mqtt_property_create_utf8_pair(
    uint8_t id,
    const char* name,
    const char* value
);

//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//

CO_EXTERN_C_END

#endif // CO_MQTT_MESSAGE_H_INCLUDED
