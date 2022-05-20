#include "apsta.h"
#include "websocket.h"


extern const char *TAGAPSTA;
extern const char *TAGWS;

void app_main(void)
{
	static httpd_handle_t server = NULL;

    esp_err_t err = nvs_flash_init();
	if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
		ESP_ERROR_CHECK( nvs_flash_erase() );
		err = nvs_flash_init();
	}
	ESP_ERROR_CHECK(err);

	initialise_wifi();


	ESP_LOGW(TAGAPSTA, "Start APSTA Mode");
	wifi_apsta(CONFIG_STA_CONNECT_TIMEOUT*1000);


	 ESP_ERROR_CHECK(esp_netif_init());

    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_AP_STAIPASSIGNED, &connect_handler, &server));

    /* Start the server for the first time */
    server = start_webserver();

}