#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>
#include <Adafruit_AHTX0.h>
#include <Wire.h>

// Target MAC address
uint8_t target_mac[] = {0x12, 0x34, 0x56, 0x78, 0x90, 0xAB};

// Deep sleep timer
int dst = 60;

esp_now_peer_info_t peerInfo;
Adafruit_AHTX0 aht;

// Structure to hold sensor data
struct sensor_data {
  float temperature;
  float humidity;
};

// ESP-NOW callback function
void onDataSent(const uint8_t *mac_addr, esp_now_send_status_t status)
{
  Serial.println("Data sent, status: " + String(status));
}

void setup()
{
  // Initialize serial port for debugging
  Serial.begin(115200);
  
  Wire.begin(32,33);

  if (aht.begin()) {
    Serial.println("Found AHT20");
  } else {
    Serial.println("Didn't find AHT20");
  }  

  // Initialize ESP-NOW
  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  Serial.println("ESP-NOW initialized");

  // Set ESP-NOW callback function
  esp_now_register_send_cb(onDataSent);

  // Register peer
  memcpy(peerInfo.peer_addr, target_mac, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  
  // Add peer        
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }

}

void loop()
{

sensors_event_t hum, temp;

// Read sensors
aht.getEvent(&hum,&temp);

  // Print sensor data to serial port
  Serial.println("Temperature: " + String(temp.temperature) + " C, Humidity: " + String(hum.relative_humidity) + "%");

  // Create structure to hold sensor data
  sensor_data data = {temp.temperature, hum.relative_humidity};

  // Broadcast sensor data using ESP-NOW
  esp_err_t result = esp_now_send(target_mac, (uint8_t*) &data, sizeof(data));

  if (result != ESP_OK) {
    Serial.println("Error sending data");
  }

  // Delay for sending 
  delay(50);

  // Deep sleep
  esp_deep_sleep(dst*1000*1000);
}
