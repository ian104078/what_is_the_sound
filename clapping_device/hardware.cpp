/*
 * hardware.cpp — 硬體抽象層 (HAL) 統一實作
 * 
 * 依據 SA 文件四個 HAL 子層，合併為單一檔案：
 *   1. 超音波感測層 — Trig/Echo 脈衝、移動平均濾波 (SD §2.1)
 *   2. 馬達驅動層   — Servo.h PWM 角度控制
 *   3. 音效驅動層   — SoftwareSerial + DFPlayer, Rate Limiting (SD §4)
 *   4. 感測開關層   — INPUT_PULLUP, 50ms Debounce (SD §4)
 */

#include "hardware.h"
#include "config.h"
#include <Servo.h>
#include <SoftwareSerial.h>
#include <DFRobotDFPlayerMini.h>

// ============================================================
// 1. 超音波感測 (HC-SR04)
// ============================================================

static long _usBuffer[MOVING_AVG_SIZE];
static uint8_t _usIndex = 0;
static bool _usFilled = false;

void ultrasonicInit() {
    pinMode(PIN_ULTRASONIC_TRIG, OUTPUT);
    pinMode(PIN_ULTRASONIC_ECHO, INPUT);
    digitalWrite(PIN_ULTRASONIC_TRIG, LOW);
    for (uint8_t i = 0; i < MOVING_AVG_SIZE; i++) _usBuffer[i] = 0;
    _usIndex = 0;
    _usFilled = false;
}

long readDistanceCm() {
    // 發送 10μs 觸發脈衝
    digitalWrite(PIN_ULTRASONIC_TRIG, LOW);
    delayMicroseconds(2);
    digitalWrite(PIN_ULTRASONIC_TRIG, HIGH);
    delayMicroseconds(10);
    digitalWrite(PIN_ULTRASONIC_TRIG, LOW);

    // 讀取 Echo 回波 (超時 30ms ≈ 510cm)
    long duration = pulseIn(PIN_ULTRASONIC_ECHO, HIGH, 30000);
    long distance = duration / 29 / 2;

    // 過濾異常值
    if (distance <= 0 || distance > DISTANCE_OUT_OF_RANGE) return 0;
    return distance;
}

long readSmoothedDistance() {
    long raw = readDistanceCm();
    if (raw == 0) return 0;

    _usBuffer[_usIndex] = raw;
    _usIndex = (_usIndex + 1) % MOVING_AVG_SIZE;
    if (_usIndex == 0) _usFilled = true;

    uint8_t count = _usFilled ? MOVING_AVG_SIZE : _usIndex;
    if (count == 0) return raw;

    long sum = 0;
    for (uint8_t i = 0; i < count; i++) sum += _usBuffer[i];
    return sum / count;
}

// ============================================================
// 2. 伺服馬達 (MG90S x2)
// ============================================================

static Servo _servoLeft;
static Servo _servoRight;

void servoInit() {
    _servoLeft.attach(PIN_SERVO_LEFT);
    _servoRight.attach(PIN_SERVO_RIGHT);
    servosOpen();
}

void servosOpen() {
    _servoLeft.write(SERVO_LEFT_OPEN);
    _servoRight.write(SERVO_RIGHT_OPEN);
}

void servosClose() {
    _servoLeft.write(SERVO_LEFT_CLOSE);
    _servoRight.write(SERVO_RIGHT_CLOSE);
}

void servosDetach() {
    _servoLeft.detach();
    _servoRight.detach();
}

// ============================================================
// 3. DFPlayer Mini 音效
// ============================================================

static SoftwareSerial _dfSerial(PIN_DFPLAYER_RX, PIN_DFPLAYER_TX);
static DFRobotDFPlayerMini _player;
static unsigned long _lastPlayTime = 0;

bool audioInit() {
    _dfSerial.begin(9600);
    delay(1000);  // 等待 DFPlayer 開機

    if (!_player.begin(_dfSerial, true, true)) return false;

    _player.setTimeOut(500);
    _player.volume(VOLUME_MIN);
    _player.EQ(DFPLAYER_EQ_NORMAL);
    _player.outputDevice(DFPLAYER_DEVICE_SD);
    return true;
}

void audioSetVolume(uint8_t vol) {
    _player.volume(constrain(vol, 0, 30));
}

void audioPlay() {
    if (!audioIsReady()) return;
    _player.play(1);
    _lastPlayTime = millis();
}

bool audioIsReady() {
    return (millis() - _lastPlayTime) >= AUDIO_RATE_LIMIT_MS;
}

// ============================================================
// 4. 微動開關 (Debounce — SD §4)
// ============================================================

static bool _swCurrent = HIGH;
static bool _swLastReading = HIGH;
static bool _swTriggered = false;
static unsigned long _swDebounceTime = 0;

void switchInit() {
    pinMode(PIN_SWITCH, INPUT_PULLUP);
    _swCurrent = digitalRead(PIN_SWITCH);
    _swLastReading = _swCurrent;
    _swTriggered = false;
}

void switchUpdate() {
    bool reading = digitalRead(PIN_SWITCH);

    if (reading != _swLastReading) _swDebounceTime = millis();
    _swLastReading = reading;

    if ((millis() - _swDebounceTime) > DEBOUNCE_DELAY_MS) {
        if (reading != _swCurrent) {
            bool prev = _swCurrent;
            _swCurrent = reading;
            // Falling Edge: HIGH → LOW
            if (prev == HIGH && _swCurrent == LOW) _swTriggered = true;
        }
    }
}

bool switchTriggered() {
    if (_swTriggered) {
        _swTriggered = false;
        return true;
    }
    return false;
}
