/**
 * main.c
 */

#include <stdint.h>
#include <stdbool.h>
#include"mini_library/mini_regmap.h"
#include"mini_library/mini_gpio.h"
#include"mini_library/mini_timer.h"
#include"mini_library/mini_adc.h"
#include"mini_library/mini_interrupt.h"
#include"mini_library/mini_sysctl.h"
#include"mini_library/mini_ssi.h"
#include "PeripheralConfigure/PeripheralConfigure.h"
#include "AudioProcess/AudioProcess.h"

//Declare signal and parameter buffer
volatile uint32_t parameter[4];
uint16_t input = 0, output = 0;

//Declare working variables
volatile uint8_t pedalMode = 0;
volatile bool onOffPedal = true;
uint16_t debouncingCount = 0;
uint8_t mode = 0;
volatile bool bypassed = true;


/*
 * Reading guitar signal with a sample rate of 41kHz, and output the signal
 */
void ADC0SS3Handler(void){
    //Read adc value to buffer
    input = ADCSS3ReturnData(ADC0_BASE);
    output = AudioProcessSampleBySample(input, parameter, bypassed);
    SSI1_16bitWrite(output);
    ADCClearInterruptFlag(ADC0_BASE);
}


/*
 * Reading parameters and pass to the array
 */
void ADC1SS2Handler(void){
    ADCSS2ReturnData(ADC1_BASE, parameter);
}

/*
 * Idle Timer Handle, for future upgrade purpose
 */
void Timer1AHandler(void){
    HWREG(TIMER1_BASE + TIMER_ICR_OFFSET) |= 0x00000001;
}

/*
 * Idle Timer Handle, for future upgrade purpose
 */
void Timer2AHandler(void){
    HWREG(TIMER2_BASE + TIMER_ICR_OFFSET) |= 0x00000001;
}

/*
 * Interrupt generated by bypassed switch
 */
void GPIOPortAIntHandler(void){
    //If the pedal is bypassed
    if (GPIOPinRead(GPIO_PORTA_BASE,GPIO_PIN_2) == 4){
        bypassed = true;
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3|GPIO_PIN_2, GPIO_PIN_3|GPIO_PIN_2);
        GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_2, GPIO_PIN_2);
    }
    else bypassed = false;
    //Clear the interrupt flag
    HWREG(GPIO_PORTA_BASE + GPIO_ICR_OFFSET) |= 4;
}


int main(void)
{
    //Configuration
    GeneralConfigure();
    Timer0A_ADCConfigure();
    Timer1A_Configure();
    Timer2A_SSIConfigure();
    ADCConfigure();
    SSI1_FrameConfigure();

    //Bypassed if PA2 is high
    if (GPIOPinRead(GPIO_PORTA_BASE,GPIO_PIN_2) == 4) bypassed = true;
    else bypassed = false;

    while(1){}
}


