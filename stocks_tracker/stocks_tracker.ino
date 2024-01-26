#include <NTPClient.h>
#include <WiFiUdp.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <ESP8266HTTPClient.h>

const char ISRG_Root_X1 [] PROGMEM = R"CERT(
-----BEGIN CERTIFICATE-----
MIICGzCCAaGgAwIBAgIQQdKd0XLq7qeAwSxs6S+HUjAKBggqhkjOPQQDAzBPMQsw
CQYDVQQGEwJVUzEpMCcGA1UEChMgSW50ZXJuZXQgU2VjdXJpdHkgUmVzZWFyY2gg
R3JvdXAxFTATBgNVBAMTDElTUkcgUm9vdCBYMjAeFw0yMDA5MDQwMDAwMDBaFw00
MDA5MTcxNjAwMDBaME8xCzAJBgNVBAYTAlVTMSkwJwYDVQQKEyBJbnRlcm5ldCBT
ZWN1cml0eSBSZXNlYXJjaCBHcm91cDEVMBMGA1UEAxMMSVNSRyBSb290IFgyMHYw
EAYHKoZIzj0CAQYFK4EEACIDYgAEzZvVn4CDCuwJSvMWSj5cz3es3mcFDR0HttwW
+1qLFNvicWDEukWVEYmO6gbf9yoWHKS5xcUy4APgHoIYOIvXRdgKam7mAHf7AlF9
ItgKbppbd9/w+kHsOdx1ymgHDB/qo0IwQDAOBgNVHQ8BAf8EBAMCAQYwDwYDVR0T
AQH/BAUwAwEB/zAdBgNVHQ4EFgQUfEKWrt5LSDv6kviejM9ti6lyN5UwCgYIKoZI
zj0EAwMDaAAwZQIwe3lORlCEwkSHRhtFcP9Ymd70/aTSVaYgLXTWNLxBo1BfASdW
tL4ndQavEi51mI38AjEAi/V3bNTIZargCyzuFJ0nN6T5U6VR5CmD1/iQMVtCnwr1
/q4AaOeMSQ+2b1tbFfLn
-----END CERTIFICATE-----
)CERT";

// Replace with your network credentials
const char* ssid = "T-2_45f0d9";
const char* password = "pedri12345";

char c = 'a';

WiFiClientSecure client;
HTTPClient https;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

int stock_market_opens = 1530;
int stock_market_closes = 2200;
int current_time = 0;

// Timing
long interval = 120000;
unsigned long previousMillis = 0;
unsigned long currentMillis;

// Create a list of certificates with the server certificate
X509List cert(ISRG_Root_X1);

void setup() {
  Serial.begin(9600);
  //Serial.setDebugOutput(true);

  //Connect to Wi-Fi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println();
  Serial.println();
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }

  timeClient.begin();

  // Set time via NTP, as required for x.509 validation
  configTime(3 * 3600, 0, "pool.ntp.org", "time.nist.gov");

  Serial.print("Waiting for NTP time sync: ");
  time_t now = time(nullptr);
  while (now < 8 * 3600 * 2) {
    delay(500);
    Serial.print(".");
    now = time(nullptr);
  }
  Serial.println("");
  struct tm timeinfo;
  gmtime_r(&now, &timeinfo);
  Serial.print("Current time: ");
  Serial.print(asctime(&timeinfo));
}

void loop() {

  timeClient.update();
  //Serial.println(timeClient.getFormattedTime());
  //Serial.println(timeClient.getHours()*100 + timeClient.getMinutes());
  current_time = timeClient.getHours()*100 + timeClient.getMinutes();

  currentMillis = millis();

  // wait for WiFi connection
  if ((WiFi.status() == WL_CONNECTED)) {

    client.setTrustAnchors(&cert);

    //Serial.println(current_time);

    // Speed of requesting data from the website: every 2mins between 15:28 and 22:00 (market open hours) +1 because UTC-0
    if(((currentMillis - previousMillis) > interval) && (current_time >= 1428) && (current_time <= 2200)){
      
      previousMillis = currentMillis;

      // Serial.print("[HTTPS] begin...\n");
      if (https.begin(client, "https://api.twelvedata.com/price?symbol=AMD,NOC,PLTR,LMT&dp=2&apikey=9d5aabf6c0224949b69be96a2a53ab93")) {

        // Serial.print("[HTTPS] GET...\n");
        // Start connection and send HTTP header
        int httpCode = https.GET();

        // httpCode will be negative on error
        if (httpCode > 0) {
          // HTTP header has been send and Server response header has been handled
          // Serial.printf("[HTTPS] GET... code: %d\n", httpCode);
          // Serial.println(https.getString());

          // Data found at server
          if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {

            String market_data = https.getString();

            // Sends market_data via serial channel:
            for(int i = 0; i < strlen((market_data).c_str()); i++ ) {
              c = market_data[i];
              Serial.write(c);
            }
            Serial.write('\n');

            
          }
        } else {
          Serial.printf("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
        }

        https.end();
      } else {
        Serial.printf("[HTTPS] Unable to connect\n");
      }
  
    }
  }
}