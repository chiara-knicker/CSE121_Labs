# CSE121_Labs
Labs for CSE 121 Spring 2023

# Hardware
- Raspberyy Pi 4
- ESP32 board

# Setup Pi4
- install Pi Imager: https://www.raspberrypi.com/software/
- open Pi Imager and select Ubuntu Server 22.04.2 LTS (64-bit) as OS
- configure SSH and Wifi
- flash microSD
- insert microSD in Pi4

# Setup to run labs
sudo apt update  
sudo apt upgrade  
sudo apt-get install fish neovim g++ git wget flex bison gperf python3 python3-venv cmake ninja-build ccache libffi-dev libssl-dev dfu-util libusb-1.0-0  
cd ~/CSE121_Labs  
git clone --recursive https://github.com/espressif/esp-idf.git  
cd esp-idf  
./install.sh esp32c  
. export.sh  

# Run labs
cd ~/CSE121_Labs/Labx_x  
idf.py set-target esp32c3  
idf.py flash  
idf.py monitor  

NOTE: flash automatically builds before flashing, to only build, run this after setting the target:  
cd build  
ninja  

To stop monitor: ctrl+] or ctrl+t ctrl+x for some keyboards (e.g. German)

# Use GitHub on Pi4
ssh-keygen -t ed25519 -C "your_email@example.com"  
cat [path-to-public-key]/id_ed25519.pub
  
copy the public key and add it to your GitHub account:  
"settings" > "SSH and GPG keys", paste the public key you copied

git config --global user.email "your email"  
git config --global user.name "your name"  
