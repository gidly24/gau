#include <Arduino.h>
#include <WiFi.h>
#include <WebSocketsClient.h>

#define PULSE_PIN 4
volatile unsigned long pulseCount = 0;
bool pulseActive = false;
unsigned long pulseStartTime = 0;
const unsigned long PULSE_LOW_TIME = 30000; // 30 сек LOW

// --- Wi-Fi ---
const char* ssid = "Rus";
const char* password = "guikguik";

// --- WebSocket ---
WebSocketsClient webSocket;
const char* ws_host = "192.168.0.110"; // IP сервера
const uint16_t ws_port = 8080;         // порт WebSocket сервера

void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
  switch(type) {
    case WStype_DISCONNECTED:
      Serial.println("WS отключен");
      break;
    case WStype_CONNECTED:
      Serial.println("WS подключен");
      break;
    case WStype_TEXT:
      Serial.printf("Сообщение от сервера: %s\n", payload);
      break;
    case WStype_BIN:
      Serial.println("Получены бинарные данные");
      break;
    case WStype_PING:
      Serial.println("PING получен");
      break;
    case WStype_PONG:
      Serial.println("PONG получен");
      break;
  }
}


void setup() {
  Serial.begin(115200);
  pinMode(PULSE_PIN, INPUT_PULLUP);

  WiFi.begin(ssid, password);
  Serial.print("Подключение к WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi подключен!");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());

  // Подключаем WebSocket
  webSocket.begin(ws_host, ws_port, "/");
  webSocket.onEvent(webSocketEvent);
  webSocket.setReconnectInterval(5000); // переподключение каждые 5 сек
}

void loop() {
  webSocket.loop();

  int pulseState = digitalRead(PULSE_PIN);

  if (pulseState == LOW && !pulseActive) {
    pulseActive = true;
    pulseStartTime = millis();
  }

  if (pulseState == HIGH && pulseActive) {
    unsigned long pulseDuration = millis() - pulseStartTime;
    if (pulseDuration >= (PULSE_LOW_TIME - 5000) && pulseDuration <= (PULSE_LOW_TIME + 5000)) {
      pulseCount++;
      Serial.printf("Новый импульс! Всего: %lu\n", pulseCount);

      // Отправляем сразу через WS
      String message = String(pulseCount);
      webSocket.sendTXT(message);
    }
    pulseActive = false;
  }
}
