#include "mbed.h"
#include "network-helper.h"
#include "http_request.h"
Timeout watchdog;
DigitalOut status_led(PIN_NAME_LED_RED);
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
    printf("\nBody (%d bytes):\n\n%s\n", res->get_body_length(), res->get_body_as_string().c_str());
}
void system_reset_callback(void)
{
    system_reset();
}
int main() {
   //get nrf52840 MAC adderes
    uint32_t device_address = NRF_FICR->DEVICEADDR[0];

    //turn on led until connected
     status_led = 0;
    
    char buf[255] = "";
    char msg_buf[60] = "";
    
    //print Atlas web address to serial terminal
    snprintf(buf,255,"https://dweet.io/follow/Atlas_%lu",device_address);
    printf("Device Dweet Address: %s\r\n",buf);
    
    //dummy temp variable to be replaced with Atlas temp sensor fxn
    float temp = 0;
    NetworkInterface* network = connect_to_default_network_interface();
    if (!network) {
        printf("Cannot connect to the network, see serial output\n");
        return 1;
    }

    //set watchdog to reset for unrecoverable network failure
     watchdog.attach(&system_reset_callback, 30.0);
     status_led = 1;
    while(1) {
        // Create a TCP socket
       
        printf("\n----- Setting up TCP connection -----\n");
        TCPSocket* socket = new TCPSocket();
        nsapi_error_t open_result = socket->open(network);
        if (open_result != 0) {
            printf("Opening TCPSocket failed... %d\n", open_result);
            delete socket;
            continue;
            //return 1;
        }
        nsapi_error_t connect_result = socket->connect("dweet.io", 80);
        if (connect_result != 0) {
            printf("Connecting over TCPSocket failed... %d\n", connect_result);
            socket->close();
            delete socket;
            continue;
            //return 1;
        }
        printf("Connected over TCP to dweet.io:80\n");

        {
            snprintf(buf,255,"https://dweet.io:443/dweet/for/Atlas_%lu",device_address);
            HttpRequest* post_req = new HttpRequest(socket, HTTP_POST, buf);
            post_req->set_header("Content-Type", "application/json");
            //const char body[] = "{\"hello\":\"world\"}";
            temp = getTemp();
            // Send a simple http request
            printf("Sending HTTP POST\r\n");
            snprintf(msg_buf,60,"{\"Temperature\":%.2f}",temp );            
            HttpResponse* post_res = post_req->send(msg_buf, strlen(msg_buf));
            if (!post_res) {
                printf("HttpRequest failed (error code %d)\n", post_req->get_error());
                socket->close();
                delete socket;
                delete post_req;
                continue;
                //return 1;
            }
            else
            {
                printf("\n----- HTTP POST response -----\n");
                dump_response(post_res);
                delete post_req;
            }
        }
        socket->close();
        delete socket;
        printf("Waiting...\n");
        //reset watchdog
        watchdog.detach();

        //blink 5x
        for (int i = 0; i<10; i++)
        {
            status_led = !status_led;
            ThisThread::sleep_for(100);
        }
        ThisThread::sleep_for(2000);
         watchdog.attach(&system_reset_callback, 30.0);
    }
    wait(osWaitForever);
}
