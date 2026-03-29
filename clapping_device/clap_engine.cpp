/*
 * clap_engine.cpp — 拍手動作 + Fallback 引擎實作
 * 
 * 嚴格依據 SD §3.1 流程圖與 §3.2 虛擬碼：
 * 1. servosClose()
 * 2. 啟動 millis() 計時器，200ms 時間窗輪詢微動開關
 * 3. 主路徑：Falling Edge → audioPlay()
 * 4. Fallback：超時 → 警告 → audioPlay()
 * 5. delay(CLAP_HOLD_MS) 維持閉合
 * 6. servosOpen() 歸位
 */

#include "clap_engine.h"
#include "config.h"
#include "hardware.h"

static bool _lastWasFallback = false;

void clapExecute() {
    _lastWasFallback = false;

    // Step 1: 馬達閉合
    servosClose();

    // Step 2: 啟動計時器，進入 200ms 時間窗
    unsigned long startTime = millis();
    bool soundPlayed = false;

    // Step 3: 輪詢微動開關
    while (millis() - startTime <= FALLBACK_TIMEOUT_MS) {
        switchUpdate();
        if (switchTriggered()) {
            // ==== 主路徑 (Primary) ====
            audioPlay();
            soundPlayed = true;
            break;
        }
    }

    // Step 4: Fallback
    if (!soundPlayed) {
        _lastWasFallback = true;
        if (DEBUG_ENABLED) {
            Serial.println(F("[FALLBACK] Switch not detected. (Fallback disabled for testing)"));
        }
        // audioPlay(); // ✋ 在此將 Fallback 發聲功能暫時關閉，以利測試微動開關
    }

    // Step 5: 維持閉合
    delay(CLAP_HOLD_MS);

    // Step 6: 歸位張開
    servosOpen();
}

bool clapWasFallback() {
    return _lastWasFallback;
}
