/* WebSocket Echo Server Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include "websocket.h"

/* A simple example that demonstrates using websocket echo server
 */
extern const uint8_t html_file_array[] asm("_binary_upload_html_start");
const char *TAGWS = "ws_echo_server"; //attribut privé, visible uniquement dans le fichier


// httpd_uri_t
/* 
structure which has members including uri name, method type (eg. HTTPD_GET/HTTPD_POST/HTTPD_PUT etc.),
function pointer of type esp_err_t *handler (httpd_req_t *req) and user_ctx pointer to user context data.
*/
static const httpd_uri_t credential = { 
    .uri       = "/", 
    .method    = HTTP_GET,
    .handler   = credential_get_handler, 
    /* Let's pass response string in user
     * context to demonstrate it's usage */
    .user_ctx  = html_file_array
};


static const httpd_uri_t ws = {
        .uri        = "/ws",
        .method     = HTTP_GET,
        .handler    = echo_handler,
        .user_ctx   = NULL,
        .is_websocket = true
};


/*
 * Structure holding server handle
 * and internal socket fd in order
 * to use out of request send
 */
// struct async_resp_arg {
//     httpd_handle_t hd;
//     int fd;
// };

/*
 * async send function, which we put into the httpd work queue
 */
void ws_async_send(void *arg)
{
    static const char * data = "Async data";
    /*struct*/ async_resp_arg *resp_arg = arg;
    httpd_handle_t hd = resp_arg->hd; // -> pour acceder au membre d'une structure
    int fd = resp_arg->fd;
    httpd_ws_frame_t ws_pkt;
    memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
    ws_pkt.payload = (uint8_t*)data;
    ws_pkt.len = strlen(data);
    ws_pkt.type = HTTPD_WS_TYPE_TEXT;

    httpd_ws_send_frame_async(hd, fd, &ws_pkt);
    free(resp_arg);
}

esp_err_t trigger_async_send(httpd_handle_t handle, httpd_req_t *req)
{
    /*struct*/ async_resp_arg *resp_arg = malloc(sizeof(/*struct*/ async_resp_arg));
    resp_arg->hd = req->handle;

    // httpd_req_to_sockfd
    /*
    Get the Socket Descriptor from the HTTP request.

    This API will return the socket descriptor of the session for which URI handler was executed on reception of HTTP request. 
    
    Parameters
        r – [in] The request whose socket descriptor should be found

    Returns
        Socket descriptor : The socket descriptor for this request
        -1 : Invalid/NULL request pointer
    */
    resp_arg->fd = httpd_req_to_sockfd(req);
    return httpd_queue_work(handle, ws_async_send, resp_arg);
}

esp_err_t credential_get_handler (httpd_req_t *req) // ledOFF_get_handler --> hello_get_handler
{
 esp_err_t error; 
 //ESP_LOGI(TAGWS, "LED Turned OFF"); 
 const char *response = (const char *) req->user_ctx; 
 error = httpd_resp_send(req, response, strlen(response)); 
 if (error != ESP_OK) 
 { 
  ESP_LOGI(TAGWS, "Error %d while sending Response", error); 
 } 
 else ESP_LOGI(TAGWS, "Response sent Successfully"); 
 return error;


//   if (req->method == HTTP_GET) {
//        const char *response = (const char *) req->user_ctx;
//        esp_err_t error; 
//        error = httpd_resp_send(req, response, strlen(response));       
//        
//        if (error != ESP_OK) 
//         { 
//          ESP_LOGI(TAGWS, "Error %d while sending Response", error); 
//         } else 
//         {
//             ESP_LOGI(TAGWS, "Handshake done, the new connection was opened");
//         }             
//        return error;
//    }
// 
//    httpd_ws_frame_t ws_pkt;
//    uint8_t *buf = NULL;
//    memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
//    ws_pkt.type = HTTPD_WS_TYPE_TEXT;
//    /* Set max_len = 0 to get the frame len */
//    esp_err_t ret = httpd_ws_recv_frame(req, &ws_pkt, 0);
//    if (ret != ESP_OK) {
//        ESP_LOGE(TAGWS, "httpd_ws_recv_frame failed to get frame len with %d", ret);
//        return ret;
//    }
//    ESP_LOGI(TAGWS, "frame len is %d", ws_pkt.len);
//    if (ws_pkt.len) {
//        /* ws_pkt.len + 1 is for NULL termination as we are expecting a string */
//        buf = calloc(1, ws_pkt.len + 1);
//        if (buf == NULL) {
//            ESP_LOGE(TAGWS, "Failed to calloc memory for buf");
//            return ESP_ERR_NO_MEM;
//        }
//        ws_pkt.payload = buf;
//        /* Set max_len = ws_pkt.len to get the frame payload */
//        ret = httpd_ws_recv_frame(req, &ws_pkt, ws_pkt.len);
//        if (ret != ESP_OK) {
//            ESP_LOGE(TAGWS, "httpd_ws_recv_frame failed with %d", ret);
//            free(buf);
//            return ret;
//        }
//        ESP_LOGI(TAGWS, "Got packet with message: %s", ws_pkt.payload); //////////////////////////////////////////////////
//        
//    }
//    ESP_LOGI(TAGWS, "Packet type: %d", ws_pkt.type);
//    if (ws_pkt.type == HTTPD_WS_TYPE_TEXT &&
//        strcmp((char*)ws_pkt.payload,"Trigger async") == 0) {
//        free(buf);
//        return trigger_async_send(req->handle, req);
//    }
// 
//    ret = httpd_ws_send_frame(req, &ws_pkt);
//    if (ret != ESP_OK) {
//        ESP_LOGE(TAGWS, "httpd_ws_send_frame failed with %d", ret);
//    }
//    free(buf);
//    return ret;
}

esp_err_t echo_handler(httpd_req_t *req)
{
    if (req->method == HTTP_GET) {
        ESP_LOGI(TAGWS, "Handshake done, the new connection was opened");
        return ESP_OK;
    }
    httpd_ws_frame_t ws_pkt;
    uint8_t *buf = NULL;
    memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
    ws_pkt.type = HTTPD_WS_TYPE_TEXT;
    /* Set max_len = 0 to get the frame len */
    esp_err_t ret = httpd_ws_recv_frame(req, &ws_pkt, 0);
    if (ret != ESP_OK) {
        ESP_LOGE(TAGWS, "httpd_ws_recv_frame failed to get frame len with %d", ret);
        return ret;
    }
    ESP_LOGI(TAGWS, "frame len is %d", ws_pkt.len);
    if (ws_pkt.len) {
        /* ws_pkt.len + 1 is for NULL termination as we are expecting a string */
        buf = calloc(1, ws_pkt.len + 1);
        if (buf == NULL) {
            ESP_LOGE(TAGWS, "Failed to calloc memory for buf");
            return ESP_ERR_NO_MEM;
        }
        ws_pkt.payload = buf;
        /* Set max_len = ws_pkt.len to get the frame payload */
        ret = httpd_ws_recv_frame(req, &ws_pkt, ws_pkt.len);
        if (ret != ESP_OK) {
            ESP_LOGE(TAGWS, "httpd_ws_recv_frame failed with %d", ret);
            free(buf);
            return ret;
        }
        ESP_LOGI(TAGWS, "Got packet with message: %s", ws_pkt.payload); //////////////////////////////////////////////////
        
    }
    ESP_LOGI(TAGWS, "Packet type: %d", ws_pkt.type);
    if (ws_pkt.type == HTTPD_WS_TYPE_TEXT &&
        strcmp((char*)ws_pkt.payload,"Trigger async") == 0) {
        free(buf);
        return trigger_async_send(req->handle, req);
    }

    ret = httpd_ws_send_frame(req, &ws_pkt);
    if (ret != ESP_OK) {
        ESP_LOGE(TAGWS, "httpd_ws_send_frame failed with %d", ret);
    }
    free(buf);
    return ret;
}


esp_err_t http_404_error_handler(httpd_req_t *req, httpd_err_code_t err)
{
    
    /* For any other URI send 404 and close socket */
    httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "Some 404 error message");
    return ESP_FAIL;
}



httpd_handle_t start_webserver(void)
{
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.lru_purge_enable = true;


    // Start the httpd server
    ESP_LOGI(TAGWS, "Starting server on port: '%d'", config.server_port);
    if (httpd_start(&server, &config) == ESP_OK) {
        // Registering the ws handler
        ESP_LOGI(TAGWS, "Registering URI handlers");
        httpd_register_uri_handler(server, &credential);
        httpd_register_uri_handler(server, &ws);
        return server;
    }

    ESP_LOGI(TAGWS, "Error starting server!");
    return NULL;
}

void stop_webserver(httpd_handle_t server)
{
    // Stop the httpd server
    httpd_stop(server);
}

void disconnect_handler(void* arg, esp_event_base_t event_base,
                               int32_t event_id, void* event_data)
{
    httpd_handle_t* server = (httpd_handle_t*) arg;
    if (*server) {
        ESP_LOGI(TAGWS, "Stopping webserver");
        stop_webserver(*server);
        *server = NULL;
    }
}

void connect_handler(void* arg, esp_event_base_t event_base,
                            int32_t event_id, void* event_data)
{
    httpd_handle_t* server = (httpd_handle_t*) arg;
    if (*server == NULL) {
        ESP_LOGI(TAGWS, "Starting webserver");
        *server = start_webserver();
    }
}



