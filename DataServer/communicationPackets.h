struct dataServer_client_req_packet{
    int command_no;
    char token[256];
    char payload[1025];
};
struct server_resp_packet{
    int response_no;
    char payload[1024];
};
