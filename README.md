# What Is The Sound — Arduino 拍手互動裝置

透過超音波感測距離，驅動伺服馬達模擬拍手動作，並搭配 DFPlayer Mini 播放音效。距離越近，拍手越快、音量越大。

## 專案結構

```
what_is_the_sound/
├── clapping_device/                     ← Arduino sketch (IDE 入口)
│   ├── clapping_device.ino              ← 主程式：狀態機 + mapping 引擎
│   ├── config.h                         ← 所有 Pin 定義與可調常數
│   ├── hardware.h / .cpp                ← HAL 硬體驅動 (超音波/馬達/音效/開關)
│   └── clap_engine_fallback.h / .cpp    ← 拍手動作 + Fallback 雙軌觸發引擎
├── test_device/                         ← 測試 sketch
│   └── test_device.ino                  ← 互動式 Serial 選單測試
├── doc/                                 ← 軟體工程文件
│   ├── Arduino 拍手互動裝置產品需求文件 (PRD).md
│   ├── Arduino 拍手互動裝置系統架構文件 (SA).md
│   └── Arduino 拍手互動裝置系統設計文件 (SD).md
└── README.md
```

## 硬體接線

| 元件 | Arduino Pin | 備註 |
|---|---|---|
| 微動開關 | D2 | INPUT_PULLUP，支援 INT0 |
| HC-SR04 Trig | D7 | |
| HC-SR04 Echo | D8 | |
| Servo Left | D9 | Timer1 PWM |
| Servo Right | D10 | Timer1 PWM |
| DFPlayer RX | D11 | 經 **1KΩ** 電阻降壓 |
| DFPlayer TX | D12 | |

> ⚠️ **電源隔離**：馬達與 DFPlayer 的 VCC 必須從 5V/2A 行動電源獨立供電，**不可**由 Arduino 5V 腳位取電。所有 GND 必須共地。

## 使用方式

### 前置作業
1. Arduino IDE Library Manager 安裝 `DFRobotDFPlayerMini`
2. SD 卡格式化為 **FAT32**，根目錄放置 `0001.mp3`（拍手音效）

### 主程式
1. 雙擊 `clapping_device/clapping_device.ino` 開啟 Arduino IDE
2. 上傳至 Arduino Uno
3. 開啟 Serial Monitor (9600 baud)，觀察狀態輸出

### 測試
1. 雙擊 `test_device/test_device.ino` 開啟 Arduino IDE
2. 上傳後開啟 Serial Monitor，輸入數字選擇測試項目：

| 指令 | 測試項目 |
|---|---|
| `1` | 超音波距離讀取（5 秒連續）|
| `2` | 馬達開合動作（x3 循環）|
| `3` | DFPlayer 播放測試 |
| `4` | 微動開關狀態監控（5 秒）|
| `5` | 拍手 + Fallback 觸發測試 |
| `6` | 完整迴圈模擬（15 秒）|
| `7` | Mapping 參數對照表 |

## 核心設計

### 系統架構（SA 三層）
- **HAL 層** (`hardware.h/cpp`)：超音波感測、馬達 PWM、DFPlayer UART、微動開關防彈跳
- **應用邏輯層** (`clap_engine_fallback`, `clapping_device.ino`)：狀態機、Mapping 引擎、Fallback 管理
- **展示層**：馬達實際動作 + 喇叭聲響

### 狀態機（SD §1）
```
IDLE → SENSING → CLAPPING → SENSING → ... → IDLE (5s 無人)
```

### 雙軌觸發 + Fallback（SD §3）
- **主路徑**：馬達閉合 → 微動開關 Falling Edge → 播放音效
- **Fallback**：200ms 內未收到開關訊號 → 軟體強制播放音效

## 相關文件
- [PRD 產品需求文件](doc/Arduino%20拍手互動裝置產品需求文件%20(PRD).md)
- [SA 系統架構文件](doc/Arduino%20拍手互動裝置系統架構文件%20(SA).md)
- [SD 系統設計文件](doc/Arduino%20拍手互動裝置系統設計文件%20(SD).md)
