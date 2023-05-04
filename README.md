# CSE121_Labs
Labs for CSE 121 Spring 2023

# Hardware
- Raspberyy Pi 4
- ESP32C3 board

# Setup Pi4
1. Install Pi Imager: https://www.raspberrypi.com/software/
2. Open Pi Imager and select Ubuntu Server 22.04.2 LTS (64-bit) as OS
3. Configure SSH and Wifi
4. Flash microSD
5. Insert microSD in Pi4

# Setup to run labs
1. Upgrade Ubuntu and install required packages
```
sudo apt update
```  
```
sudo apt upgrade
```
```
sudo apt-get install fish neovim g++ git wget flex bison gperf python3 python3-venv cmake ninja-build ccache libffi-dev libssl-dev dfu-util libusb-1.0-0
```

2. Get ESP32 software toolchain
```
cd ~/CSE121_Labs
```
```
git clone --recursive https://github.com/espressif/esp-idf.git
``` 
```
cd esp-idf
```
```
./install.sh esp32c3
```
```
. export.sh
```

Need to rerun . export.sh after each shutdown of the Pi4

# Run labs
```
cd ~/CSE121_Labs/Labx_x
``` 
```
idf.py set-target esp32c3
```
```
idf.py flash
```
```
idf.py monitor
```

To stop monitor:  
ctrl+] or ctrl+t ctrl+x for some keyboards (e.g. German)

Flash automatically builds before flashing. To only build, run this after setting the target:  
```
cd build
```
```
ninja
```

# Use GitHub on Pi4
1. Create ssh key
```
ssh-keygen -t ed25519 -C "your_email@example.com"
```
  
2. Copy the public key and add it to your GitHub account
```
cat [path-to-public-key]/id_ed25519.pub
```
"Settings" > "SSH and GPG keys"

Paste the public key you copied

3. Configure user
```
git config --global user.email "your_email"
```
```
git config --global user.name "your_name"
```

# Issues encountered
When trying to flash the first time, I got this Error:
```
(need to reproduce error)
```
It turned out that the cable I was using to connect the ESP32 to the Pi4 was only for charging. When I used a different cable it worked.

# Resources
## ESP32C3
ESP32C3 Github <br>
https://github.com/esp-rs/esp-rust-board

Temperature and humidity sensor datasheet <br>
https://www.mouser.com/datasheet/2/682/Sensirion_04202018_HT_DS_SHTC3_Preliminiary_D2-1323493.pdf

Accelerometer and gyroscope datasheet <br>
https://invensense.tdk.com/wp-content/uploads/2021/07/DS-000451-ICM-42670-P-v1.0.pdf

## Lab 2
https://riscv.org/wp-content/uploads/2015/01/riscv-calling.pdf

https://web.cecs.pdx.edu/~apt/cs491/gdb.pdf
