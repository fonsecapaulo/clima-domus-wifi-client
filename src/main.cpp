#include <Arduino.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <DHT.h>
#include "secrets.h"
#include "config.h"

DHT dht(DHTPIN, DHTTYPE);

void setup() {
    
    pinMode(LED_PIN, OUTPUT);
    
    Serial.begin(115200);                                 //Serial connection
    
    // Set your Static IP address
    IPAddress local_IP(192, 168, 2, IP_ENDING);
    // Set your Gateway IP address
    IPAddress gateway(192, 168, 2, 254);

    IPAddress subnet(255, 255, 0, 0);
    IPAddress primaryDNS(8, 8, 8, 8);   //optional
    IPAddress secondaryDNS(8, 8, 4, 4); //optional

    // Configures static IP address
    if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {
        Serial.println("Wifi Failed to configure");
    }    
    
    WiFi.begin(SECRET_SSID, SECRET_PASSWORD);             //WiFi connection

    while (WiFi.status() != WL_CONNECTED) {               //Wait for the WiFI connection completion

        delay(500);
        Serial.println("Waiting for connection");

    }
    //Initialize the DHT
    dht.begin();
}

void loop() {
    digitalWrite(LED_PIN, LOW);                         // Turn the LED on by making the voltage LOW
    delay(1000);                                        // Wait for a second
    digitalWrite(LED_PIN, HIGH);

    // Reading temperature or humidity takes about 250 milliseconds!
    // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
    float h = dht.readHumidity();
    // Read temperature as Celsius (the default)
    float t = dht.readTemperature();

    if (isnan(h) || isnan(t)) { // || isnan(f)) {
        Serial.println("Failed to read from DHT sensor!");
    } else {
        char print_buffer[60];
        sprintf(print_buffer, "Humidity: %.2fRH/tTemperature: %.2f*C", h, t);
        Serial.println(print_buffer);

        if (WiFi.status() == WL_CONNECTED) { //Check WiFi connection status

            char request[70];
            sprintf(request, "{\"sensor_id\":\"%d\",\"temperature\":\"%.2f\",\"humidity\":\"%.2f\"}", SENSOR_ID, t, h);

            WiFiClient wifi_client;
            HTTPClient http;    //Declare object of class HTTPClient

            http.begin(wifi_client, API_REQUEST);     //Specify request destination
            http.addHeader("Content-Type", "application/json");  //Specify content-type header

            int httpCode = http.POST(request);
            String payload = http.getString();                  //Get the response payload

            Serial.println(httpCode);   //Print HTTP return code
            Serial.println(payload);    //Print request response payload

            http.end();  //Close connection

        } else {
            Serial.println("Error in WiFi connection");
        }
    }
    delay(1000*60*MIN_DELAY);  //Send a request every 30 seconds
}
