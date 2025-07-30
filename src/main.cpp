#include <Arduino.h>
#include <FS.h>
#include <SPIFFS.h>

#define PULSE_PIN 4 // Пин для импульсного выхода
volatile unsigned long pulseCount = 0; // Счётчик импульсов

void IRAM_ATTR pulseCounter() {
    pulseCount++; // Увеличиваем счётчик при каждом импульсе
}

void setup() {
    Serial.begin(115200);

    // Инициализация SPIFFS
    if (!SPIFFS.begin(true)) {
        Serial.println("Ошибка инициализации SPIFFS!");
        while (1);
    }
    Serial.println("SPIFFS инициализирован");

    // Читаем предыдущее значение из файла
    if (SPIFFS.exists("/pulse_count.txt")) {
        File file = SPIFFS.open("/pulse_count.txt", FILE_READ);
        pulseCount = file.parseInt(); // Читаем число из файла
        file.close();
        Serial.print("Предыдущее значение: ");
        Serial.println(pulseCount);
    }

    pinMode(PULSE_PIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(PULSE_PIN), pulseCounter, FALLING);
}

void loop() {
    static unsigned long lastSave = 0;
    if (millis() - lastSave >= 5000) { // Сохранение каждые 5 секунд
        File file = SPIFFS.open("/pulse_count.txt", FILE_WRITE);
        if (file) {
            file.print(pulseCount); // Записываем текущее значение
            file.close();
            Serial.print("Сохранено: ");
            Serial.println(pulseCount);
        } else {
            Serial.println("Ошибка записи в файл!");
        }
        lastSave = millis();
    }
}