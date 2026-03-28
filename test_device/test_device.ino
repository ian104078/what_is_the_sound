/*
 * test_device.ino — 互動式模組測試
 * 
 * Serial Monitor (9600 baud) 輸入數字選擇測試項目。
 * 透過相對路徑引用 clapping_device/ 的模組，測試同一份程式碼。
 */

#include "../clapping_device/config.h"
#include "../clapping_device/hardware.h"
#include "../clapping_device/hardware.cpp"
#include "../clapping_device/clap_engine.h"
#include "../clapping_device/clap_engine.cpp"

void printMenu();
void testUltrasonic();
void testServo();
void testDFPlayer();
void testSwitch();
void testClapFallback();
void testFullLoop();
void testMapping();

void setup() {
    Serial.begin(SERIAL_BAUD_RATE);
    delay(500);
    Serial.println(F(""));
    Serial.println(F("################################"));
    Serial.println(F("#  Clapping Device Test Suite  #"));
    Serial.println(F("################################"));
    Serial.println(F(""));

    ultrasonicInit();  Serial.println(F("[INIT] Ultrasonic OK"));
    servoInit();       Serial.println(F("[INIT] Servo OK"));
    switchInit();      Serial.println(F("[INIT] Switch OK"));

    if (audioInit()) Serial.println(F("[INIT] DFPlayer OK"));
    else             Serial.println(F("[WARN] DFPlayer FAILED"));

    Serial.println(F(""));
    printMenu();
}

void loop() {
    if (Serial.available()) {
        char cmd = Serial.read();
        while (Serial.available()) Serial.read();
        Serial.println(F(""));

        switch (cmd) {
            case '1': testUltrasonic();   break;
            case '2': testServo();        break;
            case '3': testDFPlayer();     break;
            case '4': testSwitch();       break;
            case '5': testClapFallback(); break;
            case '6': testFullLoop();     break;
            case '7': testMapping();      break;
            case '0': printMenu();        break;
            default:  Serial.println(F("Unknown. Press 0 for menu.")); break;
        }
    }
}

void printMenu() {
    Serial.println(F("===================================="));
    Serial.println(F("[1] Ultrasonic  - 5s distance reading"));
    Serial.println(F("[2] Servo       - Open/close x3"));
    Serial.println(F("[3] DFPlayer    - Play test sound"));
    Serial.println(F("[4] Switch      - 5s monitor"));
    Serial.println(F("[5] Clap+FB     - One clapExecute()"));
    Serial.println(F("[6] Full Loop   - 15s simulation"));
    Serial.println(F("[7] Mapping     - Parameter table"));
    Serial.println(F("[0] Show menu"));
    Serial.println(F("===================================="));
}

void testUltrasonic() {
    Serial.println(F("--- Ultrasonic (5s) ---"));
    Serial.println(F("  raw | smoothed"));
    unsigned long end = millis() + 5000;
    while (millis() < end) {
        long raw = readDistanceCm();
        long sm = readSmoothedDistance();
        Serial.print(F("  ")); Serial.print(raw == 0 ? F("---") : String(raw));
        Serial.print(F(" | ")); Serial.println(sm == 0 ? F("---") : String(sm));
        delay(200);
    }
    Serial.println(F("--- Done ---\n"));
}

void testServo() {
    Serial.println(F("--- Servo (3 cycles) ---"));
    for (int i = 1; i <= 3; i++) {
        Serial.print(F("  Cycle ")); Serial.print(i);
        servosClose(); delay(500);
        Serial.print(F(" close.."));
        servosOpen();  delay(500);
        Serial.println(F(" open OK"));
    }
    Serial.println(F("--- Done ---\n"));
}

void testDFPlayer() {
    Serial.println(F("--- DFPlayer ---"));
    audioSetVolume(20);
    delay(100);
    audioPlay();
    Serial.println(F("  Playing 0001.mp3 at vol=20"));
    Serial.println(F("--- Done ---\n"));
}

void testSwitch() {
    Serial.println(F("--- Switch (5s) ---"));
    Serial.println(F("  Press switch to see triggers."));
    unsigned long end = millis() + 5000;
    while (millis() < end) {
        switchUpdate();
        if (switchTriggered()) Serial.println(F("  >>> TRIGGERED <<<"));
        delay(5);
    }
    Serial.println(F("--- Done ---\n"));
}

void testClapFallback() {
    Serial.println(F("--- Clap + Fallback ---"));
    audioSetVolume(20);
    delay(100);
    unsigned long t = millis();
    clapExecute();
    Serial.print(F("  Duration: ")); Serial.print(millis() - t); Serial.println(F("ms"));
    Serial.print(F("  Path: ")); Serial.println(clapWasFallback() ? F("FALLBACK") : F("PRIMARY"));
    Serial.println(F("--- Done ---\n"));
}

void testFullLoop() {
    Serial.println(F("--- Full Loop (15s) ---"));
    Serial.println(F("  dist | delay | vol | action"));
    uint8_t vol = VOLUME_MIN;
    unsigned long lastClap = millis();
    unsigned long end = millis() + 15000;
    while (millis() < end) {
        long dist = readSmoothedDistance();
        if (dist > 0 && dist <= DISTANCE_MAX_CM) {
            long d = constrain(dist, DISTANCE_MIN_CM, DISTANCE_MAX_CM);
            long del = map(d, DISTANCE_MIN_CM, DISTANCE_MAX_CM, CLAP_DELAY_MIN_MS, CLAP_DELAY_MAX_MS);
            uint8_t tv = map(d, DISTANCE_MIN_CM, DISTANCE_MAX_CM, VOLUME_MAX, VOLUME_MIN);
            if (vol < tv) vol = min((int)vol + VOLUME_SMOOTH_STEP, (int)tv);
            else if (vol > tv) vol = max((int)vol - VOLUME_SMOOTH_STEP, (int)tv);
            audioSetVolume(vol);
            if (millis() - lastClap >= (unsigned long)del) {
                Serial.print(F("  ")); Serial.print(dist);
                Serial.print(F(" | ")); Serial.print(del);
                Serial.print(F(" | ")); Serial.print(vol);
                Serial.print(F(" | "));
                clapExecute();
                lastClap = millis();
                Serial.println(clapWasFallback() ? F("FALLBACK") : F("PRIMARY"));
            }
        }
        delay(50);
    }
    servosOpen();
    Serial.println(F("--- Done ---\n"));
}

void testMapping() {
    Serial.println(F("--- Mapping Table (SD §2) ---"));
    Serial.println(F("  dist | delay | vol"));
    for (int d = 10; d <= 100; d += 10) {
        long del = map(d, DISTANCE_MIN_CM, DISTANCE_MAX_CM, CLAP_DELAY_MIN_MS, CLAP_DELAY_MAX_MS);
        uint8_t v = map(d, DISTANCE_MIN_CM, DISTANCE_MAX_CM, VOLUME_MAX, VOLUME_MIN);
        Serial.print(F("  ")); if (d < 100) Serial.print(F(" "));
        Serial.print(d); Serial.print(F("  | "));
        if (del < 1000) Serial.print(F(" "));
        Serial.print(del); Serial.print(F("  | "));
        Serial.println(v);
    }
    Serial.println(F("--- Done ---\n"));
}
