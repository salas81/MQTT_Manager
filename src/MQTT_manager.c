#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sys.h"

/* The examples use WiFi configuration that you can set via project configuration menu
   If you'd rather not, just change the below entries to strings with
   the config you want - ie #define EXAMPLE_WIFI_SSID "mywifissid"
*/
// #define WIFI_SSID                           "NETGEAR28"
// #define WIFI_PASSWORD                       "orangeplum881"
// #define WIFI_CONNECT_RETRIES                3

// /* FreeRTOS event group to signal when we are connected*/
// static EventGroupHandle_t s_wifi_event_group;

// /* The event group allows multiple bits for each event, but we only care about two events:
//  * - we are connected to the AP with an IP
//  * - we failed to connect after the maximum amount of retries */
// #define WIFI_CONNECTED_BIT BIT0
// #define WIFI_FAIL_BIT      BIT1

// static const char *TAG = "wifi station";

// static int s_retry_num = 0;

// static void event_handler(void* arg, esp_event_base_t event_base,
//                                 int32_t event_id, void* event_data)
// {
//     if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
//         esp_wifi_connect();
//     } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
//         if (s_retry_num < WIFI_CONNECT_RETRIES) {
//             esp_wifi_connect();
//             s_retry_num++;
//             ESP_LOGI(TAG, "retry to connect to the AP");
//         } else {
//             xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
//         }
//         ESP_LOGI(TAG,"connect to the AP fail");
//     } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
//         ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
//         ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
//         s_retry_num = 0;
//         xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
//     }
// }

// void wifi_init_sta(void)
// {
//     s_wifi_event_group = xEventGroupCreate();

//     ESP_ERROR_CHECK(esp_netif_init());

//     ESP_ERROR_CHECK(esp_event_loop_create_default());
//     esp_netif_create_default_wifi_sta();

//     wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
//     ESP_ERROR_CHECK(esp_wifi_init(&cfg));

//     ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
//     ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL));

//     wifi_config_t wifi_config = {
//         .sta = {
//             .ssid = WIFI_SSID,
//             .password = WIFI_PASSWORD
//         },
//     };
//     ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
//     ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config) );
//     ESP_ERROR_CHECK(esp_wifi_start() );

//     ESP_LOGI(TAG, "wifi_init_sta finished.");

//     /* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
//      * number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above) */
//     EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
//             WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
//             pdFALSE,
//             pdFALSE,
//             portMAX_DELAY);

//     /* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
//      * happened. */
//     if (bits & WIFI_CONNECTED_BIT) {
//         ESP_LOGI(TAG, "connected to ap SSID:%s password:%s",
//                  WIFI_SSID, WIFI_PASSWORD);
//     } else if (bits & WIFI_FAIL_BIT) {
//         ESP_LOGI(TAG, "Failed to connect to SSID:%s, password:%s",
//                  WIFI_SSID, WIFI_PASSWORD);
//     } else {
//         ESP_LOGE(TAG, "UNEXPECTED EVENT");
//     }

//     ESP_ERROR_CHECK(esp_event_handler_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler));
//     ESP_ERROR_CHECK(esp_event_handler_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler));
//     vEventGroupDelete(s_wifi_event_group);
// }

// #include <stdio.h>
// #include <string.h>
// #include "mqtt.h"
// #include "mqtt_client.h"
// #include "cJSON.h"
// #include "cJSON_Utils.h"
// #include "mdns.h"
// #include "esp_log.h"
// #include "esp_system.h"

// #define nullptr ((void*)0)

// static const char TAG[] = "MQTT_Manager";

// esp_mqtt_client_handle_t MQTTClient;

// bool debugActive = true;
// bool rebootMessageSent = false;

// void publish_status_task(void *pvParameter) {
//     // ESP_LOGI(TAG, "Starting publish_status Task");

//     // while (1) {
//     //     EventBits_t event_bits = xEventGroupWaitBits(event_group, THERMOSTAT_STATUS_UPDATED, pdTRUE, pdTRUE, PUBLISH_STATUS_TASK_BLOCK_TICKS);

//     //     if ( (event_bits & THERMOSTAT_STATUS_UPDATED) == THERMOSTAT_STATUS_UPDATED) {
//     //         // New data available
//     //         cJSON *root;
//     //         root = cJSON_CreateObject();
            
//     //         float currentTemperature;
//     //         float targetTemperature;
//     //         CurrentHeatingCoolingState currentHeatingCoolingState;

//     //         if (thermostat.getCurrentHeatingCoolingState(&currentHeatingCoolingState) == ESP_OK) {
//     //             cJSON_AddNumberToObject(root, CurrentHeatingCoolingStateKey, currentHeatingCoolingState);
//     //         }

//     //         if (thermostat.getCurrentTemperature(&currentTemperature) == ESP_OK) {
//     //             cJSON_AddNumberToObject(root, CurrentTemperatureKey, currentTemperature);
//     //         }

//     //         if (thermostat.getTargetTemperature(&targetTemperature) == ESP_OK) {
//     //             cJSON_AddNumberToObject(root, TargetTemperatureKey, targetTemperature);
//     //         }

//     //         char *renderedJSON = cJSON_Print(root);

//     //         // Checking if MQTT client is connected
//     //         EventBits_t mqtt_event_bits = xEventGroupWaitBits(event_group, MQTT_CONNECTED_BIT, pdFALSE, pdTRUE, 0);

//     //         if ( (mqtt_event_bits & MQTT_CONNECTED_BIT) == MQTT_CONNECTED_BIT) {
//     //             // MQTT client connected, can publish
//     //             int msg_id = esp_mqtt_client_publish(MQTTClient, MQTT_THERMOSTAT_TOPIC, renderedJSON, 0, 1, 0);
//     //             xEventGroupClearBits(event_group, THERMOSTAT_STATUS_UPDATED);
//     //         } else {
//     //             // MQTT not connected, skipping publish
//     //             ESP_LOGI(TAG, "MQTT not connected, skipping");
//     //         }   
//     //     } else {
//     //         // xEventGroupWaitBits returned because of timeout
//     //         // TODO: handle missing DHT reading error
//     //         ESP_LOGI(TAG, "No thermostat update available");
//     //         esp_restart();
//     //     }
//     // }
// }

// void handleMQTTMessage( char* topic, const char* data) {
//     // cJSON *message = cJSON_Parse(data);
//     // if (message == NULL) {
//     //     const char *error_ptr = cJSON_GetErrorPtr();
//     //     if (error_ptr != NULL) {
//     //         ESP_LOGI(TAG, "Error parsing JSON: %s\n", error_ptr);
//     //     }
//     //     return;
//     // }

//     // if (cJSON_HasObjectItem(message, TargetHeatingCoolingStateKey)) {
//     //     cJSON *requestedState_JSON = cJSON_GetObjectItem(message, TargetHeatingCoolingStateKey);
//     //     if (cJSON_IsNumber(requestedState_JSON)) {
//     //         TargetHeatingCoolingState requestedState = (TargetHeatingCoolingState)requestedState_JSON->valueint;
//     //         thermostat.setTargetHeatingCoolingState(requestedState);
//     //     }
//     // }

//     // if (cJSON_HasObjectItem(message, TargetTemperatureKey)) {
//     //     cJSON *requestedTargetTemperature_JSON = cJSON_GetObjectItem(message, TargetTemperatureKey);
//     //     if (cJSON_IsNumber(requestedTargetTemperature_JSON)) {
//     //         float requestedTargetTemperature = requestedTargetTemperature_JSON->valuedouble;
//     //         thermostat.setTargetTemperature(requestedTargetTemperature);
//     //     }
//     // }

//     // cJSON_Delete(message);
// }

// // MQTT
// static esp_err_t mqtt_event_handler(esp_mqtt_event_handle_t event){
//     esp_mqtt_client_handle_t client = event->client;
//     int msg_id;

//     // your_context_t *context = event->context;
//     switch (event->event_id) {
//         case MQTT_EVENT_CONNECTED:
//             ESP_LOGI(TAG, "MQTT Client connected");
//             xEventGroupSetBits(event_group, MQTT_CONNECTED_BIT);
//             esp_mqtt_client_subscribe(MQTTClient, MQTT_THERMOSTAT_SET_TOPIC, 0);
//             xEventGroupSetBits(event_group, THERMOSTAT_STATUS_UPDATED); // Send initial thermostat state
//             break;
//         case MQTT_EVENT_DISCONNECTED:
//             ESP_LOGI(TAG, "MQTT Client disconnected");
//             xEventGroupClearBits(event_group, MQTT_CONNECTED_BIT);
//             ESP_LOGI(TAG, "Reconnecting MQTT client");
//             esp_mqtt_client_start(MQTTClient);
//             break;
//         case MQTT_EVENT_SUBSCRIBED:
//             ESP_LOGI(TAG, "MQTT client subscribed to %s", MQTT_THERMOSTAT_SET_TOPIC);
//             break;
//         case MQTT_EVENT_UNSUBSCRIBED:
//             ESP_LOGI(TAG, "MQTT client unsubscribed from %s", MQTT_THERMOSTAT_SET_TOPIC);
//             break;
//         case MQTT_EVENT_PUBLISHED:
//             break;
//         case MQTT_EVENT_DATA:
//             ESP_LOGI(TAG, "MQTT client received new message:\nTopic: %s\nData: %s", event->topic, event->data);
//             handleMQTTMessage(event->topic, event->data);
//             break;
//         case MQTT_EVENT_ERROR:
//             ESP_LOGI(TAG, "MQTT client error!");
//             break;
//         default:
//             // ESP_LOGI(TAG, "Other event id:%d", event->event_id);
//             break;
//     }
//     return ESP_OK;
// }

// void mqttClientInit(char* hostname) {
//     esp_mqtt_client_config_t mqtt_cfg = {
//     .event_handle = &mqtt_event_handler,
//     .host = hostname,
//     .uri =  nullptr, 
//     .port = MQTT_PORT,               
//     .client_id = nullptr,               
//     .username = nullptr, 
//     .password = nullptr, 
//     .lwt_topic = nullptr, 
//     .lwt_msg = nullptr,
//     .lwt_qos = 0,                      
//     .lwt_retain = 0,
//     .lwt_msg_len = 0,
//     .disable_clean_session = true,
//     .keepalive = 120,
//     .disable_auto_reconnect = false,
//     .user_context = nullptr,
//     .task_prio = 5,
//     .task_stack = 6144,
//     .buffer_size = 1024,
//     .cert_pem = nullptr,
//     .client_cert_pem = nullptr,
//     .client_key_pem = nullptr,
//     .transport = MQTT_TRANSPORT_OVER_TCP,
//     .refresh_connection_after_ms = 0
// };

//     MQTTClient = esp_mqtt_client_init(&mqtt_cfg);
//     esp_mqtt_client_start(MQTTClient);
// }

// // mDNS
// void start_mdns_service() {
//     //initialize mDNS service
//     esp_err_t err = mdns_init();
//     if (err) {
//         ESP_LOGI(TAG, "MDNS Init failed: %d", err);
//         return;
//     }

//     //set hostname
//     mdns_hostname_set("esp32_mDNS_test");
//     //set default instance
//     mdns_instance_name_set("ESP32 device testing mDNS");
// }

// void resolve_mdns_host(const char * host_name, char * resolvedIP) {
//     ESP_LOGI(TAG, "Resolving hostname %s", host_name);

//     struct ip4_addr addr;
//     addr.addr = 0;

//     esp_err_t err = mdns_query_a(host_name, 2000,  &addr);
//     if(err){
//         if(err == ESP_ERR_NOT_FOUND){
//             ESP_LOGI(TAG, "MQTT Broker was not found!");
//             return;
//         }
//         ESP_LOGI(TAG, "Query Failed");
//         return;
//     }

//     sprintf(resolvedIP, "%d.%d.%d.%d", IP2STR(&addr));

//     ESP_LOGI(TAG, "MQTT host resolved: %s", resolvedIP);
//     return;
// }

// void init_mqtt(const char * mqtt_broker) {
//         // start_mdns_service();
//         char hostname[15];
//         resolve_mdns_host(MQTT_BROKER, hostname);
//         mqttClientInit(hostname);
// }