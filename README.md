# PianoNeopixel
M5Stick driving a Neopixel strip in response to MIDI commands   

The story so far...   

Created BLE_MIDI_Device to make M5Stick C appear as MIDI device over BLE, just to see what can connect to it.  
BLE_MIDI_Device2 has better MIDI processing (ie it works, rather than the flaky MIDI in the original BLE_MIDI_Device).   

Dependencies (tested with):   

NimBLE library v1.3.6

