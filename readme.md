<div align="center">

# Running Dino Uno

<br><br><br><br>

### Arduino Rapid Prototyping Class

<br /><br /><br /><br />

#### End-of-semeter Project

<br><br><br><br>

#### Vincent Martin

<br><br><br><br><br>

###### Politechnika Krakowska - Winter 2020

</div>

\pagebreak

<div align="justify">

## 1. Project Description

### 1.1. Inspiration

This is the documentation of the **Running Dino** Arduino Game. The concept is the same as Google Chrome's game, available
when someone's computer is offline and trying to access to a website. It consists in a character controlled by the user
that has to jump over or crouch under coming obstacles.

The LCD screen is perfect to realize such a project. Indeed, having 16x2 characters, it allows for the obstacles to
occupy the first and second line to give the user the opportunity to crouch or jump, respectively.

### 1.2. How to Play
The user has to avoid the obstacles coming from the right. When an obstacle is on the top line, the user has to jump
using the button B2. When an obstacle is on the bottom line, the user has to crouch using the button B3.

### 1.3. Lives and Difficulty
Not avoiding an obstacle will result in the loss of a life. The user has 4 lives, which are 4 tries to improve their score.
The score is calculed as the amount of time survived multiplied by the difficulty multiplier.

The difficulty can be chosen when the user enters the game, right after the welcome screen. There are 4 difficulty levels,
each bringing different advantages. The higher the difficulty, the higher the score multiplier : the difficulty level
corresponds to the multiplier (diff. level 1 is 1x, diff. level 2 is 2x, etc.).

The difficulty can be finely adjusted thanks to the potentiometer available on the board. The difficulty level depends on
the value chosen by the user, using the potentiometer. The value on screen represents the delay between 2 game steps, being
directly related to the speed at which the player will walk - or run : the higher the delay, the slower the player will be.
On the contrary, the lower the delay, the faster the player will be. After a certain threshold, the game becomes unplayable
because the refresh rate of the screen is not high enough to allow the user to see the obstacles. Changing the difficulty
level also changes the probability of a new obstacle appearing at each game step.

Below are the details of each difficulty level :

| Difficulty level | Multiplier | From (ms) | To (ms) | Probability of new obstacles |
|------------------|------------|-----------|---------|------------------------------|
| Level 1          | 1x         | 1024      | 769     | 20%                          |
| Level 2          | 2x         | 768       | 513     | 50%                          |
| Level 3          | 3x         | 512       | 257     | 80%                          |
| Level 4          | 4x         | 256       | 0       | 90%                          |

### 1.3. Game Walkthrough

At first, the player arrives on the main screen, introducing them to the game with the "Running Dino" title, and a prompt
asking to press the button B4.

Then, they have to choose the difficulty thanks to the potentiometer. At this point, the LEDs at the bottom of the screen
show the difficulty level that is currently chosen.

Next up comes a screen with a summary of the chosen difficulty level, with the number of the current try, out of 4 lives.
Now, the diodes show the number of lives left.

After what the game shows up and the player has to avoid obstacles to earn points.

When the player dies, the "GAME OVER" screen pops up, followed by the user's total score.

The final screens notifies the user that they have lost a life, decreasing the diode counter, and displays the number of the next try. Those scenes are
repeated on and on until the player uses their 4 lives.

Once all the lives have expired, a screen with the player's total score is presented. The next one asks them if they want
to restart the game. By pressing B2, the game will restart. Any other button will terminate the game.

## 2. Technical Informations

### 2.1. Technical description

#### Used in this project :
- Arduino Uno, Atmega328p
- LCD screen of type HD44780
- USART communication for debugging purposes
- Diodes available on the board
- Potentiometer available on the board

The buttons are numbered B1, B2, B3 and B4 going from top to bottom. Unless contrary indication on screen, the button
to go from one scene to the next is **B4**.

The potentiometer has to be used to choose the difficulty.

The diodes are used to show the difficulty during the choice, and to show how many lives are left during the game.

USART communication through Serial was only used for debugging purposes, and some lines can be seen at some points in
the code.

### 2.2. Libraries

I have used the libraries that were given during the Class Laboratories : `hd44780` and `uartLib`.

I have also `vector` used a library found on the Internet, that comes from [Derek Bikoff (@dhbikoff)](https://github.com/dhbikoff)
on GitHub, under "**Generic-C-Library**". It reproduces the dynamic behavior of the C++ `vector` class from C++ STL
(Standard Template Library), which is not available when developing Arduino embedded software, instead of using regular
static C++ arrays.

### 2.2. How to Run

To run the project, simply run the `int main` function in `project.cpp` file in the directory.

The file has to be ran and uploaded onto the Arduino Uno board like any other project.

**If the obstacles are not appearing during any of the tries**, it means that the Random Number Generator's seed have
been corrupted or poorly generated. Resetting the board with the bottom-left reset button is not enough. The flash
memory of the board has to be flushed to generate a new RNG seed. To do so, **please unplug the board from the computer
and re-plug it**. Once the board is plugged in the computer again, please try again. This problem may occur multiple times
in a row.

# 3. Disclaimer

This project have been realized by Vincent Martin, in the frame of the Arduino Rapid Prototyping Class at Politechnika
Krakowska during the Winter 2020 Semester. This project have been developed entirely from scratch, except from the "vector"
library, and the code examples and libraries that come from the Class Laboratories.

This project was developed using JetBrains' CLion IDE 2020.3.1 Build #CL-203.6682.181, with plugins `Arduino Support`,
`Serial Port Monitor` and `PlatformIO for CLion` on macOS Big Sur 11.0.1.

The `Arduino AVR Boards` plugin for Eclipse have recently been deprecreated. None of the Eclipse versions compatible with
macOS Big Sur are still compatible with the plugin. This is why I decided to switch to CLion.

The project works totally fine on my computer and build, and I can provide you screenshots and videos of the project
working if needed. Any compilation or C++ Standard-related problem might come from the IDE I used not being Eclipse.
</div>
