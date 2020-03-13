
#define MQTT_BROKER                         "raspberrypi"
#define MQTT_PORT                           1883

typedef enum {
    CONNECTING,
    CONNECTED,
    DISCONNECTED,
    FAILURE,
} wifi_connection_status_t;

typedef void (*wifi_callback_t)(wifi_connection_status_t status);

void wifi_init_sta(wifi_callback_t);
void mqtt_init();

void init_mqtt(const char * mqtt_broker);