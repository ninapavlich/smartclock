

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
#include <SPI.h>
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
const char* MEDIA_HOST_SSL_FINGERPRINT = "D2 F9 F6 7E F6 59 5B 5E B4 B6 EC 43 B4 36 6F 57 7F DC 03 63";


const String JSON_FILENAME = "ALARMS.TXT";
const String API_ALARM_ROOT = String(API_ROOT) + "/api/alarms/?format=json&?limit=10";
const String API_ALARM_CLIENT_ROOT = String(API_ROOT) + "/api/alarmclients/"+API_CLIENT_ID+"/";
const String API_ALARM_CLIENT_SYNC_ROOT = String(API_ROOT) + "/api/alarmclients/"+API_CLIENT_ID+"/synchronize/";
const String API_AUTHORIZATION_HEADER = "Token "+ String(API_KEY);
const int CHECK_ALARMS_SEC = 10;
const int API_POLL_SEC = 600;

const char* MQTT_USERNAME = "REPLACEME";
const char* MQTT_PASSWORD = "REPLACEME";
const char* MQTT_HOST = "REPLACEME";
const int MQTT_PORT = 1883;
const char* MQTT_PATH = "iot/alarm/updated/";
const int PING_SEC = 300;


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



//Adafruit_MQTT_Client mqtt(&client, MQTT_HOST, MQTT_PORT, MQTT_USERNAME, MQTT_PASSWORD);
//Adafruit_MQTT_Subscribe alarmFeed = Adafruit_MQTT_Subscribe(&mqtt, MQTT_PATH);

uint32_t nextMQTTPing = 0;       // Next time a MQTT ping should be sent.
uint32_t nextAPIPoll = 0;        // Next time server should be polled
uint32_t nextAlarmCheck = 0;     // Next time we should check the alarms
const int STREAM_SIZE = 128;

//LOGGING Helper
bool mp3_inited = false;
bool sd_inited = false;
bool time_inited = false;
const int WRITE_LOG_SECONDS = 30; //auto empty the log queue every X seconds or when it fills up
uint32_t nextLogWrite = 0;
const int MAX_LOG_QUEUE = 30;
String log_queue[MAX_LOG_QUEUE];
int log_queue_millis[MAX_LOG_QUEUE];
int log_queue_count = 0;

void connectMQTT(); //function prototype HACK



/* --------------------------------------------------
 * MISC UTILITIES -----------------------------------
 * --------------------------------------------------
 */
void audioAlert(int beeps){
  if(!mp3_inited){
    return;
  }
  /* helpful for debugging progress when serial isnt visible */
  for( int i=0; i < beeps; i = i + 1 ) {
    musicPlayer.sineTest(0x44, 100);
    delay(50);
  }
  delay(200);
}
String zeroFill(int num, int digits){
  String num_str = String(num);
  int len = num_str.length();
  for (int i = 0; i < digits - len; i++){
    num_str = "0" + num_str;
  }
  return num_str;
}

/* --------------------------------------------------
 * TIMING UTILITIES --------------------------------
 * --------------------------------------------------
 */
class DateTime {
public:  

    int sync_year = 0;
    int sync_month = 0;
    int sync_day = 0;
    int sync_hour = 0;
    int sync_minute = 0;
    int sync_second = 0;
    uint32_t sync_time = 0;
    
    void parse_json(JsonVariant json) {
      
      this->current_year = this->sync_year = json["current_time"]["year"].as<int>();
      this->current_month = this->sync_month = json["current_time"]["month"].as<int>();
      this->current_day = this->sync_day = json["current_time"]["day"].as<int>();
      this->current_hour = this->sync_hour = json["current_time"]["hour"].as<int>();
      this->current_minute = this->sync_minute = json["current_time"]["minute"].as<int>();
      this->current_second = this->sync_second = json["current_time"]["second"].as<int>();
      this->last_tick = this->sync_time = millis();
    }

    String getTimeStamp(){
//      this->tick();
      return String(this->current_year) + ":" + zeroFill(this->current_month, 2) + ":" + zeroFill(this->current_day, 2) + " " + zeroFill(this->current_hour, 2) + ":" + zeroFill(this->current_minute, 2) + ":" + zeroFill(this->current_second, 2);
    }
    int getCurrentYear(){
      this->tick();
      return this->current_year;
    }
    int getCurrentMonth(){
      this->tick();
      return this->current_month;
    }
    int getCurrentDay(){
      this->tick();
      return this->current_day;
    }
    int getCurrentHour(){
      this->tick();
      return this->current_hour;
    }
    int getCurrentMinute(){
      this->tick();
      return this->current_minute;
    }
    int getCurrentSecond(){
      this->tick();
      return this->current_second;
    }

    void tick(){
      if(!time_inited){
        return;
      }
      uint32_t dt = (millis() - last_tick);
      int ds = int(dt/1000);

//      Serial.println("need to add "+String(ds)+" seconds to the internal time values");
  
      if( dt < 1000){
//        Serial.println("tick is less than a second");
        return;
      }

      this->current_second = this->current_second + ds;
      if(this->current_second >= 60){
        this->current_second = this->current_second % 60;
        this->current_minute = this->current_minute + 1;
//        Serial.println("Increment minutes...");
      }

      if(this->current_minute >= 60){
        this->current_minute = this->current_minute % 60;
        this->current_hour = this->current_hour + 1;
//        Serial.println("Increment hours...");
      }
      //NOTE: Below here shouln't apply, since we will receive an updated value from the server before then:
      if(this->current_hour >= 24){
        this->current_hour = this->current_hour % 24;
        this->current_day = this->current_day + 1;
//        Serial.println("Increment days...");
      }
      if(this->current_day >= 31){
        this->current_day = this->current_day % 31;
        this->current_month = this->current_month + 1;
//        Serial.println("Increment months...");
      }
      if(this->current_month >= 12){
        this->current_month = this->current_month % 12;
        this->current_year = this->current_year + 1;
//        Serial.println("Increment years...");
      }
      
      last_tick = millis();

//      Serial.println(this->getTimeStamp());
    }

private:

  uint32_t last_tick = 0;
  int current_year = 0;
  int current_month = 0;
  int current_day = 0;
  int current_hour = 0;
  int current_minute = 0;
  int current_second = 0;
};

DateTime now;

void getTimeAt(int ms){
  //
  Serial.println("going to create a datetime object at a certain ms offset");
}
//String millisToTimeStamp(int ms){
//  return String(current_year) + ":" + zeroFill(current_month, 2) + ":" + zeroFill(current_day, 2) + " " + zeroFill(current_hour, 2) + ":" + zeroFill(current_minute, 2) + ":" + zeroFill(current_second, 2);
//}

/* --------------------------------------------------
 * LOGGING UTILITIES --------------------------------
 * --------------------------------------------------
 */

String getLogFilename(){
  return String(now.getCurrentYear()) +  zeroFill(now.getCurrentMonth(), 2) + zeroFill(now.getCurrentDay(), 2) + ".LOG";
}

void writeLogs(){
  if(log_queue_count == 0){return;}
  
//  Serial.println("write file to");
//  Serial.println(getLogFilename());
  String logFilename = getLogFilename();
  File logFile = SD.open(logFilename, FILE_WRITE);
  if (!logFile) {
    Serial.println("ERROR: Could not open log file at "+logFilename);
  }else{

    for (int i=0; i < log_queue_count; i++){
      String message = now.getTimeStamp()+" "+log_queue[i];
//      Serial.println("writing log: ");
//      Serial.println(message);
      logFile.println(message);
    }
    
  }
  log_queue_count = 0;
  logFile.close();

  
}
void checkLogs(){
  
  //If either WRITE_LOG_SECONDS has gone by, or the log is full, write all the logs to a file 
  if(log_queue_count >= MAX_LOG_QUEUE){
    writeLogs();
  }
  
  if (millis() >= nextLogWrite) {
    nextLogWrite = millis() + WRITE_LOG_SECONDS*1000L;
    writeLogs();
  }
}
void debug(String message){
  Serial.println(message);
  // FAILS HERE:
//  checkLogs();
//  log_queue[log_queue_count%MAX_LOG_QUEUE] = message;
//  log_queue_millis[log_queue_count%MAX_LOG_QUEUE] = millis();
//  log_queue_count = (log_queue_count + 1)%MAX_LOG_QUEUE;
}



/* --------------------------------------------------
 * Models -------------------------------------------
 * --------------------------------------------------
 */
class Alarm {
public:  
    const char* url;
    int pk;
    const char* name;
    const char* sound;
    const char* sound_md5;
    const char* sound_filename_83;
    const char* sound_filename_md5;
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
      this->url = json["url"].as<char*>();
      this->pk = json["pk"];
      this->name = json["name"].as<char*>();
      this->sound = json["sound"].as<char*>();
      this->sound_filename_83 = json["sound_filename_83"].as<char*>();
      this->sound_filename_md5 = json["sound_filename_md5"].as<char*>();
      this->sound_md5 = json["sound_md5"].as<char*>();
      this->time = json["time"].as<char*>();
      this->active = bool(json["active"]);
      this->allow_snooze = bool(json["allow_snooze"]);
      this->next_alarm_time = json["next_alarm_time"].as<char*>();
      this->last_stopped_time = json["last_stopped_time"].as<char*>();
      this->last_snoozed_time = json["last_snoozed_time"].as<char*>();
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
    Serial.println("ERROR: SD failed, or not present");
    while (1);  // don't do anything more
  }
  sd_inited = true;
  Serial.println("SD initialization done.");
}

bool downloadFileToSD(String url, String filename){
  bool successful = false;
  
  Serial.println("Download "+url+" to "+filename);

  //Remove existing, may be corrupted.
  SD.remove(filename);

  File file = SD.open(filename, FILE_WRITE);
  if (!file) {
    Serial.println("ERROR: Could not create file "+filename+" for downloading "+url);
  }else{
    
    HTTPClient http;
    http.begin( url, MEDIA_HOST_SSL_FINGERPRINT );
    auto httpCode = http.GET();
    if (httpCode > 0) {
      
      // HTTP header has been send and Server response header has been handled
      bool okay = (httpCode == HTTP_CODE_OK);
      Serial.println("-- [HTTP] GET "+String(url)+" code: "+String(httpCode)+" okay? "+String(okay));
      if (okay) {
        int len = http.getSize();
        int total_len = len;
        Serial.println("-- [HTTP] response size: "+String(len));
        
        uint8_t buff[STREAM_SIZE] = { 0 };
        WiFiClient * stream = http.getStreamPtr();
        // read all data from server
        while (http.connected() && (len > 0 || len == -1)) {
          size_t size = stream->available();
          if (size) {
            Serial.println("stream size: "+String(size)+" remaining: "+String(len)+" of total: "+String(total_len));
            int c = stream->readBytes(buff, ((size > sizeof(buff)) ? sizeof(buff) : size));
            file.write(buff, c);
            if (len > 0) {
              len -= c;
            }
          }
          delay(1);
        }
        successful = true;
        Serial.print("-- [HTTP] connection closed or file end.\n");
      }
    } else {
        Serial.println("-- [HTTP] GET "+String(url)+" failed, error: "+ String(http.errorToString(httpCode).c_str()));
    }
    http.end();
  }
  file.close();
  return successful;
}
bool createFile(String filename, String contents){
  bool successful = false;
  //Remove existing
  SD.remove(filename);
  
  debug("Creating file "+filename+" with contents "+contents);
  File file = SD.open(filename, FILE_WRITE);
  if (!file) {
    debug("ERROR: Could not create file "+filename+" with contents "+contents);
  }else{
    file.println(contents);
    successful = true;
  }
  file.close();
  return successful;
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
String getFileContents(String filename){
  File file = SD.open(filename, FILE_READ);
  String file_contents = "";
  if (!file) {
    return file_contents;
  }else{
    char c;
    while (file.available()) {
      c=file.read();
      file_contents=file_contents+String(c);
    }
    file_contents.trim();
  }
  file.close();
  return file_contents;
}
bool doFileContentsMatch(String filename, String match_contents){
  File file = SD.open(filename);
  bool matches = false;
  
  if (file) {
    String file_contents = "";
    char c;
    while (file.available()) {
      c=file.read();
      file_contents=file_contents+String(c);
    }
    file_contents.trim();
  
    if(file_contents == match_contents){
      matches = true;
    }
  } else {
    // if the file didn't open, print an error:
    debug("ERROR: Could not open file "+filename);
  }
  file.close();
  return matches;
}

/* --------------------------------------------------
 * Music Utilities ----------------------------------
 * --------------------------------------------------
 */
void initMusicPlayer(){
  Serial.println(F("initMusicPlayer()"));
  if (! musicPlayer.begin()) { // initialise the music player
     Serial.println(F("Couldn't find VS1053, do you have the right pins defined?"));
     while (1);
  }
  mp3_inited = true;
  Serial.println(F("initMusicPlayer() - COMPLETE"));
  musicPlayer.sineTest(0x44, 2000);

  musicPlayer.setVolume(2,2);

#if defined(__AVR_ATmega32U4__) 
  // Timer interrupts are not suggested, better to use DREQ interrupt!
  // but we don't have them on the 32u4 feather...
  musicPlayer.useInterrupt(VS1053_FILEPLAYER_TIMER0_INT); // timer int
#elif defined(ESP32)
  // no IRQ! doesn't work yet :/
#else
  // If DREQ is on an interrupt pin we can do background
  // audio playing
  musicPlayer.useInterrupt(VS1053_FILEPLAYER_PIN_INT);  // DREQ int
#endif
}

/* --------------------------------------------------
 * WIFI UTILITIES -----------------------------------
 * --------------------------------------------------
 */
void initWifi(){
  Serial.println("Connecting to Wifi with "+String(WIFI_SSID)+" : "+String(WIFI_PASSWORD));
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println(F("."));
  }
  Serial.println("WiFi connected with IP address: "+WiFi.localIP().toString());
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
    alarm.parse_json(alarm_json);
    debug("New alarm ID"+String(alarm.pk));
    alarm_hash[alarm.pk] = alarm;
    alarm_pks[alarm_count] = alarm.pk;
    alarm_count += 1;
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
bool downloadAlarmMusic(Alarm alarm);  // function prototype
bool downloadAlarmMusic(Alarm alarm){
  debug("downloadAlarmMusic("+String(alarm.pk)+")");
  
  bool alarmFileExists = doesFileExist(alarm.sound_filename_83);
  bool md5ValuesMatch = false;
  if(alarmFileExists == true){
    md5ValuesMatch = doFileContentsMatch(alarm.sound_filename_md5, alarm.sound_md5);
  }
 
  bool synchronized = false;
  
  debug("-- alarmFileExists:"+String(alarmFileExists)+" md5ValuesMatch:"+String(md5ValuesMatch));
  if(alarmFileExists == false || md5ValuesMatch == false){
    delay(2000);
    bool download_successful = downloadFileToSD(alarm.sound, alarm.sound_filename_83);
    if(download_successful){
      createFile(alarm.sound_filename_md5, alarm.sound_md5);  
      debug("Alarm "+String(alarm.pk)+" successfully synchronized");
      synchronized = true;
    }else{
      createFile(alarm.sound_filename_md5, "0");  
    }
  }else{
    synchronized = true;
  }
  debug("downloadAlarmMusic("+String(alarm.pk)+") - COMPLETE");
  return synchronized;
}


void initTime(){
  //TODO -- this is temporary until we get an RTC
  debug(F("initTime()"));
  HTTPClient http;
  http.begin( API_ALARM_CLIENT_ROOT, API_HOST_SSL_FINGERPRINT );
  http.addHeader("Authorization", API_AUTHORIZATION_HEADER);
  auto httpCode = http.GET();
  debug( "-- [HTTP] GET "+String(API_ALARM_CLIENT_ROOT)+"... code: "+String(httpCode) );
  if (httpCode > 0) {
    DynamicJsonBuffer jsonBuffer;
    JsonObject& alarm_client_json = jsonBuffer.parseObject(http.getString());
    now.parse_json(alarm_client_json);
    time_inited = true;
  } else {
      debug( "-- [HTTP] GET "+String(API_ALARM_CLIENT_ROOT)+" failed, error: "+ String(http.errorToString(httpCode).c_str()) );
  }
  http.end();   //Close connection
  debug("initTime() - COMPLETE: "+now.getTimeStamp());
}

void getAndParseAlarmData(){
  bool download_successful = getAlarmData();
  bool has_alarm_data = download_successful || doesFileExist(JSON_FILENAME);
  if(has_alarm_data){
    debug(F("Valid alarm data found."));
    parseAlarmData();
  }
}
bool getAlarmData(){
  debug(F("getAlarmData()"));
  bool successful = false;
  HTTPClient http;
  http.begin( API_ALARM_ROOT, API_HOST_SSL_FINGERPRINT );
  http.addHeader("Authorization", API_AUTHORIZATION_HEADER);
  auto httpCode = http.GET();
  if (httpCode > 0) {
    bool okay = (httpCode == HTTP_CODE_OK);
    debug("-- [HTTP] GET "+String(API_ALARM_ROOT)+" code: "+String(httpCode)+" okay? "+String(okay));
    if (httpCode == HTTP_CODE_OK) {
      successful = createFile(JSON_FILENAME, http.getString());
    }
  } else {
      debug("-- [HTTP] GET "+String(API_ALARM_ROOT)+" failed, error: "+String(http.errorToString(httpCode).c_str()));
  }
  http.end();   //Close connection

  debug(F("getAlarmData() - COMPLETE"));
  return successful;
}
void parseAlarmData(){
  
  int new_alarm_count = 0;
  int new_alarm_pks[MAX_ALARMS];
  
  DynamicJsonBuffer jsonBuffer;
  JsonArray& alarms_json = jsonBuffer.parseArray(getFileContents(JSON_FILENAME));
  for (auto& alarm_json : alarms_json) {
     Alarm alarm = getOrCreateAlarm(alarm_json);
     new_alarm_pks[new_alarm_count++] = alarm.pk;
  }

  debug("Found "+String(new_alarm_count)+" alarms.");
  
 // TODO:
//  int old_alarm_count = alarm_count;
//  int old_alarm_pks[MAX_ALARMS];
//  old_alarm_pks = alarm_pks;

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
//}
  
}

String getSynchronizeAlarmsList(){
  debug(F("getSynchronizeAlarmsList()"));
  String alarm_sync_list = "";
  for (int i=0; i < alarm_count; i++){
    int alarm_pk = alarm_pks[i];
    Alarm alarm = alarm_hash[alarm_pk];    
    bool synchronized = downloadAlarmMusic(alarm);
    if(synchronized){
      alarm_sync_list = alarm_sync_list + (String(alarm_pk)+",");
    }
  } 
  debug(F("getSynchronizeAlarmsList() - COMPLETE"));
  return alarm_sync_list;
}
 void synchronizeAlarmData(String alarm_sync_list){
  debug(F("synchronizeAlarmData()"));
  String data = "";
  String url = API_ALARM_CLIENT_SYNC_ROOT + "?alarms="+alarm_sync_list;

  HTTPClient http;
  http.begin( url, API_HOST_SSL_FINGERPRINT );
  http.addHeader("Authorization", API_AUTHORIZATION_HEADER);
  auto httpCode = http.POST(data);
  debug("-- [HTTP] POST "+String(url)+" code: "+String(httpCode));
  if (httpCode > 0) {
    DynamicJsonBuffer jsonBuffer;
    JsonObject& alarm_client_json = jsonBuffer.parseObject(http.getString());

    now.parse_json(alarm_client_json);
    time_inited = true;
    
  } else {
      debug("-- [HTTP] POST "+String(url)+" failed, error: "+ String( http.errorToString(httpCode).c_str()));
  }
  http.end();   //Close connection
  debug(F("synchronizeAlarmData() - COMPLETE"));
}
void getAndSynchronizeAlarms(){
  synchronizeAlarmData(getSynchronizeAlarmsList());
}

void pollAlarms(){
  if (millis() >= nextAPIPoll) {
    getAndParseAlarmData();
    getAndSynchronizeAlarms();
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
  
  // Wait for serial port to be opened, remove this line for 'standalone' operation
  while (!Serial) { delay(1); }

  initSDCard();
  initMusicPlayer();
  audioAlert(3);
  
  initWifi();//TODO -- once RTC is in place, WIFI can be moved down in the pecking order
//  initTime();//This should be moved to top once RTC module is in place.


    
  getAndParseAlarmData();
  getAndSynchronizeAlarms();
//  checkAlarms();


}

void loop() {
//  delay(5000);
//  Serial.println("--- loop ---");
////  audioAlert(3);
////  Serial.println(now.getTimeStamp());
//  now.tick();
//////  
//////  // Ensure the connection to the MQTT server is alive (this will make the first
//////  // connection and automatically reconnect when disconnected).  See the connectMQTT
//////  // function definition further below.
//////  connectMQTT();
//////  checkForNewMQTTMessages();
//////  keepAliveMQTT();
////  pollAlarms();
//  checkLogs();
}


