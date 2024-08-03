# Real-time Executive (RTX)
This is a real-ime execuive (RTX) on the ARM Cortex M4 microprocessor. The specific chip that is being used is the STM32F401RE (or STM32F411RE), which has 96KB of RAM and 512KB of on-chip lash memory. 
The chip also contains an on-chip UART, JTAG debug interface accessed via the STLink uility, and several on-chip imers.
The RTX will provide a basic muli-programming environment that supports priority (via deadlines), pre- empion, and dynamic memory management. The RTX is designed for co-operaive, non-malicious sotfware.
