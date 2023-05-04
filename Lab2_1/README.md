# Lab 2.1
Create new project
```
idf.py create-project Lab2_1
```
Set target
```
cd Lab2_1
```
```
idf.py set-target esp32c3
```
Move elf file to build folder and rename it to existing elf file name
```
mv [file_path]/lab2_image.elf build
```
```
cd build
```
```
mv lab2_image.elf [existing_elf_file_name]
```
Go back to Lab2_1 directory and create and edit gdbinit file
```
cd ..
```
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
c
```
Install riscv-esp-elf-gdb
```
cd ..
```
```
wget https://github.com/espressif/binutils-gdb/releases/download/esp-gdb-v12.1_20221002/riscv32-esp-elf-gdb-12.1_20221002-aarch64-linux-gnu.tar.gz
```
```
tar -xvzf riscv32-esp-elf-gdb-12.1_20221002-aarch64-linux-gnu.tar.gz
```
```
rm -r riscv32-esp-elf-gdb-12.1_20221002-aarch64-linux-gnu.tar.gz
```
Run openocd in separate terminal (remember to run . export.sh for each new session)
```
cd Lab2_1
```
```
idf.py openocd
```
Run gdb
```
riscv32-esp-elf-gdb -x gdbinit build/Lab2_1.elf
```
Find address of compute function
```
info functions compute
```
Set breakpoint at compute address
```
b *[address]
```
Run
```
run
```
Look at registers
```
info registers
```
Look at https://riscv.org/wp-content/uploads/2015/01/riscv-calling.pdf to figure out which registers contain function arguments.
Find the registers in output (a0 - a2), should look like this:
```
a0             0x21     33
a1             0x0      0
a2             0x4      4
```
33, 0, 4 are the values we wanted to find

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

Error: couldn't bind tcl to socket on port 6666: Address already in use
Solution:
```
ps aux | grep openocd
```
kill all openocd processes
