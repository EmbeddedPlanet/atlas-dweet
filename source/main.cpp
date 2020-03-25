#define MBED_CONF_MBED_TRACE_ENABLE 1
#include "mbed.h"
#include "mbed_trace.h"
#include "https_request.h"
#include "network-helper.h"
#include "cert.h"

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

int main() {

    mbed_trace_init();


    uint32_t device_address = NRF_FICR->DEVICEADDR[0];
    char buf[60] = "";
    snprintf(buf,60,"https://dweet.io/follow/Atlas_%lu",device_address);
    printf("Device Dweet Address: %s\r\n",buf);

    snprintf(buf,60,"https://dweet.io:443/dweet/for/Atlas_%lu",device_address);

    NetworkInterface* network = connect_to_default_network_interface();
    if (!network) {
        printf("Cannot connect to the network, see serial output\n");
        return 1;
    }

    HttpResponse* post_res;
    while(1)
    {
        printf("\n----- HTTPS POST request -----\n");
        snprintf(buf,60,"https://dweet.io:443/dweet/for/Atlas_%lu",device_address);

        HttpsRequest* post_req = new HttpsRequest(network, SSL_CA_PEM, HTTP_POST, buf);//"https://dweet.io:443/dweet/for/Atlas_MAC");
        post_req->set_header("Content-Type", "application/json");

        float temp = getTemp();
        memset(buf, 0, 60);
        snprintf(buf,60,"{\"temperature\":%f}",temp);

        //const char body[] = buf;

        post_res = post_req->send(buf, strlen(buf));
        if (!post_res) {
            printf("HttpRequest failed (error code %d)\n", post_req->get_error());
            //return 1;
        }

        printf("\n----- HTTPS POST response -----\n");
        dump_response(post_res);

        delete post_req;

        ThisThread::sleep_for(5000);
    }

}


