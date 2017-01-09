typedef char * (handle_request_t)(const char * url);
int server_init(handle_request_t * handle_request);
void server_stop(void);
void server_run(void);
