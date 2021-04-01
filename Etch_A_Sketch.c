#include "msp.h"
#include "lcd.h"
#include "adc.h"

#define X_INPUT_CHAN 15 //use A15 connected to p6.0
#define Y_INPUT_CHAN 9 //use A9 connected to p4.4

#define ONE_CYCLE 3006 //1ms delay

#define PB2 BIT5
#define PB1 BIT1

#define S1_PIN P5
#define S2_PIN P3

#define RATIO 129
#define FULL 16384.0

int colourArray[] = { BLACK, RED, GREEN, BLUE, CYAN, YELLOW, MAGENTA, WHITE };
int index = 0;

void msDelay(unsigned int ms)
{
    while (ms > 0)
    {
        _delay_cycles(ONE_CYCLE);
        ms--;
    }
}

void main(void)
{
    WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;		// stop watchdog timer

    int result_y = 0;
    int result_x = 0;
    int i_y;
    int i_x;

    S2_PIN->DIR &= ~PB2;        //set up P3.5 as input for S2
    S1_PIN->DIR &= ~PB1;        //set up P5.1 as input for S1

    //enable port3 and port 5 for interrupt
    S2_PIN->IE |= PB2;
    S1_PIN->IE |= PB1;

    //edge trigger on falling for port3.5 and port5.1
    S2_PIN->IES |= PB2;
    S1_PIN->IES |= PB1;

    //clear flag for port3.5 and 5.1
    S2_PIN->IFG &= ~PB2;
    S1_PIN->IFG &= ~PB1;

    //enable NVIC for port 3 and port5
    NVIC_EnableIRQ(PORT3_IRQn);
    NVIC_EnableIRQ(PORT5_IRQn);

    //global interrupt enable
    __enable_interrupts();

    //set up for ICD
    lcdInit();

    //set up adc sample and record the y value
    adcInit(Y_INPUT_CHAN);
    result_y = adcSample();
    i_y = result_y / RATIO;

    //set up adc sample and record x value
    adcInit(X_INPUT_CHAN);
    result_x = adcSample();
    i_x = result_x / RATIO;

    //put down a pixel for the first value
    lcdSetPixel(LCD_MAX_X - result_x, result_y, colourArray[index]);

    while (1)
    {
        //sample vertical value
        adcInit(Y_INPUT_CHAN);
        result_y = adcSample();

        //check for joystick y position and increment the the pixel in y
        if ((result_y) > (0.8 * FULL))
            i_y++;
        else if ((result_y) < (0.2 * FULL))
            i_y--;

        //sample x direction value
        adcInit(X_INPUT_CHAN);
        result_x = adcSample();

        //check for joystick x position and increment the pixel in x
        if ((result_x) > (0.8 * FULL))
            i_x--;
        else if ((result_x) < (0.2 * FULL))
            i_x++;

        //check for out of range pixel position and fix it
        if (i_x > LCD_MAX_X)
            i_x = 0;
        else if (i_x < 0)
            i_x = LCD_MAX_X;

        if (i_y > LCD_MAX_Y)
            i_y = 0;
        else if (i_y < 0)
            i_y = LCD_MAX_Y;

        //put down a pixel according to the x and y value and the colour
        lcdSetPixel(i_x, i_y, colourArray[index]);

        //slow down the drawing
        msDelay(25);

    }

}

void PORT3_IRQHandler(void)
{
    int arraySize;
    //check for colour array size
    arraySize = sizeof(colourArray) / sizeof(colourArray[0]);

    msDelay(10);        //delay for 10ms to miss the bounce

    //S2 toggle the red LED and clear the flag after the delay
    S2_PIN->IFG &= ~PB2;

    //debounce the push button by checking if the button is still pushed after 10ms delay
    if (S2_PIN->IN & PB2)
    {
    }
    else
    {
        //increment colour array accordingly
        if (index < arraySize)
        {
            index++;
        }
        else
            index = 0;
    }

}

void PORT5_IRQHandler(void)
{
    msDelay(10);    //delay for 10ms to miss the bounce

    //S1 toggle the blue LED and clear the flag after the delay
    S1_PIN->IFG &= ~PB1;

    //debounce the push button by checking if the button is still pushed after 10ms delay
    if (S1_PIN->IN & PB1)
    {
    }
    else
    {
        //clear the screen when S1 is pressed
        lcdClear(BLACK);
    }

}
