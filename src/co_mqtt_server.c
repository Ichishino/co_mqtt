#include <coldforce/core/co_std.h>

#include <coldforce/mqtt/co_mqtt_server.h>
#include <coldforce/mqtt/co_mqtt_client.h>
#include <coldforce/mqtt/co_mqtt_log.h>

//---------------------------------------------------------------------------//
// mqtt server
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
// private
//---------------------------------------------------------------------------//

void
co_mqtt_server_on_receive_ready(
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

//---------------------------------------------------------------------------//
// public
//---------------------------------------------------------------------------//

co_mqtt_client_t*
co_tcp_upgrade_to_mqtt(
    co_tcp_client_t* tcp_client
)
{
    co_mqtt_client_t* client =
        (co_mqtt_client_t*)co_mem_alloc(sizeof(co_mqtt_client_t));

    if (client == NULL)
    {
        return NULL;
    }

    client->tcp_client = tcp_client;

    co_mqtt_client_setup(client);

    client->tcp_client->callbacks.on_receive =
        (co_tcp_receive_fn)co_mqtt_server_on_receive_ready;
    client->tcp_client->callbacks.on_close =
        (co_tcp_close_fn)co_mqtt_client_on_close;

    return client;
}
