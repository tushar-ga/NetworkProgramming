struct client_req_packet{
    int command_no;
    char src[256];
    char dest[256];
    int size;
};
struct server_resp_packet{
    int response_no;
    char payload[1024];
};
struct dataServer_client_req_packet{
    int command_no;
    char token[256];
    char payload[1025];
};