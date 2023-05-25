#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/adc.h"
#include "esp_system.h"

#include <esp_timer.h>
#include <string.h>
#include <unistd.h>

#define DOT_DURATION 250    // Duration of a dot in milliseconds
#define DASH_DURATION 750   // Duration of a dash in milliseconds
#define CHARACTER_DELAY 750 // Delay between characters in milliseconds
#define THRESHOLD 60       // ADC threshold value for detecting signal changes

// Morse code dictionary
const char* morseCode[] = {
    ".-",   // A
    "-...", // B
    "-.-.", // C
    "-..",  // D
    ".",    // E
    "..-.", // F
    "--.",  // G
    "....", // H
    "..",   // I
    ".---", // J
    "-.-",  // K
    ".-..", // L
    "--",   // M
    "-.",   // N
    "---",  // O
    ".--.", // P
    "--.-", // Q
    ".-.",  // R
    "...",  // S
    "-",    // T
    "..-",  // U
    "...-", // V
    ".--",  // W
    "-..-", // X
    "-.--", // Y
    "--.."  // Z
};

// Function to decode Morse code signal into character
char decodeMorse(const char* signal) {
    for (int i = 0; i < 26; i++) {
        if (strcmp(signal, morseCode[i]) == 0) {
            return 'A' + i;
        }
    }
    return '_';  // Return '' if no match is found
}

#define ADC_CHANNEL ADC_CHANNEL_0

int getADCVal()
{
    int val = adc1_get_raw(ADC_CHANNEL);
    return val;
}


// Function to measure signal durations and decode Morse code in real time
void processSignal() {
    int aboveThreshold = 0;
    int signalOn = 0;
    unsigned long startTime = esp_timer_get_time();
    unsigned long duration = 0;
    char morseSignal[10] = "";  // Maximum Morse code length
    char decodedChar;

    while (1) {
        int adcValue = getADCVal();  // Function to get ADC value from the photodiode
	//printf("ADC Value: %d\n", adcValue);
        if (adcValue >= THRESHOLD) {
            aboveThreshold = 1;

            if (!signalOn) {
		duration = esp_timer_get_time() - startTime;
		if (duration > 500000) {
		    // Decode the previous signal and print character
                    decodedChar = decodeMorse(morseSignal);
                    printf("%c\n", decodedChar);
                    fflush(stdout);
                    memset(morseSignal, 0, sizeof(morseSignal));
		}
                startTime = esp_timer_get_time();
                signalOn = 1;
            }
        } else {
            aboveThreshold = 0;

            if (signalOn) {
                duration = esp_timer_get_time() - startTime;
                signalOn = 0;

                // Determine signal type based on duration
                if (duration <= 500000) {
                    strcat(morseSignal, ".");
                } else {
                    strcat(morseSignal, "-");
		}

		startTime = esp_timer_get_time();
            }
	}

        vTaskDelay(pdMS_TO_TICKS(100));  // delay for 100 millisecond
    }
}

int app_main() 
{
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC_CHANNEL, ADC_ATTEN_DB_11);
       
    // Call the function to process the Morse code signal in real time
    processSignal();

    return 0;
}
