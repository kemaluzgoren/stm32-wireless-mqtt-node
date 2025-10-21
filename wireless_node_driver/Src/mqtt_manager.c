/*
 * mqtt_manager.c
 *
 *  Created on: Feb 10, 2025
 *      Author: Kemal UZGOREN
 */

#include "lwip.h"
#include "mqtt.h"
#include "string.h"

#include "mqtt_manager.h"

mqtt_client_t *client;


static void do_connect(mqtt_client_t *client);
static void mqtt_connection_cb(mqtt_client_t *client, void *arg, mqtt_connection_status_t status);
static void mqtt_incoming_data_cb(void *arg, const u8_t *data, u16_t len, u8_t flags);
static void mqtt_incoming_publish_cb(void *arg, const char *topic, u32_t tot_len);
static void mqtt_sub_request_cb(void *arg, err_t result);
static void mqtt_pub_request_cb(void *arg, err_t result);
static void example_publish(mqtt_client_t *client, void *arg);
static void parse_value(const char *message, const char *key, char *buffer, size_t buffer_size);




uint32_t gCurrent_mqtt_subscribe_sec;
char buffer[100];


/**
  * @brief Initializes the MQTT client
  * @param  None
  * @retval None
  *
  */
void Cyber_mqtt_init(void)
{

	client = mqtt_client_new();

	gCurrent_mqtt_subscribe_sec = HAL_GetTick() + 2000;

}


/**
  * @brief Run the MQTT client
  * @param  None
  * @retval None
  *
  */
void Cyber_mqtt_run(void)
{


	if(HAL_GetTick - gCurrent_mqtt_subscribe_sec > 2000) {

		do_connect(client);

		gCurrent_mqtt_subscribe_sec = HAL_GetTick() + 5;
	}

}


/**
  * @brief Connects the MQTT client to the broker
  * @param client 	Pointer to the MQTT client structure
  * @retval None
  *
  */
static void do_connect(mqtt_client_t *client) {

	HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);

	struct mqtt_connect_client_info_t ci;
	err_t err;

	memset(&ci, 0, sizeof(ci));

	// Minimal amount of information required is client identification, so set it here
	ci.client_id = "lwip_test";
	ci.keep_alive = 120;

	ip_addr_t mqtt_broker_ip;

	IP4_ADDR(&mqtt_broker_ip, 192, 168, 0, 101);


	err = mqtt_client_connect(client, &mqtt_broker_ip, MQTT_PORT,
			mqtt_connection_cb, 0, &ci);

//	if(err != -10) {
//		(err == ERR_OK) ? ERROR_LED_OFF : ERROR_LED_ON;
//	}

}


/**
  * @brief Callback function for handling MQTT connection events
  * @param client 	Pointer to the MQTT client structure
  * @param arg 		User-defined argument (can be NULL)
  * @param status 	Status of the MQTT connection
  * @retval None
  *
  */
static void mqtt_connection_cb(mqtt_client_t *client, void *arg, mqtt_connection_status_t status) {

	err_t err;

	if(status == MQTT_CONNECT_ACCEPTED) {

		// TODO Durum ledi ekleyip başarılı olduğunu bildir.

		mqtt_set_inpub_callback(client, mqtt_incoming_publish_cb,
				mqtt_incoming_data_cb, arg);

		err = mqtt_subscribe(client, "LWIP/Topic", 1, mqtt_sub_request_cb, arg);


//		(err == ERR_OK) ? ERROR_LED_OFF : ERROR_LED_ON;

	}
	else {
		do_connect(client);
	}

}


/**
  * @brief Callback function for handling incoming MQTT data
  * @param arg 		User-defined argument (can be NULL)
  * @param data 	Pointer to the incoming data
  * @param len 		Length of the incoming data
  * @param flags 	Flags indicating the state of the data
  * @retval None
  *
  */
static void mqtt_incoming_data_cb(void *arg, const u8_t *data, u16_t len, u8_t flags) {

	if(flags & MQTT_DATA_FLAG_LAST) {

		char message[len + 1];
		strncpy(message, (const char*)data, len);
		message[len] = '\0';

		char temperature[16];
		char humidity[16];

		parse_value(message, "Temp: \"", temperature, sizeof(temperature));
		parse_value(message, "Hum: \"", humidity, sizeof(humidity));

//		if(temperature[0] != '\0' && humidity[0] != '\0') {
//			Cyber_lcd_send_string("t1", temperature);
//			Cyber_lcd_send_string("t3", humidity);
//		}

//		if(strncmp((const char*)data, "LED1-ON", 7) == 0) {
//			TEST_LED_ON;
//		}
//		else if(strncmp((const char*)data, "LED1-OFF", 8) == 0) {
//			TEST_LED_OFF;
//		}
	}

}


/**
  * @brief Callback function for handling incoming MQTT publish events
  * @param arg 		User-defined argument (can be NULL)
  * @param topic 	Pointer to the topic string of the incoming publish
  * @param tot_len 	Total length of the incoming publish data
  * @retval None
  *
  */
uint8_t inpub_id;
static void mqtt_incoming_publish_cb(void *arg, const char *topic, u32_t tot_len) {

	// printf("Incoming publish at topic %s with total length %u\n", topic, (unsigned int)tot_len);

	/* Decode topic string into a user defined reference */
	if(strcmp(topic, "Cyber") == 0) {
		inpub_id = 0;
	} else if(topic[0] == 'A') {
		inpub_id = 1;
	} else {
		inpub_id = 2;
	}

}

/**
  * @brief Callback function for handling the result of a subscription request
  * @param arg 		User-defined argument (can be NULL)
  * @param result 	Result of the subscription request
  * @retval None
  *
  */
static void mqtt_sub_request_cb(void *arg, err_t result) {
	printf("Subscribe result: %d\n", result);
}


/**
  * @brief Callback function for handling the result of a publish request
  * @param arg 		User-defined argument (can be NULL)
  * @param result 	Result of the publish request
  * @retval None
  *
  */
static void mqtt_pub_request_cb(void *arg, err_t result)
{
//  (result == ERR_OK) ? ERROR_LED_OFF : ERROR_LED_ON;
}


/**
  * @brief Publishes a message to a specified topic
  * @param client 	Pointer to the MQTT client structure
  * @param arg 		Pointer to the payload to be published
  * @retval None
  *
  */
static void example_publish(mqtt_client_t *client, void *arg) {

	  const char *pub_payload= arg;
	  err_t err;
	  u8_t qos = 2;
	  u8_t retain = 0;
	  err = mqtt_publish(client, "CyberPath/Topic", pub_payload, strlen(pub_payload), qos, retain, mqtt_pub_request_cb, arg);

//	  (err == ERR_OK) ? ERROR_LED_OFF : ERROR_LED_ON;

}

/**
  * @brief	Parses a value from a message string based on a given key
  * @param 	message 	The message string to search for
  * @param	key 		The key string to search
  * @param 	buffer		Variable to be inserted when the desired value is found
  * @param 	buffer_size	Size of buffer
  * @retval None
  */
static void parse_value(const char *message, const char *key, char *buffer, size_t buffer_size) {

    char *start_ptr = strstr(message, key);

    if (start_ptr) {

        start_ptr += strlen(key);
        char *end_ptr = strchr(start_ptr, '\"');

        if (end_ptr) {

            size_t value_length = end_ptr - start_ptr;

            if (value_length < buffer_size - 1) {

                strncpy(buffer, start_ptr, value_length);
                buffer[value_length] = '\0';
                return;

            }
        }
    }

    buffer[0] = '\0';
}
