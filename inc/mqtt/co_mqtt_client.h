#ifndef CO_MQTT_CLIENT_H_INCLUDED
#define CO_MQTT_CLIENT_H_INCLUDED

#include <coldforce/net/co_tcp_client.h>
#include <coldforce/net/co_url.h>

#include <coldforce/tls/co_tls.h>

#include <coldforce/mqtt/co_mqtt.h>
#include <coldforce/mqtt/co_mqtt_packet.h>

CO_EXTERN_C_BEGIN

//---------------------------------------------------------------------------//
// mqtt client
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//

struct co_mqtt_client_t;

typedef void(*co_mqtt_connect_fn)(
    co_thread_t* self, struct co_mqtt_client_t* client,
    int error_code);

typedef void(*co_mqtt_close_fn)(
    co_thread_t* self, struct co_mqtt_client_t* client,
    int error_code);

typedef void(*co_mqtt_receive_packet_fn)(
    co_thread_t* self, struct co_mqtt_client_t* client,
    const co_mqtt_packet_t* packet);

typedef struct
{
    co_mqtt_connect_fn on_connect;
    co_mqtt_close_fn on_close;
    co_mqtt_receive_packet_fn on_receive_packet;

} co_mqtt_callbacks_st;

typedef struct co_mqtt_client_t
{
    co_tcp_client_t* tcp_client;
    co_tcp_client_module_t module;
    co_mqtt_callbacks_st callbacks;

    co_url_st* base_url;

    struct mqtt_receive_data
    {
        size_t index;
        co_byte_array_t* ptr;

    } receive_data;

} co_mqtt_client_t;

//---------------------------------------------------------------------------//
// private
//---------------------------------------------------------------------------//

void
co_mqtt_client_setup(
    co_mqtt_client_t* client
);

void
co_mqtt_client_cleanup(
    co_mqtt_client_t* client
);

void
co_mqtt_client_on_receive_packet(
    co_thread_t* thread,
    co_mqtt_client_t* client,
    co_mqtt_packet_t* packet
);

void
co_mqtt_client_on_close(
    co_thread_t* thread,
    co_mqtt_client_t* client,
    int error_code
);

//---------------------------------------------------------------------------//
// public
//---------------------------------------------------------------------------//

CO_MQTT_API
co_mqtt_client_t*
co_mqtt_client_create(
    const char* base_url,
    const co_net_addr_t* local_net_addr,
    co_tls_ctx_st* tls_ctx
);

CO_MQTT_API
void
co_mqtt_client_destroy(
    co_mqtt_client_t* client
);

CO_MQTT_API
co_mqtt_callbacks_st*
co_mqtt_get_callbacks(
    co_mqtt_client_t* client
);

CO_MQTT_API
bool
co_mqtt_connect(
    co_mqtt_client_t* client
);

CO_MQTT_API
void
co_mqtt_close(
    co_mqtt_client_t* client
);

CO_MQTT_API
bool
co_mqtt_send_packet(
    co_mqtt_client_t* client,
    co_mqtt_packet_t* packet
);

//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//

CO_EXTERN_C_END

#endif // CO_MQTT_CLIENT_H_INCLUDED
