#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"
#include "MQTT_manager.h"
#include "cJSON.h"
#include "cJSON_Utils.h"
#include "mdns.h"
#include "freertos/queue.h"
#include "mqtt_client.h"



#define nullptr                             ((void*)0)
#define MQTT_PORT                           1883

#define MQTT_CONNECTED_BIT                  BIT0
#define MQTT_DISCONNECTED_BIT               BIT1
#define MQTT_SUBSCRIBED_BIT                 BIT2
#define MQTT_UNSUBSCRIBED_BIT               BIT3
#define MQTT_PUBLISHED_BIT                  BIT4
#define MQTT_RECEIVED_DATA_BIT              BIT5
#define MQTT_ERROR_BIT                      BIT6




static const char TAG[] = "MQTT_Manager";
static mqtt_callback_t mqtt_callback;
static esp_mqtt_client_handle_t mqtt_client;



static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    esp_mqtt_event_handle_t event = (esp_mqtt_event_handle_t)event_data;
    mqtt_callback(event);
}

void mqtt_client_init(char* hostname) {
    esp_mqtt_client_config_t mqtt_cfg = { 
        .host = hostname
    };

    mqtt_client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(mqtt_client, ESP_EVENT_ANY_ID, mqtt_event_handler, mqtt_client);
    esp_mqtt_client_start(mqtt_client);
}

void mqtt_reconnect() 
{
    esp_mqtt_client_start(mqtt_client);
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
    mqtt_callback = callback;
    start_mdns_service();
    char hostname[15];
    if (resolve_mdns_host(mqtt_broker, hostname) == ESP_OK) {
        mqtt_client_init(hostname);
    }
}

void mqtt_subscribe(const char * topic)
{
    esp_mqtt_client_subscribe(mqtt_client, topic, 0);
}

void mqtt_publish(const char * topic, const char * message) 
{
    int msg_id = esp_mqtt_client_publish(mqtt_client, topic, message, 0, 1, 0);
}