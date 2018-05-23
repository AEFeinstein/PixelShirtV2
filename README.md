View this project on [CADLAB.io](https://cadlab.io/project/1073). 

# PixelShirtV2
PixelShirtV2 is an Arduino based collection of video games running on a matrix of WS2812b LEDs and controlled by [ATmega328p based wireless controllers](https://github.com/AEFeinstein/WirelessController). It's meant to be mounted on a shirt, but can be put anywhere.

## The Firmware

The firmware can be compiled two different ways. If one uses the makefile, it compiles as a Linux terminal application that uses the keyboard as input and draws to the terminal using ncurses. This is used for debugging. If one loads it into the Arduino IDE, it compiles as an Arduino executable that uses NRF24L01+ controllers as input and draws to a matrix of WS2812B LEDs.

Each game is a subclass of ArduinoGame. This allows the main function to keep an array of ArduinoGame pointers, and swap between them at runtime. The virtual functions each game must implement are:
* `void UpdatePhysics(void)` - This is called on an IRQ_HZ clock, currently 32Hz. Game state should be updated here.
* `void ProcessInput(int32_t p1, int32_t p2)` - This is called every time there is new player input, which may be more or less often than 32Hz. Each 32 bit parameter contains bitpacked analog joystick and digital button state.
* `void ResetGame(uint8_t isInit, uint8_t whoWon)` - This is called during initialization (noted by isInit). It should reset the game to a known state. It is usually called at the end of each round of a game with which player won (noted by whoWon), though that is game-specific.

The firmware uses the [Adafruit Neopixel Library](https://github.com/adafruit/Adafruit_NeoPixel) to drive the WS2812B LEDs.

The firmware uses the [nrf24L01+ Radio Library](https://github.com/kehribar/nrf24L01_plus) to talk to the controllers.

The current game roster is
* Pong - Move your paddle and don't let the ball get past you.
* Team Tetris - Like regular Tetris, but one player slides the piece while the other rotates. Each time a new piece spawns, the player roles swap.
* Lightcycle - Drive around, leaving a trail, and don't crash into either your or your opponent's trail. The field wraps around top-bottom and left-right.
* PixelFighter - A simple 2D fighter. Attack or block high or low and knock off your opponent's life bar,
* Space Invader - Two players shoot upwards as invaders slide to the left, drop, and reverse direction. The more invaders shot, the faster they go.

## The Hardware

The hardware consists of two parts, detailed below. 

### The Console

The first part is the pocket-sized console, which has the Arduino Micro, NRF24L01+ radio, two RJ45 jacks which break out Arduino pins, and two USB connectors which draw power from an external battery. There are two USB connectors because I used an external battery with two USB connectors which could each source 1A. Not that I ever turned the display on to max brightness, but 2A never hurt anyone...

Screenshots of the PCB layout, the top view is on the left and the bottom view is on the right:
<img src="https://raw.githubusercontent.com/AEFeinstein/PixelShirtV2/master/Hardware/Images/console-top.png" width="425"><img src="https://raw.githubusercontent.com/AEFeinstein/PixelShirtV2/master/Hardware/Images/console-bottom.png" width="425">

This is what it looks like all soldered together:
<br><img src="https://raw.githubusercontent.com/AEFeinstein/PixelShirtV2/master/Hardware/Images/console-pcb.png" width="425">

### The Display

The second part is the chest-sized display, which has 256 WS2812B LEDs and an RJ45 connector to connect to the console for power and data. This is a one layer board, though it uses four 0-ohm resistors to jump over traces.

Screenshot of the PCB layout:
<br><img src="https://raw.githubusercontent.com/AEFeinstein/PixelShirtV2/master/Hardware/Images/display.png" width="425">

This is what it looks like all soldered together. Not pictured, RJ45 connector at the end of the cable:
<br><img src="https://raw.githubusercontent.com/AEFeinstein/PixelShirtV2/master/Hardware/Images/display-pcb.png" width="425">

## The Final Product
Here's me at Magfest 14 wearing the display mounted underneath a shirt and holding two controllers. The PixelShirtV2 is playing the PixelFighter game.
<br><img src="https://raw.githubusercontent.com/AEFeinstein/PixelShirtV2/master/Hardware/Images/worn.png">

# Earlier Versions

## Version 1
The first version of the PixelShirt used blue LEDs poked through a t-shirt. The cathodes were soldered together in rows, a layer of insulating fabric was layered, then the anodes were soldered in columns. The 32 rows and columns were controlled by four daisy chained 8-bit shift registers and the display would refresh one row at a time, kind of like a CRT TV.
<br><img src="https://github.com/AEFeinstein/PixelShirtV2/raw/master/Hardware/Images/v1_grid.jpg">

The upside to this design was that the LED grid was flexible and comfortable. The downside was that it wasn't mechanically strong (a few solder joints broke), the grid wasn't very regular (I'm not great with fabrics), and it just generally didn't look good. The controllers are [SparkFun Joystick Shield Kits](https://www.sparkfun.com/products/9760). 
<br><img src="https://github.com/AEFeinstein/PixelShirtV2/raw/master/Hardware/Images/v1_standing.jpg" width="425">

But people seemed to get a kick out of it, so I iterated on the design. I wore it first at Magfest 12, but I think this picture is from AwesomeCon. At the time it only had Pong, Lightcycle, and Tetris.
<br><img src="https://github.com/AEFeinstein/PixelShirtV2/raw/master/Hardware/Images/v1_playing.jpg">

## Version 2.0
The next year I swapped out the display for the giant PCB with RGB LEDs. Pros: colors! mechanical strength! no undershirt required! Cons: less flexible. I kept the same Sparkfun controllers. I wore this one at Magfest 13. With the advent of color, I added Space Invader.
<br><img src="https://github.com/AEFeinstein/PixelShirtV2/raw/master/Hardware/Images/v1.5_playing.jpg">

## Version 2.1
For Magfest 14, I got over my fear of non-tethered controllers and made [wireless ones](https://github.com/AEFeinstein/WirelessController), which are mentioned above. They worked great and I didn't lose either one, but I did accidentally short their batteries a little when carelessly throwing them in a pocket. They got warm, but no harm no foul.

## Version 2.2
Magfest 15 only had software updates. I added the Fighter and Super Square games. There were no changes for Magfest 16, and I ended up not wearing it at all :(
