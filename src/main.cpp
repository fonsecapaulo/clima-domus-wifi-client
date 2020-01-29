#include <Arduino.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <DHT.h>
#include "secrets.h"
#include "config.h"

DHT dht(DHTPIN, DHTTYPE);

/******************************************************
*SETUP GPIO
* *****************************************************/
void setupGPIO(){
    digitalWrite(LED_PIN, LOW); // Turn the LED on by making the voltage LOW
}

/******************************************************
*SETUP UART
******************************************************/
void setupUART(){
    #ifdef DEBUG    
        Serial.begin(115200); //Serial connection
    #endif 
}

/******************************************************
*SETUP WIFI + HTTP  
******************************************************/
void setupWifiHTTP(){
    // Set up Static IP address
    IPAddress local_IP(192, 168, 2, IP_ENDING);
    // Set up Gateway IP address
    IPAddress gateway(192, 168, 2, 254);
    // Set up subnet
    IPAddress subnet(255, 255, 0, 0);
    // Set up DNS
    IPAddress primaryDNS(8, 8, 8, 8);   //optional
    IPAddress secondaryDNS(8, 8, 4, 4); //optional
    // Configures static IP address
    if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {
        #ifdef DEBUG 
            Serial.println("Wifi Failed to configure");
        #endif
    }    
    
    WiFi.begin(SECRET_SSID, SECRET_PASSWORD);             //WiFi connection
    while (WiFi.status() != WL_CONNECTED) {               //Wait for the WiFI connection completion
        delay(500);
        #ifdef DEBUG 
            Serial.println("Waiting for connection");
        #endif
    }
    
    #ifdef DEBUG
        //Prints MAC address
        Serial.print("MAC address:\t");
        Serial.println(WiFi.macAddress());
        //Prints IP address
        Serial.print("IP address:\t");
        Serial.println(WiFi.localIP());         // Send the IP address of the ESP8266 to the computer
    #endif
}

/******************************************************
*SETUP DTH Sensor
*******************************************************/
void setupDHT(){
    dht.begin();
}


/************************************************************
*Publish in the Server the Humidity and Temperature reading
*************************************************************/
void publishClimaReading(){
    // Reading temperature or humidity takes about 250 milliseconds!
    // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
    float h = dht.readHumidity();
    // Read temperature as Celsius (the default)
    float t = dht.readTemperature();

    if (isnan(h) || isnan(t)) { // || isnan(f)) {
        #ifdef DEBUG 
            Serial.println("Failed to read from DHT sensor!");
        #endif
    } else {
        char print_buffer[60];
        sprintf(print_buffer, "Humidity: %.2fRH\tTemperature: %.2f*C", h, t);
        Serial.println(print_buffer);

        if (WiFi.status() == WL_CONNECTED) { //Check WiFi connection status

            char request[70];
            sprintf(request, "{\"sensor_id\":\"%d\",\"temperature\":\"%.2f\",\"humidity\":\"%.2f\"}", SENSOR_ID, t, h);

            WiFiClient wifi_client;
            HTTPClient http;            //Declare object of class HTTPClient

            http.begin(wifi_client, API_REQUEST);                //Specify request destination
            http.addHeader("Content-Type", "application/json");  //Specify content-type header

            int httpCode = http.POST(request);
            String payload = http.getString();      //Get the response payload

            http.end();  //Close connection

            #ifdef DEBUG 
                Serial.println(httpCode);   //Print HTTP return code
                Serial.println(payload);    //Print request response payload
            #endif
        } else {
            #ifdef DEBUG 
                Serial.println("Error in WiFi connection");
            #endif
        }
    }
}

void setup() {
    setupGPIO();
    #ifdef DEBUG    
        //Signalize start of setup
        pinMode(LED_PIN, OUTPUT);
    #endif
    setupUART();

    setupWifiHTTP();

    setupDHT();

    publishClimaReading();

    #ifdef DEBUG
        Serial.println("Going to deepsleep for 1 minute");
        //Signalize going to deep sleep
        digitalWrite(LED_PIN, HIGH);
    #endif    

    ESP.deepSleep(SLEEP_MINUTES* 60e6);
}

void loop() {
}
