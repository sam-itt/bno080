# BNO080 I2C driver in C.

This repo contains minimal code to get a rotation vector (heading/pitch/roll) 
from a BNO080 on I2C. It works on the Raspberry Pi.

This code started as a port of the [python driver][url1] from Adafruit. If you
want a more sophisticated driver, you can also checkout [this project][url2].
Unfortunatelty it doesn't target generic Linux but it's worth the read (or 
port). Really clean code with lots of comments. 

## Raspberry Pi
This driver will run on the Pi (using the onboard i2c), but you need to edit
the config file to avoid the clock stretching bug. Add the following lines 
to `/boot/config.txt` and reboot:

```
dtparam=i2c_arm=on
dtparam=i2c_arm_baudrate=400000
```

## Build
```sh
$ git clone https://github.com/sam-itt/bno080.git
$ make
$ ./test-bno /dev/i2c-0
Part number: 10004148
Software version: 3.2.13
Build: 6
******Init complete**********
heading: 129.189448 pitch:-8.052019 roll: 0.260000
heading: 120.612399 pitch:-5.338937 roll: 2.973388
heading: 120.365681 pitch:-5.242212 roll: 3.029900
heading: 120.223943 pitch:-5.444344 roll: 3.001359
heading: 120.210721 pitch:-5.449982 roll: 3.061907
heading: 120.371263 pitch:-5.347484 roll: 3.114182
<Ctrl-C>
```

[url1]: https://github.com/adafruit/Adafruit_CircuitPython_BNO08x
[url2]: https://os.mbed.com/users/MultipleMonomials/code/BNO080
