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

1. You should assemble everything as on diagram. 
2. Dismantle the mosquito racket and lead the wires like on the picture. Pair (black and red) directly from battery (button in my case is connected directly to battery - it was easier to solder). One wire from the button - that will inform our controller that it's pressed (blue) and two wires from voltage divider (yellow and green).
![Mosquito Racket](https://raw.githubusercontent.com/Saur0o0n/MosquitoKiller/main/documentation/MosquitZap-wires1.jpg)
3. I've also add additional LED on top of the racket, so I could easier see when it's activated (sometimes, when pressed not in the middle button may not work) - so I've lead double wire from the green LED to the top of the device.
4. Voltage divider - we can measure up to 5,2V with ADS1015 and Arduino Nano (5v+threshold). The mosquito zapper uses somewhere around 2000V - so no luck with direct measurement (I don't even have multimeter that will allow this voltage levels). 

32kOhm for voltage divider. According to color description, bleed resistors used for capacitor should be 22MOhm, but they are more like 18MOhm (my multimeter at hand, had some troubles to measure it).

