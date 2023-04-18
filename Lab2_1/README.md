# Lab 2.1
Create new project
```
idf.py create-project Lab2_1
```
Download elf file and set target
```
cd Lab2_1
```
```
idf.py set-target esp32c3
```
Move elf file to build folder and rename it to existing elf file name
```
mv [elf_file_name.elf] build
```
```
mv [elf_file_name.elf] [existing_elf_file_name]
```
Run openocd in separate terminal
```
idf.py openocd
```
Create and edit gdbinit file
```
touch gdbinit
```
```
sudo vim gdbinit
```
Edit file to look like this:
```
target remote :3333
set remote hardware-watchpoint-limit 2
mon reset halt
flushregs
thb app_main
c
```
Install riscv-esp-elf-gdb
```
wget https://github.com/espressif/binutils-gdb/releases/download/esp-gdb-v12.1_20221002/riscv32-esp-elf-gdb-12.1_20221002-aarch64-linux-gnu.tar.gz
```
Run gdb
```
riscv32-esp-elf-gdb -x gdbinit build/xxx.elf
```

# Issues
Issue with openocd permissions: LIBUSB_ERROR_ACCESS
Solution:
```
cd ~/etc/udev/rules.d
```
```
sudo wget https://raw.githubusercontent.com/espressif/openocd-esp32/master/contrib/60-openocd.rules 
```
```
sudo udevadm trigger
```
This gives openocd access to usb connections, which is how it will have access to the esp32.