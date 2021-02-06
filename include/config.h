//#define DEBUG                   1
//#define DHCP                    0

#define SENSOR_ID               4                   //1 = Office location

#define DHTPIN                  4                   // what digital pin the DHT22 is conected to
#define DHTTYPE                 DHT22

#define LED_PIN                 2                   //GPIO2

#define IP_ENDING               170 + SENSOR_ID

#define MIN_DELAY               0
#define SLEEP_MINUTES           20

#define BATTERY_THRESHOLD       2900//3050

const char* CLIMA_DOMUS_ROOMS[] = {"Office", "2nd\\ Floor\\ Room", "Guest\\ Room", "Bed\\ Room"};

const char* MQTT_SERVER         = "192.168.2.170";
#define MQTT_SERVER_PORT        1883
#define MQTT_TOPIC              "clima-domus"
#define MQTT_INFLUXDB_DATABASE  "clima-domus"