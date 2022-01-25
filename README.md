# PianoNeopixel
M5Stick driving a Neopixel strip in response to MIDI commands   

The story so far...   

- BLE_MIDI_Device will make M5Stick C appear as MIDI device over BLE, just to see what can connect to it.  
- BLE_MIDI_Device2 has better MIDI processing (ie it works, rather than the flaky MIDI in the original BLE_MIDI_Device).   
- Pixels will light a 144 led 1m strip of WS2812B Neopixels different colours (driven by 3.3v on the M5Stick - seems to work on my pixel strip.   
- BLE_Pixles will listen to MIDI messages over BLE and light a pixel on the strip - it assumes two pixels per note spacing).   


Dependencies (tested with):   

NimBLE library v1.3.6    
Adafruit NeoPixel v1.10.0    

