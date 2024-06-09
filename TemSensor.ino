#include "DHT.h"
#include <WiFiNINA.h>
#include "secrets.h"

#define DHTPIN 2
#define DHTTYPE DHT22 

#define LED_READ 5    // LED pin for successful sensor read
#define LED_SEND 6    // LED pin for successful data send

DHT dht(DHTPIN, DHTTYPE);

WiFiClient client;

const char* server = "192.168.0.124"; // IP address of the Raspberry Pi
const int port = 5000; // Port number for the server

unsigned long lastReadTime = 0;
unsigned long lastSendTime = 0;
const unsigned long readInterval = 3000;  // Interval for reading sensor data (3 seconds)
const unsigned long sendInterval = 50000; // Interval for sending data (50 seconds)

float temperature;
float humidity;

void setup() {
  Serial.begin(115200);
  while (!Serial); // wait for serial port to connect
  dht.begin();
  
  pinMode(LED_READ, OUTPUT);
  pinMode(LED_SEND, OUTPUT);

  // Connect to Wi-Fi
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(SECRET_SSID);
    WiFi.begin(SECRET_SSID, SECRET_PASS);
    while (WiFi.status() != WL_CONNECTED) {
      Serial.print(".");
      delay(5000);     
    }
  }
  Serial.println("\nConnected.");
}

void loop() {
  unsigned long currentTime = millis();

  // Read sensor data every 3 seconds
  if (currentTime - lastReadTime >= readInterval) {
    lastReadTime = currentTime;

    humidity = dht.readHumidity();
    temperature = dht.readTemperature(); // Temperature in Celsius

    if (isnan(humidity) || isnan(temperature)) {
      Serial.println("Failed to read from DHT sensor!");
      digitalWrite(LED_READ, LOW);
    } else {
      Serial.print("Humidity: ");
      Serial.print(humidity);
      Serial.print("%  Temperature: ");
      Serial.print(temperature);
      Serial.println("°C");
      digitalWrite(LED_READ, HIGH); // Turn on LED for successful read
    }
  }

  // Send data every 50 seconds
  if (currentTime - lastSendTime >= sendInterval) {
    lastSendTime = currentTime;

    if (WiFi.status() != WL_CONNECTED) {
      // Attempt to reconnect to Wi-Fi
      Serial.print("Reconnecting to Wi-Fi...");
      WiFi.begin(SECRET_SSID, SECRET_PASS);
      while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(5000);     
      }
      Serial.println("\nReconnected.");
    }

    if (client.connect(server, port)) {
      // Construct HTTP request
      String postData = "temperature=" + String(temperature) + "&humidity=" + String(humidity);
      client.println("POST /data HTTP/1.1");
      client.println("Host: " + String(server));
      client.println("Content-Type: application/x-www-form-urlencoded");
      client.println("Content-Length: " + String(postData.length()));
      client.println();
      client.println(postData);
      client.stop();
      Serial.println("Data sent to server.");
      digitalWrite(LED_SEND, HIGH); // Turn on LED for successful send
      delay(1000); // Keep the send LED on for 1 second for visual confirmation
      digitalWrite(LED_SEND, LOW);
    } else {
      Serial.println("Connection to server failed.");
      digitalWrite(LED_SEND, LOW);
    }
  }
}
