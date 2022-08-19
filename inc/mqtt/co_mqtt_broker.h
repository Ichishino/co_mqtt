#ifndef CO_MQTT_BROKER_H_INCLUDED
#define CO_MQTT_BROKER_H_INCLUDED

#include <coldforce/core/co_list.h>
#include <coldforce/core/co_thread.h>

#include <coldforce/mqtt/co_mqtt.h>
#include <coldforce/mqtt/co_mqtt_packet.h>
#include <coldforce/mqtt/co_mqtt_client.h>

CO_EXTERN_C_BEGIN

//---------------------------------------------------------------------------//
// mqtt client
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//

typedef struct
{
    char* client_id;
    co_mqtt_topic_filter_t* topic_filter;

} co_mqtt_subscriber_t;

typedef struct
{
    char* id;
    bool online;

    co_mqtt_will_t* will;

} co_mqtt_session_t;

typedef struct
{
    char* name;
    size_t data_size;
    uint8_t* data;
    uint64_t updated_at;

} co_mqtt_topic_t;

typedef struct
{
    int dummy;

} co_mqtt_broker_callbacks_t;

typedef struct
{
    co_thread_t* owner_thread;
    co_mqtt_broker_callbacks_t callbacks;

    struct co_mqtt_broker_config_t
    {
        uint8_t qos;
        bool retain;

    } config;
    
    co_list_t* sessions;
    co_list_t* subscribers;
    co_list_t* topics;

} co_mqtt_broker_t;

//---------------------------------------------------------------------------//
// private
//---------------------------------------------------------------------------//

char*
co_mqtt_create_session_id(
    void
);

co_mqtt_session_t*
co_mqtt_broker_find_session(
    co_mqtt_broker_t* broker,
    const char* id
);

int
co_mqtt_broker_open_session(
    co_mqtt_broker_t* broker,
    const char* id
);

void
co_mqtt_broker_close_session(
    co_mqtt_broker_t* broker,
    const char* id
);

int
co_mqtt_broker_subscribe(
    co_mqtt_broker_t* broker,
    const char* client_id,
    const char* topic_filter,
    const co_mqtt_subscribe_options_t* options
);

int
co_mqtt_broker_unsubscribe(
    co_mqtt_broker_t* broker,
    const char* client_id,
    const char* topic_filter
);

int
co_mqtt_broker_publish(
    co_mqtt_broker_t* broker,
    const char* client_id,
    co_mqtt_topic_t* topic
);

//---------------------------------------------------------------------------//
// public
//---------------------------------------------------------------------------//

CO_MQTT_API
co_mqtt_broker_t*
co_mqtt_broker_create(
    void
);

CO_MQTT_API
void
co_mqtt_broker_destroy(
    co_mqtt_broker_t* broker
);

CO_MQTT_API
co_mqtt_broker_callbacks_t*
co_mqtt_broker_get_callbacks(
    co_mqtt_broker_t* broker
);

CO_MQTT_API
void
co_mqtt_broker_action(
    co_mqtt_broker_t* broker,
    co_thread_t* thread,
    co_mqtt_client_t* client,
    const co_mqtt_packet_t* packet
);

//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//

CO_EXTERN_C_END

#endif // CO_MQTT_BROKER_H_INCLUDED
