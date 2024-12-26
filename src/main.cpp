#include <Arduino.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <WebServer.h>
#include <DHTesp.h>

DHTesp              dht;
int                 interval = 2000;
unsigned long       lastDHTReadMillis = 0;
float               humidity = 0;
float               temperature = 0;
char                dht_buffer[10];

void readDHT22() {
    unsigned long currentMillis = millis();

    if(currentMillis - lastDHTReadMillis >= interval) {
        lastDHTReadMillis = currentMillis;

        humidity = dht.getHumidity();              // Read humidity (percent)
        temperature = dht.getTemperature();             // Read temperature as Fahrenheit
    }
}

const char* ssid = "IoT518";
const char* password = "iot123456";

WebServer server(80);

void handleRoot() {
    String message = (server.method() == HTTP_GET)?"GET":"POST";
    message += " " + server.uri() + "\n";
    for (uint8_t i=0; i < server.args(); i++){
        message += " " + server.argName(i) + " : " + server.arg(i) + "\n";
    }
    message += "\nHello from ESP32!\n";
    server.send(200, "text/plain", message);
}

void handleNotFound() {
    String message = "File Not Found\n\n";
    server.send(404, "text/plain", message);
}

void thermo() {
    char    mBuf[500];
    char    tmplt[] =   "<html><head><meta charset=\"utf-8\">"
                        "<meta http-equiv='refresh' content='5'/>"
                        "<title>온습도계</title></head>"
                        "<body>"
                        "<script></script>"
                        "<center><br>"
                        "<p>온도 : %.1f</p>"
                        "<p>습도 : %.1fs</p>"
                        "</center>"
                        "</body></html>";

    sprintf(mBuf, tmplt, temperature, humidity);
    Serial.println("serving");
    server.send(200, "text/html", mBuf);
}

void setup(void) {
    Serial.begin(115200);
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    Serial.println("");
    dht.setup(15, DHTesp::DHT22);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.printf("\nConnected to %s, and Server IP : %s\n",ssid, WiFi.localIP().toString().c_str());
    MDNS.begin("ThermoWeb");

    server.on("/", handleRoot);
    server.on("/thermo", thermo);
 
    server.onNotFound(handleNotFound);

    server.begin();
    Serial.println("HTTP server started");
}

void loop(void) {
    readDHT22();
    server.handleClient();
}