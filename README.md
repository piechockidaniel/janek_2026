# Robo Brain — Arduino IDE sketch

Cały firmware mózgu robota w jednym płaskim folderze, gotowe do importu w Arduino IDE.

## Pierwszy raz — konfiguracja Arduino IDE

### 1. Zainstaluj wsparcie dla UNO R4 WiFi

W Arduino IDE:

1. **File → Preferences**. W "Additional boards manager URLs" wklej (jeśli nie ma):
   ```
   https://downloads.arduino.cc/packages/package_renesas_index.json
   ```
2. **Tools → Board → Boards Manager**, wyszukaj `uno r4`, zainstaluj **"Arduino UNO R4 Boards"** (najnowszą stabilną).

### 2. Zainstaluj biblioteki

**Tools → Manage Libraries...**, wyszukaj i zainstaluj:

- **ArduinoJson** (by Benoit Blanchon, wersja 7.x)
- **Adafruit GFX Library** (by Adafruit)
- **Adafruit ST7735 and ST7789 Library** (by Adafruit)

Pozostałe (`WiFiS3`, `Arduino_LED_Matrix`, `SPI`) są częścią pakietu UNO R4 — instalują się razem z board package.

## Otwieranie projektu

1. Skopiuj cały folder `brain/` do swojego sketchbook'a Arduino. Domyślnie:
   ```
   C:\Users\<TwojaNazwa>\Documents\Arduino\brain\
   ```
   Ważne: folder MUSI się nazywać `brain` żeby pasował do pliku `brain.ino`.

2. Otwórz `brain.ino` (dwuklik albo File → Open w Arduino IDE).

3. Arduino IDE pokaże wszystkie pliki jako zakładki na górze — `brain.ino`, `pins.h`, `RobotState.cpp`, itd. Tak ma być.

## Konfiguracja przed kompilacją

### Edytuj `secrets.h`

Otwórz zakładkę `secrets.h` w Arduino IDE. Zmień te dwie linie na swoje WiFi:

```cpp
#define WIFI_SSID     "TwojaSiec"
#define WIFI_PASSWORD "TwojeHaslo"
```

Pozostałe definicje (`ROBOT_HOSTNAME`, `CAMERA_STREAM_URL`) zostaw — nawet jeśli nie używasz kamery, kompilacja ich potrzebuje.

### Ustawienia board'a

**Tools → Board → Arduino UNO R4 Boards → Arduino UNO R4 WiFi**

**Tools → Port** — wybierz COM port robota (sprawdź w Device Manager Windows).

## Kompilacja i upload

1. **Sketch → Verify/Compile** (Ctrl+R) — sprawdza czy się kompiluje. Powinno trwać 30-60 sekund.
2. **Sketch → Upload** (Ctrl+U) — kompiluje i wgrywa do Arduino.

Po uploadzie Arduino restartuje się automatycznie. Sprawdź na TFT:
- Po ~5 sekundach: `WIFI ON` w prawym górnym rogu
- Mode `Idle` z buźką sleepy

## Otwarcie UI

W przeglądarce na telefonie (na tej samej sieci WiFi co robot):

```
http://[IP-robota]/
```

Adres IP znajdziesz w monitorze szeregowym albo w widoku podłączonych urządzeń routera.

## Monitor szeregowy

**Tools → Serial Monitor** (Ctrl+Shift+M), ustaw baud na `115200`. Po RESET na Arduino zobaczysz:

```
=== Robo brain booting ===
WiFi: connecting to ...
WiFi: up, IP=192.168.x.y
Robo brain ready
```

## Pliki w projekcie — co robi co

| Plik | Funkcja |
|---|---|
| `brain.ino` | Główny program, setup() i loop() |
| `pins.h` | Mapowanie pinów Arduino na funkcje (silniki, czujniki, TFT, buzzer) |
| `web_ui.h` | HTML/JS interfejs sterowania jako PROGMEM string |
| `secrets.h` | WiFi credentials — edytuj przed kompilacją |
| `Geometry.h` | Typy: Velocity, MotorOutput, Distances |
| `MotorMixer.*` | Skid-steer math — liniowa+kątowa → lewy+prawy PWM |
| `ObstacleAvoidance.*` | Filtr bezpieczeństwa + polityka eksploracji |
| `RobotState.*` | Maszyna stanów (Idle/ManualDrive/Autonomous/Avoiding/Alert/Failsafe) |
| `MoodSelector.*` | Mapowanie Mode → Mood (buźka) |
| `MotorDriver.*` | Sterownik H-bridge TA6586 |
| `UltrasonicArray.*` | Cztery HC-SR04 ze wspólnym TRIG |
| `BuzzerDriver.*` | Sygnały dźwiękowe SFM-27 |
| `MoodMatrix.*` | Buźki na wbudowanej matrycy LED 12x8 |
| `TftTelemetry.*` | Wyświetlacz 1.8" ST7735 |

## Typowe problemy

**"esp32 by Espressif" zamiast "Arduino UNO R4 Boards"** — zainstalowałeś pakiet do ESP32, nie do UNO R4. To dwie zupełnie różne paczki. Wróć do Boards Manager.

**Błąd `WiFiS3.h: No such file or directory`** — pakiet UNO R4 Boards nie jest zainstalowany albo wybrano złą płytkę.

**Port COM nie pojawia się** — sprawdź czy Arduino jest podłączone USB-C, czy nie ma uruchomionego monitora szeregowego z innej aplikacji (PlatformIO, drugi Arduino IDE).

**Po uploadzie nic się nie dzieje** — sprawdź czy Tools → Board jest faktycznie `Arduino UNO R4 WiFi` (nie Minima, nie R3).

## Watchdog wyłączony

Wersja w tym folderze ma **watchdog wyłączony** (sensor i command timeout ustawione na 1 godzinę). Mode Failsafe nie tripuje przy starcie ani podczas pracy. To celowe — uproszczenie dla szybkiego ruszenia z miejsca.

Żeby przywrócić zabezpieczenia: w `brain.ino` znajdź linię z `RobotState        state(...)` i zmień na `RobotState        state;` (domyślne timeouty: 500ms sensor, 1000ms command).
