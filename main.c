/*
 * Project  : UART-Based Command & Control System
 * MCU      : NXP LPC1768 (ARM Cortex-M3)
 * IDE      : Keil uVision
 * Author   : Bandi Srinivas Reddy
 * Email    : bvenkatareddy2003@gmail.com
 * GitHub   : https://github.com/bsrinivasreddy2003
 *
 * Description:
 *   A command-driven embedded firmware system.
 *   User sends commands from PC via UART serial terminal.
 *   MCU receives, parses, executes commands and sends response back.
 *
 * Supported Commands:
 *   LED ON    -> Turns ON LED on P0.15  -> Response: "LED ON OK"
 *   LED OFF   -> Turns OFF LED on P0.15 -> Response: "LED OFF OK"
 *   READ ADC  -> Reads ADC CH0 (P0.23)  -> Response: "ADC: <value>"
 *   STATUS    -> System health check    -> Response: "SYSTEM OK"
 *   (other)   -> Invalid command        -> Response: "INVALID CMD"
 *
 * Hardware Connections:
 *   P0.2 (TXD0) -> FTDI RX
 *   P0.3 (RXD0) -> FTDI TX
 *   P0.15       -> LED (via 220 ohm resistor to GND)
 *   P0.23       -> LM35 / Analog sensor (ADC CH0)
 *   GND         -> FTDI GND
 *
 * UART Settings: 9600 baud, 8 data bits, no parity, 1 stop bit
 */

#include <lpc17xx.h>
#include <string.h>
#include <stdio.h>

/* -------------------------------------------------------
 * DEFINES
 * ------------------------------------------------------- */
#define LED_PIN     (1 << 15)   /* P0.15 - LED output */
#define MAX_CMD_LEN  20          /* Maximum command buffer size */

/* -------------------------------------------------------
 * FUNCTION PROTOTYPES
 * ------------------------------------------------------- */
void uart_init(void);
char uart_receive(void);
void uart_send(char ch);
void uart_print(char *str);
int  adc_read(void);
void execute_command(char *cmd);

/* -------------------------------------------------------
 * MAIN FUNCTION
 * ------------------------------------------------------- */
int main(void)
{
    char cmd[MAX_CMD_LEN];  /* Command buffer */
    int  i = 0;             /* Buffer index   */
    char ch;

    /* Initialize peripherals */
    uart_init();

    /* Configure P0.15 as GPIO output for LED */
    LPC_GPIO0->FIODIR |= LED_PIN;
    LPC_GPIO0->FIOCLR  = LED_PIN;   /* LED OFF initially */

    /* Welcome message */
    uart_print("=== UART Command Control System ===\r\n");
    uart_print("Commands: LED ON | LED OFF | READ ADC | STATUS\r\n");
    uart_print("Ready...\r\n");

    /* Main loop */
    while (1)
    {
        ch = uart_receive();    /* Wait and receive one character */

        /* Echo character back to terminal */
        uart_send(ch);

        if (ch == '\r')         /* Enter key pressed = command complete */
        {
            cmd[i] = '\0';      /* Null-terminate the command string */
            uart_print("\r\n"); /* New line on terminal */
            execute_command(cmd); /* Parse and execute */
            i = 0;              /* Reset buffer index */
        }
        else if (ch == '\b' && i > 0)  /* Backspace handling */
        {
            i--;                /* Remove last character from buffer */
        }
        else if (i < MAX_CMD_LEN - 1)  /* Guard against buffer overflow */
        {
            cmd[i++] = ch;      /* Store character in buffer */
        }
    }
}

/* -------------------------------------------------------
 * UART INITIALIZATION
 * Configure UART0 for 9600 baud @ 25MHz PCLK
 * ------------------------------------------------------- */
void uart_init(void)
{
    /* Step 1: Enable power to UART0 */
    LPC_SC->PCONP |= (1 << 3);

    /* Step 2: Configure pins P0.2 = TXD0, P0.3 = RXD0 */
    LPC_PINCON->PINSEL0 |= (1 << 4) | (1 << 6);

    /* Step 3: Set 8-bit data, no parity, 1 stop bit, DLAB=1 (to set baud) */
    LPC_UART0->LCR = 0x83;

    /* Step 4: Set baud rate divisor for 9600 baud
     * Formula: DLL = PCLK / (16 x BaudRate) = 25MHz / (16 x 9600) = ~163
     * Using DLL = 163 for standard 9600 baud */
    LPC_UART0->DLL = 163;
    LPC_UART0->DLM = 0;

    /* Step 5: Lock baud rate, clear DLAB */
    LPC_UART0->LCR = 0x03;

    /* Step 6: Enable and reset FIFOs */
    LPC_UART0->FCR = 0x07;
}

/* -------------------------------------------------------
 * UART RECEIVE
 * Waits until a character is received and returns it
 * ------------------------------------------------------- */
char uart_receive(void)
{
    /* Wait until Receiver Data Ready (RDR) bit is set in LSR */
    while (!(LPC_UART0->LSR & (1 << 0)));
    return (char)(LPC_UART0->RBR);  /* Return received byte */
}

/* -------------------------------------------------------
 * UART SEND (single character)
 * Waits until transmit buffer is empty, then sends character
 * ------------------------------------------------------- */
void uart_send(char ch)
{
    /* Wait until Transmitter Holding Register Empty (THRE) bit is set */
    while (!(LPC_UART0->LSR & (1 << 5)));
    LPC_UART0->THR = ch;    /* Load character into Transmit Holding Register */
}

/* -------------------------------------------------------
 * UART PRINT (string)
 * Sends a null-terminated string character by character
 * ------------------------------------------------------- */
void uart_print(char *str)
{
    while (*str)            /* Loop until null terminator */
    {
        uart_send(*str++);  /* Send each character */
    }
}

/* -------------------------------------------------------
 * ADC READ
 * Reads 12-bit ADC value from Channel 0 (P0.23)
 * Returns: 0 to 4095
 * ------------------------------------------------------- */
int adc_read(void)
{
    /* Enable ADC power */
    LPC_SC->PCONP |= (1 << 12);

    /* Configure P0.23 as ADC input (AD0.0) */
    LPC_PINCON->PINSEL1 |= (1 << 14);

    /* Select CH0, set clock divider, power ON ADC */
    LPC_ADC->ADCR = (1 << 0)   /* Select CH0       */
                  | (4 << 8)   /* Clock divider = 4 */
                  | (1 << 21); /* ADC Power ON      */

    /* Start conversion (bit 24 = START NOW) */
    LPC_ADC->ADCR |= (1 << 24);

    /* Wait for conversion complete (DONE bit 31 in ADGDR) */
    while (!(LPC_ADC->ADGDR & (1 << 31)));

    /* Extract 12-bit result (bits 4:15) and return */
    return (int)((LPC_ADC->ADGDR >> 4) & 0xFFF);
}

/* -------------------------------------------------------
 * EXECUTE COMMAND
 * Compares received command string and executes action
 * ------------------------------------------------------- */
void execute_command(char *cmd)
{
    char buffer[30];    /* Buffer for formatted response strings */
    int  adc_val;

    if (strcmp(cmd, "LED ON") == 0)
    {
        LPC_GPIO0->FIOSET = LED_PIN;        /* Set P0.15 HIGH -> LED ON  */
        uart_print("LED ON OK\r\n");
    }
    else if (strcmp(cmd, "LED OFF") == 0)
    {
        LPC_GPIO0->FIOCLR = LED_PIN;        /* Set P0.15 LOW -> LED OFF  */
        uart_print("LED OFF OK\r\n");
    }
    else if (strcmp(cmd, "READ ADC") == 0)
    {
        adc_val = adc_read();               /* Read 12-bit ADC value     */
        sprintf(buffer, "ADC: %d\r\n", adc_val);
        uart_print(buffer);
    }
    else if (strcmp(cmd, "STATUS") == 0)
    {
        uart_print("SYSTEM OK\r\n");
    }
    else if (cmd[0] == '\0')
    {
        /* Empty command — do nothing, just show prompt */
    }
    else
    {
        uart_print("INVALID CMD\r\n");      /* Unknown command response   */
    }

    uart_print("> ");   /* Show prompt for next command */
}
