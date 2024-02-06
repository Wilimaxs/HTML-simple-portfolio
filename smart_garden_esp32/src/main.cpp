#include <Arduino.h>
#include <DHT.h>
#include <DHT_U.h>
#include <WiFi.h>
#include <HTTPClient.h>

#define sensorPin A0
#define Pompa 21

const char *ssid = ""; // name of wifi
const char *password = ""; // pass of wifi
const char *serverName = ""; //name of server

float adcValue;

String apiKeyValue = ""; //key same with your php file
String point;

//custom value
unsigned const long interval = 2000;
unsigned long zero = 0;

void setup()
{

  Serial.begin(9600);
  WiFi.begin(ssid, password);
  Serial.println("Connecting");

  // menunggu koneksi
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void loop()
{

  adcValue = analogRead(sensorPin);

  if (adcValue > 0 && adcValue < 1400)
  {
    Serial.println("basah");
    point = "Basah";
  }
  else if (adcValue > 1400 && adcValue < 3000)
  {
    Serial.println("Kering");
    point = "Kering";
  }

  // Post data from sensor and send to Database
  if (WiFi.status() == WL_CONNECTED)
  {
    HTTPClient http;

    // Your Domain name
    http.begin(serverName);

    // Specify content-type header
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");

    // post data same with your database
    String httpRequestData = "api_key=" + apiKeyValue + "&kelembabantanah=" + adcValue + "&data=" + point + "";
    Serial.print("httpRequestData: ");
    Serial.println(httpRequestData);

    int httpResponseCode = http.POST(httpRequestData);

    if (httpResponseCode > 0)
    {
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
    }
    else
    {
      Serial.print("Error code: ");
      Serial.println(httpResponseCode);
    }
    http.end();
  }
  else
  {
    Serial.println("WiFi Disconnected");
  }

  // GET data from database
  if (millis() - zero > interval)
  {

    HTTPClient http;
    http.begin("");//your domain for get
    int httpCode = http.GET();

    String payload = http.getString();
    http.begin("");//your domain for get
    int httpCode1 = http.GET();

    String payload_control = http.getString();

    if (httpCode1 > 0)
    {
      ESP_LOGI("HASIL", "baca data dari database %s", payload);
      ESP_LOGI("HASIL", "baca data dari database %s", payload_control);

      if (payload_control == "DISABLE")
      {
        ESP_LOGI("Hasil", "Mode Otomatis DIaktifkan");
        if (point == "Basah")
        {
          pinMode(Pompa, INPUT);
        }
        else if (point == "Kering")
        {
          pinMode(Pompa, OUTPUT);
        }
      }
      else if (payload_control == "ENABLE")
      {
        ESP_LOGI("Hasil", "Mode Otomatis Dimatikan");
        if (payload == "Nyala")
        {
          pinMode(Pompa, OUTPUT);
        }
        else
        {
          pinMode(Pompa, INPUT);
        }
      }
    }
    else
    {
      Serial.print("error ");
      Serial.println(httpCode1);
    }

    zero = millis();
  }
  delay(5000);
}