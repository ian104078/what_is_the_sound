/*
 * clap_engine.h — 拍手動作 + Fallback 引擎
 * 
 * SA 應用邏輯層：錯誤與 Fallback 管理員
 * SD §3：主路徑 (微動開關觸發) + 備援路徑 (200ms 超時強制觸發)
 */

#ifndef CLAP_ENGINE_H
#define CLAP_ENGINE_H

#include <Arduino.h>

void clapExecute();
bool clapWasFallback();

#endif // CLAP_ENGINE_H
