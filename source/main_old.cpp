/*
#include "mbed.h"
#include "mbed_trace.h"
#include "https_request.h"
#include "network-helper.h"
#include "cert.h"

#define MBED_CONF_MBED_TRACE_ENABLE 1
#define MBED_TRACE_MAX_LEVEL TRACE_LEVEL_DEBUG
//#define MBEDTLS_TEST_NULL_ENTROPY 1

float getTemp(void)
{
    static float temp = 0;
    temp = temp + 0.1;
    if(temp >= 100)temp=0;
    return temp;
}

void dump_response(HttpResponse* res) {
    printf("Status: %d - %s\n", res->get_status_code(), res->get_status_message().c_str());

    printf("Headers:\n");
    for (size_t ix = 0; ix < res->get_headers_length(); ix++) {
        printf("\t%s: %s\n", res->get_headers_fields()[ix]->c_str(), res->get_headers_values()[ix]->c_str());
    }
    printf("\nBody (%lu bytes):\n\n%s\n", res->get_body_length(), res->get_body_as_string().c_str());
}

void disconnect(Socket *socket, NetworkInterface* network  )
{
    //delete[] buf;
    // Close the socket to return its memory
    socket->close();
    delete socket;

    // Bring down the network interface
    network->disconnect();
    printf("Done\n");
    while(1){};
}

int main_old() {

    mbed_trace_init();

    uint8_t fail_count = 0;

    nsapi_size_or_error_t result;
    nsapi_size_t size;

    float temp = 0;
    uint16_t content_length = 0;


    uint32_t device_address = NRF_FICR->DEVICEADDR[0];
    char buf[255] = "";
    char msg_buf[60] = "";
    snprintf(buf,60,"https://dweet.io/follow/Atlas_%lu",device_address);
    printf("Device Dweet Address: %s\r\n",buf);

    snprintf(buf,60,"https://dweet.io:443/dweet/for/Atlas_%lu",device_address);

    NetworkInterface* network = connect_to_default_network_interface();
    if (!network) {
        printf("Cannot connect to the network, see serial output\n");
        return 1;
    }

    // Create a TLS socket (which holds a TCPSocket)
    printf("\n----- Setting up TLS connection -----\n");
 
    //TLSSocket *socket = new TLSSocket;
    // result = socket->set_root_ca_cert(SSL_CA_PEM);
    // if (result != 0) {
    //     printf("Error: socket->set_root_ca_cert() returned %d\n", result);
    //     return result;
    // }

    //TCPSocket *socket = new TCPSocket;

    // result = socket->open(network);
    // if (result != 0) {
    //     printf("Error! socket->open() returned: %d\n", result);
    //     return result;
    // }

    // printf("Connecting to dweet.io\n");
    // result = socket->connect("dweet.io", 443);
    // if (result != 0) {
    //     printf("Error! socket->connect() returned: %d\n", result);
    //     disconnect(socket,network);
    // }

    //const char query[] = "GET / HTTP/1.1\r\nHost: ifconfig.io\r\nConnection: close\r\n\r\n";
    const char query[] = "POST /dweet/for/Atlas_443288936 HTTP/1.1\r\nHost: dweet.io\r\nContent-Type: application/json\r\nContent-Length: 12\r\n\r\n{\"test\": 51}\r\n\r\n";
    //snprintf(buf,60,"https://dweet.io:443/dweet/for/Atlas_%lu",device_address);
    while(1)
    {
        //tls only
        // TLSSocket *socket = new TLSSocket;
        // result = socket->set_root_ca_cert(SSL_CA_PEM);
        // if (result != 0) {
        //     printf("Error: socket->set_root_ca_cert() returned %d\n", result);
        //     //return result;
        //     ThisThread::sleep_for(3000);
        //     continue;
        // }
        TCPSocket *socket = new TCPSocket;

        result = socket->open(network);
        if (result != 0) {
            printf("Error! socket->open() returned: %d\n", result);
            //return result;
            ThisThread::sleep_for(3000);
            continue;
        }

        printf("Connecting to dweet.io\n");
        result = socket->connect("dweet.io", 443);
        if (result != 0) {
            printf("Error! socket->connect() returned: %d\n", result);
            //disconnect(socket,network);
            socket->close();
            ThisThread::sleep_for(3000);
            continue;
        }
        temp = getTemp();
        // Send a simple http request
        printf("Sending HTTP POST\r\n");
        snprintf(msg_buf,60,"{\"Temperature\":%.2f}",temp );
        content_length = strlen(msg_buf);
        snprintf(buf,255,"POST /dweet/for/Atlas_%lu HTTP/1.1\r\nHost: dweet.io\r\nContent-Type: application/json\r\nContent-Length: %d\r\n\r\n%s\r\n\r\n",device_address,content_length,msg_buf);
        printf("buf content: %s\n\r",buf);
        size = strlen(buf);
        result = socket->send(buf, size);
        if (result != size) {
            printf("Error! socket->send() returned: %d\n", result);
            //disconnect(socket,network);
            socket->close();
            ThisThread::sleep_for(3000);
            continue;
        }

        printf("Receieve an HTTP response and print out the response line\n\r");
        // while ((result = socket->recv(buf, 255)) >= 0) {
        //     buf[result] = 0;
        //     printf("%s", buf);
        // } 
        // while ((result = socket->recv(buf, 255)) > 0) {
        //     buf[result] = 0;
        //     printf("%s", buf);
        // }
        result = socket->recv(buf, 255);
        socket->close();
        printf("%s\n\r", buf);

        printf("\n");

        if (result < 0) {
            printf("Error! socket->recv() returned: %d\n", result);
            //disconnect(socket,network);
            socket->close();
            ThisThread::sleep_for(3000);
            continue;
        } 
        result = 0;
        memset(buf, 0, 255);
        memset(msg_buf, 0, 60);
        //socket->close();
        delete socket;
       //ThisThread::sleep_for(3000);


    }
    // while(1)
    // {
    //     //status = network->get_connection_status();
    //     // printf("network status: %d\n\r", status);
    //     // if (status == 2)
    //     // {
    //     //     network = connect_to_default_network_interface();
    //     // }
    //     printf("\n----- HTTPS POST request -----\n");
    //     snprintf(buf,60,"https://dweet.io:443/dweet/for/Atlas_%lu",device_address);

    //     HttpsRequest* post_req = new HttpsRequest(network, SSL_CA_PEM, HTTP_POST, buf);
    //     post_req->set_header("Content-Type", "application/json");

    //     float temp = getTemp();
    //     memset(buf, 0, 60);
    //     snprintf(buf,60,"{\"temperature\":%f}",temp);

    //     //const char body[] = buf;

    //     HttpResponse* post_res = post_req->send(buf, strlen(buf));
    //     if (!post_res) {
    //         printf("HttpRequest failed (error code %d)\n", post_req->get_error());
    //         fail_count++;
    //         if(fail_count >=3)NVIC_SystemReset();
    //     }
    //     else
    //     {
    //         printf("\n----- HTTPS POST response -----\n");
    //         dump_response(post_res);
    //     }
    //     delete post_req;

    //     ThisThread::sleep_for(5000);
    // }


}


*/