# MipsEmulator
A simple mips emulator I made for fun. do not rely on it to work properly.  
Due to it being difficult to find any straight forward documentation on how real mips binaries are formatted I decided on a simple formatting with only 2 sections.
2 sections, data and text.
Data section starts with the string ".data" and must come before .text section. After the signifier ".text" the binary data should come (strings must be little endian to display correctly)
.text section must start with the string .text  then the binary encoded instructions come

So input files should be formatted like this:  
.data\<**BINARY-ENCODED-DATA**\>.text\<**BINARY-ENCODED-INSTRUCTIONS**\>

Supports
- The first 32 registers (so no floating point instructions or special register instructions like mflo/mfhi)
- A subset of syscalls (printing numbers / strings, exiting the program, taking user input)
- a subset of instructions (most of the common instructions, no mflo/mfhi etc.)
- 512 byte stack memory
- 4096 byte of program memory
- 4096 bytes of data memory


There are probably bugs.
