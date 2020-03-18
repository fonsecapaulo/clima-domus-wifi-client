#define DEBUG                   1
#define DHCP                    0

#define SENSOR_ID               4                   //1 = Office location

#define DHTPIN                  4                   // what digital pin the DHT22 is conected to
#define DHTTYPE                 DHT22

#define LED_PIN                 2                   //GPIO2

#define SERVER_IP               "192.168.2.170"
#define API_ENDPOINT            "/api/insert"
#define SERVER_PROTOCOL         "http://"
#define API_REQUEST             SERVER_PROTOCOL SERVER_IP API_ENDPOINT

#define IP_ENDING               174

#define MIN_DELAY               0
#define SLEEP_MINUTES           20

#define BATTERY_THRESHOLD       2900//3050

const char* CLIMA_DOMUS_ROOMS[] ={"Office", "3rd Floor Room", "Guest Room", "Bed Room"};