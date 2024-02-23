#include <WiFi.h>
#include <HTTPClient.h>
#include <Arduino_JSON.h>
#include "DHT.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define DHTPIN 15
#define DHTTYPE DHT11
DHT dht11_sensor(DHTPIN, DHTTYPE);

#define ON_Board_LED 2
#define LED_01 18
#define LED_02 19

const char* ssid = "Redmi4x";
const char* password = "29292929";

String postData = "";
String payload = "";

float send_Temp;
int send_Humd;
String send_Status_Read_DHT11 = "";

// Initialize the OLED display using Adafruit_SSD1306
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Function declarations
void control_LEDs();
void get_DHT11_sensor_data();

void setup() {
  Serial.begin(115200);

  pinMode(ON_Board_LED, OUTPUT);
  pinMode(LED_01, OUTPUT);
  pinMode(LED_02, OUTPUT);
  
  digitalWrite(ON_Board_LED, HIGH);
  digitalWrite(LED_01, HIGH);
  digitalWrite(LED_02, HIGH);

  delay(2000);

  digitalWrite(ON_Board_LED, LOW);
  digitalWrite(LED_01, LOW);
  digitalWrite(LED_02, LOW);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  Serial.println();
  Serial.println("-------------");
  Serial.print("Connecting");

  int connecting_process_timed_out = 20;
  connecting_process_timed_out = connecting_process_timed_out * 2;
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    digitalWrite(ON_Board_LED, HIGH);
    delay(250);
    digitalWrite(ON_Board_LED, LOW);
    delay(250);
    
    if(connecting_process_timed_out > 0) connecting_process_timed_out--;
    if(connecting_process_timed_out == 0) {
      delay(1000);
      ESP.restart();
    }
  }
  
  digitalWrite(ON_Board_LED, LOW);
  
  Serial.println();
  Serial.print("Successfully connected to : ");
  Serial.println(ssid);
  Serial.println("-------------");

  dht11_sensor.begin();

  // Initialize the OLED display
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }

  // Clear the display buffer
  display.clearDisplay();

  // Set text size and color
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  // Display temperature and humidity labels
  display.setCursor(0, 0);
  display.println("Temperature:");
  display.setCursor(0, 16);
  display.println("Humidity:");

  // Display the buffer
  display.display();
}

void loop() {
  if(WiFi.status()== WL_CONNECTED) {
    HTTPClient http;
    int httpCode;
    
    postData = "id=esp32_01";
    payload = "";
  
    digitalWrite(ON_Board_LED, HIGH);
    Serial.println();
    Serial.println("---------------getdata.php");
    http.begin("http://192.168.43.153/ESP32_MySQL_Database/getdata.php");
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
   
    httpCode = http.POST(postData);
    payload = http.getString();
  
    Serial.print("httpCode : ");
    Serial.println(httpCode);
    Serial.print("payload  : ");
    Serial.println(payload);
    
    http.end();
    Serial.println("---------------");
    digitalWrite(ON_Board_LED, LOW);
    
    control_LEDs();
    delay(1000);
    get_DHT11_sensor_data();
    
    // Display temperature and humidity on OLED
    display.clearDisplay();
    display.setCursor(0, 0);
    display.print("Temperature: ");
    display.println(send_Temp);
    display.setCursor(0, 16);
    display.print("Humidity: ");
    display.println(send_Humd);
    display.display();
  
    postData = "id=esp32_01";
    postData += "&temperature=" + String(send_Temp);
    postData += "&humidity=" + String(send_Humd);
    postData += "&status_read_sensor_dht11=" + send_Status_Read_DHT11;
    payload = "";
  
    digitalWrite(ON_Board_LED, HIGH);
    Serial.println();
    Serial.println("---------------updateDHT11data_and_recordtable.php");
    http.begin("http://192.168.43.153/ESP32_MySQL_Database/updateDHT11data_and_recordtable.php");
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
   
    httpCode = http.POST(postData);
    payload = http.getString();
  
    Serial.print("httpCode : ");
    Serial.println(httpCode);
    Serial.print("payload  : ");
    Serial.println(payload);
    
    http.end();
    Serial.println("---------------");
    digitalWrite(ON_Board_LED, LOW);
    
    delay(4000);
  }
}

// Function definitions
void control_LEDs() {
  Serial.println();
  Serial.println("---------------control_LEDs()");
  JSONVar myObject = JSON.parse(payload);

  if (JSON.typeof(myObject) == "undefined") {
    Serial.println("Parsing input failed!");
    Serial.println("---------------");
    return;
  }

  if (myObject.hasOwnProperty("LED_01")) {
    Serial.print("myObject[\"LED_01\"] = ");
    Serial.println(myObject["LED_01"]);
  }

  if (myObject.hasOwnProperty("LED_02")) {
    Serial.print("myObject[\"LED_02\"] = ");
    Serial.println(myObject["LED_02"]);
  }

  if(strcmp(myObject["LED_01"], "ON") == 0)   {digitalWrite(LED_01, HIGH);  Serial.println("LED 01 ON"); }
  if(strcmp(myObject["LED_01"], "OFF") == 0)  {digitalWrite(LED_01, LOW);   Serial.println("LED 01 OFF");}
  if(strcmp(myObject["LED_02"], "ON") == 0)   {digitalWrite(LED_02, HIGH);  Serial.println("LED 02 ON"); }
  if(strcmp(myObject["LED_02"], "OFF") == 0)  {digitalWrite(LED_02, LOW);   Serial.println("LED 02 OFF");}

  Serial.println("---------------");
}
void get_DHT11_sensor_data() {
  Serial.println();
  Serial.println("-------------get_DHT11_sensor_data()");
  
  send_Temp = dht11_sensor.readTemperature();
  send_Humd = dht11_sensor.readHumidity();
  
  if (isnan(send_Temp) || isnan(send_Humd)) {
    Serial.println("Failed to read from DHT sensor!");
    send_Temp = 0.00;
    send_Humd = 0;
    send_Status_Read_DHT11 = "FAILED";
  } else {
    send_Status_Read_DHT11 = "SUCCEED";
  }
  
  Serial.printf("Temperature : %.2f °C\n", send_Temp);
  Serial.printf("Humidity : %d %%\n", send_Humd);
  Serial.printf("Status Read DHT11 Sensor : %s\n", send_Status_Read_DHT11);
  Serial.println("-------------");
}


