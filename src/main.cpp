#include <Arduino.h>
#include <FS.h>
#include <SPIFFS.h>

#define PULSE_PIN 4       // Пин импульсного выхода
volatile unsigned long pulseCount = 0;  // Счётчик импульсов
bool pulseActive = false;  // Флаг активного импульса
unsigned long pulseStartTime = 0;  // Время начала импульса
const unsigned long PULSE_LOW_TIME = 30000;  // 30 сек LOW (в миллисекундах!)

void setup() {
    Serial.begin(115200);
    SPIFFS.begin(true);  // Инициализация SPIFFS

    // Загрузка предыдущего значения
    if (SPIFFS.exists("/pulse_count.txt")) {
        File file = SPIFFS.open("/pulse_count.txt", FILE_READ);
        pulseCount = file.parseInt();
        file.close();
        Serial.print("Предыдущее значение: ");
        Serial.println(pulseCount);
    }

    pinMode(PULSE_PIN, INPUT_PULLUP);  // Подтяжка к HIGH
}

void loop() {
    int pulseState = digitalRead(PULSE_PIN);  // Читаем состояние пина

    // Если сигнал LOW и импульс ещё не начат
    if (pulseState == LOW && !pulseActive) {
        pulseActive = true;
        pulseStartTime = millis();  // Запоминаем время начала
    }

    // Если сигнал снова HIGH и импульс был активен
    if (pulseState == HIGH && pulseActive) {
        unsigned long pulseDuration = millis() - pulseStartTime;

        // Если LOW длился достаточно долго (30±5 сек)
        if (pulseDuration >= (PULSE_LOW_TIME - 5000) && pulseDuration <= (PULSE_LOW_TIME + 5000)) {
            pulseCount++;  // Считаем валидный импульс
            Serial.print("Новый импульс! Всего: ");
            Serial.println(pulseCount);
        }
        pulseActive = false;  // Сбрасываем флаг
    }

    // Сохранение в файл каждые 5 секунд
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
}