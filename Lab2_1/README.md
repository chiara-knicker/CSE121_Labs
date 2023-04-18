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
Run openocd
```
idf.py openocd
```
Run gdb
```
riscv32-esp-elf-gdb -x gdbinit build/xxx.elf
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
