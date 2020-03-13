
typedef enum {
    MQTT_CONNECTED,
    MQTT_DISCONNECTED,
    MQTT_SUBSCRIBED,
    MQTT_UNSUBSCRIBED,
    MQTT_PUBLISHED,
    MQTT_RECEIVED_DATA,
    MQTT_ERROR,
} mqtt_connection_status_t;

typedef void (*mqtt_callback_t)(mqtt_connection_status_t status);

void mqtt_init(const char * mqtt_broker, mqtt_callback_t);
void mqtt_subscribe(const char * topic);