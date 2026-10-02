#include <NimBLEDevice.h>
#include <Preferences.h>
#include <cctype>

#define BUTTON_PIN 9
#define LED_BUILTIN_PIN 8

// ============================================================================
// --- DATA STRUCTURES & PROTOCOL COMMANDS ---
// ============================================================================

struct InstaCommand {
  const char* name;
  const uint8_t* data;
  size_t length;
};

struct __attribute__((packed)) InstaHeader {
  uint32_t length;
  uint8_t  reserved1;
  uint8_t  reserved2;
  uint8_t  reserved3;
  uint8_t  cmdCode;
};

namespace Commands {
  // --- PAIRING & KEEP-ALIVE ---
  const uint8_t RAW_PAIRING[]            = {0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
  const uint8_t RAW_HEARTBEAT[]          = {0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

  // --- STANDARD MODE & MODE SWITCH COMMANDS ---
  const uint8_t RAW_APP_MODE[]           = {0x10, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x01, 0x00, 0x02, 0x01, 0x00, 0x00, 0x80, 0x00, 0x00};
  const uint8_t RAW_SWITCH_CLOSE_APP[]   = {0x10, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x02, 0x00, 0x02, 0x01, 0x00, 0x00, 0x80, 0x00, 0x00};
  const uint8_t RAW_SET_MODE_PHOTO[]     = {0x10, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x08, 0x00, 0x02, 0x01, 0x00, 0x00, 0x80, 0x00, 0x00}; // Switch to Photo mode
  const uint8_t RAW_SET_MODE_VIDEO[]     = {0x10, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x0A, 0x00, 0x02, 0x01, 0x00, 0x00, 0x80, 0x00, 0x00}; // Switch to Video mode

  // --- PHOTO CAPTURE ---
  const uint8_t RAW_TAKE_PHOTO[]         = {0x10, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x03, 0x00, 0x02, 0x0C, 0x00, 0x00, 0x80, 0x00, 0x00};
  const uint8_t RAW_TAKE_HDR_PHOTO[]     = {0x10, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x13, 0x00, 0x02, 0x01, 0x00, 0x00, 0x80, 0x00, 0x00}; // HDR / PureShot Photo
  const uint8_t RAW_TAKE_BURST_PHOTO[]   = {0x10, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x15, 0x00, 0x02, 0x01, 0x00, 0x00, 0x80, 0x00, 0x00}; // Burst Photo

  // --- STANDARD VIDEO & RECORDING ---
  const uint8_t RAW_START_VIDEO[]        = {0x10, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x04, 0x00, 0x02, 0x01, 0x00, 0x00, 0x80, 0x00, 0x00};
  const uint8_t RAW_STOP_VIDEO[]         = {0x10, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x05, 0x00, 0x02, 0x01, 0x00, 0x00, 0x80, 0x00, 0x00};
  const uint8_t RAW_SHUTTER_BUTTON[]     = {0x10, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x0E, 0x00, 0x02, 0x01, 0x00, 0x00, 0x80, 0x00, 0x00}; // Toggle Shutter/Rec (Red Button)
  const uint8_t RAW_MARK_HILIGHT[]       = {0x10, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x2B, 0x00, 0x02, 0x01, 0x00, 0x00, 0x80, 0x00, 0x00}; // Add HiLight tag during recording

  // --- SPECIAL VIDEO MODES & TIMELAPSE ---
  const uint8_t RAW_START_TIMELAPSE[]    = {0x10, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x1B, 0x00, 0x02, 0x01, 0x00, 0x00, 0x80, 0x00, 0x00}; // Video Timelapse
  const uint8_t RAW_STOP_TIMELAPSE[]     = {0x10, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x1C, 0x00, 0x02, 0x01, 0x00, 0x00, 0x80, 0x00, 0x00};

  const uint8_t RAW_START_INTERVAL_PHOTO[] = {0x10, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x23, 0x00, 0x02, 0x01, 0x00, 0x00, 0x80, 0x00, 0x00}; // Interval Photo
  const uint8_t RAW_STOP_INTERVAL_PHOTO[]  = {0x10, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x24, 0x00, 0x02, 0x01, 0x00, 0x00, 0x80, 0x00, 0x00};

  const uint8_t RAW_START_BULLET_TIME[]  = {0x10, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x29, 0x00, 0x02, 0x01, 0x00, 0x00, 0x80, 0x00, 0x00};
  const uint8_t RAW_STOP_BULLET_TIME[]   = {0x10, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x30, 0x00, 0x02, 0x01, 0x00, 0x00, 0x80, 0x00, 0x00};

  const uint8_t RAW_START_SLOW_MOTION[]  = {0x10, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x31, 0x00, 0x02, 0x01, 0x00, 0x00, 0x80, 0x00, 0x00}; // Slow Motion
  const uint8_t RAW_STOP_SLOW_MOTION[]   = {0x10, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x32, 0x00, 0x02, 0x01, 0x00, 0x00, 0x80, 0x00, 0x00};

  const uint8_t RAW_START_HDR_VIDEO[]    = {0x10, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x33, 0x00, 0x02, 0x01, 0x00, 0x00, 0x80, 0x00, 0x00};
  const uint8_t RAW_STOP_HDR_VIDEO[]     = {0x10, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x34, 0x00, 0x02, 0x01, 0x00, 0x00, 0x80, 0x00, 0x00};

  const uint8_t RAW_START_TIMESHIFT[]    = {0x10, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x3D, 0x00, 0x02, 0x01, 0x00, 0x00, 0x80, 0x00, 0x00};
  const uint8_t RAW_STOP_TIMESHIFT[]     = {0x10, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x3E, 0x00, 0x02, 0x01, 0x00, 0x00, 0x80, 0x00, 0x00};

  const uint8_t RAW_START_LOOP_REC[]     = {0x10, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x45, 0x00, 0x02, 0x01, 0x00, 0x00, 0x80, 0x00, 0x00};
  const uint8_t RAW_STOP_LOOP_REC[]      = {0x10, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x46, 0x00, 0x02, 0x01, 0x00, 0x00, 0x80, 0x00, 0x00};

  const uint8_t RAW_START_STARLAPSE[]    = {0x10, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x49, 0x00, 0x02, 0x01, 0x00, 0x00, 0x80, 0x00, 0x00}; // Starlapse / Nightlapse
  const uint8_t RAW_STOP_STARLAPSE[]     = {0x10, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x4A, 0x00, 0x02, 0x01, 0x00, 0x00, 0x80, 0x00, 0x00};

  // --- STATUS, BATTERY & HARDWARE QUERIES ---
  const uint8_t RAW_QUERY_STATUS[]       = {0x10, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x1F, 0x00, 0x02, 0x01, 0x00, 0x00, 0x80, 0x00, 0x00}; // Status Ping
  const uint8_t RAW_QUERY_BATTERY[]      = {0x10, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x21, 0x00, 0x02, 0x01, 0x00, 0x00, 0x80, 0x00, 0x00}; // Query battery level
  const uint8_t RAW_QUERY_WIFI_INFO[]    = {0x10, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x22, 0x00, 0x02, 0x01, 0x00, 0x00, 0x80, 0x00, 0x00}; // Query Wi-Fi SSID/Pass
  const uint8_t RAW_QUERY_STORAGE[]      = {0x10, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x53, 0x00, 0x02, 0x01, 0x00, 0x00, 0x80, 0x00, 0x00}; // Query remaining SD space and file info

  // --- SYSTEM & HARDWARE CONTROL ---
  const uint8_t RAW_TOGGLE_WIFI[]        = {0x10, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x25, 0x00, 0x02, 0x01, 0x00, 0x00, 0x80, 0x00, 0x00}; // Toggle camera Wi-Fi
  const uint8_t RAW_POWER_OFF[]          = {0x10, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x0D, 0x00, 0x02, 0x01, 0x00, 0x00, 0x80, 0x00, 0x00}; // Complete camera power off
  const uint8_t RAW_WIPE_SD[]            = {0x10, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x18, 0x00, 0x02, 0x01, 0x00, 0x00, 0x80, 0x00, 0x00}; // Format SD card
  const uint8_t RAW_REBOOT[]             = {0x10, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x20, 0x00, 0x02, 0x01, 0x00, 0x00, 0x80, 0x00, 0x00}; // System reboot
  const uint8_t RAW_FACTORY_RESET[]      = {0x10, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x39, 0x00, 0x02, 0x01, 0x00, 0x00, 0x80, 0x00, 0x00}; // Factory reset

  const InstaCommand PAIRING     = {"PAIRING_HANDSHAKE", RAW_PAIRING, sizeof(RAW_PAIRING)};
  const InstaCommand HEARTBEAT   = {"HEARTBEAT", RAW_HEARTBEAT, sizeof(RAW_HEARTBEAT)};
  const InstaCommand START_VIDEO = {"START_VIDEO", RAW_START_VIDEO, sizeof(RAW_START_VIDEO)};
  const InstaCommand STOP_VIDEO  = {"STOP_VIDEO", RAW_STOP_VIDEO, sizeof(RAW_STOP_VIDEO)};
}

// ============================================================================
// --- BLE UUIDs & STATES ---
// ============================================================================
static NimBLEUUID serviceUUID("0000be80-0000-1000-8000-00805f9b34fb");
static NimBLEUUID charWriteUUID("0000be81-0000-1000-8000-00805f9b34fb");
static NimBLEUUID charNotifyUUID("0000be82-0000-1000-8000-00805f9b34fb");

enum ConnectionState {
  STATE_DISCONNECTED,
  STATE_SCANNING,
  STATE_CONNECTING,
  STATE_CONNECTED
};

ConnectionState currentState = STATE_DISCONNECTED;

// Global Objects
NimBLERemoteCharacteristic* pWriteCharacteristic = nullptr;
NimBLERemoteCharacteristic* pNotifyCharacteristic = nullptr;
NimBLEClient* pClient = nullptr;
const NimBLEAdvertisedDevice* targetDevice = nullptr;
Preferences preferences;

// State Variables
bool isRecording = false;
String savedMacAddress = "";

bool lastButtonState = HIGH;
unsigned long lastBlinkTime = 0;
unsigned long lastHeartbeatTime = 0;
unsigned long connectStartTime = 0;
const unsigned long CONNECT_TIMEOUT = 5000; // 5-second GATT handshake timeout

volatile bool pendingAutoStop = false;
volatile bool pendingAutoStart = false;

// ============================================================================
// --- FUNCTIONAL HELPERS ---
// ============================================================================
bool bufferContains(const uint8_t* data, size_t len, const char* target) {
  size_t targetLen = strlen(target);
  if (len < targetLen) return false;
  for (size_t i = 0; i <= len - targetLen; i++) {
    if (memcmp(data + i, target, targetLen) == 0) return true;
  }
  return false;
}

bool sendBleCommand(const InstaCommand& cmd) {
  if (pWriteCharacteristic != nullptr && pWriteCharacteristic->canWrite()) {
    bool result = pWriteCharacteristic->writeValue(cmd.data, cmd.length, true);
    if (result) {
      Serial.printf(">> Sent: [%s]\n", cmd.name);
    } else {
      Serial.printf("!! Send error: [%s]\n", cmd.name);
    }
    return result;
  }
  return false;
}

void handleLedBlink(unsigned long interval) {
  if (millis() - lastBlinkTime >= interval) {
    lastBlinkTime = millis();
    digitalWrite(LED_BUILTIN_PIN, !digitalRead(LED_BUILTIN_PIN));
  }
}

// ============================================================================
// --- NOTIFICATION PARSING ---
// ============================================================================
void parseNotification(uint8_t* pData, size_t length) {
  if (length < 5) return;
  if (length == 7 && pData[4] == 0x05) return; // Heartbeat filter

  Serial.print("[NOTIFY BE82] RAW: ");
  for (size_t i = 0; i < length; i++) {
    Serial.printf("%02X ", pData[i]);
  }
  Serial.println();

  if (length >= sizeof(InstaHeader)) {
    const InstaHeader* header = reinterpret_cast<const InstaHeader*>(pData);

    if (pData[4] == 0x04) {
      switch (header->cmdCode) {
        case 0x03:
          if (length >= 18) {
            Serial.printf(" -> CAMERA STATUS: Battery %d%%\n", pData[length - 1]);
          }
          break;

        case 0x06:
          // MicroSD file save/close notification (Buffer Flush)
          Serial.println(" -> EVENT: Saving/Finalizing file on MicroSD in progress...");
          break;

        case 0x0D:
          Serial.println(" -> NOTIFICATION: Bluetooth searching...");
          break;

        case 0x10:
          if (length >= 18) {
            uint8_t statusByte = pData[17];
            if (statusByte == 0x01) {
              isRecording = true;
              Serial.println(" -> CAMERA STATUS: Recording ACTIVE");
            } else if (statusByte == 0x00) {
              isRecording = false;
              digitalWrite(LED_BUILTIN_PIN, LOW);
              Serial.println(" -> CAMERA STATUS: Recording STOPPED");
            }
          }
          break;

        case 0x17:
          // System telemetry (battery, SD, general status)
          // Uncomment below line for verbose logging:
          // Serial.println(" -> NOTIFICATION: System telemetry updated.");
          break;

        case 0xC8:
          Serial.println(" -> NOTIFICATION: Response OK (0xC8).");
          break;

        case 0xF4:
          if (bufferContains(pData, length, "busy")) {
            Serial.println(" -> CAMERA BUSY: Queuing STOP_VIDEO...");
            isRecording = true;
            pendingAutoStop = true;
          } else if (bufferContains(pData, length, "msg execute err.")) {
            Serial.println(" -> CAMERA IDLE: Queuing START_VIDEO...");
            isRecording = false;
            pendingAutoStart = true;
          }
          break;

        case 0x90:
          Serial.println(" -> NOTIFICATION: Command execution error.");
          break;
      }
    }
  }
}

void notifyCallback(NimBLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify) {
  parseNotification(pData, length);
}

// ============================================================================
// --- BLE MANAGEMENT & SCANNING ---
// ============================================================================
class MyClientCallback : public NimBLEClientCallbacks {
  void onConnect(NimBLEClient* pclient) override {
    Serial.println(" -> BLE Connection established!");
  }

  void onDisconnect(NimBLEClient* pclient, int reason) override {
    currentState = STATE_DISCONNECTED;
    isRecording = false;
    Serial.printf("Disconnected from Insta360 (Reason: %d). Returning to scan...\n", reason);
  }
};

bool setupBLEService(NimBLEClient* pClient) {
  NimBLERemoteService* pRemoteService = pClient->getService(serviceUUID);
  if (!pRemoteService) return false;

  pWriteCharacteristic = pRemoteService->getCharacteristic(charWriteUUID);
  if (!pWriteCharacteristic) return false;

  pNotifyCharacteristic = pRemoteService->getCharacteristic(charNotifyUUID);
  if (pNotifyCharacteristic && pNotifyCharacteristic->canNotify()) {
    pNotifyCharacteristic->subscribe(true, notifyCallback);
    Serial.println("Notification subscription active.");
  } else {
    return false;
  }

  return sendBleCommand(Commands::PAIRING);
}

class MyScanCallbacks: public NimBLEScanCallbacks {
  void onResult(const NimBLEAdvertisedDevice* advertisedDevice) override {
    bool isInsta = advertisedDevice->isAdvertisingService(serviceUUID) ||
                   (advertisedDevice->haveName() && advertisedDevice->getName().find("Insta360") != std::string::npos);

    // If a saved MAC exists, verify match
    if (savedMacAddress != "") {
      isInsta = (advertisedDevice->getAddress().toString() == savedMacAddress.c_str());
    }

    if (isInsta) {
      Serial.printf("Insta360 Found! MAC: %s (Type %d)\n",
                    advertisedDevice->getAddress().toString().c_str(),
                    advertisedDevice->getAddress().getType());

      NimBLEDevice::getScan()->stop();

      // Save MAC to Flash if first-time pairing
      if (savedMacAddress == "") {
        savedMacAddress = advertisedDevice->getAddress().toString().c_str();
        preferences.begin("insta360", false);
        preferences.putString("mac", savedMacAddress);
        preferences.end();
        Serial.println("MAC saved to Flash.");
      }

      targetDevice = advertisedDevice;
      currentState = STATE_CONNECTING;
      connectStartTime = millis();
    }
  }
};

void startScan() {
  Serial.println("Waiting for Insta360 camera (BLE scan active)...");
  currentState = STATE_SCANNING;

  NimBLEScan* pScan = NimBLEDevice::getScan();
  pScan->setScanCallbacks(new MyScanCallbacks(), false);
  pScan->setActiveScan(true);
  pScan->setInterval(100);
  pScan->setWindow(99);
  pScan->start(0, false, false); // Continuous non-blocking scan
}

// ============================================================================
// --- SETUP & LOOP ---
// ============================================================================
void setup() {
  Serial.begin(115200);

  unsigned long start = millis();
  while (!Serial && (millis() - start < 3000));

  Serial.println("\n--- ESP32-C3 INSTA360 REMOTE (SCAN-TO-CONNECT) ---");

  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(LED_BUILTIN_PIN, OUTPUT);
  digitalWrite(LED_BUILTIN_PIN, HIGH); // LED Off

  NimBLEDevice::init("Insta360 Remote");

  preferences.begin("insta360", true);
  savedMacAddress = preferences.getString("mac", "");
  preferences.end();

  if (savedMacAddress != "") {
    Serial.printf("MAC filter loaded from Flash: %s\n", savedMacAddress.c_str());
  }

  pClient = NimBLEDevice::createClient();
  pClient->setClientCallbacks(new MyClientCallback());

  startScan();
}

void loop() {
  // 1. STATE MACHINE
  switch (currentState) {
    case STATE_DISCONNECTED:
      handleLedBlink(100);
      startScan();
      break;

    case STATE_SCANNING:
      handleLedBlink(100); // LED blinks fast while camera is offline/scanning
      break;

    case STATE_CONNECTING:
      handleLedBlink(100);

      // Attempt direct connection using fresh scan data
      if (targetDevice != nullptr) {
        Serial.println("Initiating connection to discovered device...");

        if (pClient->connect(targetDevice)) {
          Serial.println("Physical connection OK! Configuring GATT...");
          if (setupBLEService(pClient)) {
            currentState = STATE_CONNECTED;
            digitalWrite(LED_BUILTIN_PIN, LOW); // Solid LED: Ready!
            Serial.println(">> REMOTE CONNECTED AND READY <<");
          } else {
            Serial.println("GATT Configuration failed.");
            pClient->disconnect();
            currentState = STATE_DISCONNECTED;
          }
        } else {
          Serial.println("Error during pClient->connect(). Returning to scan.");
          currentState = STATE_DISCONNECTED;
        }
        targetDevice = nullptr;
      }
      break;

    case STATE_CONNECTED:
      break;
  }

  if (currentState != STATE_CONNECTED) return;

  // --- DEFERRED AUTO-CORRECTION EXECUTION ---
  if (pendingAutoStop) {
    pendingAutoStop = false;
    delay(50);
    sendBleCommand(Commands::STOP_VIDEO);
  }

  if (pendingAutoStart) {
    pendingAutoStart = false;
    delay(50);
    sendBleCommand(Commands::START_VIDEO);
  }

  // 2. Periodic Heartbeat (1200ms)
  if (millis() - lastHeartbeatTime >= 1200) {
    lastHeartbeatTime = millis();
    if (pWriteCharacteristic != nullptr) {
      pWriteCharacteristic->writeValue(Commands::HEARTBEAT.data, Commands::HEARTBEAT.length, false);
    }
  }

  // 3. Recording LED (Slow blink while recording)
  if (isRecording) {
    handleLedBlink(1000);
  }

  // 4. Button Press Handling
  bool currentButtonState = digitalRead(BUTTON_PIN);

  if (lastButtonState == HIGH && currentButtonState == LOW) {
    delay(50); // Debounce
    if (digitalRead(BUTTON_PIN) == LOW) {
      bool success = false;

      if (!isRecording) {
        success = sendBleCommand(Commands::START_VIDEO);
      } else {
        success = sendBleCommand(Commands::STOP_VIDEO);
      }

      if (!success) {
        Serial.println("Command transmission error. Forcing reconnection...");
        currentState = STATE_DISCONNECTED;
        isRecording = false;
      }
    }
  }

  lastButtonState = currentButtonState;
}