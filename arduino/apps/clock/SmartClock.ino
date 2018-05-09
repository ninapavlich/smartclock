

// SmartAlarm
// Uses Feather Huzzah ESP8266 WiFi board and Music Maker FeatherWing + AMP
// along with a simple web server (https://github.com/ninapavlich/smartclock)
// written by Nina Pavlich
// MIT license
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
//#include "Adafruit_MQTT.h"
//#include "Adafruit_MQTT_Client.h"
#include <Adafruit_VS1053.h>
#include <SD.h>

const char* WIFI_SSID     = "REPLACEME";
const char* WIFI_PASSWORD = "REPLACEME";

const char* API_ROOT = "https://REPLACEME.herokuapp.com";
const char* API_KEY = "REPLACEME";
const char* API_CLIENT_ID = "flip-clock";
// To determine API Host fingerprint, run the following two commands in the command-line:
// echo Q |openssl s_client -connect fanciful-falafal.herokuapp.com:443 > cert.pem
// openssl x509 -in cert.pem -sha1 -noout -fingerprint | sed 's/:/ /g' | sed 's/SHA1 Fingerprint=//g'
const char* API_HOST_SSL_FINGERPRINT = "REPLACEME";
// Then do the same thing with your AWS domain:
const char* MEDIA_HOST_SSL_FINGERPRINT = "REPLACEME";



const String API_ALARM_ROOT = String(API_ROOT) + "/api/alarms/?format=json";
const String API_AUTHORIZATION_HEADER = "Token "+ String(API_KEY);
const int CHECK_ALARMS_SEC = 10;
const int API_POLL_SEC = 60;

const char* MQTT_USERNAME = "REPLACEME";
const char* MQTT_PASSWORD = "REPLACEME";
const char* MQTT_HOST = "REPLACEME";
const int MQTT_PORT = 1883;
const char* MQTT_PATH = "iot/alarm/updated/";
const int PING_SEC = 60;


// These are the pins used
#define VS1053_RESET   -1     // VS1053 reset pin (not used!)

// Feather M0 or 32u4
#if defined(__AVR__) || defined(ARDUINO_SAMD_FEATHER_M0)
  #define VS1053_CS       6     // VS1053 chip select pin (output)
  #define VS1053_DCS     10     // VS1053 Data/command select pin (output)
  #define CARDCS          5     // Card chip select pin
  // DREQ should be an Int pin *if possible* (not possible on 32u4)
  #define VS1053_DREQ     9     // VS1053 Data request, ideally an Interrupt pin

// Feather ESP8266
#elif defined(ESP8266)
  #define VS1053_CS      16     // VS1053 chip select pin (output)
  #define VS1053_DCS     15     // VS1053 Data/command select pin (output)
  #define CARDCS          2     // Card chip select pin
  #define VS1053_DREQ     0     // VS1053 Data request, ideally an Interrupt pin

// Feather ESP32
#elif defined(ESP32)
  #define VS1053_CS      32     // VS1053 chip select pin (output)
  #define VS1053_DCS     33     // VS1053 Data/command select pin (output)
  #define CARDCS         14     // Card chip select pin
  #define VS1053_DREQ    15     // VS1053 Data request, ideally an Interrupt pin

// Feather Teensy3
#elif defined(TEENSYDUINO)
  #define VS1053_CS       3     // VS1053 chip select pin (output)
  #define VS1053_DCS     10     // VS1053 Data/command select pin (output)
  #define CARDCS          8     // Card chip select pin
  #define VS1053_DREQ     4     // VS1053 Data request, ideally an Interrupt pin

// WICED feather
#elif defined(ARDUINO_STM32_FEATHER)
  #define VS1053_CS       PC7     // VS1053 chip select pin (output)
  #define VS1053_DCS      PB4     // VS1053 Data/command select pin (output)
  #define CARDCS          PC5     // Card chip select pin
  #define VS1053_DREQ     PA15    // VS1053 Data request, ideally an Interrupt pin

#elif defined(ARDUINO_FEATHER52)
  #define VS1053_CS       30     // VS1053 chip select pin (output)
  #define VS1053_DCS      11     // VS1053 Data/command select pin (output)
  #define CARDCS          27     // Card chip select pin
  #define VS1053_DREQ     31     // VS1053 Data request, ideally an Interrupt pin
#endif


Adafruit_VS1053_FilePlayer musicPlayer = 
  Adafruit_VS1053_FilePlayer(VS1053_RESET, VS1053_CS, VS1053_DCS, VS1053_DREQ, CARDCS);

WiFiClient client;
HTTPClient http;


//Adafruit_MQTT_Client mqtt(&client, MQTT_HOST, MQTT_PORT, MQTT_USERNAME, MQTT_PASSWORD);
//Adafruit_MQTT_Subscribe alarmFeed = Adafruit_MQTT_Subscribe(&mqtt, MQTT_PATH);

uint32_t nextMQTTPing = 0;       // Next time a MQTT ping should be sent.
uint32_t nextAPIPoll = 0;        // Next time server should be polled
uint32_t nextAlarmCheck = 0;     // Next time we should check the alarms
const int STREAM_SIZE = 128;

void connectMQTT(); //function prototype HACK



/* --------------------------------------------------
 * MISC UTILITIES -----------------------------------
 * --------------------------------------------------
 */

void audioAlert(int beeps){
  /* helpful for debugging progress when serial isnt visible */
  for( int i=0; i < beeps; i = i + 1 ) {
    musicPlayer.sineTest(0x44, 100);
    delay(50);
  }
  delay(200);
}


/* --------------------------------------------------
 * Models -------------------------------------------
 * --------------------------------------------------
 */
class Alarm {
public:  
    String url;
    int pk;
    String name;
    String sound;
    String sound_md5;
    String sound_filename_83;
    String sound_filename_md5;
    String time; //TODO
    bool active;
    bool allow_snooze;
    String next_alarm_time;   //TODO
    String last_stopped_time; //TODO
    String last_snoozed_time; //TODO
    bool repeat_mon;
    bool repeat_tue;
    bool repeat_wed;
    bool repeat_thu;
    bool repeat_fri;
    bool repeat_sat;
    bool repeat_sun;
    
    void parse_json(JsonVariant json) {
      this->url = json["url"].as<String>();
      this->pk = json["pk"];
      this->name = json["name"].as<String>();
      this->sound = json["sound"].as<String>();
      this->sound_filename_83 = json["sound_filename_83"].as<String>();
      this->sound_filename_md5 = json["sound_filename_md5"].as<String>();
      this->sound_md5 = json["sound_md5"].as<String>();
      this->time = json["time"].as<String>();
      this->active = bool(json["active"]);
      this->allow_snooze = bool(json["allow_snooze"]);
      this->next_alarm_time = json["next_alarm_time"].as<String>();
      this->last_stopped_time = json["last_stopped_time"].as<String>();
      this->last_snoozed_time = json["last_snoozed_time"].as<String>();
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
const int MAX_ALARMS = 10;
int alarm_count = 0;
int alarm_pks[MAX_ALARMS];
Alarm alarm_hash[MAX_ALARMS];

/* --------------------------------------------------
 * SD Utilities ----------------------------------
 * --------------------------------------------------
 */

void initSDCard(){
  Serial.println("initSDCard()");
  if (!SD.begin(CARDCS)) {
    Serial.println(F("SD failed, or not present"));
    while (1);  // don't do anything more
  }
  Serial.println("SD initialization done.");
}

bool downloadFileToSD(String url, String filename){
  bool successful = false;
  
  Serial.printf("Download %s to %s\n", url.c_str(), filename.c_str());

  File file = SD.open(filename, FILE_WRITE);
  if (!file) {
    Serial.println(F("Could not create alarm file"));
  }else{
    Serial.printf("Starting download....\n");
    http.end();
    http.setReuse(true);
    http.setTimeout(15000);
    http.begin( url, MEDIA_HOST_SSL_FINGERPRINT );
    Serial.printf("post begin");
    auto httpCode = http.GET();
    if (httpCode > 0) {
      // HTTP header has been send and Server response header has been handled
      Serial.printf("[HTTP] GET... code: %d\n", httpCode);
      if (httpCode == HTTP_CODE_OK) {
        // get length of document (is -1 when Server sends no Content-Length header)
        int len = http.getSize();
        int total_len = len;
        Serial.printf("response size: %d\n", len);
        
        uint8_t buff[STREAM_SIZE] = { 0 };
        WiFiClient * stream = http.getStreamPtr();
        // read all data from server
        while (http.connected() && (len > 0 || len == -1)) {
          size_t size = stream->available();
          if (size) {
            Serial.printf("stream size: %d remaining: %d of total: %d\n", size, len, total_len);
            int c = stream->readBytes(buff, ((size > sizeof(buff)) ? sizeof(buff) : size));
            file.write(buff, c);
            if (len > 0) {
              len -= c;
            }
          }
          delay(1);
        }
        successful = true;
        Serial.print("[HTTP] connection closed or file end.\n");
      }
    } else {
        Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }
    http.end();
  }
  file.close();
  return successful;
}

void createFile(String filename, String contents){

  Serial.printf("Creating file %s with contents %s\n", filename.c_str(), contents.c_str());
  File file = SD.open(filename, FILE_WRITE);
  if (!file) {
    Serial.println("ERROR: Could not create file");
  }else{
    file.println(contents);
  }
  file.close();

  Serial.println("VERIFY:");
  file = SD.open(filename);
  if (file) {
    // read from the file until there's nothing else in it:
    while (file.available()) {
      Serial.write(file.read());
    }
    // close the file:
    file.close();
  }
  
}
bool doesFileExist(String filename){
  File file = SD.open(filename, FILE_READ);
  bool file_exists = false;
  if (!file) {
    file_exists = false;
  }else{
    file_exists = true;
  }
  file.close();
  return file_exists;
  
}
bool doFileContentsMatch(String filename, String match_contents){
  File file = SD.open(filename);
  bool matches = false;
  
  if (file) {
    
    // read from the file until there's nothing else in it:
    String file_contents = "";
    while (file.available()) {
      int line = file.read();
      Serial.write(line);
      file_contents = file_contents + line;
    }
    Serial.printf("Do file contents for %s match? %s vs %s\n", filename.c_str(), match_contents.c_str(), file_contents.c_str());
    
    if(file_contents == match_contents){
      matches = true;
    }
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening file");
  }
  file.close();
  return matches;
}

/* --------------------------------------------------
 * Music Utilities ----------------------------------
 * --------------------------------------------------
 */
void initMusicPlayer(){
  if (! musicPlayer.begin()) { // initialise the music player
     Serial.println(F("Couldn't find VS1053, do you have the right pins defined?"));
     while (1);
  }

}

/* --------------------------------------------------
 * WIFI UTILITIES -----------------------------------
 * --------------------------------------------------
 */
void initWifi(){

  
  Serial.println(F("Connecting to "));
  Serial.println(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println(F("."));
  }
 
  Serial.println(F("WiFi connected"));  
  Serial.println(F("IP address: "));
  Serial.println(WiFi.localIP());

  
  
  

}


 
/* --------------------------------------------------
 * API UTILITIES ------------------------------------
 * --------------------------------------------------
 */
Alarm getAlarmByPK(char alarm_pk); //function prototype
Alarm getAlarmByPK(char alarm_pk){
  return alarm_hash[alarm_pk];
}
Alarm getOrCreateAlarm(JsonVariant alarm_json);  // function prototype
Alarm getOrCreateAlarm(JsonVariant alarm_json){
  int pk = alarm_json["pk"];

  Alarm alarm = getAlarmByPK(pk);

//  - if not in dictionary, create new alarm object with alarm_json
//  - otherwise get alarm object and update the data on it based on alarm_json
//  - return alarm

  if(alarm.pk == 0){
    Serial.println(F("---- NEW ALARM Found! ----"));
    alarm.parse_json(alarm_json);
    alarm_hash[alarm.pk] = alarm;
    alarm_pks[alarm_count] = alarm.pk;
//    Serial.printf("New alarm %d at %d = %d\n", alarm.pk, alarm_count, alarm_pks[alarm_count]);
    alarm_count += 1;
    audioAlert(1);
  }else{
    alarm.parse_json(alarm_json);
  }
  

  return alarm;
}
void deleteAlarm(Alarm alarm); // function prototype
void deleteAlarm(Alarm alarm){
  //TODO
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


//  http.begin(API_ROOT, API_PORT, "/api/alarms/1/stop/");
//  http.addHeader("Content-Type", "application/json");
//  http.addHeader("Authorization", API_AUTHORIZATION_HEADER);
//  http.end()
}

void snoozeAlarm (Alarm alarm);  // function prototype
void snoozeAlarm (Alarm alarm){
  //- API.stop(self.client_id)
  //- stop alarm mp3
  //- active_alrm = null
  Serial.println("TODO: snooze alarm...");


//  http.begin(API_ROOT, API_PORT, "/api/alarms/1/snooze/");
//  http.addHeader("Content-Type", "application/json");
//  http.addHeader("Authorization", API_AUTHORIZATION_HEADER);
//  auto httpCode = http.POST(payload);
//  http.end()
}

void markAlarmClientSynchronized (Alarm alarm);  // function prototype
void markAlarmClientSynchronized (Alarm alarm){
  Serial.println("TODO: synchronize client...");
    
//  http.begin(API_ROOT, API_PORT, "/api/alarmclients/"+API_CLIENT_ID+"/synchronize/");
//  http.addHeader("Content-Type", "application/json");
//  http.addHeader("Authorization", API_AUTHORIZATION_HEADER);
//  auto httpCode = http.POST(payload);
//  http.end()
}
void checkAlarms(){
//  bool is_active_alarm = active_alarm.pk == alarm.pk;
//  bool is_alarm_going = isAlarmGoing(alarm);
//  
//  if( is_active_alarm == true && is_alarm_going == false ){
//    startAlarm(alarm);
//  }else if( is_active_alarm == false && is_alarm_going == true ){
//    stopAlarm(alarm);
//  }
}
void checkAlarmMusic(Alarm alarm);  // function prototype
void checkAlarmMusic(Alarm alarm){
  bool alarmFileExists = doesFileExist(alarm.sound_filename_83);
  bool md5FileExists = doesFileExist(alarm.sound_filename_md5);
  bool md5ValuesMatch = doFileContentsMatch(alarm.sound_filename_md5, alarm.sound_md5);

  Serial.printf("alarmFileExists:%d md5FileExists:%d md5ValuesMatch:%d \n", alarmFileExists, md5FileExists, md5ValuesMatch);
  if(alarmFileExists == false || md5FileExists == false ||  md5ValuesMatch == false){
    //Remove in case corrupted
    SD.remove(alarm.sound_filename_83);
    SD.remove(alarm.sound_filename_md5);
    delay(2000);
    bool download_successful = downloadFileToSD(alarm.sound, alarm.sound_filename_83);
    Serial.printf("Alarm sound downloaded successfully? %d\n", download_successful);
    if(download_successful){
      createFile(alarm.sound_filename_md5, alarm.sound_md5);  
      markAlarmClientSynchronized(alarm);
      audioAlert(3);
      
    }else{
      createFile(alarm.sound_filename_md5, "0");  
    }
    
  }else{
    Serial.println("all good, alarm file is downloaded.");
  }
}


void getAlarms(){

  int new_alarm_count = 0;
  int new_alarm_pks[MAX_ALARMS];
 // TODO:
//  int old_alarm_count = alarm_count;
//  int old_alarm_pks[MAX_ALARMS];
//  old_alarm_pks = alarm_pks;
  
  
  Serial.println("getAlarms()");
  Serial.println(API_ALARM_ROOT);
  http.end();
  http.setReuse(true);
  http.begin( API_ALARM_ROOT, API_HOST_SSL_FINGERPRINT );
  http.addHeader("Authorization", API_AUTHORIZATION_HEADER);
  auto httpCode = http.GET();
  if (httpCode > 0) {
    DynamicJsonBuffer jsonBuffer;
    JsonArray& alarms_json = jsonBuffer.parseArray(http.getString());
    for (auto& alarm_json : alarms_json) {
       Alarm alarm = getOrCreateAlarm(alarm_json);
       new_alarm_pks[new_alarm_count++] = alarm.pk;
    }
  } else {
      Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
  }
  http.end();   //Close connection

// HANDLE ARRAY DELETION:
//  for (int i=0; i < old_alarm_count; i++){
//      bool in_new_list = false;
//      int old_alarm_id = old_alarm_pks[old_alarm_count];
//      for (int i=0; i < new_alarm_count; i++){
//        int new_alarm_id = new_alarm_pks[new_alarm_count];
//        if(new_alarm_id == old_alarm_id){
//          in_new_list = true;
//        }
//      }
//      if(in_new_list == false){
//        //TODO: Remove alarm...
//        deleteAlarm(alarms[old_alarm_id]);
//      }
//  }

  
}
void checkAllAlarmMusic(){
  for (int i=0; i < alarm_count; i++){
    Serial.println(i);
    int alarm_pk = alarm_pks[i];
    Alarm alarm = alarm_hash[alarm_pk];    
    checkAlarmMusic(alarm);
  }  
}

void pollAlarms(){
  if (millis() >= nextAPIPoll) {
    getAlarms();
    checkAllAlarmMusic();
    checkAlarms();
    // Set the time for the next ping.
    nextAPIPoll = millis() + API_POLL_SEC*1000L;
  }
  if (millis() >= nextAlarmCheck) {
    checkAlarms();
    // Set the time for the next ping.
    nextAlarmCheck = millis() + CHECK_ALARMS_SEC*1000L;
  }

  
}

/* --------------------------------------------------
 * MQTT UTILITIES -----------------------------------
 * --------------------------------------------------
 */
// Function to connect and reconnect as necessary to the MQTT server.
// Should be called in the loop function and it will take care of connecting.

//void connectMQTT() {
//  int8_t ret;
//
//  // Stop if already connected.
//  if (mqtt.connected()) {
//    return;
//  }
//
//  Serial.println("Connecting to MQTT... ");
//
//  uint8_t retries = 3;
//  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
//       Serial.println(mqtt.connectErrorString(ret));
//       Serial.println("Retrying MQTT connection in 5 seconds...");
//       mqtt.disconnect();
//       delay(5000);  // wait 5 seconds
//       retries--;
//       if (retries == 0) {
//         // basically die and wait for WDT to reset me
//         while (1);
//       }
//  }
//  Serial.println("MQTT Connected!");
//  initMQTTSubscriptions();
//}
//void initMQTTSubscriptions(){
//  // Setup MQTT subscription.
//  if (mqtt.connected()) {
//    Serial.println("indeed connected");
//  }else{
//    Serial.println("not connected yet..."); 
//  }
//  
//  mqtt.subscribe(&alarmFeed);
//}
//void checkForNewMQTTMessages(){
//  Serial.println(" --- checkForNewMQTTMessages-- ");
//  // Check if any new data has been received from the alarm feed.
//  Adafruit_MQTT_Subscribe *subscription;
//  while ((subscription = mqtt.readSubscription(10))) {
//    if (subscription == &alarmFeed) {
//      // Received data from the alarm feed!
//      // Parse the data to see how to change the light.
//      char* data = (char*)alarmFeed.lastread;
//      int dataLen = strlen(data);
//      Serial.print("Got: ");
//      Serial.println(data);
//      if (dataLen < 1) {
//        // Stop processing if not enough data was received.
//        continue;
//      }
//    }
//  }
//}
//void keepAliveMQTT(){
//  // Ping the MQTT server periodically to prevent the connection from being closed.
//  if (millis() >= nextMQTTPing) {
//    // Attempt to send a ping.
//    if(! mqtt.ping()) {
//      // Disconnect if the ping failed.  Next loop iteration a reconnect will be attempted.
//      mqtt.disconnect();
//    }
//    // Set the time for the next ping.
//    nextMQTTPing = millis() + PING_SEC*1000L;
//  }
//}





  
void setup() {
  Serial.begin(115200);
  delay(100);

  //Only done once:
  initMusicPlayer();
  initSDCard();
  initWifi();
  audioAlert(2);
  
  delay(2000);
  
  //These get called on a regular interval...
  getAlarms();
  delay(5000);
  checkAllAlarmMusic();
  checkAlarms();
  audioAlert(2);
  //  connectMQTT();
  //  audioAlert(5);

}

void loop() {
  delay(5000);
  Serial.println("--- loop ---");
//  
//  // Ensure the connection to the MQTT server is alive (this will make the first
//  // connection and automatically reconnect when disconnected).  See the connectMQTT
//  // function definition further below.
//  connectMQTT();
//  checkForNewMQTTMessages();
//  keepAliveMQTT();
  pollAlarms();
}


