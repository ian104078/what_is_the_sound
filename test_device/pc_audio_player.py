import serial
import time
import sys
import ctypes
import os

# 設定您的 Arduino 連接埠 (COM Port) 和相同的 Baud Rate
# ⚠️ 請將 COM3 改成您 Arduino IDE 右下角顯示的 COM Port
COM_PORT = 'COM3' 
BAUD_RATE = 9600

# 自動取得程式碼所在的資料夾路徑，與 mp3 組合成絕對路徑
SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
AUDIO_FILE = os.path.join(SCRIPT_DIR, "0001.mp3")

# Windows 內建直接播放 MP3 的寫法 (最暴力穩定的做法：呼叫預設播放器)
def play_audio():
    # 使用系統預設應用程式在新視窗播放，避免所有解碼器問題
    os.system(f'start "" "{AUDIO_FILE}"')

# 測試音效檔是否存在
try:
    with open(AUDIO_FILE, 'r'): pass
    print(f"找到音效檔: {AUDIO_FILE}，已就緒！")
except FileNotFoundError:
    print(f"⚠️ 找不到音效檔 {AUDIO_FILE}！請確認檔案就在這個 Python 檔旁邊。")
    sys.exit(1)

# 開啟 Serial 連線
try:
    ser = serial.Serial(COM_PORT, BAUD_RATE, timeout=0.1)
    print(f"成功連線至 {COM_PORT}！正在監聽 Arduino 拍手指令...")
except serial.SerialException as e:
    print(f"無法開啟 {COM_PORT}，請確認：")
    print("1. Arduino 是否插好")
    print("2. Arduino IDE 的 Serial Monitor 是否已經關閉 (同一個 Port 不能被兩個軟體同時佔用)")
    print(f"錯誤詳情: {e}")
    sys.exit(1)

# 無窮迴圈監聽
try:
    while True:
        # 讀取 Arduino 傳來的文字
        if ser.in_waiting > 0:
            line = ser.readline().decode('utf-8', errors='ignore').strip()
            if line:
                print(f"Arduino 說: {line}")
                
            # 嚴格設定：只有當您成功按壓微動開關 (出現 PRIMARY 路徑) 時，才觸發電腦音效！
            if "[CLAP] path=PRIMARY" in line:
                print(">>> 觸發電腦拍手音效！(實體開關成功) <<<")
                play_audio()
                
        time.sleep(0.01) # 避免佔用過多 CPU

except KeyboardInterrupt:
    print("\n停止監聽。")
finally:
    ser.close()
    ctypes.windll.winmm.mciSendStringW("close MySound", None, 0, None)
