#include <Arduino.h>
#include <Servo.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <string.h>

#include <Wire.h> // lcd
#include <LiquidCrystal_I2C.h>

#include <NewPing.h> // Library untuk SR04

#include <NTPClient.h>
#include <WiFiUdp.h>

// Network Credentials
const char *SSID = "dan";
const char *PASSWORD = "test1234";

// Inisialisasi objek untuk WiFiUDP
WiFiUDP ntpUDP;

Servo servo;

AsyncWebServer server(80); // Initiate Asynchronous Webserver on port 80

#define LCD_ADDRESS 0x27 // Alamat I2C LCD
LiquidCrystal_I2C lcd(LCD_ADDRESS, 16, 2);

#define ECHO_PIN D7                                 // Pin echo SR04 dihubungkan ke pin D2 pada NodeMCU
#define TRIGGER_PIN D6                              // Pin trigger SR04 dihubungkan ke pin D1 pada NodeMCU
#define MAX_DISTANCE 200                            // Jarak maksimum yang dapat diukur (dalam cm)
NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE); // Inisialisasi objek sonar

String times[5] = {"01:00", "16:46", "10:00", "11:00", "08:10"};

// Inisialisasi objek untuk NTPClient
NTPClient timeClient(ntpUDP, "pool.ntp.org");

String timeString;

void updateWaktu()
{
  // Memperbarui waktu dari NTP server
  timeClient.update();

  // Mendapatkan waktu saat ini dari NTPClient
  unsigned long epochTime = timeClient.getEpochTime();
  time_t currentTime = epochTime;

  // Mendapatkan komponen waktu (jam, menit, detik)
  tm *timeinfo = localtime(&currentTime);
  int hour = timeinfo->tm_hour;
  int minute = timeinfo->tm_min;

  // Format waktu ke dalam bentuk string
  timeString = (((hour + 7) % 24) < 10 ? "0" + String((hour + 7) % 24) : String((hour + 7) % 24)) + ":" + ((minute < 10 ? "0" + String(minute) : String(minute)));

  lcd.setCursor(11, 0);
  lcd.print(timeString);
}

void cekJamMakan()
{
  for (int i = 0; i < 5; i++)
  {
    if (times[i] == timeString)
    {
      lcd.clear();
      lcd.setCursor(5, 0);
      lcd.print("Selamat");
      lcd.setCursor(6, 1);
      lcd.print("Makan");
      servo.write(180);
      delay(1000);
      servo.write(0);
      delay(58950);
    }
  }
}

void cekIsiMakanan()
{
  delay(50);
  int distance = sonar.ping_cm();
   Serial.println(distance);
  while (distance > 9)
  {
    delay(50);
    distance = sonar.ping_cm();
    Serial.println(distance);
    lcd.clear();
    lcd.setCursor(5, 0);
    lcd.print("Makanan");
    lcd.setCursor(6, 1);
    lcd.print("Habis");
    delay(2000);
  }
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Waktu    : ");
  lcd.setCursor(0, 1);
  lcd.print("FeedTime : ");
}

void setup()
{
  Serial.begin(9600);
  Wire.begin(2, 0);

  servo.attach(D2, 500, 2500); // We need to attach the servo to the used pin number
  servo.write(0);

  lcd.begin(16, 2);
  lcd.init();
  lcd.backlight();
  lcd.setCursor(1, 0);
  lcd.print("SELAMAT DATANG");
  lcd.setCursor(0, 1);
  lcd.print("");
  delay(2000);
  // Connect to WiFi
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(SSID);

  // Setting up WiFi
  WiFi.mode(WIFI_STA);
  WiFi.begin(SSID, PASSWORD);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(300);
    Serial.print(".");
  }

  // Displaying Connected Status
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  // endpoint
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(200, "text/html", "<html><head><title>Pakan Ayam</title></head><body><form method='get' action='/times'><table><tr><td>No</td><td>Waktu</td><td>Aksi</td></tr><tr><td>1</td><td>" + times[0] + "</td><td><input type='text' name='time1' value='" + times[0] + "'></td></tr><tr><td>2</td><td>" + times[1] + "</td><td><input type='text' name='time2' value='" + times[1] + "'></td></tr><tr><td>3</td><td>" + times[2] + "</td><td><input type='text' name='time3' value='" + times[2] + "'></td></tr><tr><td>4</td><td>" + times[3] + "</td><td><input type='text' name='time4' value='" + times[3] + "'></td></tr><tr><td>5</td><td>" + times[4] + "</td><td><input type='text' name='time5' value='" + times[4] + "'></td></tr><tr><td><button type='submit'>Submit</button></td></tr></table></form></body>"); });
  server.on("/times", HTTP_GET, [](AsyncWebServerRequest *request)
            {
    const char *time1 = request->getParam("time1")->value().c_str();
    const char *time2 = request->getParam("time2")->value().c_str();
    const char *time3 = request->getParam("time3")->value().c_str();
    const char *time4 = request->getParam("time4")->value().c_str();
    const char *time5 = request->getParam("time5")->value().c_str();
    times[0] = time1;
    times[1] = time2;
    times[2] = time3;
    times[3] = time4;
    times[4] = time5;
    request->send(200, "text/html", "<html><head><title>Pakan Ayam</title></head><body><form method='get' action='/times'><table><tr><td>No</td><td>Waktu</td><td>Aksi</td></tr><tr><td>1</td><td>"+times[0]+"</td><td><input type='text' name='time1' value='"+times[0]+"'></td></tr><tr><td>2</td><td>"+times[1]+"</td><td><input type='text' name='time2' value='"+times[1]+"'></td></tr><tr><td>3</td><td>"+times[2]+"</td><td><input type='text' name='time3' value='"+times[2]+"'></td></tr><tr><td>4</td><td>"+times[3]+"</td><td><input type='text' name='time4' value='"+times[3]+"'></td></tr><tr><td>5</td><td>"+times[4]+"</td><td><input type='text' name='time5' value='"+times[4]+"'></td></tr><tr><td><button type='submit'>Submit</button></td></tr></table></form></body>"); });

  server.begin();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Waktu    : ");
  lcd.setCursor(0, 1);
  lcd.print("FeedTime : ");
  updateWaktu();
  Serial.println("Webserver Running...");
}

void loop()
{
  for (int i = 0; i < 5; i++)
  {
    lcd.setCursor(11, 1);
    lcd.print(times[i]);
    delay(2000);
    cekIsiMakanan();
    cekJamMakan();
    updateWaktu();
  }
}
