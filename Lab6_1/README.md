# Lab 6.1

## Setup

The GPIO pinout for the Pi can be found here: https://pinout.xyz/#

The LED is connected to the 5V power pin and the GPIO 18 pin together with a appropriate resistor.

## send.c

```
const char* morseCode[] = {
  ...
}
```
This is a dictionary that contains the appropriate morse code for each letter of the alphabet.

```
void blinkLED(const char* code) {
    for (int i = 0; i < strlen(code); i++) {
        if (code[i] == '.') {
            digitalWrite(LED_PIN, LOW);
            delay(250);
            digitalWrite(LED_PIN, HIGH);
            delay(250);
        } else if (code[i] == '-') {
            digitalWrite(LED_PIN, LOW);
            delay(750);
            digitalWrite(LED_PIN, HIGH);
            delay(250);
        }
    }
}
```
This function is responsible for blinking the LED according to a provided morse code sequence. The symbol '.' respresents a short blink, the symbol '-' represents a long blink. For some reason, I have to send a LOW signal to turn the LED on and a HIGH signal to turn it off. It used to be the other way around, but stopped working suddenly, so I had to change it.

```
int main(int argc, char** argv) {
    // Check if argument is provided
    if (argc != 2) {
        printf("Usage: ./blink <string>\n");
        return 1;
    }

    // Initialize the wiringPi library
    if (wiringPiSetupGpio() == -1) {
        printf("Failed to initialize wiringPi\n");
        return 1;
    }

    // Set the LED pin as output
    pinMode(LED_PIN, OUTPUT);

    // Convert input string to uppercase
    for (int i = 0; argv[1][i]; i++) {
        argv[1][i] = toupper(argv[1][i]);
    }

    // Blink LED based on Morse code
    for (int i = 0; argv[1][i]; i++) {
        if (argv[1][i] >= 'A' && argv[1][i] <= 'Z') {
            const char* code = morseCode[argv[1][i] - 'A'];
            blinkLED(code);
            delay(500);  // Delay between characters
        }
    }

    // Turn off the LED
    digitalWrite(LED_PIN, HIGH);

    return 0;
}
```
The main function checks if exactly one argument is provided. If not, the program displays a usage message ("Usage: ./blink <string>") and returns 1, indicating an error. If the correct number of arguments is provided, the code proceeds to initialize the wiringPi library. WiringPi is a C library that allows easy access to Raspberry Pi's GPIO pins. The LED pin is set as an output pin using the pinMode function. The specific pin used for the LED is GPIO 18. The code then converts the input string to uppercase to avoid worrying about case sensitivity when converting characters to morse code. A for loop is used to iterate over each character of the converted string. It checks if the character is an uppercase letter (A-Z) and if so, it calculates the index for accessing the corresponding Morse code in the morseCode array. It subtracts the character 'A' from the current character to get the index value. The blinkLED function is called with the Morse code corresponding to the current character. After blinking the LED, the program adds a delay of 500 milliseconds which provides a pause between each character in the Morse code sequence.

Once all the characters have been processed, the LED is turned off by setting the LED pin to a high state using the digitalWrite function.

Finally, the program returns 0 to indicate successful execution.
