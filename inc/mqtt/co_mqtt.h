#ifndef CO_MQTT_H_INCLUDED
#define CO_MQTT_H_INCLUDED

#include <coldforce/core/co.h>

//---------------------------------------------------------------------------//
// platform
//---------------------------------------------------------------------------//

#ifdef _MSC_VER
#   ifdef CO_MQTT_EXPORTS
#       define CO_MQTT_API  __declspec(dllexport)
#   else
#       define CO_MQTT_API
#   endif
#else
#   define CO_MQTT_API
#endif

CO_EXTERN_C_BEGIN

//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//

#define CO_MQTT_PARSE_COMPLETE      0
#define CO_MQTT_PARSE_MORE_DATA     1
#define CO_MQTT_PARSE_ERROR         -1

#define CO_MQTT_ERROR_RECEIVED_INVALID_DATA     -8001


//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//

CO_EXTERN_C_END

#endif // CO_MQTT_H_INCLUDED
