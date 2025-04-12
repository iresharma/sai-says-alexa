#include <WiFi.h>
#include <HTTPClient.h>
#include <HardwareSerial.h>

#define RXD2 16 // Define RX2 pin
#define TXD2 17 // Define TX2 pin

// Define the serial connection for Arduino communication
// ESP32 has multiple hardware serial ports

// WiFi credentials
const char* ssid = "Winterfell";
const char* password = "forthethrone";


// OpenWeatherMap API configuration
const String openWeatherMapApiKey = "ffe8f234e7d1e07de86b9ddf5e70658b"; // Replace with your API key
const String city = "Bangalore"; // Replace with your city
const String countryCode = "IN"; // Replace with your country code

// URL to poll
const char* url = "https://sai-says-alexa.onrender.com/message";

String message = "";

void setup() {
  // Initialize Serial communication for debugging
  Serial.begin(115200);
  delay(1000);
  
  // Initialize serial communication with Arduino
  // Serial2 on ESP32 can use custom pins - adjust if needed
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2); // RX=16, TX=17
  
  // Connect to WiFi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());

  Serial.println("HTTP polling will begin in 5 seconds...");
  delay(5000);
}

String pollURL() {
  HTTPClient http;

  Serial.print("[HTTP] Polling URL: ");
  Serial.println(url);

  // Configure HTTP request
  http.begin(url);

  // Send HTTP GET request
  int httpResponseCode = http.GET();

  // Check for successful response
  if (httpResponseCode > 0) {
    Serial.print("[HTTP] Response code: ");
    Serial.println(httpResponseCode);

    // Get the response payload
    String payload = http.getString();
    Serial.println("--------- BEGIN RESPONSE ---------");
    Serial.println(payload);
    Serial.println("---------- END RESPONSE ----------");
    return payload;
  } else {
    Serial.print("[HTTP] Error code: ");
    Serial.println(httpResponseCode);
    Serial.println(http.errorToString(httpResponseCode).c_str());
    return "Error";
  }

  // Free resources
  http.end();
  Serial.println("Next poll in 30 seconds...");
}

// Function to get weather data from OpenWeatherMap
String getWeatherData() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi not connected. Cannot fetch weather data.");
    return "Error: WiFi not connected";
  }
  
  HTTPClient http;
  
  // Construct the URL for the OpenWeatherMap API request
  String weatherUrl = "http://api.openweathermap.org/data/2.5/weather?q=" + 
                      city + "," + countryCode + 
                      "&units=metric&appid=" + openWeatherMapApiKey;
  
  Serial.print("[HTTP] Getting weather from: ");
  Serial.println(weatherUrl);
  
  // Begin HTTP request
  http.begin(weatherUrl);
  
  // Send HTTP GET request
  int httpResponseCode = http.GET();
  
  if (httpResponseCode > 0) {
    Serial.print("[HTTP] Weather response code: ");
    Serial.println(httpResponseCode);
    
    // Get the response payload
    String payload = http.getString();
    
    // Parse JSON response to extract weather condition and feels_like temperature
    int weatherStart = payload.indexOf("\"main\":\"") + 8;
    int weatherEnd = payload.indexOf("\"", weatherStart);
    String weatherCondition = payload.substring(weatherStart, weatherEnd);
    
    int feelsLikeStart = payload.indexOf("\"feels_like\":") + 13;
    int feelsLikeEnd = payload.indexOf(",", feelsLikeStart);
    String feelsLike = payload.substring(feelsLikeStart, feelsLikeEnd);
    
    // Convert weather condition to simpler terms
    String simplifiedWeather = "";
    if (weatherCondition.indexOf("Rain") >= 0 || weatherCondition.indexOf("Drizzle") >= 0) {
      simplifiedWeather = "rain";
    } else if (weatherCondition.indexOf("Cloud") >= 0) {
      simplifiedWeather = "cloudy";
    } else if (weatherCondition.indexOf("Clear") >= 0) {
      simplifiedWeather = "clear";
    } else if (weatherCondition.indexOf("Snow") >= 0) {
      simplifiedWeather = "snow";
    } else if (weatherCondition.indexOf("Thunderstorm") >= 0) {
      simplifiedWeather = "storm";
    } else if (weatherCondition.indexOf("Fog") >= 0 || weatherCondition.indexOf("Mist") >= 0 || weatherCondition.indexOf("Haze") >= 0) {
      simplifiedWeather = "foggy";
    } else {
      simplifiedWeather = weatherCondition; // Use original if no match
    }
    
    // Format the weather message as a two-liner with a special character to indicate line break
    // We'll use '|' as a line break marker that the Arduino will interpret
    String weatherMessage = simplifiedWeather + "|" + feelsLike + "C";
    
    Serial.print("Weather data: ");
    Serial.println(weatherMessage);
    
    // Free resources
    http.end();
    
    return weatherMessage;
  } else {
    Serial.print("[HTTP] Weather request failed, error: ");
    Serial.println(httpResponseCode);
    
    // Free resources
    http.end();
    
    return "Weather Error: " + String(httpResponseCode);
  }
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    // Poll for message
    String response = pollURL();
    
    if (response != "Error" && response != message) {
      message = response;
      
      // Send the response to Arduino
      Serial2.println(message);
      Serial.println("Message sent to Arduino: " + message);
    }
    
    // Get and send weather data every 30 minutes (30 * 60 * 1000 ms)
    static unsigned long lastWeatherTime = 0;
    unsigned long currentMillis = millis();
    
    if (currentMillis - lastWeatherTime >= 1800000 || lastWeatherTime == 0) {
      lastWeatherTime = currentMillis;
      
      String weatherData = getWeatherData();
      if (!weatherData.startsWith("Error") && !weatherData.startsWith("Weather Error")) {
        Serial.println("Weather sent to Arduino: " + weatherData);
        Serial2.print(weatherData);
      }
    }
    
  } else {
    Serial.println("WiFi Disconnected. Attempting to reconnect...");
    WiFi.reconnect();
  }
  delay(10000);
}