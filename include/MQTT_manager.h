#include "mqtt_client.h"


typedef void (*mqtt_callback_t)(esp_mqtt_event_handle_t event);

void mqtt_init(const char * mqtt_broker, mqtt_callback_t);
void mqtt_subscribe(const char * topic);
void mqtt_publish(const char * topic, const char * message);
void mqtt_reconnect();