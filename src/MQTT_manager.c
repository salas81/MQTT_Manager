#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"
#include "MQTT_manager.h"
#include "mqtt_client.h"
#include "cJSON.h"
#include "cJSON_Utils.h"
#include "mdns.h"



#define nullptr                             ((void*)0)
#define MQTT_PORT                           1883

#define MQTT_CONNECTED_BIT                  BIT0
#define MQTT_DISCONNECTED_BIT               BIT1
#define MQTT_SUBSCRIBED_BIT                 BIT2
#define MQTT_UNSUBSCRIBED_BIT               BIT3
#define MQTT_PUBLISHED_BIT                  BIT4
#define MQTT_RECEIVED_DATA_BIT              BIT5
#define MQTT_ERROR_BIT                      BIT6




static EventGroupHandle_t mqtt_event_group;
static const char TAG[] = "MQTT_Manager";
static mqtt_callback_t mqtt_callback;
static esp_mqtt_client_handle_t MQTTClient;


void publish_status_task(void *pvParameter) {
    // ESP_LOGI(TAG, "Starting publish_status Task");

    // while (1) {
    //     EventBits_t event_bits = xEventGroupWaitBits(event_group, THERMOSTAT_STATUS_UPDATED, pdTRUE, pdTRUE, PUBLISH_STATUS_TASK_BLOCK_TICKS);

    //     if ( (event_bits & THERMOSTAT_STATUS_UPDATED) == THERMOSTAT_STATUS_UPDATED) {
    //         // New data available
    //         cJSON *root;
    //         root = cJSON_CreateObject();
            
    //         float currentTemperature;
    //         float targetTemperature;
    //         CurrentHeatingCoolingState currentHeatingCoolingState;

    //         if (thermostat.getCurrentHeatingCoolingState(&currentHeatingCoolingState) == ESP_OK) {
    //             cJSON_AddNumberToObject(root, CurrentHeatingCoolingStateKey, currentHeatingCoolingState);
    //         }

    //         if (thermostat.getCurrentTemperature(&currentTemperature) == ESP_OK) {
    //             cJSON_AddNumberToObject(root, CurrentTemperatureKey, currentTemperature);
    //         }

    //         if (thermostat.getTargetTemperature(&targetTemperature) == ESP_OK) {
    //             cJSON_AddNumberToObject(root, TargetTemperatureKey, targetTemperature);
    //         }

    //         char *renderedJSON = cJSON_Print(root);

    //         // Checking if MQTT client is connected
    //         EventBits_t mqtt_event_bits = xEventGroupWaitBits(event_group, MQTT_CONNECTED_BIT, pdFALSE, pdTRUE, 0);

    //         if ( (mqtt_event_bits & MQTT_CONNECTED_BIT) == MQTT_CONNECTED_BIT) {
    //             // MQTT client connected, can publish
    //             int msg_id = esp_mqtt_client_publish(MQTTClient, MQTT_THERMOSTAT_TOPIC, renderedJSON, 0, 1, 0);
    //             xEventGroupClearBits(event_group, THERMOSTAT_STATUS_UPDATED);
    //         } else {
    //             // MQTT not connected, skipping publish
    //             ESP_LOGI(TAG, "MQTT not connected, skipping");
    //         }   
    //     } else {
    //         // xEventGroupWaitBits returned because of timeout
    //         // TODO: handle missing DHT reading error
    //         ESP_LOGI(TAG, "No thermostat update available");
    //         esp_restart();
    //     }
    // }
}

void handleMQTTMessage( char* topic, const char* data) {
    // cJSON *message = cJSON_Parse(data);
    // if (message == NULL) {
    //     const char *error_ptr = cJSON_GetErrorPtr();
    //     if (error_ptr != NULL) {
    //         ESP_LOGI(TAG, "Error parsing JSON: %s\n", error_ptr);
    //     }
    //     return;
    // }

    // if (cJSON_HasObjectItem(message, TargetHeatingCoolingStateKey)) {
    //     cJSON *requestedState_JSON = cJSON_GetObjectItem(message, TargetHeatingCoolingStateKey);
    //     if (cJSON_IsNumber(requestedState_JSON)) {
    //         TargetHeatingCoolingState requestedState = (TargetHeatingCoolingState)requestedState_JSON->valueint;
    //         thermostat.setTargetHeatingCoolingState(requestedState);
    //     }
    // }

    // if (cJSON_HasObjectItem(message, TargetTemperatureKey)) {
    //     cJSON *requestedTargetTemperature_JSON = cJSON_GetObjectItem(message, TargetTemperatureKey);
    //     if (cJSON_IsNumber(requestedTargetTemperature_JSON)) {
    //         float requestedTargetTemperature = requestedTargetTemperature_JSON->valuedouble;
    //         thermostat.setTargetTemperature(requestedTargetTemperature);
    //     }
    // }

    // cJSON_Delete(message);
}

static void mqtt_manager_task(void *args) {

    EventBits_t bits;

    while (1) {
        bits = xEventGroupWaitBits(mqtt_event_group, MQTT_CONNECTED_BIT | MQTT_DISCONNECTED_BIT | MQTT_SUBSCRIBED_BIT | MQTT_UNSUBSCRIBED_BIT | MQTT_PUBLISHED_BIT | MQTT_RECEIVED_DATA_BIT | MQTT_ERROR_BIT, pdFALSE, pdFALSE, portMAX_DELAY); 
        if (bits & MQTT_CONNECTED_BIT) {
            xEventGroupClearBits(mqtt_event_group, MQTT_CONNECTED_BIT);
            mqtt_callback(MQTT_CONNECTED);
        } else if (bits & MQTT_DISCONNECTED_BIT) {
            xEventGroupClearBits(mqtt_event_group, MQTT_DISCONNECTED_BIT);
            mqtt_callback(MQTT_DISCONNECTED);
        } else if (bits & MQTT_SUBSCRIBED_BIT) {
            xEventGroupClearBits(mqtt_event_group, MQTT_SUBSCRIBED_BIT);
            mqtt_callback(MQTT_SUBSCRIBED);
        } else if (bits & MQTT_UNSUBSCRIBED_BIT) {
            xEventGroupClearBits(mqtt_event_group, MQTT_UNSUBSCRIBED_BIT);
            mqtt_callback(MQTT_UNSUBSCRIBED);
        } else if (bits & MQTT_PUBLISHED_BIT) {
            xEventGroupClearBits(mqtt_event_group, MQTT_PUBLISHED_BIT);
            mqtt_callback(MQTT_UNSUBSCRIBED);
        } else if (bits & MQTT_RECEIVED_DATA_BIT) {
            xEventGroupClearBits(mqtt_event_group, MQTT_RECEIVED_DATA_BIT);
            mqtt_callback(MQTT_PUBLISHED);
        } else if (bits & MQTT_ERROR_BIT) {
            xEventGroupClearBits(mqtt_event_group, MQTT_ERROR_BIT);
            mqtt_callback(MQTT_ERROR);
        } else {
            ESP_LOGE(TAG, "UNEXPECTED EVENT");
        }
    }

    vEventGroupDelete(mqtt_event_group);
}

// MQTT
static esp_err_t mqtt_event_handler(esp_mqtt_event_handle_t event){
    BaseType_t xHigherPriorityTaskWoken, xResult;
    xHigherPriorityTaskWoken = false;
    xResult = ESP_OK;

    // your_context_t *context = event->context;
    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT Client connected");
            xResult = xEventGroupSetBitsFromISR(mqtt_event_group, MQTT_CONNECTED_BIT, &xHigherPriorityTaskWoken);
            break;
        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "MQTT Client disconnected");
            ESP_LOGI(TAG, "Reconnecting MQTT client");
            esp_mqtt_client_start(MQTTClient);
            xResult = xEventGroupSetBitsFromISR(mqtt_event_group, MQTT_DISCONNECTED_BIT, xHigherPriorityTaskWoken);
            break;
        case MQTT_EVENT_SUBSCRIBED:
            // ESP_LOGI(TAG, "MQTT client subscribed to %s", event->topic);
            ESP_LOGI(TAG, "MQTT client subscribed to ");
            xResult = xEventGroupSetBitsFromISR(mqtt_event_group, MQTT_SUBSCRIBED_BIT, xHigherPriorityTaskWoken);
            break;
        case MQTT_EVENT_UNSUBSCRIBED:
            ESP_LOGI(TAG, "MQTT client unsubscribed from %s", event->topic);
            xResult = xEventGroupSetBitsFromISR(mqtt_event_group, MQTT_UNSUBSCRIBED_BIT, xHigherPriorityTaskWoken);
            break;
        case MQTT_EVENT_PUBLISHED:
            xResult = xEventGroupSetBitsFromISR(mqtt_event_group, MQTT_PUBLISHED_BIT, xHigherPriorityTaskWoken);
            break;
        case MQTT_EVENT_DATA:
            ESP_LOGI(TAG, "MQTT client received new message:\nTopic: %s\nData: %s", event->topic, event->data);
            xResult = xEventGroupSetBitsFromISR(mqtt_event_group, MQTT_RECEIVED_DATA_BIT, xHigherPriorityTaskWoken);
            handleMQTTMessage(event->topic, event->data);
            break;
        case MQTT_EVENT_ERROR:
            ESP_LOGI(TAG, "MQTT client error!");
            xResult = xEventGroupSetBitsFromISR(mqtt_event_group, MQTT_ERROR_BIT, xHigherPriorityTaskWoken);
            break;
        default:
            ESP_LOGI(TAG, "Other event id:%d", event->event_id);
            break;
    }

    // if( xResult != pdFAIL ) {
    //     portYIELD_FROM_ISR();
    // }

    return ESP_OK;
}

void mqtt_client_init(char* hostname) {
    esp_mqtt_client_config_t mqtt_cfg = {
    .event_handle = &mqtt_event_handler,
    .host = hostname,
    .uri =  nullptr, 
    .port = MQTT_PORT,               
    .client_id = nullptr,               
    .username = nullptr, 
    .password = nullptr, 
    .lwt_topic = nullptr, 
    .lwt_msg = nullptr,
    .lwt_qos = 0,                      
    .lwt_retain = 0,
    .lwt_msg_len = 0,
    .disable_clean_session = true,
    .keepalive = 120,
    .disable_auto_reconnect = false,
    .user_context = nullptr,
    .task_prio = 5,
    .task_stack = 6144,
    .buffer_size = 1024,
    .cert_pem = nullptr,
    .client_cert_pem = nullptr,
    .client_key_pem = nullptr,
    .transport = MQTT_TRANSPORT_OVER_TCP,
    .refresh_connection_after_ms = 0
};

    MQTTClient = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_start(MQTTClient);
}

// mDNS
void start_mdns_service() {
    //initialize mDNS service
    esp_err_t err = mdns_init();
    if (err) {
        ESP_LOGI(TAG, "MDNS Init failed: %d", err);
        return;
    }

    //set hostname
    mdns_hostname_set("esp32_mDNS_test");
    //set default instance
    mdns_instance_name_set("ESP32 device testing mDNS");
}

esp_err_t resolve_mdns_host(const char * host_name, char * resolvedIP)
{
    ESP_LOGI(TAG, "Resolving hostname %s", host_name);

    struct ip4_addr addr;
    addr.addr = 0;

    esp_err_t err = mdns_query_a(host_name, 2000,  &addr);
    if(err){
        if(err == ESP_ERR_NOT_FOUND){
            ESP_LOGI(TAG, "MQTT Broker was not found!");
            return ESP_ERR_NOT_FOUND;
        }
        ESP_LOGI(TAG, "Query Failed");
        return ESP_ERR_NOT_FOUND;
    }

    sprintf(resolvedIP, "%d.%d.%d.%d", IP2STR(&addr));

    ESP_LOGI(TAG, "MQTT host resolved: %s", resolvedIP);
    return ESP_OK;
}

void mqtt_init(const char * mqtt_broker, mqtt_callback_t callback)
{
    mqtt_event_group = xEventGroupCreate();
    
    mqtt_callback = callback;
    start_mdns_service();
    char hostname[15];
    if (resolve_mdns_host(mqtt_broker, hostname) == ESP_OK) {
        mqtt_client_init(hostname);
        xTaskCreate(mqtt_manager_task, "MQTT Manager Task", 4096, NULL, 3, NULL);
    }
}

void mqtt_subscribe(const char * topic)
{
    esp_mqtt_client_subscribe(MQTTClient, topic, 0);
}