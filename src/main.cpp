#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <DHT.h>
#include <EMailSender.h>
#include <PubSubClient.h>

#include "secrets.h"
#include "config.h"

DHT dht(DHTPIN, DHTTYPE);

WiFiClient espClient;
PubSubClient client(espClient);

//Set ADC to measure Voltage
ADC_MODE(ADC_VCC);

/******************************************************
*SETUP GPIO
* *****************************************************/
void setupGPIO(){
    pinMode(LED_PIN, OUTPUT);    
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
    #ifndef DHCP
        Serial.println("Static IP Config!");
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
    #endif
        
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
        //Prints IP address
        Serial.print("Channel:\t");
        Serial.println(WiFi.channel());         // Send the IP address of the ESP8266 to the computer
    #endif
}

/******************************************************
*SETUP DTH Sensor
*******************************************************/
void setupDHT(){
    dht.begin();
}

/******************************************************
*SETUP MQTT Client
*******************************************************/
void setupMqttClient(){
    client.setServer(MQTT_SERVER, MQTT_SERVER_PORT);
}

/************************************************************
*Send out Email with batery warning
*************************************************************/
void sendOutWarningEmail(uint16_t vccValue){
    EMailSender emailSend(SECRET_EMAIL, SECRET_GOOGLE_PASS);
   
    EMailSender::EMailMessage message;
    message.subject = "[CLIMA-DOMUS] Low battery Warning!";
    message.message = "[CLIMA-DOMUS] Reports a sensor with low battery!<br/>Sensor: " + String(SENSOR_ID);
    message.message += "<br/>Room: " + String(CLIMA_DOMUS_ROOMS[SENSOR_ID-1]);
    message.message += "<br/>VCC: "+ String(vccValue);
    message.message += "<br/><br/><strong>CLIMA-DOMUS</strong>";

    EMailSender::Response resp = emailSend.send("paulojfonseca@gmail.com", message);

    #ifdef DEBUG
        Serial.println("Sending status: ");
        //Serial.println(resp.code);
        Serial.println(resp.desc);
        //Serial.println(resp.status);
    #endif
}

void mqttConnectAndPublish(char * msg) {
    // Loop until we're connected
    while (!client.connected()) {
        #ifdef DEBUG
            Serial.print("Attempting MQTT connection...");
        #endif
        char clientId[50];
        sprintf(clientId, "Clima Domus Client - %s",
            CLIMA_DOMUS_ROOMS[SENSOR_ID-1]
        );
    
        // Attempt to connect
        if (client.connect(clientId)) {
            #ifdef DEBUG
                Serial.println("MQTT Client Connected");
            #endif
            client.publish(MQTT_TOPIC, msg);            
            break;      
        } else {
            #ifdef DEBUG
                Serial.print("MQTT Client failed connection, rc=");
                Serial.print(client.state());
                Serial.println(" try again in 5 seconds");
            #endif
            // Wait 5 seconds before retrying
            delay(5000);
        }
    }
    client.disconnect();
    #ifdef DEBUG
    Serial.println("MQTT Client Disconnected");
    #endif
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
        #ifdef DEBUG     
            char print_buffer[60];
            sprintf(print_buffer, "Humidity: %.2fRH\tTemperature: %.2f*C", h, t);
            Serial.println(print_buffer);
        #endif    
        if (WiFi.status() == WL_CONNECTED) { //Check WiFi connection status

            char msg[200];
            sprintf(msg, "reading,database=%s,sensor_id=%s temperature=%.2f,humidity=%.2f",
                MQTT_INFLUXDB_DATABASE,
                CLIMA_DOMUS_ROOMS[SENSOR_ID-1],
                t,
                h
            );

            mqttConnectAndPublish(msg);
            
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
        digitalWrite(LED_PIN, LOW); // Turn the LED on by making the voltage LOW
    #endif
    
    setupUART();

    setupWifiHTTP();

    setupDHT();

    //Reads the VCC voltage
    uint16_t vcc=0;
    vcc = ESP.getVcc();

    #ifdef DEBUG
        Serial.print("VCC Voltage :");
        Serial.println(vcc);
    #endif    

    //send out email if vcc is too low.
    if (vcc<BATTERY_THRESHOLD)
        sendOutWarningEmail(vcc);

    setupMqttClient();
    
    publishClimaReading();

    #ifdef DEBUG
        Serial.print("Going to deepsleep for ");
        Serial.print(SLEEP_MINUTES);
        Serial.println(" minute(s)");
        //Signalize going to deep sleep
        digitalWrite(LED_PIN, HIGH);
    #endif    

    ESP.deepSleep(SLEEP_MINUTES* 60e6);
}

void loop() {
}
