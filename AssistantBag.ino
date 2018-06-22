/**
   Copyright AssistantBag© 2018
   by Aprilia Audina and Wizdan Mohammad Gibran Shodri.

   This project was created with the aim to complete the thesis
   at the Politeknik Negeri Jakarta (PNJ).
*/

#include <ESP8266WiFi.h>
//#include <EEPROM.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <WiFiClient.h>
#include <ID20Reader.h>
#include <FirebaseArduino.h>

//#define WIFI_SSID "Santang"
//#define WIFI_PASSWORD "tiara56@"
#define FIREBASE_HOST "assistantbagskripsi.firebaseio.com"
#define FIREBASE_AUTH "EofQJEUQ998v7jL3pOhfBHAGef8kBH0TdSH3qPlu" // secret key
#define INTERVAL_MS 10000

WiFiClient client;
ID20Reader rfid(4, 5);

const unsigned long BAUD_RATE = 115200;
const unsigned long HTTP_TIMEOUT = 10000;

// For communication testing
const char* server = "www.google.com";
const int serverPort = 80;
const char* resource = "/";

// For Adjustment to local time
const int TIMEZONE = 7;
int daysAMonth[] = { //number of days a month
  31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};

unsigned long previousMillis = 0;
String dateAndTime;


void setup() {

  Serial.begin(BAUD_RATE);

//  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
//  while (WiFi.status() != WL_CONNECTED) {
//    delay(500);
//  }
//
//  // debug
//  Serial.println();
//  Serial.print("connected: ");
//  Serial.println(WiFi.localIP());

  // Default IP: 192.168.4.1
  WiFiManager wifiManager;
//  wifiManager.autoConnect("ESP_590AC5");
  wifiManager.setTimeout(60);
  if(!wifiManager.autoConnect("ESP_590AC5")) {
    Serial.println("failed to connect and hit timeout");
    delay(3000);
    //reset and try again, or maybe put it to deep sleep
    ESP.reset();
    delay(5000);
  } 

  // Connect to firebase
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);

  // debug
  Serial.println();
  Serial.println("*RFID Reader - Swipe a card ~~~~~");
}

void loop() {

  // Receive a tag from the reader if available
  rfid.read();

  if (rfid.available()) {

    if ( connect(server, serverPort) ) {
      if ( sendRequest(server, resource) ) {
        if ( findDateAndTimeInResponseHeaders() ) {

          // debug
          Serial.println();
          Serial.print("Date and Time from HTTP response header: ");
          Serial.println(dateAndTime.c_str() );
          Serial.print("Local Time GMT +7: ");
          Serial.println(localDateAndTime());
          
          localDateAndTime();
        }
      }
      disconnect();
    }
    // Get the tag
    String tag = rfid.get();
    Serial.print("Tag ID: ");
    Serial.println(tag); // debug
    Serial.println("---------------------------------------------------------------------");

    // Push to Firebase Database
    pushCatalog(tag);
    pushHistory(tag);
  }
}

// Push to CATALOG node
void pushCatalog(String tag) {

  // Get device ID and current logged user
  String deviceId = Firebase.getString("device/ESP_590AC5/id");
  String email = Firebase.getString("device/ESP_590AC5/currentUser");
  String user = email.substring(0, email.indexOf('@'));
    
  String catalog = "data/" + user + "/" + deviceId + "/catalog/" + tag;

  String lastReadDay = localDateAndTime().substring(0, 3);
  String lastReadDate = localDateAndTime().substring(5, 14);
  String lastReadTime = localDateAndTime().substring(15, 23);

  // If no tag data beforehand
  if (Firebase.getString(catalog + "/id") != tag) {
    // Set initials values
    Firebase.setString(catalog + "/id", tag);
    Firebase.setString(catalog + "/name", "(no-name)");
    Firebase.setString(catalog + "/lastReadDate", lastReadDate);
    Firebase.setString(catalog + "/lastReadTime", lastReadTime);
    Firebase.setString(catalog + "/status", "in");

    // Set for schedule
//    Firebase.setBool(catalog + "/schedule/Sun", false);
//    Firebase.setBool(catalog + "/schedule/Mon", false);
//    Firebase.setBool(catalog + "/schedule/Tue", false);
//    Firebase.setBool(catalog + "/schedule/Wed", false);
//    Firebase.setBool(catalog + "/schedule/Thu", false);
//    Firebase.setBool(catalog + "/schedule/Fri", false);
//    Firebase.setBool(catalog + "/schedule/Sat", false);
    
  } else {
    // Update values
    Firebase.setString(catalog + "/lastReadDate", lastReadDate);
    Firebase.setString(catalog + "/lastReadTime", lastReadTime);
    // Switch status
    if (Firebase.getString(catalog + "/status") == "in") {
      Firebase.setString(catalog + "/status", "out");
    } else if (Firebase.getString(catalog + "/status") == "out") {
      Firebase.setString(catalog + "/status", "in");
    }
  }

  // Handle error
  if (Firebase.failed()) {
    return;
  }
}

// Push to HISTORY node
void pushHistory(String tag) {

  // Get device ID and current logged user
  String deviceId = Firebase.getString("device/ESP_590AC5/id");
  String email = Firebase.getString("device/ESP_590AC5/currentUser");
  String user = email.substring(0, email.indexOf('@'));

  String history = "data/" + user + "/" + deviceId + "/history/";
  String catalog = "data/" + user + "/" + deviceId + "/catalog/" + tag;

  // Get values from catalog
//  String tagId = Firebase.getString(catalog + "/id");
//  String tagName = Firebase.getString(catalog + "/name");
  String tagDate = Firebase.getString(catalog + "/lastReadDate");
  String tagTime = Firebase.getString(catalog + "/lastReadTime");
  String tagStatus = Firebase.getString(catalog + "/status");

  // Create a Json object for logs message
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& newHistory = jsonBuffer.createObject();
//  newHistory["id"] = tagId;
//  newHistory["name"] = tagName;
  newHistory["date"] = tagDate;
  newHistory["time"] = tagTime;
  newHistory["status"] = tagStatus;

  JsonObject& reference = newHistory.createNestedObject("reference");
  reference[tag] = true;
  
  // Append the value to /HISTORY/
  Firebase.push(history, newHistory);

  // Handle error
  if (Firebase.failed()) {
    return;
  }
}
