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


// Define the pins used for the DHT sensor and power
#define DHTPIN 15
#define DHTPWR 18
#define led 19
#define DHTTYPE DHT11

// To check the Bluetooth is properly enable 
#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

// Define the time intervals and sleep time
#define SENSOR_INTERVAL 5000   // Interval between sensor readings (in ms)
#define BLE_INTERVAL 5000      // Interval between BLE notifications (in ms)
#define TIME_TO_SLEEP 20000       // Time ESP32 will go to sleep (in seconds)
#define WAKEUP_INTERVAL 60     // Wakeup interval for advertising (in seconds)

unsigned long previousMillis = 0;
unsigned long currentMillis = millis();

//char t;
//char h;
// To create an instance of BluetoothSerial
BluetoothSerial SerialBT;

// Create an instance of the DHT sensor
DHT dht(DHTPIN, DHTTYPE);

//#define TWOPART

#define FORMAT_LITTLEFS_IF_FAILED true


void createDir(fs::FS &fs, const char * path){
//    Serial.printf("Creating Dir: %s\n", path);
    if(fs.mkdir(path)){
//        Serial.println("Dir created");
    } else {
        Serial.println("mkdir failed");
    }
}

String readFile(fs::FS &fs, const char * path){
//    Serial.printf("Reading file: %s\r\n", path);

    File file = fs.open(path);
    if(!file || file.isDirectory()){
        Serial.println("- failed to open file for reading");
//        return;
    }

//    Serial.println("- read from file:");
    while(file.available()){
        SerialBT.write(file.read());
    }
    file.close();
}

void writeFile(fs::FS &fs, const char * path, const char * message){
//    Serial.printf("Writing file: %s\r\n", path);

    File file = fs.open(path, FILE_WRITE);
    if(!file){
        Serial.println("- failed to open file for writing");
        return;
    }
    else {
      file.println(message);
//      file.println(" , ");
//      file.println(message);
    }
//        Serial.println("- file written");
//    } else {
//        Serial.println("- write failed");
//    }
    file.close();
}

void appendFile(fs::FS &fs, const char * path, const char * message){
    Serial.printf("Appending to file: %s\r\n", path);

    File file = fs.open(path, FILE_APPEND);
    if(!file){
        Serial.println("- failed to open file for appending");
        return;
    }
    if(file.println(message)){
        Serial.println("- message appended");
    } else {
        Serial.println("- append failed");
    }
    file.close();
}


void deleteFile(fs::FS &fs, const char * path){
    Serial.printf("Deleting file: %s\r\n", path);
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
//        contents += (',');
//        contents += (char)file.read();
//        Serial.println("Reading file");
    }
    file.close();
    return contents;
}



void setup(){
    Serial.begin(115200);
pinMode(DHTPWR, OUTPUT);
digitalWrite(DHTPWR, HIGH);
dht.begin();

SerialBT.begin("ESP32test"); //Bluetooth device name
Serial.println("The device started, now you can pair it with bluetooth!");

  createDir(LITTLEFS, "/mydir");
  unsigned long currentMillis = millis();
  
    if(!LITTLEFS.begin(FORMAT_LITTLEFS_IF_FAILED)){
        Serial.println("LITTLEFS Mount Failed");
        return;
    }
    
}

void loop(){
unsigned long currentMillis = millis();
Serial.println( "SPIFFS-like write file to new path and delete it w/folders" );
    float temperature = dht.readTemperature();
    float humidity = dht.readHumidity();
    char temp[5];
    char humi[5];
    snprintf(temp, sizeof(temp), "%f", temperature);
    snprintf(humi, sizeof(humi), "%f", humidity);
    const char* t = temp;
    const char* h = humi;
    delay(1000);
//    char data[20];
//    snprintf(data, sizeof(data), "%f,%f", temperature, humidity);
//  listDir(LITTLEFS, "/", 3);
  
  appendFile(LITTLEFS, "/mydir/sensordata.txt", t);
  delay(1000);
//  appendFile(LITTLEFS, "/mydir/sensordata.txt", " , ");
//  delay(1000);
  appendFile(LITTLEFS, "/mydir/sensordata.txt", h);
//  Serial.println(t);
//  Serial.print(",");
//  Serial.println(h);
  delay(5000);
  Serial.println("Going for connection");  

//  Serial.print(dataa);
  if(!SerialBT.connected()){
    Serial.print("Device not available");
  }
  else {
//  SerialBT.connect();
  Serial.println("I got here");

    
  // Read the contents of the text file
  String fileContents = readFileAsString(LITTLEFS, "/mydir/sensordata.txt");
  Serial.print("File contents: ");
  Serial.println(fileContents);
  readFile(LITTLEFS, "/mydir/sensordata.txt");
  
    // Send the file contents to the mobile device
//  SerialBT.println(fileContents);
 
  Serial.println( "File send" ); 

//  if (SerialBT.available()) {
//    char d;
//    String del = Serial.write(SerialBT.read())
//    if (del == d){
//      deleteFile(LITTLEFS, "/mydir/sensordata.txt");
//      }
//    }
 }
  Serial.println(currentMillis);
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
