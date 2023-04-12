/* 
Logging data in the file - 
reading & Sending data - 
going to hibernation sleep mode for 60 sec - 
Connecting to device in 20 sec
*/

#include <Arduino.h>
#include "FS.h"
#include <LITTLEFS.h>
#include "DHT.h"
#include "BluetoothSerial.h"
#include <esp_sleep.h>
#include <driver/rtc_io.h>


// Define the pins used for the DHT sensor data and power
#define DHTPIN 23
#define DHTPWR 22

// Led pin 
#define led 19

// activate dht sensor
#define DHTTYPE DHT11

// To check the Bluetooth is properly enable 
#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

// Define the time intervals and sleep time
#define SENSOR_INTERVAL 5000   // Interval between sensor readings (in ms)
#define BLE_INTERVAL 5000      // Interval between BLE notifications (in ms)
#define TIME_TO_SLEEP 1000 * 60 * 3      // Time ESP32 will go to sleep (in seconds)
#define WAKEUP_INTERVAL 60*27     // Wakeup interval for advertising (in seconds)

// Time in millis
unsigned long previousMillis = 0;
unsigned long currentMillis = millis();

// To create an instance of BluetoothSerial
BluetoothSerial SerialBT;

// Create an instance of the DHT sensor
DHT dht(DHTPIN, DHTTYPE);

//#define TWOPART

#define FORMAT_LITTLEFS_IF_FAILED true

// To make directory
void createDir(fs::FS &fs, const char * path){
    if(fs.mkdir(path)){}
    else {
        Serial.println("mkdir failed");
    }
}

// To read file 
String readFile(fs::FS &fs, const char * path){
    File file = fs.open(path);
    if(!file || file.isDirectory()){
        Serial.println("- failed to open file for reading");
//        return;
    }
    while(file.available()){
        SerialBT.write(file.read());
    }
    file.close();
}


// To write in the existing file 
void appendFile(fs::FS &fs, const char * path, const char * message){
    File file = fs.open(path, FILE_APPEND);
    if(!file){
        Serial.println("- failed to open file for appending");
        return;
    }
    if(file.println(message)){} 
    else {
        Serial.println("- append failed");
    }
    file.close();
}

// To delete the file
void deleteFile(fs::FS &fs, const char * path){
    if(fs.remove(path)){
        Serial.println("- file deleted");
    } else {
        Serial.println("- delete failed");
    }
}

// Function to read a text file and return its contents as a String
String readFileAsString(fs::FS &fs, const char * path){
    File file = fs.open(path);
    if(!file){
        Serial.println("- failed to open file for reading");
        return "";
    }

    String contents = "";
    while(file.available()){
        contents += (char)file.read();
    }
    file.close();
    return contents;
}



void setup(){
  Serial.begin(115200);
  pinMode(DHTPWR, OUTPUT);
  digitalWrite(DHTPWR, HIGH);
  dht.begin();
  
  SerialBT.begin("ESP32"); //Bluetooth device name
  Serial.println("The device started, now you can pair it with bluetooth!");
  
   // To write and read file
  if(!LITTLEFS.begin(FORMAT_LITTLEFS_IF_FAILED)){
    Serial.println("LITTLEFS Mount Failed");
    return;
   }
  createDir(LITTLEFS, "/mydir");
  }

void loop(){
  unsigned long currentMillis = millis();
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();
  char temp[5];
  char humi[5];
  snprintf(temp, sizeof(temp), "%f", temperature);
  snprintf(humi, sizeof(humi), "%f", humidity);
  const char* t = temp;
  const char* h = humi;
  Serial.println("Writing in file");
  appendFile(LITTLEFS, "/mydir/sensordata.txt", t);
  appendFile(LITTLEFS, "/mydir/sensordata.txt", " , ");
  appendFile(LITTLEFS, "/mydir/sensordata.txt", h);
  delay(10000);
  Serial.println("Going for connection");  
  
  if(!SerialBT.connected()){
    Serial.print("Device not available");
    delay(20000);
  }
  else {
  Serial.println("Device found");
  delay(5000);
  while (SerialBT.available()) { 
   char input;
   input == '0'; 
   input = SerialBT.read();
   Serial.println(input);

   if (input == 'a') { 
   // Read the contents of the text file
    readFile(LITTLEFS, "/mydir/sensordata.txt");  
    String fileContents = readFileAsString(LITTLEFS, "/mydir/sensordata.txt");
    input == '0';
    Serial.println( "File send" ); 
    delay(15000);
    }
  
   if (input == 'd'){
      deleteFile(LITTLEFS, "/mydir/sensordata.txt");
    input == '0';
    delay(15000);
    }
      
   if (input == 's'){
    digitalWrite(DHTPWR, LOW);
    SerialBT.disconnect();
    Serial.println("Now sleeping");
    esp_sleep_enable_timer_wakeup(WAKEUP_INTERVAL * 1000000);
    esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_AUTO);
    esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_SLOW_MEM, ESP_PD_OPTION_ON);
    esp_deep_sleep_start();
    }
   else { delay(15000); 
   }
  } 
 }
  Serial.println(currentMillis);
  
  // To sleep in hibernation  mode
  if(currentMillis > TIME_TO_SLEEP) {
  currentMillis = previousMillis ;
  digitalWrite(DHTPWR, LOW);
  SerialBT.disconnect();
  Serial.println("Entering hibernation sleep...");
  esp_sleep_enable_timer_wakeup(WAKEUP_INTERVAL * 1000000);
  esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_AUTO);
  esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_SLOW_MEM, ESP_PD_OPTION_ON);
  esp_deep_sleep_start();   
  }
 }
