// SmartAlarm
// Uses Feather Huzzah ESP8266 WiFi board and Music Maker FeatherWing + AMP
// along with a simple web server (https://github.com/ninapavlich/smartclock)
// written by Nina Pavlich
// based on examples from Tony DiCola's SmartToilet and John Park's MusicAlertBox
// MIT license
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"


const char* WIFI_SSID     = "REPLACEME";
const char* WIFI_PASSWORD = "REPLACEME";

const char* API_ROOT = "https://REPLACEME.herokuapp.com";
const char* API_KEY = "REPLACEME";
const char* API_CLIENT_ID = "flip-clock";
// To determine API Host fingerprint, run the following two commands in the command-line:
// echo Q |openssl s_client -connect fanciful-falafal.herokuapp.com:443 > cert.pem
// openssl x509 -in cert.pem -sha1 -noout -fingerprint | sed 's/:/ /g' | sed 's/SHA1 Fingerprint=//g'
const char* API_HOST_SSL_FINGERPRINT = "REPLACEME";


const String API_ALARM_ROOT = String(API_ROOT) + "/api/alarms/?format=json";
const String API_AUTHORIZATION_HEADER = "Token "+ String(API_KEY);
const int POLL_SEC = 120;

const char* MQTT_USERNAME = "REPLACEME";
const char* MQTT_PASSWORD = "REPLACEME";
const char* MQTT_HOST = "REPLACEME";
const int MQTT_PORT = 1883;
const char* MQTT_PATH = "iot/alarm/updated/";
const int PING_SEC = 60;




WiFiClient client;
Adafruit_MQTT_Client mqtt(&client, MQTT_HOST, MQTT_PORT, MQTT_USERNAME, MQTT_PASSWORD);
Adafruit_MQTT_Subscribe alarmFeed = Adafruit_MQTT_Subscribe(&mqtt, MQTT_PATH);

uint32_t nextMQTTPing = 0;       // Next time a MQTT ping should be sent.
uint32_t nextAPIPoll = 0;        // Next time server should be polled


void connectMQTT(); //function prototype HACK

void setup() {
  Serial.begin(115200);
  delay(100);
 
  initWifi();
  getAlarms();
  connectMQTT();
}


/* --------------------------------------------------
 * WIFI UTILITIES -----------------------------------
 * --------------------------------------------------
 */
//class Alarm
//{
//  public:
//   int x;
//   virtual void f() { x=1; }
//};


class Alarm {
public:  
    std::string url;
    int pk;
    const char* name;
    const char* sound;
    const char* sound_md5;
    const char* time; //TODO
    bool active;
    bool allow_snooze;
    const char* next_alarm_time;   //TODO
    const char* last_stopped_time; //TODO
    const char* last_snoozed_time; //TODO
    bool repeat_mon;
    bool repeat_tue;
    bool repeat_wed;
    bool repeat_thu;
    bool repeat_fri;
    bool repeat_sat;
    bool repeat_sun;
  
    
    void parse_json(JsonVariant json) {
      this->url = int(json["url"]);
      this->pk = json["pk"];
      this->name = json["name"];
      this->sound = json["sound"];
      this->sound_md5 = json["sound_md5"];
      this->time = json["time"];
      this->active = bool(json["active"]);
      this->allow_snooze = bool(json["allow_snooze"]);
      this->next_alarm_time = json["next_alarm_time"];
      this->last_stopped_time = json["last_stopped_time"];
      this->last_snoozed_time = json["last_snoozed_time"];
      this->repeat_mon = bool(json["repeat_mon"]);
      this->repeat_tue = bool(json["repeat_tue"]);
      this->repeat_wed = bool(json["repeat_wed"]);
      this->repeat_thu = bool(json["repeat_thu"]);
      this->repeat_fri = bool(json["repeat_fri"]);
      this->repeat_sat = bool(json["repeat_sat"]);
      this->repeat_sun = bool(json["repeat_sun"]);
  }

};



Alarm active_alarm;
Alarm alarms[10];


/* --------------------------------------------------
 * WIFI UTILITIES -----------------------------------
 * --------------------------------------------------
 */
void initWifi(){

  
  Serial.println("Connecting to ");
  Serial.println(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println(".");
  }
 
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

}


 
/* --------------------------------------------------
 * API UTILITIES ------------------------------------
 * --------------------------------------------------
 */
Alarm getAlarmByPK(char alarm_pk); //function prototype
Alarm getAlarmByPK(char alarm_pk){
  return alarms[alarm_pk];
}
Alarm getOrCreateAlarm(JsonVariant alarm_json);  // function prototype
Alarm getOrCreateAlarm(JsonVariant alarm_json){
  int pk = alarm_json["pk"];

  Alarm alarm = getAlarmByPK(pk);

//  - if not in dictionary, create new alarm object with alarm_json
//  - otherwise get alarm object and update the data on it based on alarm_json
//  - return alarm

  if(alarm.pk == 0){
    alarm.parse_json(alarm_json);
    alarms[alarm.pk] = alarm;
  }else{
    alarm.parse_json(alarm_json);
  }
  

  return alarm;
}

bool isAlarmGoing (Alarm alarm);  // function prototype
bool isAlarmGoing(Alarm alarm){
//  - go through all my alarms and check if they should be alarming
//  -- an alarm should be alarming if a) ( if the next alarm time is now or in the past )  
//  and b) (and last stopped time is null or older than the next alarm time) 
  return false;
}

void startAlarm (Alarm alarm);  // function prototype
void startAlarm (Alarm alarm){
//  - if active_alarm: stopAlarm(active_alarm)
//  - active_alarm = alarm
//  - play alarm mp3()
  Serial.println("TODO: start alarm...");
}

void stopAlarm (Alarm alarm);  // function prototype
void stopAlarm (Alarm alarm){
  //- API.stop(self.client_id)
  //- stop alarm mp3
  //- active_alrm = null
  Serial.println("TODO: stop alarm...");

//  HTTPClient http;
//  http.begin(API_ROOT, API_PORT, "/api/alarms/1/stop/");
//  http.addHeader("Content-Type", "application/json");
//  http.addHeader("Authorization", API_AUTHORIZATION_HEADER);
}

void snoozeAlarm (Alarm alarm);  // function prototype
void snoozeAlarm (Alarm alarm){
  //- API.stop(self.client_id)
  //- stop alarm mp3
  //- active_alrm = null
  Serial.println("TODO: snooze alarm...");

//  HTTPClient http;
//  http.begin(API_ROOT, API_PORT, "/api/alarms/1/snooze/");
//  http.addHeader("Content-Type", "application/json");
//  http.addHeader("Authorization", API_AUTHORIZATION_HEADER);
//  auto httpCode = http.POST(payload);
}

void markAlarmClientSynchronized (Alarm alarm);  // function prototype
void markAlarmClientSynchronized (Alarm alarm){
  Serial.println("TODO: synchronize client...");
//  HTTPClient http;
//  http.begin(API_ROOT, API_PORT, "/api/alarmclients/"+API_CLIENT_ID+"/synchronize/");
//  http.addHeader("Content-Type", "application/json");
//  http.addHeader("Authorization", API_AUTHORIZATION_HEADER);
//  auto httpCode = http.POST(payload);
}

void checkAlarmMusic(Alarm alarm);  // function prototype
void checkAlarmMusic(Alarm alarm){
//- check if file exists in storage
//- if it does, also verify the md5
//- if not, download it to disk
  Serial.println("Verify music for: ");
  Serial.println(alarm.sound);
  Serial.println(alarm.sound_md5);
}

void getAlarms(){

  Serial.println("getAlarms()");
  Serial.println(API_ALARM_ROOT);
  HTTPClient http;
  http.begin( API_ALARM_ROOT, API_HOST_SSL_FINGERPRINT );
  http.addHeader("Authorization", API_AUTHORIZATION_HEADER);
  auto httpCode = http.GET();
  if (httpCode > 0) {
    DynamicJsonBuffer jsonBuffer;
    JsonArray& alarms_json = jsonBuffer.parseArray(http.getString());
    for (auto& alarm_json : alarms_json) {
       Alarm alarm = getOrCreateAlarm(alarm_json);
       bool is_active_alarm = active_alarm.pk == alarm.pk;
       bool is_alarm_going = isAlarmGoing(alarm);
       
       if( is_active_alarm == true && is_alarm_going == false ){
          startAlarm(alarm);
       }else if( is_active_alarm == false && is_alarm_going == true ){
          stopAlarm(alarm);
       }
       checkAlarmMusic(alarm);

    }
  } else {
      Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
  }
  http.end();   //Close connection
  
}




void pollAlarms(){
  if (millis() >= nextAPIPoll) {
    getAlarms();
    // Set the time for the next ping.
    nextAPIPoll = millis() + POLL_SEC*1000L;
  }
}

/* --------------------------------------------------
 * MQTT UTILITIES -----------------------------------
 * --------------------------------------------------
 */
// Function to connect and reconnect as necessary to the MQTT server.
// Should be called in the loop function and it will take care of connecting.

void connectMQTT() {
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }

  Serial.println("Connecting to MQTT... ");

  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
       Serial.println(mqtt.connectErrorString(ret));
       Serial.println("Retrying MQTT connection in 5 seconds...");
       mqtt.disconnect();
       delay(5000);  // wait 5 seconds
       retries--;
       if (retries == 0) {
         // basically die and wait for WDT to reset me
         while (1);
       }
  }
  Serial.println("MQTT Connected!");
  initMQTTSubscriptions();
}
void initMQTTSubscriptions(){
  // Setup MQTT subscription.
  if (mqtt.connected()) {
    Serial.println("indeed connected");
  }else{
    Serial.println("not connected yet..."); 
  }
  
  mqtt.subscribe(&alarmFeed);
}
void checkForNewMQTTMessages(){
  Serial.println(" --- checkForNewMQTTMessages-- ");
  // Check if any new data has been received from the alarm feed.
  Adafruit_MQTT_Subscribe *subscription;
  while ((subscription = mqtt.readSubscription(10))) {
    if (subscription == &alarmFeed) {
      // Received data from the alarm feed!
      // Parse the data to see how to change the light.
      char* data = (char*)alarmFeed.lastread;
      int dataLen = strlen(data);
      Serial.print("Got: ");
      Serial.println(data);
      if (dataLen < 1) {
        // Stop processing if not enough data was received.
        continue;
      }
    }
  }
}
void keepAliveMQTT(){
  // Ping the MQTT server periodically to prevent the connection from being closed.
  if (millis() >= nextMQTTPing) {
    // Attempt to send a ping.
    if(! mqtt.ping()) {
      // Disconnect if the ping failed.  Next loop iteration a reconnect will be attempted.
      mqtt.disconnect();
    }
    // Set the time for the next ping.
    nextMQTTPing = millis() + PING_SEC*1000L;
  }
}


void loop() {
  delay(5000);
  Serial.println("--- loop ---");
  
  // Ensure the connection to the MQTT server is alive (this will make the first
  // connection and automatically reconnect when disconnected).  See the connectMQTT
  // function definition further below.
  connectMQTT();
  checkForNewMQTTMessages();
  keepAliveMQTT();
  pollAlarms();
}