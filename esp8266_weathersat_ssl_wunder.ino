#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP085_U.h>
   
Adafruit_BMP085_Unified bmp = Adafruit_BMP085_Unified(10085);

#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include "DHT.h"

#define DHTPIN 2     // DHT 22 what digital pin we're connected to
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
DHT dht(DHTPIN, DHTTYPE);

 
const char* ssid     = "TashiStation";
const char* password = "xxxxxxxx";
 
const char* host = "yoursplunkwebsite.com";
const char* wund_host = "rtupdate.wunderground.com";

int LDR = A0;

// Use web browser to view and copy
// SHA1 fingerprint of the certificate
const char* fingerprint = "5F:2A:6E:0E:7B:8E:D1:80:55:C9:05:5E:82:FC:F3:76:7D:DC:63:47";

void setup(void) 
{
  Serial.begin(9600);
// We start by connecting to a WiFi network
 
  Serial.println();
  Serial.println("Waking up!");
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
 
  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  
  delay(500);
  
  //DHT setup
  Serial.println("");
  Serial.println("DHT22 intialize");
  dht.begin();
  
  /* Initialise the sensor */
  if(!bmp.begin())
  {
    /* There was a problem detecting the BMP085 ... check your connections */
    Serial.print("Ooops, no BMP085 detected ... Check your wiring or I2C ADDR!");
    while(1);
  }
  
  // Light Sensor Setup on pin A0
  Serial.println("Light Sensor Enabled");
  Serial.println();
  
  pinMode(0, OUTPUT);
  pinMode(LDR, INPUT);
}

int value = 0;
float temperature;
float pressure;

void loop(void) 
{
  delay(1000);
  
  // Get HUMIDITY and HEAT Index
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  float f = dht.readTemperature(true);

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println("Failed to read from DHT sensor!");
    dht.begin();
    return;
  }

  // Compute heat index in Fahrenheit (the default)
  float hif = dht.computeHeatIndex(f, h);
  // Compute heat index in Celsius (isFahreheit = false)
  float hic = dht.computeHeatIndex(t, h, false);
  // Compute dew point
  float dewpt =  f - ((100 - h)/5);

  Serial.print("Humidity: ");
  Serial.print(h);
  Serial.print(" %\t");
  Serial.print("Temperature: ");
  Serial.print(t);
  Serial.print(" *C ");
  Serial.print(f);
  Serial.print(" *F\t");
  Serial.print("Heat index: ");
  Serial.print(hic);
  Serial.print(" *C ");
  Serial.print(hif);
  Serial.println(" *F");

  

  // Get Light Sensor Data

  int lux = analogRead(LDR);
  
  Serial.print("Light Level: ");
  Serial.println(lux);
  
  /* Get a new sensor event */ 
  sensors_event_t event;
  bmp.getEvent(&event);
 
  /* Display the results (barometric pressure is measure in hPa) */
  if (event.pressure)
  {
    /* Display atmospheric pressue in hPa */
    Serial.print("Pressure:    ");
    Serial.print(event.pressure);
    pressure = event.pressure;
    Serial.println(" hPa");
    
    /* Calculating altitude with reasonable accuracy requires pressure    *
     * sea level pressure for your position at the moment the data is     *
     * converted, as well as the ambient temperature in degress           *
     * celcius.  If you don't have these values, a 'generic' value of     *
     * 1013.25 hPa can be used (defined as SENSORS_PRESSURE_SEALEVELHPA   *
     * in sensors.h), but this isn't ideal and will give variable         *
     * results from one day to the next.                                  *
     *                                                                    *
     * You can usually find the current SLP value by looking at weather   *
     * websites or from environmental information centers near any major  *
     * airport.                                                           *
     *                                                                    *
     * For example, for Paris, France you can check the current mean      *
     * pressure and sea level at: http://bit.ly/16Au8ol                   */
     
    /* First we get the current temperature from the BMP085 */
    
    bmp.getTemperature(&temperature);
    Serial.print("Temperature: ");
    Serial.print(temperature);
    Serial.println(" C");
 
    /* Then convert the atmospheric pressure, SLP and temp to altitude    */
    /* Update this next line with the current SLP for better results      */
    float seaLevelPressure = SENSORS_PRESSURE_SEALEVELHPA;
    Serial.print("Altitude:    "); 
    Serial.print(bmp.pressureToAltitude(seaLevelPressure,
                                        event.pressure,
                                        temperature)); 
    Serial.println(" m");
    Serial.println("");
  }
  else
  {
    Serial.println("Sensor error");
  }
  delay(1000);

  /* SEND WEATHER DATA */

    delay(500);
  ++value;
 
  Serial.print("connecting to ");
  Serial.println(host);
  
  // Use WiFiClientSecure class to create TLS connection
  WiFiClientSecure client;
  const int httpPort = 8088;
  const int httpPort_std = 80;
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
  }

 if (client.verify(fingerprint, host)) {
    Serial.println("certificate matches");
  } else {
    Serial.println("certificate doesn't match");
  }

  // We now create a URI for the request
  String url = "/services/collector";
  String payload = String("{\"event\": {\"temperature\" : \"") + temperature + String("\",\"pressure\" : \"") + pressure + String("\",\"lux\":\"") + lux + String("\",\"humidity\":\"") + h + String("\",\"dht_temp\":\"") + f + String("\",\"heat_idx\":\"") +  hif + String("\"}}");
  Serial.print("Posting to URL: ");
  Serial.println(url);
  
  // This will send the request to the server
  client.print(String("POST ") + url + " HTTP/1.1\r\n" + "Host: " + host + "\r\n");
  client.println("Authorization: Splunk xxxxxxxxxxxxxxx-xxxxxxxxx-xxxxxxxxxxx-xxxxxxxxx");
  client.print("Content-Length: ");
  client.println(payload.length());
  client.println();
  client.println(payload);
  Serial.println(payload);
  client.println();
  
  delay(2000);
  
  // Read all the lines of the reply from server and print them to Serial
  while (client.connected())
  {
    if ( client.available() )
    {
      char str=client.read();
      Serial.print(str);
    }      
  }

  Serial.println("closing connection");
  Serial.println();
  client.stop();

  delay(2000);

  WiFiClient client2;
  
  if (!client2.connect(wund_host, httpPort_std)) {
    Serial.println("connection failed");
  }
  float hg_in= (.0295300 * pressure); 
  String url2 = String("/weatherstation/updateweatherstation.php?ID=WEATHERID&PASSWORD=xxxxxxxxxxx&dateutc=now&tempf=") + f + String("&baromin=") + hg_in + String("&dewptf=") + dewpt + String("&humidity=") + h + String("&softwaretype=esp8266%20version1.0&action=updateraw&realtime=1&rtfreq=2.");
  Serial.print("Posting to URL: ");
  Serial.println(wund_host + url2);

  // Make an HTTP GET request
  client2.println(String("GET ") + url2 + "  HTTP/1.1");
  client2.print("Host: ");
  client2.println(wund_host);
  client2.println("Connection: close");
  client2.println();

  delay(2000);
  
  while (client2.connected())
  {
    if ( client2.available() )
    {
      char str=client2.read();
      Serial.print(str);
    }      
  }

  Serial.println("closing connection");
  Serial.println();
  client2.stop();
  
  // Set up wifi sleep mode to save power
  Serial.println("[Enable LIGHT_SLEEP]");
  Serial.println("Going to Sleep....");

  ESP.deepSleep(300000000, WAKE_RF_DEFAULT);
}
