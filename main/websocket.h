#ifndef __websocket_h__
#define __websocket_h__


#include <esp_wifi.h>
#include <esp_event.h>
#include <esp_log.h>
#include <esp_system.h>
#include <nvs_flash.h>
#include <sys/param.h> // cherche dans un rep de l'environnement du compilo
#include "nvs_flash.h" // cherche en local
#include "esp_netif.h"
#include "esp_eth.h"
//#include "protocol_examples_common.h"

#include <esp_http_server.h>


#include <sys/param.h>
#include "nvs_flash.h"
#include "esp_netif.h"

#include <esp_http_server.h>


typedef struct  {
     httpd_handle_t hd;
     int fd;
 } async_resp_arg ;


void ws_async_send(void *arg);
esp_err_t trigger_async_send(httpd_handle_t handle, httpd_req_t *req);
esp_err_t credential_get_handler (httpd_req_t *req);
esp_err_t echo_handler(httpd_req_t *req);
esp_err_t http_404_error_handler(httpd_req_t *req, httpd_err_code_t err);

httpd_handle_t start_webserver(void);
void stop_webserver(httpd_handle_t server);
void disconnect_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);
void connect_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);

#endif