#include "stm32f4xx_hal.h"
#include "main.h"
#include "timers.h"

#define USER_LED_PORT       LD4_GPIO_Port
#define USER_LED_QRANGE     LD3_Pin
#define USER_LED_GREEN      LD4_Pin
#define USER_LED_RED        LD5_Pin
#define USER_LED_BLUE       LD6_Pin
void orange_led_on(void);
void orange_led_off(void);

void green_led_on(void);
void green_led_off(void);

void red_led_on(void);
void red_led_off(void);

void blue_led_on(void);
void blue_led_off(void);

void orange_led_on(void)
{
    HAL_GPIO_WritePin(USER_LED_PORT, USER_LED_QRANGE, GPIO_PIN_SET);
}

void orange_led_off(void)
{
    HAL_GPIO_WritePin(USER_LED_PORT, USER_LED_QRANGE, GPIO_PIN_RESET);
}

void green_led_on(void)
{
    HAL_GPIO_WritePin(USER_LED_PORT, USER_LED_GREEN, GPIO_PIN_SET);
}

void green_led_off(void)
{
    HAL_GPIO_WritePin(USER_LED_PORT, USER_LED_GREEN, GPIO_PIN_RESET);
}

void red_led_on(void)
{
    HAL_GPIO_WritePin(USER_LED_PORT, USER_LED_RED, GPIO_PIN_SET);
}

void red_led_off(void)
{
    HAL_GPIO_WritePin(USER_LED_PORT, USER_LED_RED, GPIO_PIN_RESET);
}

void blue_led_on(void)
{
    HAL_GPIO_WritePin(USER_LED_PORT, USER_LED_BLUE, GPIO_PIN_SET);
}

void blue_led_off(void)
{
    HAL_GPIO_WritePin(USER_LED_PORT, USER_LED_BLUE, GPIO_PIN_RESET);
}


void LedEffectStop(void)
{
    for(int i = 0; i < 4; i++)
    {
        xTimerStop(handle_led_timer[i], portMAX_DELAY);
    }
}

void LedEffect(uint32_t n)
{
    LedEffectStop();
    xTimerStart(handle_led_timer[n-1], portMAX_DELAY);
}

void TurnOnAllLed(void)
{
    green_led_on();
    orange_led_on();
    red_led_on();
    blue_led_on();
}

void TurnOffAllLed(void)
{
    green_led_off();
    orange_led_off();
    red_led_off();
    blue_led_off();
}

void LedEffect01(void)
{
    static uint8_t val = 0;
    val ^= 1;
    if(val)
    {
        TurnOffAllLed();
    }
    else
    {
        TurnOnAllLed();
    }
}

void LedEffect02(void)
{
    static uint8_t val = 0;
    val ^= 1;
    TurnOffAllLed();
    if(val)
    {
        green_led_on();
        red_led_on();
    }
    else
    {
        orange_led_on();
        blue_led_on();
    }
}

void LedEffect03(void)
{
    static uint8_t val = 0;
    TurnOffAllLed();
    if(val >= 4)
    {
        val = 0;
    }
    switch (val++)
    {
        case 0:
        {
            orange_led_on();
        }
        break;

        case 1:
        {
            red_led_on();
        }
        break;

        case 2:
        {
            blue_led_on();
        }
        break;

        case 3:
        {
            green_led_on();
        }
        break;

        default:
            break;
    }
}

void LedEffect04(void)
{
   static uint8_t val = 0;
    TurnOffAllLed();
    if(val >= 4)
    {
        val = 0;
    }
    switch (val++)
    {
        case 3:
        {
            orange_led_on();
        }
        break;

        case 2:
        {
            red_led_on();
        }
        break;

        case 1:
        {
            blue_led_on();
        }
        break;

        case 0:
        {
            green_led_on();
        }
        break;

        default:
            break;
    }
}