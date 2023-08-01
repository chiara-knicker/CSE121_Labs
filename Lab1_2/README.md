# Lab 1.2
Get hello_world example project

https://github.com/espressif/esp-idf/tree/master/examples/get-started/hello_world
```
cp -a ~/CSE121_Labs/esp-idf/examples/get-started/hello_world ~/CSE121_Labs/Lab1_1
```
Edit hello_world_main.c
```
cd ~/CSE121_Labs/Lab1_1/main
```
```
sudo vim hello_world_main.c
```
Edit file so that after printing "Minimum free heap..." it prints your name
```
...
printf("Your Name\n");
...
```
