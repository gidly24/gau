#include <Arduino.h>
#include <FS.h>
#include <SPIFFS.h>
#include <WiFi.h>
#include <WebServer.h>

#define PULSE_PIN 4       // Пин импульсного выхода
volatile unsigned long pulseCount = 0;  // Счётчик импульсов
bool pulseActive = false;               // Флаг активного импульса
unsigned long pulseStartTime = 0;       // Время начала импульса
const unsigned long PULSE_LOW_TIME = 30000;  // 30 сек LOW

// --- Wi-Fi настройки ---
const char* ssid = "Rus";
const char* password = "guikguik";

WebServer server(80);

// HTML страница с автообновлением раз в секунду
void handleRoot() {
  String html = "<html><head><meta http-equiv='refresh' content='1'>";
  html += "<meta charset='UTF-8'></head><body style='font-family:sans-serif;'>";
  html += "<h1>Счётчик импульсов</h1>";
  html += "<p>Текущее значение: <b>" + String(pulseCount) + "</b></p>";
  html += "<p>Обновление раз в секунду</p>";
  html += "</body></html>";
  server.send(200, "text/html", html);
}

void setup() {
  Serial.begin(115200);
  SPIFFS.begin(true);

  // Загружаем сохранённое значение
  if (SPIFFS.exists("/pulse_count.txt")) {
    File file = SPIFFS.open("/pulse_count.txt", FILE_READ);
    pulseCount = file.parseInt();
    file.close();
    Serial.print("Предыдущее значение: ");
    Serial.println(pulseCount);
  }

  pinMode(PULSE_PIN, INPUT_PULLUP);

  // Подключаемся к Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("Подключение к Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi подключен!");
  Serial.print("IP-адрес: ");
  Serial.println(WiFi.localIP());

  // Запуск веб-сервера
  server.on("/", handleRoot);
  server.begin();
}

void loop() {
  int pulseState = digitalRead(PULSE_PIN);

  // Начало импульса
  if (pulseState == LOW && !pulseActive) {
    pulseActive = true;
    pulseStartTime = millis();
  }

  // Конец импульса
  if (pulseState == HIGH && pulseActive) {
    unsigned long pulseDuration = millis() - pulseStartTime;

    // Проверка длительности LOW
    if (pulseDuration >= (PULSE_LOW_TIME - 5000) && pulseDuration <= (PULSE_LOW_TIME + 5000)) {
      pulseCount++;
      Serial.print("Новый импульс! Всего: ");
      Serial.println(pulseCount);
    }
    pulseActive = false;
  }

  // Сохранение в SPIFFS каждые 5 секунд
  static unsigned long lastSave = 0;
  if (millis() - lastSave >= 5000) {
    File file = SPIFFS.open("/pulse_count.txt", FILE_WRITE);
    if (file) {
      file.print(pulseCount);
      file.close();
      Serial.print("Сохранено: ");
      Serial.println(pulseCount);
    }
    lastSave = millis();
  }

  // Обработка веб-запросов
  server.handleClient();
}
