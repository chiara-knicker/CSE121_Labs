# Lab 6.2

## Setup

The GPIO pinout for the Pi can be found here: https://pinout.xyz/#

[TODO: Explain wiring}

## Lab6_2.c

```
#define DOT_DURATION 250
#define DASH_DURATION 750
#define CHARACTER_DELAY 750
```
This defines how long the LED should be turned on for dots and dashes, and how long the delay between characters should be.

```
const char* morseCode[] = {
  ...
}
```
This is a dictionary that contains the appropriate morse code for each letter of the alphabet.

```
char decodeMorse(const char* signal) {
    for (int i = 0; i < 26; i++) {
        if (strcmp(signal, morseCode[i]) == 0) {
            return 'a' + i;
        }
    }
    return '?';  // Return '?' if no match is found
}
```
This function takes a signal in Morse code as input and returns the corresponding decoded character. It iterates over the array of Morse code mappings (morseCode) and checks if the signal matches any of the Morse code patterns. If a match is found, it returns the corresponding character. Otherwise, it returns a question mark ('?').
```
#define ADC_CHANNEL ADC_CHANNEL_0
```
This line defines the ADC channel used for the light sensor. In this case, it's channel 0.
```
int getADCVal()
{
    int val = adc1_get_raw(ADC_CHANNEL);
    return val;
}
```
This function reads the ADC value from the light sensor connected to the ADC_CHANNEL. It uses the adc1_get_raw function provided by the ESP32C3 to obtain the raw ADC value and returns it.
```
void processSignal() {}
```
The processSignal function is the main function responsible for decoding the Morse code.

It initializes variables for tracking the state of the signal (aboveThreshold, signalOn), measuring time (startTime, duration), storing the current Morse code signal (morseSignal), and storing the decoded character (decodedChar). morseSignal has a maximum length of 10 to accommodate the longest Morse code characters.

It calls the getADCVal function to obtain the baseline ADC value (THRESHOLD) and prints it. This value will be used as a reference to differentiate between the LED being on and off. The threshold value (THRESHOLD) is increased by 10 to set a higher threshold for detecting the LED being on.

Inside an infinite loop, it continuously monitors the ADC value and decides if the signal is a dot, dash, or space (end of a character).

If the ADC value is above the threshold (THRESHOLD), it indicates that the LED is on. It sets the aboveThreshold flag and checks if the signal was off previously (signalOn is false). If so, it records the current time (startTime) and sets signalOn to true.

If the ADC value is below the threshold, it indicates that the LED is off. It clears the aboveThreshold flag and checks if the signal was on previously (signalOn is true). If so, it calculates the duration of the signal by subtracting the startTime from the current time and stores it in duration. It then determines the signal type (dot or dash) based on the duration and appends the corresponding symbol ('.' or '-') to the morseSignal string.

If the signal was not on previously, it checks if the duration exceeds a certain threshold (500000 microseconds) and if the morseSignal string is not empty. If both conditions are met, it means that it's the end of a character, so it calls the decodeMorse function to decode the Morse code stored in morseSignal and assigns the result to decodedChar. It then prints the decoded character and clears the morseSignal string.

After each iteration of the loop, it introduces a 100 millisecond delay using vTaskDelay to control the processing rate. This prevents the code from continuously checking the ADC value too frequently and provides some time for the LED signal to change.
```
int app_main() 
{
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC_CHANNEL, ADC_ATTEN_DB_11);
       
    processSignal();

    return 0;
}
```
The app_main function is the entry point of the program. It configures the ADC width to 12 bits using adc1_config_width and sets the attenuation of the ADC channel to 11 dB using adc1_config_channel_atten. Then, it calls the processSignal function to start decoding the Morse code in real time.
