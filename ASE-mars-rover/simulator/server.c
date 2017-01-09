#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <microhttpd.h>
#include <string.h>
#include <stdio.h>
#include "server.h"
#include "constants.h"

struct MHD_Daemon *srv;

handle_request_t * handler;

int answer_to_connection (void *cls, struct MHD_Connection *connection,
                          const char *url,
                          const char *method, const char *version,
                          const char *upload_data,
                          size_t *upload_data_size, void **con_cls)
{
    struct MHD_Response *response;
    int ret;

    char * page = handler(url);

    response = MHD_create_response_from_buffer (strlen (page),
                                                (void*) page,
                                                MHD_RESPMEM_PERSISTENT);
    ret = MHD_queue_response (connection, MHD_HTTP_OK, response);
    MHD_destroy_response (response);

    return ret;
}

int server_init(handle_request_t * hr) {
    handler = hr;
    srv = MHD_start_daemon (MHD_NO_FLAG, PORT, NULL, NULL,
                            &answer_to_connection, NULL, MHD_OPTION_END);
    return (NULL != srv);
}

void server_stop(void) {
    MHD_stop_daemon (srv);
}

void server_run(void) {
    MHD_run (srv);
}
