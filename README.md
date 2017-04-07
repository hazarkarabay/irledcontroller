# irledcontroller
Simple LED Strip controller with IR remote control.

This project controls brightness of single LED strip from an IR remote. Currently it is using RC6 protocol because I have a MCE remote nearby. Thanks to [IRLib2](https://github.com/cyborg5/IRLib2), it can be configured to work with nearly any IR remote. 

Hardware
--------

A single-sided, toner transfer friendly PCB created for this project. Schematic and board files included in Hardware folder. You'll need [EAGLE](http://www.autodesk.com/products/eagle/free-download) for opening them.  
ATMega328 receives and decodes IR signals, reads an LDR to decide its status while first power-on and PWMs a N-Channel MOSFET to control lights. You'll need supply required voltage for your LED strip to this board. LED strips are generally 12V and board will work with 9-25V LED strips. 30V is absolute maximum for on-board regulator. On-board power MOSFET AO3406 can support up to 3.5A load, please check your strips' power requirement.  
Board also can support Arduino environment (supports DTS reset).

Software
--------

Software written in Arduino environment and commented heavily. Currently it uses 3 IR remote buttons to toggle light and increase/decrease brightness. It uses psychometric lightness formula (CIE 1931) for adjusting brightness and can fade in/out very smoothly (to human eye).  
Currently it uses Timer1 at 12bit mode for PWM. ATmega328 can switch at ~4kHz on this mode. Faster PWM frequencies needs reduced duty cycle resolution. 

Things to know
--------------

Since switching frequency of 4kHz is in audible range, this can leads audible noise. I did not hear anything in my application but you must aware of that.  
The board has an LDR and it only used in first power up. If the ambient light is low while powering on, it will fade lights to %100. After first start-up LDR status is ignored currently (no auto lights-on at night, sorry).  
ATMega328 is way overkill for this kind of project but I don't have any ATTinys in my hand. Because I have relatively high computing power at my hand, I didn't used lookup tables for CIE 1931 calculation, which deals with floats and exponentials. You probably want use a lookup table if you need to use ATTiny.  
If you want use serial debug output, uncomment `#define DEBUG` at line 21.
