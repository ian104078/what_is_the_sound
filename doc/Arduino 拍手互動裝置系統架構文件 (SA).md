# 軟體工程文件：系統架構文件 (SA)
**專案名稱：** Arduino 拍手互動裝置 (微動開關觸發版)
**文件狀態：** 正式版本 v1.0

## 1. 系統總覽
本文件依據 PRD 產品需求文件發展而來，旨在定義系統之高階架構、軟硬體整合邊界，以及各功能模組之權責分配。本系統透過超音波感測器連續計算空間距離，並將其轉換為互動回饋參數（馬達拍手頻率、輸出音量）。為確保系統的穩健性，特別導入了「實體微動開關觸發為主，軟體定時觸發為輔 (Fallback)」的雙軌觸發機制。

## 2. 系統架構圖 (System Block Diagram)

```mermaid
graph TD
    subgraph 電源管理系統
        P[5V/2A 行動電源] --> |提供 5V 高電流| M1(MG90S 伺服馬達 x2)
        P --> |提供 5V| A[外部自備電源線材分流]
        A --> |供電+雜訊隔離保護| MCU
        A --> |5V獨立供電| DF[DFPlayer MP3模組]
    end

    subgraph 核心微控制器 (Arduino Uno/Nano)
        MCU[Arduino MCU]
        MCU --> |控制 PWM| M1
        MCU --> |UART TX/RX指令| DF
        SW[微動開關 x2] --> |Interrupt/Polling (Falling Edge)| MCU
        US[HC-SR04 超音波感測器] <--> |Trig/Echo| MCU
    end
    
    subgraph 執行與回饋
        DF --> |聲音輸出| SPK[3W喇叭]
        M1 --> |機械閉合| Hands[機構手掌]
        Hands -.-> |物理壓按| SW
    end
```

> [!WARNING]
> 硬體電源必須實施分流隔離（Isolation），馬達與 DFPlayer 的 VCC **不得**直接連接至 Arduino 的 5V 腳位，以避免馬達瞬間抽載造成 Arduino 電壓不足重啟 (Brown-out Reset)。所有組件的 GND 必須共地 (Common Ground)。

## 3. 軟體架構分層 (Software Architecture Layers)

系統軟體可分為三層：

1. **硬體抽象層 (Hardware Abstraction Layer - HAL)：**
   - **超音波感測層**：負責發送 Trig 脈衝並讀取 Echo 脈衝寬度，過濾異常雜訊值。
   - **音效驅動層**：透過 SoftwareSerial / HardwareSerial 控制 DFPlayer Mini，處理指令與 Ack。
   - **馬達驅動層**：負責 PWM 訊號生成與角度控制（`Servo.h` 庫封裝）。
   - **感測開關層**：負責輪詢 GPIO 腳位或觸發硬體中斷 (Interrupt)，並套用軟體防彈跳 (Debounce)。
2. **應用邏輯層 (Application Logic Layer)：**
   - **參數映射引擎 (Mapping Engine)**：負責將原始感測距離數值 (cm) 轉換為應用所需的延遲時間 (ms) 以及播放音量 (0~30)。
   - **狀態機控制器 (State Machine Controller)**：管控從待機、感測、作動到系統休眠的狀態轉移。
   - **錯誤與 Fallback 管理員 (Error & Fallback Manager)**：監測硬體狀態並負責故障轉移路徑（當實體開關未依預期觸發時的處置）。
3. **展示層 (Presentation Layer)：**
   - 即馬達實際的動作軌跡頻率與喇叭實際播放的聲響。

## 4. 雙軌觸發與 Fallback 架構

為了達成 PRD 要求之「動態互動同步」以及提供「Fallback」保證，系統的發音機制採行雙軌檢測：

* **主路徑 (硬體訊號驅動)：**
  當馬達發動機構閉合指令後，軟體開始監聽微動開關的數位訊號。一旦接收到從 HIGH 變為 LOW (Falling Edge) 的訊號，代表手掌確實物理接觸，立刻發送播放音效指令。
* **備援路徑 (Fallback - 軟體同步定時驅動)：**
  若機構變形、微動開關故障或排線損壞，導致開關訊號未如預期送達 MCU。在馬達指令發出並經過一特定預期時間視窗（如：200ms）後，尚未收到硬體下降緣觸發時，系統將由計時器中斷（或輪詢超時）**強制觸發**音效播放，以此作為安全網確保仍有互動回饋。

> [!TIP]
> 此 Fallback 設計能容許硬體機構有一定程度的公差變化或衰老耗損，在不影響使用者核心體驗（聲音回饋）的大前提下，達成高容錯性運行。
