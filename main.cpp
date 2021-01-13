/**
 * ---- Running Dino Uno ----
 * @author Vincent Martin
 *
 * This game was developed in the frame of the Arduino Rapid Prototyping Class, during the Winter 2020 semester at
 * Politechnika Krakowska, Kracow, Poland.
 *
 * It is based on Google Chrome's offline game, where the user can play as a dinosaur running in the desert, trying to
 * avoid obstacles by jumping over and crouching under them.
 *
 * It was developed on JetBrains' CLion 2020.3.1, Build #CL-203.6682.181, on macOS 11.0.1 Big Sur.
 * The plugins Arduino Support, PlatformIO and Serial Port Monitor were used.
 *
 * IMPORTANT: Please read the documentation before playing the game. Important informations related to how to launch and
 * run the game, why do some errors can happen and what to do on a user-level to solve them.
 *
 */

// Import standard C++ libraries
#include <avr/io.h>
#include <util/delay.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Import custom libraries
#include "uartLib/uart.hpp"
#include "hd44780/HD44780.hpp"
#include "vector/vector.h"

// USART configuration macros
//#define F_CPU 16000000
#define BAUD 9600
#define MYUBRR  F_CPU/16/BAUD-1

// Button macros
#define B1 PIND0
#define B2 PIND1
#define B3 PIND2
#define B4 PIND3

// LED macros
#define LED4 PORTB2
#define LED3 PORTB3
#define LED2 PORTB4
#define LED1 PORTB5

// Constants
#define MAX_LIVES 4

/* -- Global variables -- */
bool jumping = false; // Whether the player is currently jumping, or not
bool crouching = false; // Whether the player is currently crouching ,or not
int lap = 0; // Lap in the current game
int step = 0; // Step : 0, 1 or 2 ; defines if the character is standing or walking
bool step_up = true; // Defines if the step is currently going up (0, next 1, next 2) or not (2, next 1, next 0)
char* str = ""; // String variable used to display various characters on screen in the program
double chance_gen_obs; // Chances not to generate an obstacle. The chances to generate an obstacle is 1-chance_gen_obs
uint16_t ms; // Delay between each game step, in ms.
bool restart = true; // Whether the player wants to restart a new game, or not
uint8_t lives = MAX_LIVES; // Player's current number of lives
int score = 0; // Player's total score
int diff = 0; // Chosen difficulty : 1, 2, 3 or 4

/**
 * Class: Obstacle
 * Creates an obstacle, with a x and y positions.
 * @public int posx - x-position of the obstacle.
 * @public int posy - y-position of the obstacle.
 * @public closer - brings the obstacle closer to the player.
 */
class Obstacle {
public:
    int posx = 0;
    int posy = 0;

    void closer() { posx--; }
};

// Vector of obstacles
vector obstacles;

/* --- Utility functions --- */

/**
 * Function; disp(unsigned char, unsigned char, char*)
 * Displays a string at the given (x,y) position passed in parameter.
 * Avoid using both LCD_GoTo and LCD_WriteText every time a message has to be written on screen.
 * @param x - x-position of the first character
 * @param y - y-position of the first character
 * @param s - string to display ; max size if 16 characters.
 */
void disp(unsigned char x, unsigned char y, char * s) {
    LCD_GoTo(x,y);
    LCD_WriteText(s);
}

/**
 * Function: clear_bit(unsigned char)
 * Clears the bit corresponding to a given button, i.e. sets it to 0 in the PIND register.
 * @param b - button to clear : B1, B2, B3 or B4.
 */
void clear_bit(unsigned char b) {
    PIND &= ~(1 << b);
}

/**
 * Function: is_released(unsigned char)
 * @param b - button to check : B1, B2, B3 or B4.
 * @return bool - whether the button is released, or not.
 */
bool is_released(unsigned char b) {
    return PIND & 1 << b;
}

/**
 * Function: is_pressed(unsigned char)
 * Checks if a given button is pressed or not.
 * @param b - button to check : B1, B2, B3 or B4.
 * @return bool - whether the button is pressed, or not.
 */
bool is_pressed(unsigned char b) {
    return !is_released(b);
}

/**
 * Function: wait(unsigned char)
 * Waits for the input of a given button.
 * @param b - button to wait for : B1, B2, B3 or B4.
 */
void wait(unsigned char b) {
    while (is_released(b));
}

/**
 * Function: vector_get(int)
 * Returns a pointer to the address of the obstacle placed in position 'position' in the vector.
 * @param position - position of the obstacle in the vector
 * @return Obstacle* - pointer to the address of said obstacle
 */
Obstacle *vector_get(int position) {
    void *void_pointer = VectorNth(&obstacles, position);
    Obstacle *obstacle_pointer = static_cast<Obstacle *>(void_pointer);
    return obstacle_pointer;
}

/**
 * Function: vector_first
 * Returns the oldest element of the vector
 * @return  Obstacle - the first element of the vector.
 */
Obstacle vector_first() {
    return *vector_get(0);
}

/**
 * Function: vector_last
 * Returns the most recent element of the vector.
 * @return Obstacle - the last element of the vector
 */
Obstacle vector_last() {
    return *vector_get(VectorLength(&obstacles)-1);
}

/**
 * Function: vector_full
 * Checks if the vector containing the obstacles is full or not.
 * @return bool - whether the vector is full, or not
 */
bool vector_full() {
    return VectorLength(&obstacles) == 8;
}

/**
 * Function: nb_of_digits(int)
 * Counts the number of digits in a number.
 * @param k - number to counts the digits from
 * @return int - number of digits in k
 * @example nb_of_digits(10) >>> 2
 * @example nb_of_digits(6192) >>> 4
 */
int nb_of_digits(int k) {
    int n = 0;
    do {
        k /= 10;
        n++;
    } while (k > 0);
    return n;
}

/**
 * Function: rand_double
 * Creates a random number of type 'double' between 0 and 1.
 * To do so, generates a random number, counts the number of digits of the number, and divide it by the corresponding
 * power of 10.
 * @return double - random number generated between 0 and 1.
 */
double rand_double() {
    int tmp = random();
    return tmp / (nb_of_digits(tmp)*10);
}

/**
 * Function: on(unsigned char)
 * Turns on the LED passed as parameter.
 * @param LED - diode to turn on : LED1, LED2, LED3 or LED4.
 */
void on(unsigned char LED) {
    // Turns on the LED passed as parameter
    PORTB &= ~(1<<LED);
}

/**
 * Function: off(unsigned char)
 * Turns off the LED passed as parameter.
 * @param LED - diode to turn off : LED1, LED2, LED3 or LED4.
 */
void off(unsigned char LED) {
    PORTB |= (1<<LED);
}

/**
 * Function: ADC_init
 * Initializes the Analogic-Digital Converter to read the value of the potentiometer.
 */
void ADC_Init(void){
    ADCSRA = (1<<ADEN)|(1<<ADPS0)|(1<<ADPS1)|(1<<ADPS2);
    ADMUX  =  (1<<REFS0);
}

/**
 * Function: debug(char*)
 * Sends the string 's' passed as a parameter via USART.
 * Used to avoid writing 'USART_Transmit_String' every time, as 'debug' is shorter.
 * DEBUGGING PURPOSES ONLY.
 * @param s - string to send via USART.
 */
void debug(char *s) {
    USART_Transmit_String(s);
}

/* --- Obstacles functions --- */

/**
 * Function: init_obstacles
 * Initialize the vector of obstacles.
 */
void init_obstacles() {
    VectorNew(&obstacles, sizeof(Obstacle), NULL, 8);
}

/**
 * Function: debug_obstacles
 * Send the position of each obstacle via USART.
 *
 * DEBUGGING PURPOSES ONLY.
 * To use, uncomment line of button B4 in function game().
 */
void debug_obstacles() {
    char *tmp = "";
    Obstacle *obs;
    for (int i = 0 ; i < VectorLength(&obstacles) ; i++) {
        obs = vector_get(i);
        sprintf(tmp, "x:%d, y:%d", obs->posx, obs->posy);
        debug(tmp);
    }
    wait(B4);
}

/**
 * Function: generate_obstacle
 * Generate, or not, an obstacle, depending on generated random numbers.
 */
void generate_obstacle() {
    //debug("generating obstacles...");
    // Check if it is possible to generate a new obstacle
    if (vector_last().posx <= 14 && !vector_full()) {
        // Generate a random number to see if a new obstacle will be generated or not
        double rd_gen_obs = rand_double();
        if (rd_gen_obs > chance_gen_obs) {
            // If the generated random number is greater than the chance of creating a new obstacle,
            // create a new obstacle.
            Obstacle new_obs;
            new_obs.posx = 16;

            // 50-50 chance for it to be on the top line or the bottom line
            double rd_posy = rand_double();
            if (rd_posy < 0.50)
                new_obs.posy = 0;
            else
                new_obs.posy = 1;
            // Add obstacle to vector
            VectorAppend(&obstacles, &new_obs);
            debug("--- New obstacle generated ---");
        }
    }
}

/**
 * Function: update_obstacles
 * Update the obstacles : bring them 1 step closer to the player.
 * If an obstacle is behind the player (posx < 0), delete it.
 */
void update_obstacles() {
    for (int i = 0 ; i < VectorLength(&obstacles) ; i++) {
        vector_get(i)->closer();
        if (vector_get(i)->posx < 0) {
            VectorDelete(&obstacles, i);
            vector_get(0)->closer();
        }
    }
}

/**
 * Function: disp_obstacles
 * Draw the obstacles on screen.
 */
void disp_obstacles() {
    disp(1, 0, "               ");
    disp(1, 1, "               ");

    Obstacle *obs;
    for (int i = 0 ; i < VectorLength(&obstacles) ; i++) {
        obs = vector_get(i);
        char *c;
        // If the obstacle collides with the player's head, write 'x'.
        if (obs->posx == 0 && obs->posy == 0 && !crouching)
            c = "x";
        // If the obstacle collides with the player's legs, write 'X'.
        else if (obs->posx == 0 && obs->posy == 1 && !jumping)
            c = "X";
        // Else, write '-'.
        else
            c = "-";
        disp(obs->posx, obs->posy, c);
    }
}

/* --- Game update / run functions --- */

/**
 * Function: update_LEDs
 * Update the diodes regarding the number of lives left.
 */
void update_LEDs() {
    // Turn off all the diodes (in case of a reset).
    off(LED1);
    off(LED2);
    off(LED3);
    off(LED4);

    // Turn on the correct diodes
    if (lives == 4) {
        on(LED1);
    }
    if (lives >= 3) {
        on(LED2);
    }
    if (lives >= 2) {
        on(LED3);
    }
    if (lives >= 1) {
        on(LED4);
    }
}

/**
 * Function: update_step
 * Update the step : 0, 1 or 2. This is used to determine the position of the legs of the player : standing or walking.
 * Also increment the 'lap' variable, saying how many game cycles the player have survived. This is used to compute the
 * score.
 */
void update_step() {
    lap++;

    if (step_up)
        step++;
    else
        step--;

    if (step == 2)
        step_up = false;
    else if (step == 0)
        step_up = true;
}

/**
 * Function: disp_player
 * Draws the player on the screen on position (0,0) and (0,1). Draw only (0,0) is the player is jumping, or only (0,1)
 * if the player is crouching.
 */
void disp_player() {
    //debug("displaying player...");
    disp(0,0," ");
    disp(0,1," ");

    if (jumping)
        disp(0,0,"o");
    else if (crouching)
        disp(0, 1, "o");
    else {
        char* bottom;
        switch(step) {
            case 0:
                bottom = ">";
                break;
            case 1:
                bottom = "|";
                break;
            case 2:
                bottom = ">";
                break;
            default:
                bottom = "A";
        }
        disp(0, 0, "o");
        disp(0, 1, bottom);
    }
    //debug("done - displaying player");
}

/**
 * Function: check_if_game_over
 * Checks if the game is over, i.e. if the player is colliding with the closest obstacle.
 * @return bool - whether the game is over or not
 */
bool check_if_game_over() {
    Obstacle obs = vector_first();
    return (obs.posx == 0 && obs.posy == 0 && !crouching)
        || (obs.posx == 0 && obs.posy == 1 && !jumping);
}

/* --- Main functions --- */

void game() {

    // Update the diodes at the beginning of a game
    update_LEDs();

    // Wait 500 ms for the screen to refresh before launching the game
    _delay_ms(500);

    /* Life Number Screen */
    sprintf(str, "*  Life : %d/4  *", MAX_LIVES-lives+1);
    disp(0,0, str);
    disp(0,1,"*--*---**---*--*");

    wait(B4);
    _delay_ms(500);

    // Initialize the obstacles vector
    init_obstacles();

    // Run the game while the player has not lost
    while(!check_if_game_over()) {
        // Manually clear B1, B2, B3 and B4 inputs to avoid false inputs
        clear_bit(B1);
        clear_bit(B2);
        clear_bit(B2);
        clear_bit(B4);

        // Manually set jumping and crouching to false at the beginning of each step to avoid problems
        jumping = false;
        crouching = false;

        // Update the position of every obstacles
        update_obstacles();

        // Draw the player on screen
        disp_player();

        // Generate a new obstacle
        generate_obstacle();

        // Draw the obstacles on screen
        disp_obstacles();

        /* BUTTON 1 */
        if (is_pressed(B1)) {
        }

        /* BUTTON 2 */
        else if (is_pressed(B2)) {
            // Transmit via USART for debugging purposes
            USART_Transmit_String("jumping");

            // Set jumping to true because the player is jumping
            jumping = true;

            // Draw the player again once they have jumped
            disp_player();

            // Draw the obstacles once again
            disp_obstacles();
        }

        /* BUTTON 3 */
        else if (is_pressed(B3)) {
            // Transmit via USART for debugging purposes
            USART_Transmit_String("crouching");

            // Set crouching to true because the player is crouching
            crouching = true;

            // Draw the player again once they have crouched
            disp_player();

            // Draw the obstacles once again
            disp_obstacles();
        }

        /* BUTTON 4 */
        else if(is_pressed(B4)){
            // See function debug_obstacles().
            //debug_obstacles();
        }

        /* IDLE */
        else {
        }

        // Update the game step
        update_step();

        // If the game is over, exit the loop before making the delay until next step
        if (check_if_game_over())
            break;

        // Delay until next step ---- Explanation :
        //   It is not possible to use _delay_ms with a variable inside. The _delay_ms function wants the programmer
        //   to use a constant number (10, 1000, 500, 491...) as a parameter. To delay with a variable, I had to use
        //   _delay_ms(1) to delay of 1 ms, and repeat this operation in a loop that cycles 'ms' times
        for (int i = 0 ; i < ms ; i++)
            _delay_ms(1);
    }

    /* Game Over Screen */
    disp(0, 0, "** GAME  OVER **");
    disp(0, 1, "****************");

    wait(B4);
    _delay_ms(1000);

    /* Score Screen */
    LCD_Clear();
    score += lap*diff;
    disp(0, 0, "*    Score    *");
    sprintf(str, "    %d pts", lap*diff);
    disp(0, 1, str);

    wait(B4);
    _delay_ms(1000);

    /* Lives Screen */
    if (lives >= 1)
        lives--;
    update_LEDs();
    disp(0, 0, "*   -1  life   *");
    sprintf(str, "* Lives : %d/4  *", lives);
    disp(0, 1, str);

    // If the player has no lives left, exit the function to get back to the 'main' function.
    if (lives == 0)
        return;

    wait(B4);
    _delay_ms(1000);
}

int main (){

    // Set a new seed based on the current time for the Random Number Generator
    srandom(time(NULL));

    // Loop while the player wants to restart
    while(restart) {
        // Initialize values of the whole run
        restart = false;
        score = 0;
        lap = 0;
        step = 0;

        /* Initialization */
        DDRB |= (1<<DDB2) |    (1<<DDB3) |(1<<DDB4) | (1<<DDB5);
        PORTB |= (1<<PORTB2) | (1<<PORTB3) | (1<<PORTB4) |(1<<PORTB5);

        init_uart(MYUBRR);
        ADC_Init();
        LCD_Initalize();
        LCD_Clear();

        /* Welcome Screen */
        disp(0, 0, "* Running Dino *");
        disp(0, 1, "**  Press B4  **");

        wait(B4);
        _delay_ms(500);

        /* Difficulty Screen */
        while(!is_pressed(B4)) {
            sprintf(str, "* Difficulty %d *", diff);
            disp(0, 0, str);

            // Read value from potentiometer
            ADCSRA |= (1<<ADSC);
            uint16_t adc_value = ADC;
            sprintf(str, "* ADC : %04dms *", adc_value);
            disp(0, 1, str);

            // Turn off all the diodes
            off(LED1);
            off(LED2);
            off(LED3);
            off(LED4);

            // Turn on the right diodes according to the value of the potentiometer
            if (adc_value >= 0 && adc_value <= 256) {
                diff = 4;
                chance_gen_obs = 0.8;
                on(LED1);
                on(LED2);
                on(LED3);
                on(LED4);
            } else if (adc_value > 256 && adc_value <= 512) {
                diff = 3;
                chance_gen_obs = 0.5;
                off(LED1);
                on(LED2);
                on(LED3);
                on(LED4);
            } else if (adc_value > 512 && adc_value <= 768) {
                diff = 2;
                chance_gen_obs = 0.2;
                off(LED1);
                off(LED2);
                on(LED3);
                on(LED4);
            } else {
                diff = 1;
                chance_gen_obs = 0.1;
                off(LED1);
                off(LED2);
                off(LED3);
                on(LED4);
            }
            ms = adc_value;
        }

        wait(B4);
        _delay_ms(500);

        /* Difficulty Recap Screen */
        disp(0, 0, "*  Diff. is :  *");
        sprintf(str, "*  %d - %04dms  *", diff, ms);
        disp(0, 1, str);

        wait(B4);

        // Run the game while the player still has lives
        while(lives > 0)
            game();

        /* Total Score Screen */
        disp(0, 0, "* Total Score *");
        sprintf(str, "    %d pts", score);
        disp(0, 1, str);

        _delay_ms(1000);

        wait(B4);

        /* Restart Screen */
        disp(0, 0, "*  Restart  ?  *");
        disp(0, 1, "B2 - Y    B3 - N");

        while(!is_pressed(B2) && !is_pressed(B3))
            if(is_pressed(B2))
                restart = true;
    }

    disp(0, 0, "* Running Dino *");
    disp(0, 1, "* Game is Over *");

    return 0;
}
