# Lab 1.3
Get blink_example example project

https://github.com/espressif/esp-idf/tree/master/examples/get-started/blink
```
cp -a ~/CSE121_Labs/esp-idf/examples/get-started/blink_example ~/CSE121_Labs/Lab1_3
```
As per the lab instructions, I moved the code in main to a function called blink_task and then used xTaskCreate in main to call blink_task. I also changed the GPIO number to 2 (GPIO_NUM_2). The ESP32 GitHub linked in the root README under Resources shows the GPIO layout.
```
(Put code snippets)
```
