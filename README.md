# MosquitoKiller

It's a modification of existing hand mosquito zapping devices so it will make more fun to do it - if it's not enought already :)

It ads kill counter, high score and sound effects.

## Components

* Arduino nano (any Arduino will work, this one I had at hand and it's small)
* OLED display 128x32 (called OLED-091)
* ADS1015 board
* DFPlayer Mini with SD card and speaker (I've used old laptop speakers)
* some resistors

## Assembly

![MosquitoKiller schematic](https://github.com/Saur0o0n/MosquitoKiller/blob/2a2c9f64fb0cd5d484beb640bc2a6db06d57f0b8/documentation/MosquitoKiller-schematic.png)

32kOhm for voltage divider. According to color description, bleed resistors used for capacitor should be 22MOhm, but they are more like 18MOhm (my multimeter at hand, had some troubles to measure it).

10kOhm for rest
