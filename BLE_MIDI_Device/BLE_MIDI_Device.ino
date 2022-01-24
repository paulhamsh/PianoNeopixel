// Dependencies:
//   NimBLE

#include <M5StickC.h>
#include "RingBuffer.h"

////////////////////////////////////////////
// BLE
////////////////////////////////////////////

#include "NimBLEDevice.h"

#define MIDI_SERVICE     "03b80e5a-ede8-4b33-a751-6ce34ec4c700"
#define MIDI_CHAR        "7772e5db-3868-4112-a1a9-f2669d106bf3"

NimBLEServer *pServer;
NimBLEService *pServiceMIDI;
NimBLECharacteristic *pCharacteristicMIDI;
NimBLEAdvertising *pAdvertising;

RingBuffer ble_midi_in;

class MyMIDIServerCallback : public NimBLEServerCallbacks
{
  void onConnect(NimBLEServer *pserver)
  {
    Serial.println("callback: BLE MIDI client connected");
  }

  void onDisconnect(NimBLEServer *pserver)
  {
    Serial.println("callback: BLE MIDI client disconnected");
  }
};

class MIDICharacteristicCallbacks: public NimBLECharacteristicCallbacks {
  void onWrite(BLECharacteristic* pCharacteristic) {
        int j, l;
        const char *p;
        byte b;
        l = pCharacteristic->getValue().length();
        p = pCharacteristic->getValue().c_str();
        for (j=0; j < l; j++) {
          b = p[j];
          ble_midi_in.add(b);
        }
        ble_midi_in.commit();
  };
};

static MIDICharacteristicCallbacks chrCallbacksMIDI;

void setup_ble() {
  NimBLEDevice::init("BLE MIDI Device");

  pServer = NimBLEDevice::createServer();
  pServer->setCallbacks(new MyMIDIServerCallback());  
  
  pServiceMIDI = pServer->createService(MIDI_SERVICE);
  pCharacteristicMIDI = pServiceMIDI->createCharacteristic(MIDI_CHAR, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::WRITE_NR | NIMBLE_PROPERTY::NOTIFY);
  pCharacteristicMIDI->setCallbacks(&chrCallbacksMIDI);

  pServiceMIDI->start();

  pCharacteristicMIDI->setValue("1234");

  pAdvertising = NimBLEDevice::getAdvertising(); // create advertising instance
  pAdvertising->addServiceUUID(MIDI_SERVICE); // tell advertising the UUID of our service
  pAdvertising->setScanResponse(true);  
//  pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
//  pAdvertising->setMinPreferred(0x12);
  pAdvertising->start();  
}

////////////////////////////////////////////
// MIDI
////////////////////////////////////////////

int ble_midi_state;

void setup_midi() {
  ble_midi_state = 0;
}

bool update_midi(byte *mid) {
  bool got_midi;
  byte b;
  
  got_midi = false;

  // process MIDI - both USB and BLE and  Serial DIN

  if (!ble_midi_in.is_empty()) {
    ble_midi_in.get(&b);
    if (b & 0x80) {
      mid[0] = b;
      ble_midi_state = 1;
    }
    else {
      if (ble_midi_state == 1) {
        mid[1] = b;
        ble_midi_state++;
      } 
      else {
        if (ble_midi_state == 2) {
          mid[2] = b;
          ble_midi_state = 0;
          got_midi = true;
        }
      }
    }
  }

  if (got_midi) {
    Serial.print("MIDI ");
    Serial.print(mid[0], HEX);
    Serial.print(" ");
    Serial.print(mid[1]);
    Serial.print(" ");   
    Serial.println(mid[2]);
  }
  return got_midi;
}

////////////////////////////////////////////
// Main program
////////////////////////////////////////////

void setup() {
  M5.begin();
  M5.Lcd.setRotation(-1);
  M5.Lcd.fillScreen(TFT_BLACK);
  M5.Lcd.setTextColor(TFT_WHITE);
  M5.Lcd.setTextSize(2);
  M5.Lcd.setCursor(0, 0);
  M5.Lcd.println("BLE MIDI");
  M5.Lcd.print("Nothing");
  
  setup_midi();
  setup_ble();
}

void loop() {
  byte mi[3];
  int midi_chan, midi_cmd;
  char str[20];
  
  M5.update();

  if (update_midi(mi)) {
    midi_chan = (mi[0] & 0x0f) + 1;
    midi_cmd = mi[0] & 0xf0;

    sprintf(str, "%x %x %x %x ", midi_chan, midi_cmd, mi[1], mi[2]);
    M5.Lcd.setRotation(-1);
    M5.Lcd.fillScreen(TFT_BLACK);
    M5.Lcd.setTextColor(TFT_WHITE);
    M5.Lcd.setTextSize(2);
    M5.Lcd.setCursor(0, 0);
    M5.Lcd.println("BLE MIDI");
    M5.Lcd.print(str);
  }
}
