#include <coldforce/core/co_std.h>
#include <coldforce/core/co_string.h>

#include <coldforce/net/co_byte_order.h>

#include <coldforce/mqtt/co_mqtt_packet.h>
#include <coldforce/mqtt/co_mqtt_property.h>
#include <coldforce/mqtt/co_mqtt_log.h>

//---------------------------------------------------------------------------//
// mqtt packet
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
// private
//---------------------------------------------------------------------------//

void
co_mqtt_serialize_variable_byte_integer(
    uint32_t value,
    co_byte_array_t* buffer
)
{
    do
    {
        uint8_t encodedByte = value % 128;
        value /= 128;

        if (value > 0)
        {
            encodedByte |= 128;
        }

        co_byte_array_add(
            buffer, &encodedByte, sizeof(uint8_t));

    } while (value > 0);
}

bool
co_mqtt_deserialize_variable_byte_integer(
    const uint8_t* data,
    size_t* index,
    size_t data_size,
    uint32_t* variable_byte_integer
)
{
    size_t temp_index = (*index);

    uint8_t encodedByte = 0x00;
    uint32_t multiplier = 1;
    uint32_t value = 0;

    do
    {
        if ((data_size - temp_index) == 0)
        {
            return false;
        }

        encodedByte = data[temp_index];
        value += (encodedByte & 127) * multiplier;

        if (multiplier > (128 * 128 * 128))
        {
            return false;
        }

        multiplier *= 128;
        ++temp_index;

    } while ((encodedByte & 128) != 0);

    (*variable_byte_integer) = value;
    (*index) = temp_index;

    return true;
}

void
co_mqtt_serialize_utf8_string(
    const char* str,
    co_byte_array_t* buffer
)
{
    uint16_t length =
        (str != NULL) ? (uint16_t)strlen(str) : 0;

    co_mqtt_serialize_u16(length, buffer);

    if (length > 0)
    {
        co_byte_array_add(buffer, str, length);
    }
}

bool
co_mqtt_deserialize_utf8_string(
    const uint8_t* data,
    size_t* index,
    size_t data_size,
    char** str
)
{
    size_t temp_index = (*index);

    uint16_t length =
        co_mqtt_deserialize_u16(data, &temp_index);

    if ((data_size - temp_index) < length)
    {
        return false;
    }

    (*str) = (char*)co_mem_alloc(length + 1);

    if (length > 0)
    {
        memcpy((*str), &data[temp_index], length);
        temp_index += length;
    }

    (*str)[length] = '\0';
    (*index) = temp_index;
    
    return true;
}

void
co_mqtt_serialize_utf8_string_pair(
    const char* str1,
    const char* str2,
    co_byte_array_t* buffer
)
{
    co_mqtt_serialize_utf8_string(str1, buffer);
    co_mqtt_serialize_utf8_string(str2, buffer);
}

bool
co_mqtt_deserialize_utf8_string_pair(
    const uint8_t* data,
    size_t* index,
    size_t data_size,
    char** str1,
    char** str2
)
{
    if (!co_mqtt_deserialize_utf8_string(data, index, data_size, str1))
    {
        return false;
    }

    if (!co_mqtt_deserialize_utf8_string(data, index, data_size, str2))
    {
        return false;
    }

    return true;
}

void
co_mqtt_serialize_binary_data(
    const void* data,
    uint16_t data_size,
    co_byte_array_t* buffer
)
{
    co_mqtt_serialize_u16(data_size, buffer);

    if (data_size > 0)
    {
        co_byte_array_add(buffer, data, data_size);
    }
}

bool
co_mqtt_deserialize_binary_data(
    const uint8_t* data,
    size_t* index,
    size_t data_size,
    uint8_t** binary_ptr,
    uint16_t* binary_size
)
{
    size_t temp_index = (*index);

    uint16_t size =
        co_mqtt_deserialize_u16(data, &temp_index);

    if ((data_size - temp_index) < size)
    {
        return false;
    }

    if (size > 0)
    {
        (*binary_ptr) = (uint8_t*)co_mem_alloc(size);
        memcpy((*binary_ptr), &data[temp_index], size);
        temp_index += size;
    }

    (*binary_size) = size;
    (*index) = temp_index;
    
    return true;
}

void
co_mqtt_serialize_u8(
    uint8_t value,
    co_byte_array_t* buffer
)
{
    co_byte_array_add(
        buffer, &value, sizeof(uint8_t));
}

uint8_t
co_mqtt_deserialize_u8(
    const uint8_t* data,
    size_t* index
)
{
    return data[(*index)++];
}

void
co_mqtt_serialize_u16(
    uint16_t value,
    co_byte_array_t* buffer
)
{
    value =
        co_byte_order_16_host_to_network(value);

    co_byte_array_add(
        buffer, &value, sizeof(uint16_t));
}

uint16_t
co_mqtt_deserialize_u16(
    const uint8_t* data,
    size_t* index
)
{
    uint16_t u16;

    memcpy(&u16, &data[(*index)], sizeof(uint16_t));
    (*index) += sizeof(uint16_t);

    return co_byte_order_16_network_to_host(u16);
}

void
co_mqtt_serialize_u32(
    uint32_t value,
    co_byte_array_t* buffer
)
{
    value =
        co_byte_order_32_host_to_network(value);

    co_byte_array_add(
        buffer, &value, sizeof(uint32_t));
}

uint32_t
co_mqtt_deserialize_u32(
    const uint8_t* data,
    size_t* index
)
{
    uint32_t u32;

    memcpy(&u32, &data[(*index)], sizeof(uint32_t));
    (*index) += sizeof(uint32_t);

    return co_byte_order_32_network_to_host(u32);
}

static void
co_mqtt_destroy_topic_filter(
    co_mqtt_topic_filter_t* topic_filter
)
{
    co_string_destroy(topic_filter->str);
    co_mem_free(topic_filter);
}

void
co_mqtt_packet_serialize(
    co_mqtt_packet_t* packet,
    co_byte_array_t* buffer
)
{
    uint8_t byte1 = 0x00;

    byte1 |= (packet->header.type << 4);
    byte1 |= (packet->header.dup ? 0x08 : 0x00);
    byte1 |= (packet->header.qos << 1);
    byte1 |= (packet->header.retain ? 0x01 : 0x00);

    co_mqtt_serialize_u8(byte1, buffer);

    co_byte_array_t* temp_buffer = co_byte_array_create();

    switch (packet->header.type)
    {
    case CO_MQTT_PACKET_TYPE_CONNECT:
    {
        co_mqtt_serialize_utf8_string(
            packet->body.connect.variable_header.protocol.name,
            temp_buffer);

        co_mqtt_serialize_u8(
            packet->body.connect.variable_header.protocol.version,
            temp_buffer);

        uint8_t flags = 0x00;

        flags |= (packet->body.connect.variable_header.flags.user_name ?
            0x80 : 0x00);
        flags |= (packet->body.connect.variable_header.flags.password ?
            0x40 : 0x00);
        flags |= (packet->body.connect.variable_header.flags.will_retain ?
            0x20 : 0x00);
        flags |= ((packet->body.connect.variable_header.flags.will_qos & 0x02) ?
            0x10 : 0x00);
        flags |= ((packet->body.connect.variable_header.flags.will_qos & 0x01) ?
            0x08 : 0x00);
        flags |= (packet->body.connect.variable_header.flags.will ?
            0x04 : 0x00);
        flags |= (packet->body.connect.variable_header.flags.clean_start ?
            0x02 : 0x00);

        co_mqtt_serialize_u8(
            flags,
            temp_buffer);

        co_mqtt_serialize_u16(
            packet->body.connect.variable_header.keep_alive_timer,
            temp_buffer);

        co_mqtt_serialize_properties(
            packet->properties,
            temp_buffer);

        co_mqtt_serialize_utf8_string(
            packet->body.connect.payload.client_id,
            temp_buffer);

        if (packet->body.connect.variable_header.flags.will)
        {
            co_mqtt_serialize_properties(
                packet->body.connect.payload.will.properties,
                temp_buffer);

            co_mqtt_serialize_utf8_string(
                packet->body.connect.payload.will.topic,
                temp_buffer);

            co_mqtt_serialize_binary_data(
                packet->body.connect.payload.will.payload.data,
                packet->body.connect.payload.will.payload.length,
                temp_buffer);
        }

        if (packet->body.connect.variable_header.flags.user_name)
        {
            co_mqtt_serialize_utf8_string(
                packet->body.connect.payload.user_name,
                temp_buffer);
        }

        if (packet->body.connect.variable_header.flags.password)
        {
            co_mqtt_serialize_utf8_string(
                packet->body.connect.payload.password,
                temp_buffer);
        }

        break;
    }
    case CO_MQTT_PACKET_TYPE_CONNACK:
    {
        uint8_t flags = 0x00;

        flags |= (packet->body.connack.variable_header.session_present ?
            0x01 : 0x00);

        co_mqtt_serialize_u8(
            flags,
            temp_buffer);

        co_mqtt_serialize_u8(
            packet->body.connack.variable_header.reason_code,
            temp_buffer);

        co_mqtt_serialize_properties(
            packet->properties,
            temp_buffer);

        break;
    }
    case CO_MQTT_PACKET_TYPE_PUBLISH:
    {
        co_mqtt_serialize_utf8_string(
            packet->body.publish.variable_header.topic_name,
            temp_buffer);

        if (packet->header.qos == CO_MQTT_QOS_1 ||
            packet->header.qos == CO_MQTT_QOS_2)
        {
            co_mqtt_serialize_u16(
                packet->body.publish.variable_header.id,
                temp_buffer);
        }

        co_mqtt_serialize_properties(
            packet->properties,
            temp_buffer);

        co_mqtt_serialize_binary_data(
            packet->body.publish.payload.data,
            packet->body.publish.payload.length,
            temp_buffer);

        break;
    }
    case CO_MQTT_PACKET_TYPE_PUBACK:
    {
        co_mqtt_serialize_u16(
            packet->body.puback.variable_header.id,
            temp_buffer);

        co_mqtt_serialize_u8(
            packet->body.puback.variable_header.reason_code,
            temp_buffer);

        co_mqtt_serialize_properties(
            packet->properties,
            temp_buffer);

        break;
    }
    case CO_MQTT_PACKET_TYPE_PUBREC:
    {
        co_mqtt_serialize_u16(
            packet->body.pubrec.variable_header.id,
            temp_buffer);

        co_mqtt_serialize_u8(
            packet->body.pubrec.variable_header.reason_code,
            temp_buffer);

        co_mqtt_serialize_properties(
            packet->properties,
            temp_buffer);

        break;
    }
    case CO_MQTT_PACKET_TYPE_PUBREL:
    {
        co_mqtt_serialize_u16(
            packet->body.pubrel.variable_header.id,
            temp_buffer);

        co_mqtt_serialize_u8(
            packet->body.pubrel.variable_header.reason_code,
            temp_buffer);

        co_mqtt_serialize_properties(
            packet->properties,
            temp_buffer);

        break;
    }
    case CO_MQTT_PACKET_TYPE_PUBCOMP:
    {
        co_mqtt_serialize_u16(
            packet->body.pubcomp.variable_header.id,
            temp_buffer);

        co_mqtt_serialize_u8(
            packet->body.pubcomp.variable_header.reason_code,
            temp_buffer);

        co_mqtt_serialize_properties(
            packet->properties,
            temp_buffer);

        break;
    }
    case CO_MQTT_PACKET_TYPE_SUBSCRIBE:
    {
        co_mqtt_serialize_u16(
            packet->body.subscribe.variable_header.id,
            temp_buffer);

        const co_list_iterator_t* it = 
            co_list_get_const_head_iterator(
                packet->body.subscribe.payload.topic_filters);

        while (it != NULL)
        {
            const co_list_data_st* data =
                co_list_get_const_next(
                    packet->body.subscribe.payload.topic_filters, &it);

            const co_mqtt_topic_filter_t* topic_filter =
                (const co_mqtt_topic_filter_t*)data->value;

            co_mqtt_serialize_utf8_string(
                topic_filter->str,
                temp_buffer);

            uint8_t options = 0x00;

            options |= ((topic_filter->options.retain_handling & 0x03) << 4);
            options |= (topic_filter->options.retain_as_published ? 0x08 : 0x00);
            options |= (topic_filter->options.no_local ? 0x04 : 0x00);
            options |= (topic_filter->options.qos & 0x03);

            co_mqtt_serialize_u8(
                options,
                temp_buffer);
        }

        break;
    }
    case CO_MQTT_PACKET_TYPE_SUBACK:
    {
        co_mqtt_serialize_u16(
            packet->body.suback.variable_header.id,
            temp_buffer);

        co_mqtt_serialize_properties(
            packet->properties,
            temp_buffer);

        for (size_t index = 0;
            index < packet->body.suback.payload.reason_code_count;
            ++index)
        {
            co_mqtt_serialize_u8(
                packet->body.suback.payload.reason_codes[index],
                temp_buffer);
        }

        break;
    }
    case CO_MQTT_PACKET_TYPE_UNSUBSCRIBE:
    {
        co_mqtt_serialize_u16(
            packet->body.unsubscribe.variable_header.id,
            temp_buffer);

        co_mqtt_serialize_properties(
            packet->properties,
            temp_buffer);

        const co_list_iterator_t* it =
            co_list_get_const_head_iterator(
                packet->body.unsubscribe.payload.topic_filters);

        while (it != NULL)
        {
            const co_list_data_st* data =
                co_list_get_const_next(
                    packet->body.unsubscribe.payload.topic_filters, &it);

            co_mqtt_serialize_utf8_string(
                data->value,
                temp_buffer);
        }

        break;
    }
    case CO_MQTT_PACKET_TYPE_UNSUBACK:
    {
        co_mqtt_serialize_u16(
            packet->body.unsuback.variable_header.id,
            temp_buffer);

        co_mqtt_serialize_properties(
            packet->properties,
            temp_buffer);

        for (size_t index = 0;
            index < packet->body.unsuback.payload.reason_code_count;
            ++index)
        {
            co_mqtt_serialize_u8(
                packet->body.unsuback.payload.reason_codes[index],
                temp_buffer);
        }

        break;
    }
    case CO_MQTT_PACKET_TYPE_PINGREQ:
    {
        break;
    }
    case CO_MQTT_PACKET_TYPE_PINGRESP:
    {
        break;
    }
    case CO_MQTT_PACKET_TYPE_DISCONNECT:
    {
        co_mqtt_serialize_u8(
            packet->body.disconnect.variable_header.reason_code,
            temp_buffer);

        co_mqtt_serialize_properties(
            packet->properties,
            temp_buffer);

        break;
    }
    case CO_MQTT_PACKET_TYPE_AUTH:
    {
        co_mqtt_serialize_u8(
            packet->body.auth.variable_header.reason_code,
            temp_buffer);

        co_mqtt_serialize_properties(
            packet->properties,
            temp_buffer);

        break;
    }
    default:
        break;
    }

    uint32_t remaining_length =
        (uint32_t)co_byte_array_get_count(temp_buffer);

    co_mqtt_serialize_variable_byte_integer(
        remaining_length, buffer);

    if (remaining_length > 0)
    {
        co_byte_array_add(buffer,
            co_byte_array_get_ptr(temp_buffer, 0),
            remaining_length);
    }
    
    co_byte_array_destroy(temp_buffer);
}

int
co_mqtt_packet_deserialize(
    co_mqtt_packet_t* packet,
    const uint8_t* data,
    const size_t data_size,
    size_t* index
)
{
    size_t temp_index = (*index);

    uint8_t byte1 =
        co_mqtt_deserialize_u8(data, &temp_index);

    packet->header.type = ((byte1 & 0xf0) >> 4);
    packet->header.dup = ((byte1 & 0x08) != 0x00);
    packet->header.qos = ((byte1 & 0x06) >> 1);
    packet->header.retain = ((byte1 & 0x01) != 0x00);

    if (!co_mqtt_deserialize_variable_byte_integer(
        data, &temp_index, data_size,
        &packet->header.remaining_length))
    {
        return CO_MQTT_PARSE_ERROR;
    }

    if ((data_size - temp_index) <
        packet->header.remaining_length)
    {
        return CO_MQTT_PARSE_MORE_DATA;
    }

    switch (packet->header.type)
    {
    case CO_MQTT_PACKET_TYPE_CONNECT:
    {
        if (!co_mqtt_deserialize_utf8_string(
            data, &temp_index, data_size,
            &packet->body.connect.variable_header.protocol.name))
        {
            return CO_MQTT_PARSE_ERROR;
        }

        packet->body.connect.variable_header.protocol.version =
            co_mqtt_deserialize_u8(data, &temp_index);

        uint8_t flags =
            co_mqtt_deserialize_u8(data, &temp_index);

        packet->body.connect.variable_header.flags.user_name =
            ((flags & 0x80) != 0x00);
        packet->body.connect.variable_header.flags.password =
            ((flags & 0x40) != 0x00);
        packet->body.connect.variable_header.flags.will_retain =
            ((flags & 0x20) != 0x00);
        packet->body.connect.variable_header.flags.will_qos =
            ((flags & 0x10) >> 3) |
            ((flags & 0x08) >> 3);
        packet->body.connect.variable_header.flags.will =
            ((flags & 0x04) != 0x00);
        packet->body.connect.variable_header.flags.clean_start =
            ((flags & 0x02) != 0x00);

        packet->body.connect.variable_header.keep_alive_timer =
            co_mqtt_deserialize_u16(data, &temp_index);

        if (!co_mqtt_deserialize_properties(
            data, &temp_index, data_size,
            packet->properties))
        {
            return CO_MQTT_PARSE_ERROR;
        }

        if (!co_mqtt_deserialize_utf8_string(
            data, &temp_index, data_size,
            &packet->body.connect.payload.client_id))
        {
            return CO_MQTT_PARSE_ERROR;
        }

        if (packet->body.connect.variable_header.flags.will)
        {
            co_list_ctx_st list_ctx = { 0 };
            list_ctx.destroy_value =
                (co_item_destroy_fn)co_mqtt_property_destroy;
            packet->body.connect.payload.will.properties = co_list_create(&list_ctx);

            if (!co_mqtt_deserialize_properties(
                data, &temp_index, data_size,
                packet->body.connect.payload.will.properties))
            {
                return CO_MQTT_PARSE_ERROR;
            }

            if (!co_mqtt_deserialize_utf8_string(
                data, &temp_index, data_size,
                &packet->body.connect.payload.will.topic))
            {
                return CO_MQTT_PARSE_ERROR;
            }

            if (!co_mqtt_deserialize_binary_data(
                data, &temp_index, data_size,
                &packet->body.connect.payload.will.payload.data,
                &packet->body.connect.payload.will.payload.length))
            {
                return CO_MQTT_PARSE_ERROR;
            }
        }

        if (packet->body.connect.variable_header.flags.user_name)
        {
            if (!co_mqtt_deserialize_utf8_string(
                data, &temp_index, data_size,
                &packet->body.connect.payload.user_name))
            {
                return CO_MQTT_PARSE_ERROR;
            }
        }

        if (packet->body.connect.variable_header.flags.password)
        {
            if (!co_mqtt_deserialize_utf8_string(
                data, &temp_index, data_size,
                &packet->body.connect.payload.password))
            {
                return CO_MQTT_PARSE_ERROR;
            }
        }

        break;
    }
    case CO_MQTT_PACKET_TYPE_CONNACK:
    {
        uint8_t flags =
            co_mqtt_deserialize_u8(data, &temp_index);
        packet->body.connack.variable_header.session_present =
            ((flags & 0x01) != 0x00);

        packet->body.connack.variable_header.reason_code =
            co_mqtt_deserialize_u8(data, &temp_index);

        if (!co_mqtt_deserialize_properties(
            data, &temp_index, data_size,
            packet->properties))
        {
            return CO_MQTT_PARSE_ERROR;
        }

        break;
    }
    case CO_MQTT_PACKET_TYPE_PUBLISH:
    {
        if (!co_mqtt_deserialize_utf8_string(
            data, &temp_index, data_size,
            &packet->body.publish.variable_header.topic_name))
        {
            return CO_MQTT_PARSE_ERROR;
        }

        if (packet->header.qos == CO_MQTT_QOS_1 ||
            packet->header.qos == CO_MQTT_QOS_2)
        {
            packet->body.publish.variable_header.id =
                co_mqtt_deserialize_u16(data, &temp_index);
        }

        if (!co_mqtt_deserialize_properties(
            data, &temp_index, data_size,
            packet->properties))
        {
            return CO_MQTT_PARSE_ERROR;
        }

        uint32_t payload_length =
            (uint32_t)(data_size - temp_index);
        packet->body.publish.payload.length =
            payload_length;

        if (payload_length > 0)
        {
            packet->body.publish.payload.data =
                (uint8_t*)co_mem_alloc(payload_length);
            memcpy(packet->body.publish.payload.data,
                &data[temp_index], payload_length);
        }

        temp_index += payload_length;

        break;
    }
    case CO_MQTT_PACKET_TYPE_PUBACK:
    {
        packet->body.puback.variable_header.id =
            co_mqtt_deserialize_u16(data, &temp_index);

        packet->body.puback.variable_header.reason_code =
            co_mqtt_deserialize_u8(data, &temp_index);

        if (!co_mqtt_deserialize_properties(
            data, &temp_index, data_size,
            packet->properties))
        {
            return CO_MQTT_PARSE_ERROR;
        }

        break;
    }
    case CO_MQTT_PACKET_TYPE_PUBREC:
    {
        packet->body.pubrec.variable_header.id =
            co_mqtt_deserialize_u16(data, &temp_index);

        packet->body.pubrec.variable_header.reason_code =
            co_mqtt_deserialize_u8(data, &temp_index);

        if (!co_mqtt_deserialize_properties(
            data, &temp_index, data_size,
            packet->properties))
        {
            return CO_MQTT_PARSE_ERROR;
        }

        break;
    }
    case CO_MQTT_PACKET_TYPE_PUBREL:
    {
        packet->body.pubrel.variable_header.id =
            co_mqtt_deserialize_u16(data, &temp_index);

        packet->body.pubrel.variable_header.reason_code =
            co_mqtt_deserialize_u8(data, &temp_index);

        if (!co_mqtt_deserialize_properties(
            data, &temp_index, data_size,
            packet->properties))
        {
            return CO_MQTT_PARSE_ERROR;
        }

        break;
    }
    case CO_MQTT_PACKET_TYPE_PUBCOMP:
    {
        packet->body.pubcomp.variable_header.id =
            co_mqtt_deserialize_u16(data, &temp_index);

        packet->body.pubcomp.variable_header.reason_code =
            co_mqtt_deserialize_u8(data, &temp_index);

        if (!co_mqtt_deserialize_properties(
            data, &temp_index, data_size,
            packet->properties))
        {
            return CO_MQTT_PARSE_ERROR;
        }

        break;
    }
    case CO_MQTT_PACKET_TYPE_SUBSCRIBE:
    {
        packet->body.subscribe.variable_header.id =
            co_mqtt_deserialize_u16(data, &temp_index);

        if (!co_mqtt_deserialize_properties(
            data, &temp_index, data_size,
            packet->properties))
        {
            return CO_MQTT_PARSE_ERROR;
        }

        co_list_ctx_st list_ctx = { 0 };
        list_ctx.destroy_value = co_mqtt_destroy_topic_filter;
        packet->body.subscribe.payload.topic_filters =
            co_list_create(&list_ctx);

        while ((data_size - temp_index) > 0)
        {
            co_mqtt_topic_filter_t* topic_filter =
                (co_mqtt_topic_filter_t*)co_mem_alloc(
                    sizeof(co_mqtt_topic_filter_t));

            if (!co_mqtt_deserialize_utf8_string(
                data, &temp_index, data_size,
                &topic_filter->str))
            {
                co_mem_free(topic_filter);

                return CO_MQTT_PARSE_ERROR;
            }

            uint8_t options =
                co_mqtt_deserialize_u8(data, &temp_index);

            topic_filter->options.retain_handling =
                ((options & 0x30) >> 4);

            topic_filter->options.retain_as_published =
                ((options & 0x08) != 0x00);

            topic_filter->options.no_local =
                ((options & 0x04) != 0x00);

            topic_filter->options.qos =
                (options & 0x03);

            co_list_add_tail(
                packet->body.subscribe.payload.topic_filters,
                topic_filter);
        }

        break;
    }
    case CO_MQTT_PACKET_TYPE_SUBACK:
    {
        packet->body.suback.variable_header.id =
            co_mqtt_deserialize_u16(data, &temp_index);

        if (!co_mqtt_deserialize_properties(
            data, &temp_index, data_size,
            packet->properties))
        {
            return CO_MQTT_PARSE_ERROR;
        }

        size_t reason_code_count = (data_size - temp_index);

        if (reason_code_count > 0)
        {
            packet->body.suback.payload.reason_codes =
                (uint8_t*)co_mem_alloc(reason_code_count);

            for (size_t reason_code_index = 0;
                reason_code_index < reason_code_count;
                ++reason_code_index)
            {
                packet->body.suback.payload.reason_codes[reason_code_index] =
                    co_mqtt_deserialize_u8(data, &temp_index);
            }
        }

        packet->body.suback.payload.reason_code_count =
            reason_code_count;

        break;
    }
    case CO_MQTT_PACKET_TYPE_UNSUBSCRIBE:
    {
        packet->body.unsubscribe.variable_header.id =
            co_mqtt_deserialize_u16(data, &temp_index);

        if (!co_mqtt_deserialize_properties(
            data, &temp_index, data_size,
            packet->properties))
        {
            return CO_MQTT_PARSE_ERROR;
        }

        packet->body.unsubscribe.payload.topic_filters =
            co_string_list_create();

        while ((data_size - temp_index) > 0)
        {
            char* topic_filter;

            if (!co_mqtt_deserialize_utf8_string(
                data, &temp_index, data_size, &topic_filter))
            {
                return CO_MQTT_PARSE_ERROR;
            }

            co_string_list_add_tail(
                packet->body.unsubscribe.payload.topic_filters,
                topic_filter);

            co_string_destroy(topic_filter);
        }

        break;
    }
    case CO_MQTT_PACKET_TYPE_UNSUBACK:
    {
        packet->body.unsuback.variable_header.id =
            co_mqtt_deserialize_u16(data, &temp_index);

        if (!co_mqtt_deserialize_properties(
            data, &temp_index, data_size,
            packet->properties))
        {
            return CO_MQTT_PARSE_ERROR;
        }

        size_t reason_code_count = (data_size - temp_index);

        if (reason_code_count > 0)
        {
            packet->body.unsuback.payload.reason_codes =
                (uint8_t*)co_mem_alloc(reason_code_count);

            for (size_t reason_code_index = 0;
                reason_code_index < reason_code_count;
                ++reason_code_index)
            {
                packet->body.unsuback.payload.reason_codes[reason_code_index] =
                    co_mqtt_deserialize_u8(data, &temp_index);
            }
        }

        break;
    }
    case CO_MQTT_PACKET_TYPE_PINGREQ:
    {
        break;
    }
    case CO_MQTT_PACKET_TYPE_PINGRESP:
    {
        break;
    }
    case CO_MQTT_PACKET_TYPE_DISCONNECT:
    {
        packet->body.disconnect.variable_header.reason_code =
            co_mqtt_deserialize_u8(data, &temp_index);

        if (!co_mqtt_deserialize_properties(
            data, &temp_index, data_size,
            packet->properties))
        {
            return CO_MQTT_PARSE_ERROR;
        }

        break;
    }
    case CO_MQTT_PACKET_TYPE_AUTH:
    {
        packet->body.auth.variable_header.reason_code =
            co_mqtt_deserialize_u8(data, &temp_index);

        if (!co_mqtt_deserialize_properties(
            data, &temp_index, data_size,
            packet->properties))
        {
            return CO_MQTT_PARSE_ERROR;
        }

        break;
    }
    default:
    {
        return CO_MQTT_PARSE_ERROR;
    }
    }

    (*index) = temp_index;

    return CO_MQTT_PARSE_COMPLETE;
}

//---------------------------------------------------------------------------//
// public
//---------------------------------------------------------------------------//

co_mqtt_packet_t*
co_mqtt_packet_create(
    void
)
{
    co_mqtt_packet_t* packet =
        (co_mqtt_packet_t*)co_mem_alloc(sizeof(co_mqtt_packet_t));

    if (packet == NULL)
    {
        return NULL;
    }

    memset(packet, 0x00, sizeof(co_mqtt_packet_t));

    co_list_ctx_st list_ctx = { 0 };
    list_ctx.destroy_value =
        (co_item_destroy_fn)co_mqtt_property_destroy;
    packet->properties = co_list_create(&list_ctx);

    return packet;
}

void
co_mqtt_packet_destroy(
    co_mqtt_packet_t* packet
)
{
    if (packet == NULL)
    {
        return; 
    }

    switch (packet->header.type)
    {
    case CO_MQTT_PACKET_TYPE_CONNECT:
    {
        co_string_destroy(
            packet->body.connect.variable_header.protocol.name);
        co_string_destroy(
            packet->body.connect.payload.client_id);
        co_string_destroy(
            packet->body.connect.payload.will.topic);
        co_mem_free(
            packet->body.connect.payload.will.payload.data);
        co_list_destroy(
            packet->body.connect.payload.will.properties);
        co_string_destroy(
            packet->body.connect.payload.user_name);
        co_string_destroy(
            packet->body.connect.payload.password);

        break;
    }
    case CO_MQTT_PACKET_TYPE_PUBLISH:
    {
        co_mem_free(
            packet->body.publish.payload.data);

        break;
    }
    case CO_MQTT_PACKET_TYPE_SUBSCRIBE:
    {
        co_list_destroy(
            packet->body.subscribe.payload.topic_filters);

        break;
    }
    case CO_MQTT_PACKET_TYPE_SUBACK:
    {
        co_mem_free(
            packet->body.suback.payload.reason_codes);

        break;
    }
    case CO_MQTT_PACKET_TYPE_UNSUBSCRIBE:
    {
        co_string_list_destroy(
            packet->body.unsubscribe.payload.topic_filters);

        break;
    }
    case CO_MQTT_PACKET_TYPE_UNSUBACK:
    {
        co_mem_free(
            packet->body.unsuback.payload.reason_codes);

        break;
    }
    default:
        break;
    }

    co_list_destroy(packet->properties);
    co_mem_free(packet);
}

co_mqtt_packet_t*
co_mqtt_packet_create_connack(
    bool session_present,
    uint8_t reason_code
)
{
    co_mqtt_packet_t* packet = co_mqtt_packet_create();

    if (packet == NULL)
    {
        return NULL;
    }

    packet->header.type = CO_MQTT_PACKET_TYPE_CONNACK;
    packet->body.connack.variable_header.session_present = session_present;
    packet->body.connack.variable_header.reason_code = reason_code;

    return packet;
}

co_mqtt_packet_t*
co_mqtt_packet_create_suback(
    uint16_t id,
    const uint8_t* reason_codes,
    size_t reason_code_count
)
{
    co_mqtt_packet_t* packet = co_mqtt_packet_create();

    if (packet == NULL)
    {
        return NULL;
    }

    packet->header.type = CO_MQTT_PACKET_TYPE_SUBACK;

    packet->body.suback.variable_header.id = id;
    packet->body.suback.payload.reason_codes =
        (uint8_t*)co_mem_alloc(sizeof(uint8_t) * reason_code_count);
    packet->body.suback.payload.reason_code_count = reason_code_count;

    for (size_t index = 0; index < reason_code_count; ++index)
    {
        packet->body.suback.payload.reason_codes[index] =
            reason_codes[index];
    }

    return packet;
}

co_mqtt_packet_t*
co_mqtt_packet_create_unsuback(
    uint16_t id,
    const uint8_t* reason_codes,
    size_t reason_code_count
)
{
    co_mqtt_packet_t* packet = co_mqtt_packet_create();

    if (packet == NULL)
    {
        return NULL;
    }

    packet->header.type = CO_MQTT_PACKET_TYPE_SUBACK;

    packet->body.unsuback.variable_header.id = id;
    packet->body.unsuback.payload.reason_codes =
        (uint8_t*)co_mem_alloc(sizeof(uint8_t) * reason_code_count);
    packet->body.unsuback.payload.reason_code_count = reason_code_count;

    for (size_t index = 0; index < reason_code_count; ++index)
    {
        packet->body.unsuback.payload.reason_codes[index] =
            reason_codes[index];
    }

    return packet;
}
