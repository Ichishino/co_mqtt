#include <coldforce/core/co_std.h>

#include <coldforce/mqtt/co_mqtt_log.h>
#include <coldforce/mqtt/co_mqtt_packet.h>

//---------------------------------------------------------------------------//
// mqtt log
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
// private
//---------------------------------------------------------------------------//

void
co_mqtt_log_write_packet(
    int level,
    const co_net_addr_t* addr1,
    const char* text,
    const co_net_addr_t* addr2,
    const co_mqtt_packet_t* packet,
    const char* format,
    ...
)
{
    co_log_t* log = co_log_get_default();

    if (level > log->category[CO_LOG_CATEGORY_MQTT].level)
    {
        return;
    }

    co_mutex_lock(log->mutex);

    co_log_write_header(
        level, CO_LOG_CATEGORY_MQTT);

    co_net_log_write_addresses(
        log, CO_LOG_CATEGORY_MQTT, addr1, text, addr2);

    FILE* fp =
        (FILE*)log->category[CO_LOG_CATEGORY_MQTT].output;

    va_list args;
    va_start(args, format);
    vfprintf(fp, format, args);
    va_end(args);

    fprintf(fp, "\n");

    co_log_write_header(
        level, CO_LOG_CATEGORY_MQTT);
    fprintf(fp,
        "-------------------------------------------------------------\n");

    const char* type = "";

    switch (packet->header.type)
    {
    case CO_MQTT_PACKET_TYPE_CONNECT:
        type = "CONNECT";
        break;
    case CO_MQTT_PACKET_TYPE_CONNACK:
        type = "CONACK";
        break;
    case CO_MQTT_PACKET_TYPE_PUBLISH:
        type = "PUBLISH";
        break;
    case CO_MQTT_PACKET_TYPE_PUBACK:
        type = "PUBACK";
        break;
    case CO_MQTT_PACKET_TYPE_PUBREC:
        type = "PUBREC";
        break;
    case CO_MQTT_PACKET_TYPE_PUBREL:
        type = "PUBREL";
        break;
    case CO_MQTT_PACKET_TYPE_PUBCOMP:
        type = "PUBCOMP";
        break;
    case CO_MQTT_PACKET_TYPE_SUBSCRIBE:
        type = "SUBSCRIBE";
        break;
    case CO_MQTT_PACKET_TYPE_SUBACK:
        type = "SUBACK";
        break;
    case CO_MQTT_PACKET_TYPE_UNSUBSCRIBE:
        type = "UNSUBSCRIBE";
        break;
    case CO_MQTT_PACKET_TYPE_UNSUBACK:
        type = "UNSUBACK";
        break;
    case CO_MQTT_PACKET_TYPE_PINGREQ:
        type = "PINGREQ";
        break;
    case CO_MQTT_PACKET_TYPE_PINGRESP:
        type = "PINGRESP";
        break;
    case CO_MQTT_PACKET_TYPE_DISCONNECT:
        type = "DISCONNECT";
        break;
    case CO_MQTT_PACKET_TYPE_AUTH:
        type = "AUTH";
        break;
    default:
        type = "????";
        break;
    }

    co_log_write_header(
        level, CO_LOG_CATEGORY_MQTT);

    fprintf(fp,
        "%s(%02x) dup(%d) qos(%d) retain(%d) payload_size (%d)\n",
        type, packet->header.type,
        packet->header.dup,
        packet->header.qos,
        packet->header.retain,
        packet->header.remaining_length);

    co_log_write_header(
        level, CO_LOG_CATEGORY_MQTT);
    fprintf(fp,
        "-------------------------------------------------------------\n");

    fflush(fp);

    co_mutex_unlock(log->mutex);
}

//---------------------------------------------------------------------------//
// public
//---------------------------------------------------------------------------//

void
co_mqtt_log_set_level(
    int level
)
{
    co_log_set_level(
        CO_LOG_CATEGORY_MQTT, level);

    co_log_add_category(
        CO_LOG_CATEGORY_MQTT,
        CO_LOG_CATEGORY_NAME_MQTT);
}
