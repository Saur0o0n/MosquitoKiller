# MosquitoKiller

It's a modification of existing hand mosquito zapping devices so it will make more fun to do it - if it's not enough already :)

It ads kill counter, high score and sound effects.

## Components

* Arduino nano (any Arduino will work, this one I had at hand and it's small)
* OLED display 128x32 (called OLED-091)
* ADS1015 board
* DFPlayer Mini with SD card and speaker (I've used old laptop speakers)
* some 10k resistors, one ~32k resistor, switch and wires

## Assembly

![MosquitoKiller schematic](https://raw.githubusercontent.com/Saur0o0n/MosquitoKiller/main/documentation/MosquitoKiller-schematic.png)

1. You should assemble everything as on diagram above.
2. Dismantle the mosquito racket and lead the wires like on the picture. Pair (black and red) directly from battery (button in my case is connected directly to battery - it was easier to solder). One wire from the button - that will inform our controller that it's pressed (blue) and two wires from voltage divider (yellow and green).
![Mosquito Racket](https://raw.githubusercontent.com/Saur0o0n/MosquitoKiller/main/documentation/MosquitZap-wires1.jpg)
3. I've also add additional LED on top of the racket, so I could easier see when it's activated (sometimes, when pressed not in the middle of the button, it tends not to work) - so I've lead double wire from the green LED to the top of the device.
4. Voltage divider - we can measure up to 5,2V with ADS1015 and Arduino Nano (5v+threshold). The mosquito zapper uses somewhere around 2000V - so no luck with direct measurement (I don't even have multimeter that will allow this voltage levels).
Easiest solution (for me) was to use voltage divider. Definitely not the best, since we have potentially high voltage (spark etc) close to micro controller. But I failed to find something better. Perhaps some capacitor coupling (as I've found suggestion on Internet) - but I've failed to implement it.
Anyway since the high voltage part is also "behind" the transformer it does not have common ground - so I've used ADS1015 in differential mode (not comparing to ground) (some other Atmega controllers also have this features - AT328 - doesn't have).
Since my racket had 3 bleeding resistor in series (they are used to discharge capacitor - if you don't use it) - all 22MOhm (which gives 66MOhm in total). So to have voltage <3V (I wanted to be ready for other controllers - and there is no need for precision) it should be additional resistor with value 100kOhm. But this produced for me voltage close to 5V - check with multimeter first!!
So I've settled with 32kOhm and I'm having max voltage close to 3V. I'm not sure if the voltage on capacitor is higher or the resistors where out of spec (my multimeter reads more like 18MOhms - but it did it hardly - so I don't believe it either). So please check it how it works for you, before connecting it to Arduino.



