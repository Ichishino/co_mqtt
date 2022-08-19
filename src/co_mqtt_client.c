#include <coldforce/core/co_std.h>
#include <coldforce/core/co_string.h>

#include <coldforce/net/co_tcp_client.h>
#include <coldforce/net/co_url.h>

#include <coldforce/tls/co_tls_client.h>

#include <coldforce/mqtt/co_mqtt_client.h>
#include <coldforce/mqtt/co_mqtt_log.h>

//---------------------------------------------------------------------------//
// mqtt client
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
// private
//---------------------------------------------------------------------------//

void
co_mqtt_client_setup(
    co_mqtt_client_t* client
)
{
#ifdef CO_CAN_USE_TLS
    if (client->tcp_client->sock.tls != NULL)
    {
        client->module.destroy = co_tls_client_destroy;
        client->module.close = co_tls_close;
        client->module.connect = co_tls_connect;
        client->module.send = co_tls_send;
        client->module.receive_all = co_tls_receive_all;
    }
    else
    {
#endif
        client->module.destroy = co_tcp_client_destroy;
        client->module.close = co_tcp_close;
        client->module.connect = co_tcp_connect;
        client->module.send = co_tcp_send;
        client->module.receive_all = co_tcp_receive_all;

#ifdef CO_CAN_USE_TLS
    }
#endif

    client->base_url = NULL;
    client->tcp_client->sock.sub_class = client;
    client->receive_data.index = 0;
    client->receive_data.ptr = co_byte_array_create();

    client->callbacks.on_connect = NULL;
    client->callbacks.on_close = NULL;
}

void
co_mqtt_client_cleanup(
    co_mqtt_client_t* client
)
{
    if (client != NULL)
    {
        co_url_destroy(client->base_url);
        client->base_url = NULL;

        co_byte_array_destroy(client->receive_data.ptr);
        client->receive_data.ptr = NULL;
    }
}

void
co_mqtt_client_on_receive_packet(
    co_thread_t* thread,
    co_mqtt_client_t* client,
    co_mqtt_packet_t* packet
)
{
    if (client->callbacks.on_receive_packet != NULL)
    {
        client->callbacks.on_receive_packet(
            thread, client, packet);
    }

    co_mqtt_packet_destroy(packet);
}

void
co_mqtt_client_on_close(
    co_thread_t* thread,
    co_mqtt_client_t* client,
    int error_code
)
{
    if (client->callbacks.on_close != NULL)
    {
        client->callbacks.on_close(
            thread, client, error_code);
    }
}

static void
co_mqtt_client_on_tcp_connect(
    co_thread_t* thread,
    co_tcp_client_t* tcp_client,
    int error_code
)
{
    co_mqtt_client_t* client =
        (co_mqtt_client_t*)tcp_client->sock.sub_class;

    if (client->callbacks.on_connect != NULL)
    {
        client->callbacks.on_connect(
            thread, client, error_code);
    }
}

void
co_mqtt_client_on_tcp_receive_ready(
    co_thread_t* thread,
    co_tcp_client_t* tcp_client
)
{
    co_mqtt_client_t* client =
        (co_mqtt_client_t*)tcp_client->sock.sub_class;

    ssize_t receive_result =
        client->module.receive_all(
            client->tcp_client,
            client->receive_data.ptr);

    if (receive_result <= 0)
    {
        return;
    }

    size_t data_size =
        co_byte_array_get_count(client->receive_data.ptr);

    while (data_size > client->receive_data.index)
    {
        if ((data_size - client->receive_data.index) <
            CO_MQTT_PACKET_HEADER_MIN_SIZE)
        {
            return;
        }

        co_mqtt_packet_t* packet = co_mqtt_packet_create();

        int result = co_mqtt_packet_deserialize(packet,
            co_byte_array_get_ptr(client->receive_data.ptr, 0),
            co_byte_array_get_count(client->receive_data.ptr),
            &client->receive_data.index);

        if (result == CO_MQTT_PARSE_COMPLETE)
        {
            co_mqtt_log_debug_packet(
                &client->tcp_client->sock.local_net_addr,
                "<--",
                &client->tcp_client->remote_net_addr,
                packet,
                "mqtt receive packet");

            co_mqtt_client_on_receive_packet(
                thread, client, packet);

            if (client->tcp_client == NULL)
            {
                return;
            }

            continue;
        }
        else if (result == CO_MQTT_PARSE_MORE_DATA)
        {
            co_mqtt_packet_destroy(packet);

            return;
        }
        else
        {
            co_mqtt_packet_destroy(packet);

            co_mqtt_close(client);

            co_mqtt_client_on_close(
                thread, client, CO_MQTT_ERROR_RECEIVED_INVALID_DATA);

            return;
        }
    }

    client->receive_data.index = 0;
    co_byte_array_clear(client->receive_data.ptr);
}

void
co_mqtt_client_on_tcp_close(
    co_thread_t* thread,
    co_tcp_client_t* tcp_client
)
{
    co_mqtt_client_t* client =
        (co_mqtt_client_t*)tcp_client->sock.sub_class;

    co_mqtt_client_on_close(thread, client, 0);
}

//---------------------------------------------------------------------------//
// public
//---------------------------------------------------------------------------//

co_mqtt_client_t*
co_mqtt_client_create(
    const char* base_url,
    const co_net_addr_t* local_net_addr,
    co_tls_ctx_st* tls_ctx
)
{
    co_mqtt_client_t* client =
        (co_mqtt_client_t*)co_mem_alloc(sizeof(co_mqtt_client_t));

    if (client == NULL)
    {
        return NULL;
    }

    co_url_st* url = co_url_create(base_url);

    if (url->host == NULL)
    {
        return false;
    }

    bool tls_scheme = false;

    if (url->scheme == NULL)
    {
        co_string_destroy(url->scheme);
        url->scheme = co_string_duplicate("http");
    }
    else
    {
        if (co_string_case_compare(url->scheme, "mqtts") == 0)
        {
            tls_scheme = true;

            co_string_destroy(url->scheme);
            url->scheme = co_string_duplicate("https");
        }
        else if (co_string_case_compare(url->scheme, "mqtt") == 0)
        {
            co_string_destroy(url->scheme);
            url->scheme = co_string_duplicate("http");
        }
        else if (co_string_case_compare(url->scheme, "https") == 0)
        {
            tls_scheme = true;
        }
    }

    if (url->port == 0)
    {
        if (tls_scheme)
        {
            url->port = 8883;
        }
        else
        {
            url->port = 1883;
        }
    }

    if (tls_scheme)
    {
#ifdef CO_CAN_USE_TLS
        client->tcp_client =
            co_tls_client_create(local_net_addr, tls_ctx);

        if (client->tcp_client != NULL)
        {
            co_tls_set_host_name(
                client->tcp_client, url->host);
        }
#else
        (void)tls_ctx;

        co_mqtt_log_error(NULL, NULL, NULL,
            "OpenSSL is not installed");

        return false;
#endif
    }
    else
    {
        client->tcp_client =
            co_tcp_client_create(local_net_addr);
    }

    if (client->tcp_client == NULL)
    {
        return false;
    }

    int address_family =
        co_net_addr_get_family(local_net_addr);

    co_net_addr_init(&client->tcp_client->remote_net_addr);

    if (!co_url_to_net_addr(
        url, address_family,
        &client->tcp_client->remote_net_addr))
    {
        co_mqtt_log_error(NULL, NULL, NULL,
            "failed to resolve hostname (%s)", url->src);

        return false;
    }

    co_mqtt_client_setup(client);

    client->base_url = url;

    return client;
}

void
co_mqtt_client_destroy(
    co_mqtt_client_t* client
)
{
    if (client != NULL)
    {

    }
}

co_mqtt_callbacks_st*
co_mqtt_get_callbacks(
    co_mqtt_client_t* client
)
{
    return &client->callbacks;
}

bool
co_mqtt_connect(
    co_mqtt_client_t* client
)
{
    client->tcp_client->callbacks.on_connect =
        (co_tcp_connect_fn)co_mqtt_client_on_tcp_connect;

    return client->module.connect(
        client->tcp_client,
        &client->tcp_client->remote_net_addr);
}

void
co_mqtt_close(
    co_mqtt_client_t* client
)
{
    if (client != NULL &&
        client->tcp_client != NULL)
    {
        client->module.close(client->tcp_client);
    }
}

bool
co_mqtt_send_packet(
    co_mqtt_client_t* client,
    co_mqtt_packet_t* packet
)
{
    co_mqtt_log_debug_packet(
        &client->tcp_client->sock.local_net_addr,
        "-->",
        &client->tcp_client->remote_net_addr,
        packet,
        "mqtt send packet");

    co_byte_array_t* buffer = co_byte_array_create();

    co_mqtt_packet_serialize(packet, buffer);

    bool result =
        client->module.send(
            client->tcp_client,
            co_byte_array_get_ptr(buffer, 0),
            co_byte_array_get_count(buffer));

    co_byte_array_destroy(buffer);

    return result;
}
