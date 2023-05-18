# Lab 5.3

## Issues

Code:
```
char* web_url_format = "https://www.wttr.in/%s?format=%%l:+%%c+%%t";
char web_url[strlen(web_url_format) + strlen(body_content) + 1];
sprintf(web_url, web_url_format, body_content);
```
Error:
```
I (4445) wifi station: Minimum free heap size: 222580 bytes
I (4445) wifi station: https_request using crt bundle (lab function)
E (4455) esp-tls: couldn't get hostname for :: getaddrinfo() returns 202, addrinfo=0x0
E (4465) esp-tls: Failed to open new connection
E (4475) wifi station: Connection failed...
Guru Meditation Error: Core  0 panic'ed (Load access fault). Exception was unhandled.

Stack dump detected
Core  0 register dump:
MEPC    : 0x40058eb6  RA      : 0x42099aee  SP      : 0x3fcaa220  GP      : 0x3fc92200
0x42099aee: _vfprintf_r at /builds/idf/crosstool-NG/.build/HOST-aarch64-linux-gnu/riscv32-esp-elf/src/newlib/newlib/libc/stdio/vfprintf.c:1524

TP      : 0x3fc76ec4  T0      : 0x42046746  T1      : 0x3fcaa2dc  T2      : 0xffffffff
0x42046746: mbedtls_platform_zeroize at /home/chiar/CSE121_Labs/esp-idf/components/mbedtls/mbedtls/library/platform_util.c:73

S0/FP   : 0x00000000  S1      : 0x00000000  A0      : 0x00000000  A1      : 0xffffffff
A2      : 0x0000001f  A3      : 0x7f7f7f7f  A4      : 0x00000000  A5      : 0x00000004
A6      : 0x00000001  A7      : 0x0000000a  S2      : 0x0000005a  S3      : 0x00000000
S4      : 0x00000001  S5      : 0x3fc998e8  S6      : 0xffffffff  S7      : 0x3fcaa2e4
S8      : 0x00000000  S9      : 0x00000073  S10     : 0x00000000  S11     : 0x00000000
T3      : 0x00000000  T4      : 0x00000000  T5      : 0x0ccccccc  T6      : 0x00000019
MSTATUS : 0x00001881  MTVEC   : 0x40380001  MCAUSE  : 0x00000005  MTVAL   : 0x00000000
0x40380001: _vector_table at ??:?

MHARTID : 0x00000000


Backtrace:


0x40058eb6 in ?? ()
#0  0x40058eb6 in ?? ()
#1  0x42099aee in _vfprintf_r (data=<optimized out>, fp=0x3fc998e8, fmt0=<optimized out>, ap=<optimized out>) at /builds/idf/crosstool-NG/.build/HOST-aarch64-linux-gnu/riscv32-esp-elf/src/newlib/newlib/libc/stdio/vfprintf.c:1524
#2  0x00000000 in ?? ()
Backtrace stopped: frame did not save the PC
ELF file SHA256: 534576e6068b915a

Rebooting...
ESP-ROM:esp32c3-api1-20210207
Build:Feb  7 2021
rst:0x3 (RTC_SW_SYS_RST),boot:0xc (SPI_FAST_FLASH_BOOT)
Saved PC:0x40048b82
SPIWP:0xee
mode:DIO, clock div:1
load:0x3fcd5820,len:0x1718
load:0x403cc710,len:0x974
load:0x403ce710,len:0x2f68
entry 0x403cc710
```
Fix:
```
char* web_url_format = "https://www.wttr.in/%s?format=%%l:+%%c+%%t";
char web_url_temp[strlen(web_url_format) + strlen(body_content) + 1];
sprintf(web_url_temp, web_url_format, body_content);
web_url = web_url_temp;
```
Explanation:

In the first code snippet, web_url is an array of characters that is being dynamically allocated on the stack using the sprintf function. Its size is calculated based on the lengths of web_url_format and body_content. This array stores a modified version of the URL with the format specifier %s replaced by the value of body_content. web_url is an array of characters that can be modified.

web_url should be of type `static const char*`, a pointer to a string literal. String literals are typically stored in read-only memory, and web_url is pointing to that memory location. It is a read-only string literal.

The second code snippet turns web_url_temp into a string literal by assigning it to web_url.
