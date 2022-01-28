// Dependencies:
//   NimBLE 1.3.6
//   Adafruit Neopixel

#include <M5StickC.h>
#include "RingBuffer.h"

#include <Adafruit_NeoPixel.h>

#define PIN 0
#define NUMPIXELS 144

Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

#define NOTE_OFFSET 30
#define NOTE_MULT 2

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

class MyMIDIServerCallback : public NimBLEServerCallbacks {
  void onConnect(NimBLEServer *pserver)  {
    Serial.println("callback: BLE MIDI client connected");
  }

  void onDisconnect(NimBLEServer *pserver)  {
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
  pAdvertising->start();  
}

////////////////////////////////////////////
// MIDI
////////////////////////////////////////////

class MIDIState 
{
  public:
    MIDIState() {};
    void initialise(RingBuffer *rb);
    bool process(byte mi_data[3]);

    RingBuffer *midi_stream;
    int midi_status;
    int midi_cmd_count;
    int midi_data_count;
    int midi_data[2];
};

void MIDIState::initialise(RingBuffer *rb)
{
  midi_stream = rb;
  midi_data_count = 0;
  midi_cmd_count = 0;
  midi_status = 0;
}

bool MIDIState::process(byte mi_data[3]) 
{
  byte b;
  bool got_data;

  got_data = false;
  
  if (!midi_stream->is_empty()) {
    midi_stream->get(&b);
    
    if (b <= 0x7F) {
      midi_data[midi_data_count] = b;
      midi_data_count++;
      midi_cmd_count = 0;
      if ((midi_status == 0xC0 || midi_status == 0xD0) || midi_data_count >= 2) {
        mi_data[0] = midi_status;
        mi_data[1] = midi_data[0];
        if (midi_data_count == 2)
          mi_data[2] = midi_data[1];
        else
          mi_data[2] = 0;
        midi_data_count = 0;
        got_data = true;
      }
    } 
    else {
      midi_cmd_count++;
      if (midi_cmd_count > 1) {
        midi_status = b;            // the last byte before a non-cmd byte will always be status unless it was a single timestamp
      }
    }
  } 
  return got_data;
}

MIDIState ble_midi;

void setup_midi() {
  ble_midi.initialise(&ble_midi_in);
}

bool update_midi(byte *mid) {
  bool got_midi;
 
  got_midi = ble_midi.process(mid);
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
  M5.Lcd.println("BLE Pixels");
  
  setup_midi();
  setup_ble();

  pixels.begin();
  pixels.clear(); 
  pixels.show(); 
}

void loop() {
  byte mi[3];
  int midi_chan, midi_cmd;
  char str[20];
  int pix;
  
  M5.update();

  if (M5.BtnA.isPressed()) {
    pixels.clear(); 
    pixels.show();  
  }
  
  if (update_midi(mi)) {
    midi_chan = (mi[0] & 0x0f) + 1;
    midi_cmd = mi[0] & 0xf0;

    if (midi_cmd == 0x90) {
      pix = (mi[1] - NOTE_OFFSET) * NOTE_MULT;
      if (pix > 143) pix = 1;
      pixels.setPixelColor(pix, pixels.Color(60,0,0));
      pixels.setPixelColor(pix+1, pixels.Color(60,0,0));
      pixels.show(); 
    }

    if (midi_cmd == 0x80) {
      pix = (mi[1] - NOTE_OFFSET) * NOTE_MULT;
      if (pix > 143) pix = 1;
      pixels.setPixelColor(pix, pixels.Color(0,0,0));
      pixels.setPixelColor(pix+1, pixels.Color(0,0,0));
      pixels.show(); 
    }    

    sprintf(str, "%x %x %x %x ", midi_chan, midi_cmd, mi[1], mi[2]);
    M5.Lcd.fillScreen(TFT_BLACK);
    M5.Lcd.setCursor(0, 0);
    M5.Lcd.println("BLE Pixels");
    M5.Lcd.print(str);
  }
}
