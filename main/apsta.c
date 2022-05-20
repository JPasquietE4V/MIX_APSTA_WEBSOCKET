#include "apsta.h"

const char *TAGAPSTA = "AP_STATION";

static EventGroupHandle_t wifi_event_group;
const int CONNECTED_BIT = BIT0;


void event_handler(void* arg, esp_event_base_t event_base,
								int32_t event_id, void* event_data)
{
	if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
		ESP_LOGI(TAGAPSTA, "WIFI_EVENT_STA_DISCONNECTED");
		esp_wifi_connect();
		xEventGroupClearBits(wifi_event_group, CONNECTED_BIT);
	} else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
		ESP_LOGI(TAGAPSTA, "IP_EVENT_STA_GOT_IP");
		xEventGroupSetBits(wifi_event_group, CONNECTED_BIT);
	}
}

void initialise_wifi(void)
{
	esp_log_level_set("wifi", ESP_LOG_WARN);
	static bool initialized = false;
	if (initialized) {
		return;
	}
	ESP_ERROR_CHECK(esp_netif_init());
	wifi_event_group = xEventGroupCreate();
	ESP_ERROR_CHECK(esp_event_loop_create_default());
	esp_netif_t *ap_netif = esp_netif_create_default_wifi_ap();
	assert(ap_netif);
	esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();
	assert(sta_netif);
	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
	ESP_ERROR_CHECK( esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &event_handler, NULL) );
	ESP_ERROR_CHECK( esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL) );

	ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );
	ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_NULL) );
	ESP_ERROR_CHECK( esp_wifi_start() );

	initialized = true;
}



bool wifi_apsta(int timeout_ms)
{

	wifi_config_t ap_config = { 0 };
	strcpy((char *)ap_config.ap.ssid,"Batterie_Connectée_ESP32");
	strcpy((char *)ap_config.ap.password, "password");
	ap_config.ap.authmode = WIFI_AUTH_WPA_WPA2_PSK;
	ap_config.ap.ssid_len = strlen("Batterie_Connectée_ESP32");
	ap_config.ap.max_connection = CONFIG_AP_MAX_STA_CONN;
	ap_config.ap.channel = CONFIG_AP_WIFI_CHANNEL;

	if (strlen("CONFIG_AP_WIFI_PASSWORD") == 0) {
		ap_config.ap.authmode = WIFI_AUTH_OPEN;
	}

	wifi_config_t sta_config = { 0 };
	strcpy((char *)sta_config.sta.ssid, "E4V");
	strcpy((char *)sta_config.sta.password, "");


	ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_APSTA) );
	ESP_ERROR_CHECK( esp_wifi_set_config(ESP_IF_WIFI_AP, &ap_config) );
	ESP_ERROR_CHECK( esp_wifi_set_config(ESP_IF_WIFI_STA, &sta_config) );
	ESP_ERROR_CHECK( esp_wifi_start() );
	ESP_LOGI(TAGAPSTA, "WIFI_MODE_AP started. SSID:%s password:%s channel:%d",
			"Batterie_Connectée_ESP32", "password", CONFIG_AP_WIFI_CHANNEL);

	ESP_ERROR_CHECK( esp_wifi_connect() );
	int bits = xEventGroupWaitBits(wifi_event_group, CONNECTED_BIT,
								   pdFALSE, pdTRUE, timeout_ms / portTICK_PERIOD_MS);
	ESP_LOGI(TAGAPSTA, "bits=%x", bits);
	if (bits) {
		ESP_LOGI(TAGAPSTA, "WIFI_MODE_STA connected. SSID:%s password:%s",
			 "E4V", "");
	} else {
		ESP_LOGI(TAGAPSTA, "WIFI_MODE_STA can't connected. SSID:%s password:%s",
			 "E4V", "");
	}
	return (bits & CONNECTED_BIT) != 0;
}