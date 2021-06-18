# MosquitoKiller

It's a modification of existing hand mosquito zapping devices so it will make more fun to do it - if it's not enough already :)

![Mosquito Racket + Arduino Nano = MosquitoKiller](https://raw.githubusercontent.com/Saur0o0n/MosquitoKiller/main/documentation/MosquitKiller-photo1.jpeg)

It ads kill counter, high score and sound effects.

[![Quick presentation](https://raw.githubusercontent.com/Saur0o0n/MosquitoKiller/main/documentation/mosquitokiller_in_use.jpg)](https://youtu.be/HYjcLB2ITXo)

# How does it work?

* MosquitoKiller is powered from zap racket battery, and it's turn on with racket - starts with welcome sound
* when you press racket zap button (to charge high voltage) - you will hear encourage sound to fight :), score is counted from 0 (new player)
* it counts every discharge as kill, unless it happens faster then 0,7 second in a row (usually it's the same mosquito then ;) )
* if kills are less, then 1,5 second apart - it counts it in for combo, and start to play combo sound (every level - different sound - up to 6)
* during killing ehm... playing ;) - from time to time, you will have random kill sound
* when you release zap button, you will see your score vs highscore, if you haven't made a highscore you will have negative sound
* if you hit high score, it will be saved
* short button press cycles through sound volume, long press turns sound on/off
* when you leave MosquitoKiller on, but not used - you will hear some encouraging quotes sporadically

# Components

* Arduino Nano (any Arduino will work, this one I had at hand and it's small)
* OLED display 128x32 (called OLED-091)
* ADS1015 board
* DFPlayer Mini with SD card and speaker (I've used old laptop speakers - they are quite powerful for their size)
* Step-Up/Boost DC-DC converter board (to have stabilized 5V output), 1A output is enough (I've used MT3608)
* NPN transistor (I've used 2SC828A - but only because I've pick this one from the box)
* some 10k and 1,8K resistors, one ~32k resistor, some capacitors (470uF) switch and wires

# Assembly

![MosquitoKiller schematic](https://raw.githubusercontent.com/Saur0o0n/MosquitoKiller/main/documentation/MosquitoKiller-schematic.png)

1. You should assemble everything as on diagram above.

2. Format your micro SD card and copy content of 'sdcard' folder - on it. Check if the button, OLED display and audio are working (there will be welcome sound on start).

3. Dismantle the mosquito racket and lead the wires like on the picture. Solder (black) BAT- directly from battery. The positive (BAT+ - red wire) should be connected to place, that is powered only when sliding button is on upper or lower position. So the MosquitoKiller addon will be powered only when racket is enabled. You can find solder places close to the sliding switch or as in my case, on one side of the zap button (check with multimeter connecting GND/BAT- and one side of the zap button - if there is voltage, without pressing button - this is the place).

4. Connect "zap button" wire (blue) to the side of the button, that is powered only when pressed (and when the high voltage capacitor is charging).
![Mosquito Racket - wires](https://raw.githubusercontent.com/Saur0o0n/MosquitoKiller/main/documentation/MosquitZap-wires1.jpg)

5. Unsolder one side of one bleed resistor - choose one, where it will be easy to add additional resistor, without closing to other components. Add resistor and solder wires to it (green and yellow) - explanation below.

6. Voltage divider - we can measure up to 5,3V with ADS1015 and Arduino Nano (5v+threshold). The mosquito zapper uses somewhere around 2000V - so no luck with direct measurement (I don't even have multimeter that will allow this voltage levels).
Easiest solution (for me) was to use voltage divider. Definitely not the best, since we have potentially high voltage (spark etc) close to micro controller. But I failed to find something better. Perhaps some capacitor coupling (as I've found suggestion on Internet) - but I've failed to implement it.
Anyway since the high voltage part is also "behind" the transformer it does not have common ground - so I've used ADS1015 in differential mode (not comparing to ground) (some other Atmega controllers also have this features - AT328 - doesn't have).
![Mosquito Racket - voltage divider](https://raw.githubusercontent.com/Saur0o0n/MosquitoKiller/main/documentation/MosquitZap-divider1.jpg)
Since my racket had 3 bleeding resistor in series (they are used to discharge capacitor - if you don't use it) - all, according to color coding, 22MOhm (which gives 66MOhm in total). So to have voltage <3V (I wanted to be ready for other controllers - and there is no need for precision) it should be ok to introduce additional resistor with value 100kOhm. But this produced voltage close to 5V - so check with multimeter first!!
So I've settled with 32kOhm and I'm having max voltage close to 3V. I'm not sure if the voltage on capacitor is higher or the resistors where out of spec (my multimeter reads more like 18MOhms - but it did it hardly - so I don't believe it either). So please check it how it works for you, before connecting it to Arduino.
Currently in the code, on ADS1015, voltage threshold is setup to 3V max (it can go up to 5,3V - if you change it).
Anyway, don't bend resistors to the PCB - let them stay like on the picture, so there couldn't be any uncontrolled discharge.

7. Connect step-up board to black/red wires, switch on the racket and with multimeter measure output - set it up to 5V exactly. I recommend to add some biffy capacitor on output (470uF or more), and perhaps on input also (I don't have it - but there where times, when hitting extra combo - board hanged. First sign of power starve is distorted sound)

8. Connect step-up board output to Arduino GND and 5V pin (not VIN), to ADS1015 and to DFPlayer (preferably add separate capacitor for DFPlayer only)

9. Have a fun!

10. I've also add additional LED on top of the racket, so I could easier see when it's activated (sometimes, when pressed not exactly in the middle of the button, it tends not to work) - so I've lead double wire from the green LED to the top of the device.

## Side notes

There are some "issue" solved in the code:
* Arduino Nano had a problem with starting up with external power source connected to 5V - it's known problem, that's why there is startup delay 1,2s
* There is heavy debouncing for ZAP button - because it had some troubles to stay enabled when not properly pressed
* Also release of ZAP button for 0,6S is not counted as release (some new "players" had problem with not pushing it correctly and restarting the game) - this produced a lot of noise (on/off sounds and counter reset)
* There is a lot of interference from high voltage transformer, and when I've assembled and tested the device it was ok. But when I've installed it on top of the racket - it was unable to count and reset after discharge. It was enough to cover step-up board (perhaps unnecessary - but I've learn from other projects, it also produces some noises) with tin foil (wrap it with electric tape, few layers of tin foil and tape again), and lay few layers of tin foil below the MosquitoKiller box (so between racket and black box) - now it works like a charm.


## Case

I've made rather crude case for components. It's definitely far from anything nice, but there is a lot of components to stuff there and I wanted (unless for start) make it easy to print - without assembly and supports. You can download it from Thingiverse: https://www.thingiverse.com/thing:4887340

