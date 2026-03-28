/*
 * clapping_device.ino — Arduino 拍手互動裝置 主程式
 * 
 * SA：三層架構 (HAL → 應用邏輯 → 展示)
 * SD §1：四狀態狀態機 (IDLE / SENSING / CLAPPING / SLEEP)
 * SD §2：距離 → 延遲/音量 Mapping 引擎
 */

#include "config.h"
#include "hardware.h"
#include "clap_engine.h"

// ---- 狀態機 (SD §1) ----
enum SystemState { STATE_IDLE, STATE_SENSING, STATE_CLAPPING, STATE_SLEEP };

static SystemState currentState = STATE_IDLE;
static unsigned long lastDetectionTime = 0;
static unsigned long lastClapTime = 0;
static uint8_t currentVolume = VOLUME_MIN;

// 前向宣告
void handleIdle(long distance);
void handleSensing(long distance);
void handleClapping();
void handleSleep(long distance);

// ============================================================
// setup()
// ============================================================
void setup() {
    Serial.begin(SERIAL_BAUD_RATE);
    Serial.println(F(""));
    Serial.println(F("========================================"));
    Serial.println(F("  Arduino Clapping Device v1.0"));
    Serial.println(F("========================================"));

    ultrasonicInit();
    Serial.println(F("[INIT] Ultrasonic OK"));

    servoInit();
    Serial.println(F("[INIT] Servo OK"));

    switchInit();
    Serial.println(F("[INIT] Switch OK"));

    if (!audioInit()) {
        Serial.println(F("[ERROR] DFPlayer FAILED! Check wiring/SD card."));
        pinMode(LED_BUILTIN, OUTPUT);
        while (true) {
            digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
            delay(200);
        }
    }
    Serial.println(F("[INIT] DFPlayer OK"));

    Serial.println(F("[OK] All ready. Entering IDLE."));
    Serial.println(F(""));

    lastDetectionTime = millis();
    lastClapTime = millis();
}

// ============================================================
// loop() — 狀態機驅動
// ============================================================
void loop() {
    long distance = readSmoothedDistance();

    switch (currentState) {
        case STATE_IDLE:     handleIdle(distance);     break;
        case STATE_SENSING:  handleSensing(distance);  break;
        case STATE_CLAPPING: handleClapping();          break;
        case STATE_SLEEP:    handleSleep(distance);    break;
    }
}

// ============================================================
// 狀態處理
// ============================================================

// IDLE: 等待目標進入範圍
void handleIdle(long distance) {
    if (distance > 0 && distance <= DISTANCE_MAX_CM) {
        currentState = STATE_SENSING;
        lastDetectionTime = millis();
        currentVolume = VOLUME_MIN;
        if (DEBUG_ENABLED) {
            Serial.print(F("[STATE] IDLE->SENSING dist="));
            Serial.println(distance);
        }
    }
}

// SENSING: 距離映射 + 判斷拍手時機 (SD §2)
void handleSensing(long distance) {
    // 目標消失超過 5 秒 → 回 IDLE (PRD NFR 4.1)
    if (distance == 0 || distance > DISTANCE_MAX_CM) {
        if (millis() - lastDetectionTime > IDLE_TIMEOUT_MS) {
            currentState = STATE_IDLE;
            servosOpen();
            if (DEBUG_ENABLED) Serial.println(F("[STATE] SENSING->IDLE (timeout)"));
        }
        return;
    }
    lastDetectionTime = millis();

    // SD §2.1: 距離 → 拍手延遲
    long d = constrain(distance, DISTANCE_MIN_CM, DISTANCE_MAX_CM);
    long clapDelay = map(d, DISTANCE_MIN_CM, DISTANCE_MAX_CM,
                         CLAP_DELAY_MIN_MS, CLAP_DELAY_MAX_MS);

    // SD §2.2: 距離 → 音量 (含平滑)
    uint8_t targetVol = map(d, DISTANCE_MIN_CM, DISTANCE_MAX_CM,
                            VOLUME_MAX, VOLUME_MIN);
    if (currentVolume < targetVol)
        currentVolume = min((int)currentVolume + VOLUME_SMOOTH_STEP, (int)targetVol);
    else if (currentVolume > targetVol)
        currentVolume = max((int)currentVolume - VOLUME_SMOOTH_STEP, (int)targetVol);
    audioSetVolume(currentVolume);

    // 到達拍手間隔 → 進入 CLAPPING
    if (millis() - lastClapTime >= (unsigned long)clapDelay) {
        currentState = STATE_CLAPPING;
        if (DEBUG_ENABLED) {
            Serial.print(F("[CLAP] dist="));
            Serial.print(distance);
            Serial.print(F(" delay="));
            Serial.print(clapDelay);
            Serial.print(F(" vol="));
            Serial.println(currentVolume);
        }
    }
}

// CLAPPING: 執行拍手 (SD §3 含 Fallback)
void handleClapping() {
    clapExecute();
    lastClapTime = millis();
    if (DEBUG_ENABLED) {
        Serial.print(F("[CLAP] path="));
        Serial.println(clapWasFallback() ? F("FALLBACK") : F("PRIMARY"));
    }
    currentState = STATE_SENSING;
}

// SLEEP: 深度休眠 (馬達釋放)
void handleSleep(long distance) {
    if (distance > 0 && distance <= DISTANCE_MAX_CM) {
        servoInit();
        currentState = STATE_IDLE;
        if (DEBUG_ENABLED) Serial.println(F("[STATE] SLEEP->IDLE (wake)"));
    }
}
