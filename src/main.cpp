#include <M5EPD.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <time.h>

M5EPD_Canvas canvas(&M5.EPD);

#define WIFI_SSID ""
#define WIFI_PASS ""
#define DEVICE_NAME "HomeDisplay"
#define AWS_IOT_ENDPOINT "xxxxxxxx-ats.iot.ap-northeast-1.amazonaws.com"
#define AWS_IOT_PORT 8883

#define TOPIC_REQUEST_LATEST "get/home/latest"
#define TOPIC_ACK_LATEST "ack/home/latest"
#define SENSOR_DEVICE_NAME "AFRStick"

#define WHEATHER_API "https://api.openweathermap.org/data/2.5/onecall?lat=35.677730&lon=139.754813&lang=ja&exclude=minutely,hourly,daily&appid=your_appid"

const char *root_ca = R"(-----BEGIN CERTIFICATE-----
MIIE0zCCA7ugAwIBAgIQGNrRniZ96LtKIVjNzGs7SjANBgkqhkiG9w0BAQUFADCB
yjELMAkGA1UEBhMCVVMxFzAVBgNVBAoTDlZlcmlTaWduLCBJbmMuMR8wHQYDVQQL
ExZWZXJpU2lnbiBUcnVzdCBOZXR3b3JrMTowOAYDVQQLEzEoYykgMjAwNiBWZXJp
U2lnbiwgSW5jLiAtIEZvciBhdXRob3JpemVkIHVzZSBvbmx5MUUwQwYDVQQDEzxW
ZXJpU2lnbiBDbGFzcyAzIFB1YmxpYyBQcmltYXJ5IENlcnRpZmljYXRpb24gQXV0
aG9yaXR5IC0gRzUwHhcNMDYxMTA4MDAwMDAwWhcNMzYwNzE2MjM1OTU5WjCByjEL
MAkGA1UEBhMCVVMxFzAVBgNVBAoTDlZlcmlTaWduLCBJbmMuMR8wHQYDVQQLExZW
ZXJpU2lnbiBUcnVzdCBOZXR3b3JrMTowOAYDVQQLEzEoYykgMjAwNiBWZXJpU2ln
biwgSW5jLiAtIEZvciBhdXRob3JpemVkIHVzZSBvbmx5MUUwQwYDVQQDEzxWZXJp
U2lnbiBDbGFzcyAzIFB1YmxpYyBQcmltYXJ5IENlcnRpZmljYXRpb24gQXV0aG9y
aXR5IC0gRzUwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQCvJAgIKXo1
nmAMqudLO07cfLw8RRy7K+D+KQL5VwijZIUVJ/XxrcgxiV0i6CqqpkKzj/i5Vbex
t0uz/o9+B1fs70PbZmIVYc9gDaTY3vjgw2IIPVQT60nKWVSFJuUrjxuf6/WhkcIz
SdhDY2pSS9KP6HBRTdGJaXvHcPaz3BJ023tdS1bTlr8Vd6Gw9KIl8q8ckmcY5fQG
BO+QueQA5N06tRn/Arr0PO7gi+s3i+z016zy9vA9r911kTMZHRxAy3QkGSGT2RT+
rCpSx4/VBEnkjWNHiDxpg8v+R70rfk/Fla4OndTRQ8Bnc+MUCH7lP59zuDMKz10/
NIeWiu5T6CUVAgMBAAGjgbIwga8wDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8E
BAMCAQYwbQYIKwYBBQUHAQwEYTBfoV2gWzBZMFcwVRYJaW1hZ2UvZ2lmMCEwHzAH
BgUrDgMCGgQUj+XTGoasjY5rw8+AatRIGCx7GS4wJRYjaHR0cDovL2xvZ28udmVy
aXNpZ24uY29tL3ZzbG9nby5naWYwHQYDVR0OBBYEFH/TZafC3ey78DAJ80M5+gKv
MzEzMA0GCSqGSIb3DQEBBQUAA4IBAQCTJEowX2LP2BqYLz3q3JktvXf2pXkiOOzE
p6B4Eq1iDkVwZMXnl2YtmAl+X6/WzChl8gGqCBpH3vn5fJJaCGkgDdk+bW48DW7Y
5gaRQBi5+MHt39tBquCWIMnNZBU4gcmU7qKEKQsTb47bDN0lAtukixlE0kF6BWlK
WE9gyn6CagsCqiUXObXbf+eEZSqVir2G3l6BFoMtEMze/aiCKm0oHw0LxOXnGiYZ
4fQRbxC1lfznQgUy286dUV4otp6F01vvpX1FQHKOtw5rDgb7MzVIcbidJ4vEZV8N
hnacRHr2lVz2XTIIM6RUthg/aFzyQkqFOFSDX9HoLPKsEdao7WNq
-----END CERTIFICATE-----
-----BEGIN CERTIFICATE-----
MIIDQTCCAimgAwIBAgITBmyfz5m/jAo54vB4ikPmljZbyjANBgkqhkiG9w0BAQsF
ADA5MQswCQYDVQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRkwFwYDVQQDExBBbWF6
b24gUm9vdCBDQSAxMB4XDTE1MDUyNjAwMDAwMFoXDTM4MDExNzAwMDAwMFowOTEL
MAkGA1UEBhMCVVMxDzANBgNVBAoTBkFtYXpvbjEZMBcGA1UEAxMQQW1hem9uIFJv
b3QgQ0EgMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALJ4gHHKeNXj
ca9HgFB0fW7Y14h29Jlo91ghYPl0hAEvrAIthtOgQ3pOsqTQNroBvo3bSMgHFzZM
9O6II8c+6zf1tRn4SWiw3te5djgdYZ6k/oI2peVKVuRF4fn9tBb6dNqcmzU5L/qw
IFAGbHrQgLKm+a/sRxmPUDgH3KKHOVj4utWp+UhnMJbulHheb4mjUcAwhmahRWa6
VOujw5H5SNz/0egwLX0tdHA114gk957EWW67c4cX8jJGKLhD+rcdqsq08p8kDi1L
93FcXmn/6pUCyziKrlA4b9v7LWIbxcceVOF34GfID5yHI9Y/QCB/IIDEgEw+OyQm
jgSubJrIqg0CAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMC
AYYwHQYDVR0OBBYEFIQYzIU07LwMlJQuCFmcx7IQTgoIMA0GCSqGSIb3DQEBCwUA
A4IBAQCY8jdaQZChGsV2USggNiMOruYou6r4lK5IpDB/G/wkjUu0yKGX9rbxenDI
U5PMCCjjmCXPI6T53iHTfIUJrU6adTrCC2qJeHZERxhlbI1Bjjt/msv0tadQ1wUs
N+gDS63pYaACbvXy8MWy7Vu33PqUXHeeE6V/Uq2V8viTO96LXFvKWlJbYK8U90vv
o/ufQJVtMVT8QtPHRh8jrdkPSHCa2XV4cdFyQzR1bldZwgJcJmApzyMZFo6IQ6XU
5MsI+yMRQ+hDKXJioaldXgjUkK642M4UwtBV8ob2xJNDd2ZhwLnoQdeXeGADbkpy
rqXRfboQnoZsG4q5WTP468SQvvG5
-----END CERTIFICATE-----
)";

const char *certificate = R"(-----BEGIN CERTIFICATE-----
-----END CERTIFICATE-----
)";

const char *private_key = R"(-----BEGIN RSA PRIVATE KEY-----
-----END RSA PRIVATE KEY-----
)";

bool mqtt_message_recived = false;
int update_counter = 0;

int get_battery()
{
    uint32_t vol = M5.getBatteryVoltage();
    if(vol < 3300){
        vol = 3300;
    }
    else if(vol > 4350){
        vol = 4350;
    }
    float battery = (float)(vol - 3300) / (float)(4350 - 3300);
    if(battery <= 0.01){
        battery = 0.01;
    }
    if(battery > 1){
        battery = 1;
    }
    return (int)(battery * 100);
}

void setup()
{

    M5.begin();
    M5.EPD.SetRotation(90);
    M5.EPD.Clear(true);
    update_counter = 0;

    // Load font files from SD Card
    canvas.loadFont("/fonts/GenSenRounded-R.ttf", SD);
    canvas.createRender(16);
    canvas.createRender(32);
    canvas.createRender(64);
    canvas.createRender(72, 256);

    // title line
    canvas.createCanvas(540, 5);
    canvas.drawLine(0, 1, 540, 1, 5, TFT_BLUE);
    canvas.pushCanvas(0,85,UPDATE_MODE_GLD16);
    canvas.deleteCanvas();

    // weather line
    canvas.createCanvas(540, 5);
    canvas.drawLine(0, 1, 540, 1, 2, TFT_BLUE);
    canvas.pushCanvas(0,341,UPDATE_MODE_GLD16);
    canvas.deleteCanvas();

    // tempture image
    canvas.createCanvas(540, 205);
    canvas.drawPngFile(SD, "/icon/temp.png");
    canvas.drawLine(0, 202, 540, 202, 2, TFT_BLUE);
    canvas.pushCanvas(1,350,UPDATE_MODE_GLD16);
    canvas.deleteCanvas();

    // humidity image
    canvas.createCanvas(540, 205);
    canvas.drawPngFile(SD, "/icon/hud.png");
    canvas.drawLine(0, 202, 540, 202, 2, TFT_BLUE);
    canvas.pushCanvas(1,560,UPDATE_MODE_GLD16);
    canvas.deleteCanvas();

    // co2 image
    canvas.createCanvas(205, 200);
    canvas.drawPngFile(SD, "/icon/co2.png");
    canvas.pushCanvas(1,770,UPDATE_MODE_GLD16);
    canvas.deleteCanvas();
}

void draw_title(tm *data_date){

    char current_date[32];
    strftime(current_date, 32, "%m/%d", data_date);

    canvas.createCanvas(400, 85);
    canvas.setTextDatum(TC_DATUM);
    canvas.setTextSize(72);
    canvas.drawString(current_date, 10, 10);

    char week_day[32];
    strftime(week_day, 32, "(%a)", data_date);
    canvas.setTextSize(32);
    canvas.drawString(week_day, 230, 38);
    canvas.pushCanvas(0,0,UPDATE_MODE_GLD16);
    canvas.deleteCanvas();

    char last_update[64];
    strftime(last_update, 32, "Update: %H:%M:%S", data_date);
    canvas.createCanvas(140, 20);
    canvas.setTextDatum(TC_DATUM);
    canvas.setTextSize(16);
    canvas.drawString(last_update, 0, 0);
    canvas.pushCanvas(400,60,UPDATE_MODE_GLD16);
    canvas.deleteCanvas();
}

// get weather data and draw it
void draw_weather_forcast(){
    Serial.println("start draw_weather_forcast()");

    // get forcast data from https://openweathermap.org/
    HTTPClient http;
    http.begin(WHEATHER_API);
    int httpCode = http.GET();
 
    if (httpCode <= 0) {
        Serial.println("Error on HTTP request");
        Serial.println("failed draw_weather_forcast()");
        http.end();
        return;
    }

    String payload = http.getString();
    Serial.println(payload);
    DynamicJsonDocument weather_json(1024);
    DeserializationError error = deserializeJson(weather_json, payload);
    if (error) {
        Serial.print("deserializeJson() failed: ");
        Serial.println(error.f_str());
        Serial.println("failed draw_weather_forcast()");
        http.end();
        return;
    }

    // weather data
    const char* icon = weather_json["current"]["weather"][0]["icon"].as<char*>();
    const char* description = weather_json["current"]["weather"][0]["description"].as<char*>();
    const double temp = weather_json["current"]["temp"].as<double>();
    const double humidity = weather_json["current"]["humidity"].as<double>();
    time_t weather_dt = weather_json["current"]["dt"];
    weather_dt += weather_json["timezone_offset"].as<int>();

    Serial.print("icon:");
    Serial.println(icon);
    Serial.print("description:");
    Serial.println(description);
    Serial.print("temperature:");
    Serial.println(temp-273.15);
    Serial.print("humidity:");
    Serial.println(humidity);

    // Title(Date)
    tm *tm_weather = localtime(&weather_dt);
    draw_title(tm_weather);

    // weather image
    canvas.createCanvas(245, 245);
    String weather_path = "/icon/" + String(icon) + ".png"; 
    canvas.drawPngFile(SD, weather_path.c_str());
    canvas.pushCanvas(1,95,UPDATE_MODE_GLD16);
    canvas.deleteCanvas();

    // weather description
    canvas.createCanvas(280, 200);
    canvas.setTextSize(72);
    canvas.setTextDatum(TC_DATUM);
    canvas.drawString(description, 5, 5);
    canvas.pushCanvas(250,120,UPDATE_MODE_GLD16);
    canvas.deleteCanvas();

    // tempture forcast
    char buf[24];
    sprintf(buf, "予報 %.1f °", temp-273.15);
    canvas.createCanvas(320, 80);
    canvas.setTextSize(64);
    canvas.setTextDatum(TC_DATUM);
    canvas.drawString(buf, 5, 5);
    canvas.pushCanvas(205,360,UPDATE_MODE_GLD16);
    canvas.deleteCanvas();

    // humidity forcast
    sprintf(buf, "予報 %.0f %%", humidity);
    canvas.createCanvas(320, 80);
    canvas.setTextSize(64);
    canvas.setTextDatum(TC_DATUM);
    canvas.drawString(buf, 5, 5);
    canvas.pushCanvas(205,570,UPDATE_MODE_GLD16);
    canvas.deleteCanvas();

    http.end();
    Serial.println("finish draw_weather_forcast()");
}

// MQTT Subscribe callback
// draw latest sensor data
void latest_data_callback(char* topic, byte* payload, unsigned int length) {
    Serial.println("start latest_data_callback()");

    DynamicJsonDocument latest_data_json(length);
    DeserializationError error = deserializeJson(latest_data_json, payload);
    if (error) {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
        return;
    }
    mqtt_message_recived = true;

    struct tm result;
    const char* latest_time = latest_data_json["latest"]["iot_timestamp"];
    if (strptime(latest_time, "%Y/%m/%d %H:%M:%S",&result) == NULL){
        Serial.println("strptime failed. skip updating title.");
    }
    else{
        draw_title(&result);
    }
    const int temp = latest_data_json["latest"]["temp"].as<int>();
    const int hud = latest_data_json["latest"]["hud"].as<int>();
    const int co2 = latest_data_json["latest"]["co2"].as<int>();

    // tempture latest
    char buf[24];
    sprintf(buf, "部屋 %d.0 °", temp);
    canvas.createCanvas(320, 80);
    canvas.setTextSize(64);
    canvas.setTextDatum(TC_DATUM);
    canvas.drawString(buf, 5, 5);
    canvas.pushCanvas(205,460,UPDATE_MODE_GLD16);
    canvas.deleteCanvas();

    // humidity latest
    sprintf(buf, "部屋 %d %%", hud);
    canvas.createCanvas(320, 80);
    canvas.setTextSize(64);
    canvas.setTextDatum(TC_DATUM);
    canvas.drawString(buf, 5, 5);
    canvas.pushCanvas(205,670,UPDATE_MODE_GLD16);
    canvas.deleteCanvas();

    // co2 latest
    sprintf(buf, "%d ppm", co2);
    canvas.createCanvas(320, 80);
    canvas.setTextSize(64);
    canvas.setTextDatum(TC_DATUM);
    canvas.drawString(buf, 5, 5);
    canvas.pushCanvas(220,820,UPDATE_MODE_GLD16);
    canvas.deleteCanvas();

    Serial.println("finish latest_data_callback()");
}

void draw_latest_data(){
    Serial.println("start draw_latest_data()");

    // setup to connect AWS IoT Core
    WiFiClientSecure https_client;
    PubSubClient mqtt_client(https_client);
    https_client.setCACert(root_ca);
    https_client.setCertificate(certificate);
    https_client.setPrivateKey(private_key);
    mqtt_client.setServer(AWS_IOT_ENDPOINT, AWS_IOT_PORT);
    mqtt_client.setKeepAlive(300);
    mqtt_client.setCallback(latest_data_callback);

    int retry = 0;
    while (!mqtt_client.connected()){
        if (mqtt_client.connect(DEVICE_NAME)){
            Serial.println("MQTT Connected");
            // subscribe to recivie the latest data from AWS IoT Core
            mqtt_client.subscribe(TOPIC_ACK_LATEST);
            mqtt_message_recived = false;
            delay(1000);

            // publish serial number(device name) to recivie it's latest data. 
            StaticJsonDocument<200> json_document;
            char json_string[100];
            json_document["serialNumber"] = SENSOR_DEVICE_NAME;
            serializeJson(json_document, json_string);
            mqtt_client.publish(TOPIC_REQUEST_LATEST, json_string);
            break;
        }
        else{
            Serial.println((String)"failed, rc=" + mqtt_client.state());
            delay(500);
            retry++;
        }
        if(retry > 5){
            Serial.println("Retrying MQTT connection failed.");
            Serial.println("failed draw_latest_data()");
            return;
        }
    }

    // wait until subscribed message recivied
    retry = 0;
    while(!mqtt_message_recived){
        mqtt_client.loop();
        delay(500);
        retry++;
        Serial.println(retry);

        if(retry >= 10){
            // message couldn't recivie in 5 seconds.
            Serial.println("MQTT message recivie timeout.");
            mqtt_message_recived = true;
        }
    }
    mqtt_client.disconnect();
    Serial.println("finished draw_latest_data()");
}

void loop() {

    // show battery
    int batt = get_battery();
    char battery[20];
    sprintf(battery, "BATT %d%% ", batt);
    canvas.createCanvas(100, 50);
    canvas.setTextDatum(TC_DATUM);
    canvas.setTextSize(16);
    canvas.drawString(battery, 0, 0);
    canvas.pushCanvas(440,10,UPDATE_MODE_GLD16);
    canvas.deleteCanvas();

    WiFi.begin(WIFI_SSID, WIFI_PASS);
    
    Serial.print("Connecting to WiFi..");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    
    Serial.println("Connected to the WiFi network");
 
    if(update_counter == 0 || update_counter > 6){
        // renew forcast data every 30 minuets
        update_counter = 0;
        draw_weather_forcast();
    }
    update_counter++;

    draw_latest_data();

    WiFi.disconnect();
    delay(1000);

    Serial.println("Goto sleep");
    // sleep 5 minuets
    M5.shutdown(300);  // if USB is connected deep sleep is available
    delay(300000);
}
