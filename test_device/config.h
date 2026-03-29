/*
 * config.h — 集中管理所有可調參數
 * 
 * 依據 SA 文件：硬體抽象層 (HAL) 所需之 Pin 定義
 * 依據 SD 文件：Mapping 參數、Debounce、Fallback 超時等常數
 * 依據 PRD NFR：電源隔離提示、防彈跳規格
 */

#ifndef CONFIG_H
#define CONFIG_H

// ============================================================
// Pin Definitions (依據 SA 硬體架構圖)
// ============================================================

// 微動開關 — 使用 D2 支援 INT0 硬體中斷 (Falling Edge)
#define PIN_SWITCH            2

// HC-SR04 超音波感測器
#define PIN_ULTRASONIC_TRIG   7
#define PIN_ULTRASONIC_ECHO   8

// MG90S 伺服馬達 — 使用 D9/D10 (Timer1 原生 PWM)
#define PIN_SERVO_LEFT        9
#define PIN_SERVO_RIGHT       10

// DFPlayer Mini — SoftwareSerial (TX 經 1KΩ 電阻降壓)
#define PIN_DFPLAYER_TX       11   // Arduino TX → DFPlayer RX (via 1KΩ)
#define PIN_DFPLAYER_RX       12   // Arduino RX ← DFPlayer TX

// ============================================================
// Servo Configuration (依據機構設計調整)
// ============================================================

// 左手馬達角度
#define SERVO_LEFT_OPEN       45    // 張開角度
#define SERVO_LEFT_CLOSE      135   // 閉合角度

// 右手馬達角度 (鏡像安裝，方向相反)
#define SERVO_RIGHT_OPEN      135   // 張開角度 (鏡像)
#define SERVO_RIGHT_CLOSE     45    // 閉合角度 (鏡像)

// ============================================================
// Ultrasonic Configuration (依據 PRD FR-01)
// ============================================================

#define DISTANCE_MIN_CM       10    // 最近有效偵測距離
#define DISTANCE_MAX_CM       100   // 最遠喚醒距離 (超過視為無目標)
#define DISTANCE_OUT_OF_RANGE 400   // 超出感測器物理範圍
#define MOVING_AVG_SIZE       3     // 移動平均濾波窗口 (SD 2.1)

// ============================================================
// Mapping Parameters (依據 SD 文件 Section 2)
// ============================================================

// 距離 → 拍手延遲 (SD 2.1: 越近越快)
#define CLAP_DELAY_MIN_MS     500   // 10cm 時的拍手間隔
#define CLAP_DELAY_MAX_MS     2000  // 100cm 時的拍手間隔

// 距離 → 音量 (SD 2.2: 越近越大聲)
#define VOLUME_MIN            10    // 100cm 時的音量
#define VOLUME_MAX            30    // 10cm 時的音量
#define VOLUME_SMOOTH_STEP    2     // 每次迴圈最大音量變化步階 (SD 2.2 平滑調整)

// ============================================================
// Timing Parameters
// ============================================================

// Fallback 超時 (SD 3.1: 200ms 時間窗)
#define FALLBACK_TIMEOUT_MS   200

// 防彈跳延遲 (SD 4: 50ms, PRD NFR 4.2: 50~100ms)
#define DEBOUNCE_DELAY_MS     50

// 閉合後維持時間 (SD 3.2: 延遲維持閉合)
#define CLAP_HOLD_MS          50

// 無目標超時進入待機 (PRD NFR 4.1: 5秒)
#define IDLE_TIMEOUT_MS       5000

// DFPlayer 指令間隔限制 (SD 4: Rate Limiting ≥ 200ms)
#define AUDIO_RATE_LIMIT_MS   200

// ============================================================
// Serial Debug
// ============================================================

#define SERIAL_BAUD_RATE      9600
#define DEBUG_ENABLED         true

#endif // CONFIG_H
