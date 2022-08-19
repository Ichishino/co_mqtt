#include <coldforce/coldforce_mqtt.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// my app object
typedef struct
{
    co_app_t base_app;

    // my app data
    co_tcp_server_t* server;
    co_list_t* clients;

} my_app;

void on_my_mqtt_receive_packet(my_app* self, co_mqtt_client_t* client, const co_mqtt_packet_t* packet)
{
    switch (packet->header.type)
    {
    case CO_MQTT_PACKET_TYPE_CONNECT:
    {
        co_mqtt_packet_t* ack_packet =
            co_mqtt_packet_create_connack(false, 0);

        co_mqtt_send_packet(client, ack_packet);

        break;
    }
    case CO_MQTT_PACKET_TYPE_SUBSCRIBE:
    {
        uint8_t reason_code = 0;

        co_mqtt_packet_t* ack_packet =
            co_mqtt_packet_create_suback(packet->body.subscribe.variable_header.id, &reason_code, 1);

        co_mqtt_send_packet(client, ack_packet);

        break;
    }
    }
}

void on_my_mqtt_close(my_app* self, co_mqtt_client_t* client, int error_code)
{
    printf("closed\n");

    co_list_remove(self->clients, client);
}

void on_my_tcp_accept(my_app* self, co_tcp_server_t* tcp_server, co_tcp_client_t* tcp_client)
{
    (void)tcp_server;

    printf("accept\n");

    co_tcp_accept((co_thread_t*)self, tcp_client);

    // upgrade to websocket
    co_mqtt_client_t* mqtt_client = co_tcp_upgrade_to_mqtt(tcp_client);

    // callback
    co_mqtt_callbacks_st* callbacks = co_mqtt_get_callbacks(mqtt_client);
    callbacks->on_receive_packet = (co_mqtt_receive_packet_fn)on_my_mqtt_receive_packet;
    callbacks->on_close = (co_mqtt_close_fn)on_my_mqtt_close;

    co_list_add_tail(self->clients, mqtt_client);
}

bool on_my_app_create(my_app* self)
{
    const co_args_st* args = co_app_get_args((co_app_t*)self);

    if (args->count <= 1)
    {
        printf("<Usage>\n");
        printf("mqtt_server <port_number>\n");

        return false;
    }

    uint16_t port = (uint16_t)atoi(args->values[1]);

    // client list
    co_list_ctx_st list_ctx = { 0 };
    list_ctx.destroy_value = (co_item_destroy_fn)co_mqtt_client_destroy;
    self->clients = co_list_create(&list_ctx);

    // local address
    co_net_addr_t local_net_addr = { 0 };
    co_net_addr_set_family(&local_net_addr, CO_ADDRESS_FAMILY_IPV4);
    co_net_addr_set_port(&local_net_addr, port);

    self->server = co_tcp_server_create(&local_net_addr);

    // socket option
    co_socket_option_set_reuse_addr(
        co_tcp_server_get_socket(self->server), true);

    // callback
    co_tcp_server_callbacks_st* callbacks = co_tcp_server_get_callbacks(self->server);
    callbacks->on_accept = (co_tcp_accept_fn)on_my_tcp_accept;

    // listen start
    co_tcp_server_start(self->server, SOMAXCONN);

    printf("ws://127.0.0.1:%d\n", port);

    return true;
}

void on_my_app_destroy(my_app* self)
{
    co_list_destroy(self->clients);
    co_tcp_server_destroy(self->server);
}

int main(int argc, char* argv[])
{
    co_mqtt_log_set_level(CO_LOG_LEVEL_MAX);
    co_tcp_log_set_level(CO_LOG_LEVEL_MAX);

    my_app app = { 0 };

    co_net_app_init(
        (co_app_t*)&app,
        (co_app_create_fn)on_my_app_create,
        (co_app_destroy_fn)on_my_app_destroy,
        argc, argv);

    // run
    int exit_code = co_app_run((co_app_t*)&app);

    co_net_app_cleanup((co_app_t*)&app);

    return exit_code;
}
