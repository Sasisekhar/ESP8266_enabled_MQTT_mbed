#include "BufferedSerial.h"
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
// main() runs in its own thread in the OS

namespace arduino{
    uint64_t millis() {
        return (us_ticker_read()/1000);
    }
}

int main() {

    DigitalOut button(BUTTON1);

    ESP8266Interface WiFi(D1, D0);

    WiFi.set_credentials((const char*)"Sx3K", (const char*)"golikuttan7577", NSAPI_SECURITY_WPA_WPA2);
    // WiFi.set_credentials((const char*)"ARSLAB", (const char*)"3928DC6C25", NSAPI_SECURITY_WEP);

    //WiFi.connect("ARSLAB", "3928DC6C25", NSAPI_SECURITY_WEP);
    // WiFi.connect("Sx3K", "golikuttan7577");
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
    //address.set_ip_address(address);
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
    while (true) {

        if(!client.connected()) {
            client.disconnect();
            if(client.connect("ARSLAB")) {
                printf("Connection Successful\n");
            }
        }

        if(arduino::millis() -  startTime > 500) {
            int temp = rand()%50;
            int hum = rand()%100;
            int co2 = rand()%5000;

            char buff[128];

            sprintf(buff, "{\"Temp\":%d, \"Hum\":%d, \"CO2\":%d}", temp, hum, co2);

            client.publish("ARSLAB/Data/Raw", buff);
            startTime = arduino::millis();
        }

        client.receive_response();

        if(button) {
            client.ping();
            ThisThread::sleep_for(500ms);
            if(button){
                break;
            }
        }
    }

    WiFi.disconnect();

    return 0;
}
