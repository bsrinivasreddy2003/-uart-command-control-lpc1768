# 🖥️ UART-Based Command & Control System

**Microcontroller:** ARM Cortex-M3 — NXP LPC1768  
**IDE:** Keil µVision  
**Language:** Embedded C  
**Interface:** USB-to-UART (FTDI) + Serial Terminal  

---

## 📌 Project Overview

A **command-driven embedded firmware system** where a user sends text commands from a PC serial terminal, and the LPC1768 microcontroller receives, parses, and executes them in real time. The system supports GPIO control, ADC reading, and status monitoring — responding with feedback over UART.

This type of system is widely used in embedded debugging, firmware testing, IoT device configuration, and industrial machine interfaces.

---

## ⚙️ Hardware Components

| Component | Description |
|---|---|
| LPC1768 | ARM Cortex-M3 Microcontroller @ 100 MHz |
| USB-to-UART (FTDI) | PC ↔ MCU serial communication |
| LED + 220Ω resistor | GPIO output on P0.15 |
| LM35 Sensor (optional) | Analog temperature input on P0.23 (ADC) |
| Serial Terminal | RealTerm / PuTTY / Tera Term on PC |

---

## 🔌 Pin Configuration

| Pin | Function |
|---|---|
| P0.2 (TX) | UART0 Transmit → FTDI RX |
| P0.3 (RX) | UART0 Receive ← FTDI TX |
| P0.15 | LED output (GPIO) |
| P0.23 | ADC input (LM35 / analog sensor) |
| GND | Common ground |

---

## 💬 Supported Commands

| Command | Action | Response |
|---|---|---|
| `LED ON` | Sets P0.15 HIGH | `LED ON OK` |
| `LED OFF` | Sets P0.15 LOW | `LED OFF OK` |
| `READ ADC` | Reads 12-bit ADC value | `ADC: <value>` |
| `STATUS` | System health check | `SYSTEM OK` |
| (invalid) | Any unrecognized command | `INVALID CMD` |

---

## 🧠 Working Logic

```
1. Initialize UART0 (9600 baud), GPIO, ADC
2. Wait for incoming characters via UART (polling)
3. Store each character in buffer until '\r' (Enter) received
4. Null-terminate buffer → pass to execute_command()
5. Compare using strcmp() → execute matching function
6. Send response string back via UART
7. Clear buffer → wait for next command
```

---

## 🔧 Register-Level Configuration

### UART0 Initialization (9600 Baud @ 25MHz PCLK)
```c
LPC_SC->PCONP        |= (1<<3);          // Enable UART0 power
LPC_PINCON->PINSEL0  |= (1<<4)|(1<<6);  // P0.2=TXD0, P0.3=RXD0
LPC_UART0->LCR        = 0x83;            // 8-bit, no parity, DLAB=1
LPC_UART0->DLL        = 13;              // Baud divisor for 9600
LPC_UART0->LCR        = 0x03;            // Lock baud, DLAB=0
```

### UART Polling
```c
// Receive: wait for RDR bit (bit 0) in LSR
while(!(LPC_UART0->LSR & (1<<0)));
return LPC_UART0->RBR;

// Transmit: wait for THRE bit (bit 5) in LSR
while(!(LPC_UART0->LSR & (1<<5)));
LPC_UART0->THR = ch;
```

### ADC Reading
```c
LPC_ADC->ADCR = (1<<21) | (1<<0);   // Power ON, select CH0
LPC_ADC->ADCR |= (1<<24);            // Start conversion
while(!(LPC_ADC->ADGDR & (1<<31)));  // Wait for DONE bit
return (LPC_ADC->ADGDR >> 4) & 0xFFF; // Extract 12-bit result
```

---

## 💡 Key Features

- ✅ **Register-level UART configuration** — no library abstraction
- ✅ **String-based command parsing** using `strcmp()`
- ✅ **Bidirectional communication** — command in, response out
- ✅ **ADC integration** — real-time sensor data via command
- ✅ **Error handling** — invalid commands return `INVALID CMD`
- ✅ **Extensible design** — new commands added easily

---

## ⚠️ Known Limitations & Improvements

| Current | Improvement |
|---|---|
| Polling-based UART | Use UART interrupt (ISR) for efficiency |
| Fixed 20-char buffer | Add circular buffer for robustness |
| No command history | Add command queue |
| 9600 baud only | Configurable baud rate |
| No timeout handling | Add watchdog/timeout for stuck states |

---

## 📁 File Structure

```
uart-command-control-lpc1768/
│
├── main.c          # Main application + command execution logic
├── lpc17xx.h       # LPC1768 peripheral register definitions
└── README.md       # Project documentation
```

---

## 🚀 How to Run

1. Open **Keil µVision** → load project
2. Build (`F7`) → flash `.hex` via **Flash Magic**
3. Connect FTDI USB-UART to LPC1768 (TX↔RX, GND)
4. Open **RealTerm / PuTTY** → 9600 baud, COM port
5. Type commands and press **Enter**:

```
> LED ON
LED ON OK

> READ ADC
ADC: 2047

> STATUS
SYSTEM OK

> HELLO
INVALID CMD
```

---

## 🗣️ Interview Answer (Quick Reference)

> *"I developed a UART-based command and control system using LPC1768 where a user sends commands from a PC, and the microcontroller interprets and executes them. It supports GPIO control, ADC reading, and error handling. This type of system is widely used in embedded debugging and device configuration during development."*

---

## 👨‍💻 Author

**Bandi Srinivas Reddy**  
B.Tech ECE — Narasaraopeta Engineering College, JNTU Kakinada  
PG Diploma in Embedded Systems — IIES Bangalore  
📧 bvenkatareddy2003@gmail.com  
🔗 [LinkedIn](https://linkedin.com/in/srinivas-reddy-28479a267)  
🔗 [GitHub](https://github.com/bsrinivasreddy2003)

---

## 📜 License
This project is open source and available under the [MIT License](LICENSE).
