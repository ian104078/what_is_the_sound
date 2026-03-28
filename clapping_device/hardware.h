/*
 * hardware.h — 硬體抽象層 (HAL) 統一介面
 * 
 * 依據 SA 文件：將超音波、馬達、音效、微動開關四個硬體驅動
 * 合併為單一 HAL 模組，對外提供簡潔的函式介面。
 */

#ifndef HARDWARE_H
#define HARDWARE_H

#include <Arduino.h>

// ---- 超音波感測 (HC-SR04) ----
void ultrasonicInit();
long readDistanceCm();
long readSmoothedDistance();     // 3 點移動平均濾波 (SD 2.1)

// ---- 伺服馬達 (MG90S x2) ----
void servoInit();
void servosOpen();
void servosClose();
void servosDetach();

// ---- DFPlayer Mini 音效 ----
bool audioInit();
void audioSetVolume(uint8_t vol);
void audioPlay();               // 含 Rate Limiting (SD §4: ≥200ms)
bool audioIsReady();

// ---- 微動開關 (Debounce) ----
void switchInit();
void switchUpdate();            // 每次 loop 呼叫，更新防彈跳狀態
bool switchTriggered();         // Falling Edge 觸發 (讀後清除)

#endif // HARDWARE_H
