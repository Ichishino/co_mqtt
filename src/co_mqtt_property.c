#include <coldforce/core/co_std.h>
#include <coldforce/core/co_string.h>

#include <coldforce/net/co_byte_order.h>

#include <coldforce/mqtt/co_mqtt_property.h>
#include <coldforce/mqtt/co_mqtt_packet.h>

//---------------------------------------------------------------------------//
// mqtt property
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
// private
//---------------------------------------------------------------------------//

void
co_mqtt_serialize_properties(
    const co_list_t* properties,
    co_byte_array_t* buffer
)
{
    co_byte_array_t* temp_buffer = co_byte_array_create();

    if (properties != NULL)
    {
        const co_list_iterator_t* it =
            co_list_get_const_head_iterator(properties);

        while (it != NULL)
        {
            const co_list_data_st* data =
                co_list_get_const_next(properties, &it);

            const co_mqtt_property_t* property =
                (const co_mqtt_property_t*)data->value;

            switch (property->id)
            {
            // byte
            case CO_MQTT_PROPERTY_ID_PAYLOAD_FORMAT_INDICATOR:
            case CO_MQTT_PROPERTY_ID_REQUEST_PROBLEM_INFORMATION:
            case CO_MQTT_PROPERTY_ID_REQUEST_RESPONSE_INFORMATION:
            case CO_MQTT_PROPERTY_ID_MAXIMUM_QOS:
            case CO_MQTT_PROPERTY_ID_RETAIN_AVAILABLE:
            case CO_MQTT_PROPERTY_ID_WILDCARD_SUBSCRIPTION_AVAILABLE:
            case CO_MQTT_PROPERTY_ID_SUBSCRIPTION_IDENTIFIER_AVAILABLE:
            case CO_MQTT_PROPERTY_ID_SHARED_SUBSCRIPTION_AVAILABLE:
            {
                co_mqtt_serialize_u8(
                    property->data.u8, temp_buffer);

                break;
            }
            // 2 byte integer
            case CO_MQTT_PROPERTY_ID_SERVER_KEEP_ALIVE:
            case CO_MQTT_PROPERTY_ID_RECEIVE_MAXIMUM:
            case CO_MQTT_PROPERTY_ID_TOPIC_ALIAS_MAXIMUM:
            case CO_MQTT_PROPERTY_ID_TOPIC_ALIAS:
            {
                co_mqtt_serialize_u16(
                    property->data.u16, temp_buffer);

                break;
            }
            // 4 byte integer
            case CO_MQTT_PROPERTY_ID_MESSAGE_EXPIRY_INTERVAL:
            case CO_MQTT_PROPERTY_ID_SESSION_EXPIRY_INTERVAL:
            case CO_MQTT_PROPERTY_ID_WILL_DELAY_INTERVAL:
            case CO_MQTT_PROPERTY_ID_MAXIMUM_PACKET_SIZE:
            {
                co_mqtt_serialize_u32(
                    property->data.u32, temp_buffer);

                break;
            }
            // variable byte integer
            case CO_MQTT_PROPERTY_ID_SUBSCRIPTION_IDENTIFIER:
            {
                co_mqtt_serialize_variable_byte_integer(
                    property->data.u32, temp_buffer);

                break;
            }
            // utf8 string
            case CO_MQTT_PROPERTY_ID_CONTENT_TYPE:
            case CO_MQTT_PROPERTY_ID_RESPONSE_TOPIC:
            case CO_MQTT_PROPERTY_ID_ASSIGNED_CLIENT_IDENTIFIER:
            case CO_MQTT_PROPERTY_ID_AUTHENTICATION_METHOD:
            case CO_MQTT_PROPERTY_ID_RESPONSE_INFORMATION:
            case CO_MQTT_PROPERTY_ID_SERVER_REFERENCE:
            case CO_MQTT_PROPERTY_ID_REASON_STRING:
            {
                co_mqtt_serialize_utf8_string(
                    property->data.utf8_str, temp_buffer);

                break;
            }
            // utf8 string pair
            case CO_MQTT_PROPERTY_ID_USER_PROPERTY:
            {
                co_mqtt_serialize_utf8_string_pair(
                    property->data.utf8_pair.name,
                    property->data.utf8_pair.value,
                    temp_buffer);

                break;
            }
            // binary
            case CO_MQTT_PROPERTY_ID_CORRELATION_DATA:
            case CO_MQTT_PROPERTY_ID_AUTHENTICATION_DATA:
            {
                co_mqtt_serialize_binary_data(
                    property->data.binary.data,
                    property->data.binary.length,
                    temp_buffer);

                break;
            }
            default:
                break;
            }
        }
    }
    
    size_t length =
        co_byte_array_get_count(temp_buffer);

    co_mqtt_serialize_variable_byte_integer(
        (uint32_t)length, buffer);

    if (length > 0)
    {
        co_byte_array_add(buffer,
            co_byte_array_get_ptr(temp_buffer, 0), length);
    }
    
    co_byte_array_destroy(temp_buffer);
}

bool
co_mqtt_deserialize_properties(
    const uint8_t* data,
    size_t* index,
    size_t data_size,
    co_list_t* properties
)
{
    size_t temp_index = (*index);

    uint32_t property_length;

    if (!co_mqtt_deserialize_variable_byte_integer(
        data, &temp_index, data_size,
        &property_length))
    {
        return false;
    }

    for (;;)
    {
        if ((temp_index - (*index)) >= property_length)
        {
            break;
        }

        co_mqtt_property_t* property =
            (co_mqtt_property_t*)co_mem_alloc(sizeof(co_mqtt_property_t));

        property->id =
            co_mqtt_deserialize_u8(data, &temp_index);

        switch (property->id)
        {
        // byte
        case CO_MQTT_PROPERTY_ID_PAYLOAD_FORMAT_INDICATOR:
        case CO_MQTT_PROPERTY_ID_REQUEST_PROBLEM_INFORMATION:
        case CO_MQTT_PROPERTY_ID_REQUEST_RESPONSE_INFORMATION:
        case CO_MQTT_PROPERTY_ID_MAXIMUM_QOS:
        case CO_MQTT_PROPERTY_ID_RETAIN_AVAILABLE:
        case CO_MQTT_PROPERTY_ID_WILDCARD_SUBSCRIPTION_AVAILABLE:
        case CO_MQTT_PROPERTY_ID_SUBSCRIPTION_IDENTIFIER_AVAILABLE:
        case CO_MQTT_PROPERTY_ID_SHARED_SUBSCRIPTION_AVAILABLE:
        {
            property->data.u8 =
                co_mqtt_deserialize_u8(data, &temp_index);

            break;
        }
        // 2 byte integer
        case CO_MQTT_PROPERTY_ID_SERVER_KEEP_ALIVE:
        case CO_MQTT_PROPERTY_ID_RECEIVE_MAXIMUM:
        case CO_MQTT_PROPERTY_ID_TOPIC_ALIAS_MAXIMUM:
        case CO_MQTT_PROPERTY_ID_TOPIC_ALIAS:
        {
            property->data.u16 =
                co_mqtt_deserialize_u16(data, &temp_index);

            break;
        }
        // 4 byte integer
        case CO_MQTT_PROPERTY_ID_MESSAGE_EXPIRY_INTERVAL:
        case CO_MQTT_PROPERTY_ID_SESSION_EXPIRY_INTERVAL:
        case CO_MQTT_PROPERTY_ID_WILL_DELAY_INTERVAL:
        case CO_MQTT_PROPERTY_ID_MAXIMUM_PACKET_SIZE:
        {
            property->data.u32 =
                co_mqtt_deserialize_u32(data, &temp_index);
           
            break;
        }
        // variable byte integer
        case CO_MQTT_PROPERTY_ID_SUBSCRIPTION_IDENTIFIER:
        {
            if (!co_mqtt_deserialize_variable_byte_integer(
                data, &temp_index, data_size,
                &property->data.u32))
            {
                return false;
            }

            break;
        }
        // utf8 string
        case CO_MQTT_PROPERTY_ID_CONTENT_TYPE:
        case CO_MQTT_PROPERTY_ID_RESPONSE_TOPIC:
        case CO_MQTT_PROPERTY_ID_ASSIGNED_CLIENT_IDENTIFIER:
        case CO_MQTT_PROPERTY_ID_AUTHENTICATION_METHOD:
        case CO_MQTT_PROPERTY_ID_RESPONSE_INFORMATION:
        case CO_MQTT_PROPERTY_ID_SERVER_REFERENCE:
        case CO_MQTT_PROPERTY_ID_REASON_STRING:
        {
            if (!co_mqtt_deserialize_utf8_string(
                data, &temp_index, data_size,
                &property->data.utf8_str))
            {
                return false;
            }

            break;
        }
        // utf8 string pair
        case CO_MQTT_PROPERTY_ID_USER_PROPERTY:
        {
            if (!co_mqtt_deserialize_utf8_string_pair(
                data, &temp_index, data_size,
                &property->data.utf8_pair.name,
                &property->data.utf8_pair.value))
            {
                return false;
            }

            break;
        }
        // binary
        case CO_MQTT_PROPERTY_ID_CORRELATION_DATA:
        case CO_MQTT_PROPERTY_ID_AUTHENTICATION_DATA:
        {
            if (!co_mqtt_deserialize_binary_data(
                data, &temp_index, data_size,
                &property->data.binary.data,
                &property->data.binary.length))
            {
                return false;
            }

            break;
        }
        default:
        {
            break;
        }
        }

        co_list_add_tail(properties, property);
    }

    (*index) = temp_index;

    return true;
}

//---------------------------------------------------------------------------//
// public
//---------------------------------------------------------------------------//

void
co_mqtt_property_destroy(
    co_mqtt_property_t* property
)
{
    if (property == NULL)
    {
        return;
    }

    switch (property->id)
    {
        // utf8 string
    case CO_MQTT_PROPERTY_ID_CONTENT_TYPE:
    case CO_MQTT_PROPERTY_ID_RESPONSE_TOPIC:
    case CO_MQTT_PROPERTY_ID_ASSIGNED_CLIENT_IDENTIFIER:
    case CO_MQTT_PROPERTY_ID_AUTHENTICATION_METHOD:
    case CO_MQTT_PROPERTY_ID_RESPONSE_INFORMATION:
    case CO_MQTT_PROPERTY_ID_SERVER_REFERENCE:
    case CO_MQTT_PROPERTY_ID_REASON_STRING:
    {
        co_string_destroy(property->data.utf8_str);

        break;
    }
    // utf8 string pair
    case CO_MQTT_PROPERTY_ID_USER_PROPERTY:
    {
        break;
    }
    // binary
    case CO_MQTT_PROPERTY_ID_CORRELATION_DATA:
    case CO_MQTT_PROPERTY_ID_AUTHENTICATION_DATA:
    {
        co_mem_free(property->data.binary.data);

        break;
    }
    default:
    {
        break;
    }
    }

    co_mem_free(property);
}

co_mqtt_property_t*
co_mqtt_property_create_u8(
    uint8_t id,
    uint8_t value
)
{
    switch (id)
    {
    case CO_MQTT_PROPERTY_ID_PAYLOAD_FORMAT_INDICATOR:
    case CO_MQTT_PROPERTY_ID_REQUEST_PROBLEM_INFORMATION:
    case CO_MQTT_PROPERTY_ID_REQUEST_RESPONSE_INFORMATION:
    case CO_MQTT_PROPERTY_ID_MAXIMUM_QOS:
    case CO_MQTT_PROPERTY_ID_RETAIN_AVAILABLE:
    case CO_MQTT_PROPERTY_ID_WILDCARD_SUBSCRIPTION_AVAILABLE:
    case CO_MQTT_PROPERTY_ID_SUBSCRIPTION_IDENTIFIER_AVAILABLE:
    case CO_MQTT_PROPERTY_ID_SHARED_SUBSCRIPTION_AVAILABLE:
    {
        co_mqtt_property_t* property =
            (co_mqtt_property_t*)co_mem_alloc(sizeof(co_mqtt_property_t));

        if (property == NULL)
        {
            return NULL;
        }

        property->id = id;
        property->data.u8 = value;

        return property;
    }
    default:
        return NULL;
    }
}
