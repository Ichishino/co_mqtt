#include <coldforce/core/co_std.h>
#include <coldforce/core/co_string.h>
#include <coldforce/core/co_random.h>

#include <coldforce/mqtt/co_mqtt_broker.h>
#include <coldforce/mqtt/co_mqtt_client.h>
#include <coldforce/mqtt/co_mqtt_log.h>

//---------------------------------------------------------------------------//
// mqtt broker
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
// private
//---------------------------------------------------------------------------//

char*
co_mqtt_create_session_id(
    void
)
{
    static const size_t length = 23;

    char* id = (char*)co_mem_alloc(length + 1);

    co_random_alnum_string(id, length);

    return id;
}

co_mqtt_session_t*
co_mqtt_broker_find_session(
    co_mqtt_broker_t* broker,
    const char* id
)
{
    co_list_iterator_t* it =
        co_list_get_head_iterator(broker->sessions);

    while (it != NULL)
    {
        co_list_data_st* data =
            co_list_get_next(broker->sessions, &it);

        co_mqtt_session_t* session =
            (co_mqtt_session_t*)data->value;

        if (strcmp(session->id, id) == 0)
        {
            return session;
        }
    }

    return NULL;
}

int
co_mqtt_broker_open_session(
    co_mqtt_broker_t* broker,
    const char* id
)
{
    co_mqtt_session_t* session =
        co_mqtt_broker_find_session(broker, id);

    if (session != NULL)
    {
        session->online = true;

        return 0;
    }

    session =
        (co_mqtt_session_t*)co_mem_alloc(
            sizeof(co_mqtt_session_t));

    session->id = co_string_duplicate(id);

    co_list_add_tail(broker->sessions, session);

    return 0;
}

void
co_mqtt_broker_close_session(
    co_mqtt_broker_t* broker,
    const char* id
)
{
    co_list_iterator_t* it =
        co_list_get_head_iterator(
            broker->sessions);

    while (it != NULL)
    {
        co_list_data_st* data =
            co_list_get(broker->sessions, it);

        co_mqtt_session_t* session =
            (co_mqtt_session_t*)data->value;

        if (strcmp(session->id, id) == 0)
        {
            co_list_remove_at(broker->sessions, it);

            return;
        }

        it = co_list_get_next_iterator(
            broker->sessions, it);
    }
}

int
co_mqtt_broker_subscribe(
    co_mqtt_broker_t* broker,
    const char* client_id,
    const char* topic_filter,
    const co_mqtt_subscribe_options_t* options
)
{
    return 0;
}

//---------------------------------------------------------------------------//
// public
//---------------------------------------------------------------------------//

co_mqtt_broker_t*
co_mqtt_broker_create(
    void
)
{
    co_mqtt_broker_t* broker =
        (co_mqtt_broker_t*)co_mem_alloc(sizeof(co_mqtt_broker_t));

    if (broker == NULL)
    {
        return NULL;
    }

    return broker;
}

void
co_mqtt_broker_destroy(
    co_mqtt_broker_t* broker
)
{
    if (broker != NULL)
    {

    }
}

co_mqtt_broker_callbacks_t*
co_mqtt_broker_get_callbacks(
    co_mqtt_broker_t* broker
)
{
    return &broker->callbacks;
}

void
co_mqtt_broker_action(
    co_mqtt_broker_t* broker,
    co_thread_t* thread,
    co_mqtt_client_t* client,
    const co_mqtt_packet_t* packet
)
{
    switch (packet->header.type)
    {
    case CO_MQTT_PACKET_TYPE_CONNECT:
    {
        if (strlen(packet->body.connect.payload.client_id) == 0)
        {

        }

     //   if (broker->callbacks.on_authorize)
        {

        }

        co_mqtt_packet_t* ack_packet =
            co_mqtt_packet_create_connack(false, 0);

        co_mqtt_send_packet(client, ack_packet);

        break;
    }
    case CO_MQTT_PACKET_TYPE_SUBSCRIBE:
    {
        packet->body.subscribe.payload.topic_filters;

        uint8_t reason_code = 0;

        co_mqtt_packet_t* ack_packet =
            co_mqtt_packet_create_suback(
                packet->body.subscribe.variable_header.id,
                &reason_code, 1);

        co_mqtt_send_packet(client, ack_packet);

        break;
    }
    case CO_MQTT_PACKET_TYPE_UNSUBSCRIBE:
    {
        uint8_t reason_code = 0;

        co_mqtt_packet_t* ack_packet =
            co_mqtt_packet_create_unsuback(
                packet->body.subscribe.variable_header.id,
                &reason_code, 1);

        co_mqtt_send_packet(client, ack_packet);

        break;
    }
    default:
        break;
    }
}
