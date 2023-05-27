#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wiringPi.h>
#include <ctype.h>

#define LED_PIN 18  // GPIO pin number to control the LED

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

// Function to blink the LED based on Morse code
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
