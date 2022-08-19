#ifndef CO_MQTT_LOG_H_INCLUDED
#define CO_MQTT_LOG_H_INCLUDED

#include <coldforce/core/co_log.h>

#include <coldforce/net/co_net_log.h>

#include <coldforce/mqtt/co_mqtt.h>

CO_EXTERN_C_BEGIN

struct co_mqtt_packet_t;

//---------------------------------------------------------------------------//
// mqtt log
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//

#define CO_LOG_CATEGORY_NAME_MQTT        "MQTT"

#define CO_LOG_CATEGORY_MQTT             (CO_LOG_CATEGORY_USER_MAX + 8)

//---------------------------------------------------------------------------//
// private
//---------------------------------------------------------------------------//

void
co_mqtt_log_write_packet(
    int level,
    const co_net_addr_t* addr1,
    const char* text,
    const co_net_addr_t* addr2,
    const struct co_mqtt_packet_t* packet,
    const char* format,
    ...
);

#define co_mqtt_log_write(level, addr1, text, addr2, format, ...) \
    co_net_log_write(level, CO_LOG_CATEGORY_MQTT, \
        addr1, text, addr2, format, ##__VA_ARGS__)

#define co_mqtt_log_error(addr1, text, addr2, format, ...) \
    co_mqtt_log_write(CO_LOG_LEVEL_ERROR, \
        addr1, text, addr2, format, ##__VA_ARGS__)

#define co_mqtt_log_warning(addr1, text, addr2, format, ...) \
    co_mqtt_log_write(CO_LOG_LEVEL_WARNING, \
        addr1, text, addr2, format, ##__VA_ARGS__)

#define co_mqtt_log_info(addr1, text, addr2, format, ...) \
    co_mqtt_log_write(CO_LOG_LEVEL_INFO, \
        addr1, text, addr2, format, ##__VA_ARGS__)

#define co_mqtt_log_debug(addr1, text, addr2, format, ...) \
    co_mqtt_log_write(CO_LOG_LEVEL_DEBUG, \
        addr1, text, addr2, format, ##__VA_ARGS__)

#define co_mqtt_log_debug_packet(addr1, text, addr2, packet, format, ...) \
    co_mqtt_log_write_packet(CO_LOG_LEVEL_DEBUG, \
        addr1, text, addr2, packet, format, ##__VA_ARGS__)

//---------------------------------------------------------------------------//
// public
//---------------------------------------------------------------------------//

CO_MQTT_API
void
co_mqtt_log_set_level(
    int level
);

//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//

CO_EXTERN_C_END

#endif // CO_MQTT_LOG_H_INCLUDED
