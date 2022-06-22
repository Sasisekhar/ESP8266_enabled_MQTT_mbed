#include "BufferedSerial.h"
#include "DigitalIn.h"
#include "DigitalOut.h"
#include "NetworkInterface.h"
#include "PinNameAliases.h"
#include "SocketAddress.h"
#include "TCPSocket.h"
#include "ThisThread.h"
#include "USBSerial.h"
#include "WiFiInterface.h"
#include "mbed.h"
#include "ESP8266Interface.h"
#include "MQTT.h"
#include <cstring>
// main() runs in its own thread in the OS

namespace arduino{
    uint64_t millis() {
        return (us_ticker_read()/1000);
    }
}

DigitalOut led1(D12);
DigitalOut led2(D13);

int main() {
    led1 = false;
    led2 = false;

    DigitalIn button(BUTTON1);

    ESP8266Interface WiFi(D1, D0);

    WiFi.set_credentials((const char*)"Sx3K", (const char*)"golikuttan7577", NSAPI_SECURITY_WPA_WPA2);
    // WiFi.set_credentials((const char*)"ARS-LAB", (const char*)"3928DC6C25", NSAPI_SECURITY_WEP);

    WiFi.connect();
    printf("Connecting");
    while(WiFi.get_connection_status() == NSAPI_STATUS_CONNECTING){
        printf(".");
        ThisThread::sleep_for(500ms);
    }

    if(WiFi.get_connection_status() == NSAPI_STATUS_GLOBAL_UP) {
        printf("\r\nConnected!\r\n");
    }

    SocketAddress address;
    WiFi.gethostbyname("broker.hivemq.com", &address);
    // address.set_ip_address("134.117.52.253\0");  //My laptop
    // address.set_ip_address("134.117.52.231\0");     //My workstation
    address.set_port(1883);

    MQTTclient client(&WiFi, address);

    if(client.connect("ARSLAB")) {
        printf("Connection Successful\n");
    }

    if(client.subscribe("ARSLAB/Control/AC")) {
        printf("Subscription successful\n");
    }

    if(client.subscribe("ARSLAB/Control/Door")) {
        printf("Subscription successful\n");
    }

    if(client.subscribe("ARSLAB/Control/Light")) {
        printf("Subscription successful\n");
    }

    uint64_t startTime = 0;
    char topic[128];
    char message[128];

    while (true) {

        if(!client.connected()) {
            client.disconnect();
            if(client.connect("ARSLAB")) {
                printf("Connection Successful\n");
            }
        }

        if(arduino::millis() -  startTime > 1000) {
            int temp = rand()%50;
            int hum = rand()%100;
            int co2 = rand()%5000;

            char buff[128];

            sprintf(buff, "{\"Temp\":%d, \"Hum\":%d, \"CO2\":%d}", temp, hum, co2);

            client.publish("ARSLAB/Data/Raw", buff);
            startTime = arduino::millis();
        }

        if(client.receive_response(topic, message)) {
            //printf("Message: %s received on topic: %s\n", message, topic);

            if(!strcmp(topic, (char*) "ARSLAB/Control/Door")) {
                if(!strcmp(message, (char*) "1")) {
                    led1 = true;
                } else {
                    led1 = false;
                }
            } else if (!strcmp(topic, (char*) "ARSLAB/Control/Light")) {
                if(!strcmp(message, (char*) "1")) {
                    led2 = true;
                } else {
                    led2 = false;
                }
            }
        }

        if(!button) {
            client.ping();
            ThisThread::sleep_for(500ms);
            if(!button){
                break;
            }
        }
    }

    WiFi.disconnect();

    return 0;
}
