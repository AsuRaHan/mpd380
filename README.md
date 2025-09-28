# МПД-380 Modbus RTU Pressure Sensor Project

This project implements communication with the МПД-380 pressure sensor using the Modbus RTU protocol over an RS485-TTL converter. The code is designed for an ESP32 microcontroller and includes functionality to read and interpret sensor data, such as pressure, units, and configuration settings. This README provides an overview of the project, details from the Modbus RTU protocol map for the МПД-380, and instructions for setup and usage.

## Project Overview

The МПД-380 is a pressure transducer that supports the Modbus RTU protocol over an RS485 interface. This project uses an ESP32 to read data from the sensor via a simple RS485-TTL converter (with VCC, GND, TX, RX, A, B pins) and displays the results in the Serial Monitor. The code handles raw pressure values, converts them to meaningful units (e.g., bar), and provides diagnostic information.

### Features
- Reads pressure data from register 0x0004.
- Retrieves configuration settings (slave address, baud rate, units, decimal places, offset).
- Calculates pressure in the sensor's configured units.
- Handles Modbus errors with basic diagnostics.
- Compatible with a simple RS485-TTL converter without manual DE/RE control.

## Hardware Requirements
- **ESP32 Development Board**: Any model with UART2 support (e.g., ESP32-WROOM-32).
- **МПД-380 Pressure Sensor**: Configured with default slave address 1 and baud rate 9600.
- **RS485-TTL Converter**: A simple module with VCC, GND, TX, RX, A, B, GND pins (e.g., based on SP3485).
- **Wiring**:
  - Converter A → Sensor A
  - Converter B → Sensor B
  - Converter GND → Sensor GND
  - Converter TX → ESP32 GPIO16 (RX2)
  - Converter RX → ESP32 GPIO17 (TX2)
  - Converter VCC → ESP32 3.3V or 5V (check converter specs)
  - Sensor power: 12–24V (separate supply)
- **USB Cable**: For programming and Serial Monitor.

## Software Requirements
- **Arduino IDE**: With ESP32 board support installed.
- **ModbusMaster Library**: Install via Library Manager (search for "ModbusMaster" by Doc Walker).
- **Serial Terminal**: For PC testing (e.g., Hercules or Tera Term).

## Modbus RTU Protocol Details (Based on МПД-380 Protocol Map)

The МПД-380 uses the Modbus RTU protocol over RS485 with the following specifications:

### General Settings
- **Protocol**: Modbus RTU, half-duplex RS485.
- **Serial Port Configuration**: 9600 baud, 8 data bits, no parity, 1 stop bit (8N1). Supported baud rates: 1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200.
- **CRC Polynomial**: 0xA001.
- **Data Format**: Signed 16-bit integers (-32768 to 32767). Floating-point values require decimal place adjustment (read from register 0x0003).

### Command Formats
1. **Read Command (Function 0x03)**:
   - Request: `[Slave Address] [0x03] [High Start Address] [Low Start Address] [High Data Count] [Low Data Count] [CRC16 Low] [CRC16 High]`
     - Example: `01 03 00 04 00 01 C4 0B` (reads register 0x0004, 1 word).
   - Response: `[Slave Address] [0x03] [Byte Count] [High Data] [Low Data] [CRC16 Low] [CRC16 High]`
     - Example: `01 03 02 00 12 79 84` (data 0x0012 = 18).

2. **Write Command (Function 0x06)**:
   - Request: `[Slave Address] [0x06] [High Start Address] [Low Start Address] [High Data] [Low Data] [CRC16 Low] [CRC16 High]`
     - Example: `01 06 00 00 00 02 08 0B` (sets slave address to 2).
   - Response: Same as request if successful.

3. **Error Response**:
   - `[Slave Address] [0x80 + Function] [Exception Code] [CRC16 Low] [CRC16 High]`
     - Exception Codes: 0x01 (illegal function), 0x02 (illegal address), 0x03 (illegal data).

### Register Map
| Register Address | Quantity | Bytes | Data Range       | Description                              |
|-------------------|----------|-------|------------------|------------------------------------------|
| 0x0000           | 1        | 2     | 1–255            | Slave address                            |
| 0x0001           | 1        | 2     | 0–7              | Baud rate (0=1200, 3=9600, 7=115200)     |
| 0x0002           | 1        | 2     | 0–9              | Pressure units (3=bar, 6=Psi, etc.)      |
| 0x0003           | 1        | 2     | 0–4              | Decimal places (0=none, 2=0.01)          |
| 0x0004           | 1        | 2     | -32768–32767     | Raw pressure value                       |
| 0x0005           | 1        | 2     | -32768–32767     | Zero range (minimum pressure)            |
| 0x0006           | 1        | 2     | -32768–32767     | Full range (maximum pressure, e.g., 400) |
| 0x000C           | 1        | 2     | -32768–32767     | Offset (usually 0)                       |
| 0x000F           | 1        | 2     | 0                | Save to user settings                    |
| 0x0010           | 1        | 2     | 1                | Restore factory settings                 |

- **Pressure Calculation**: `pressure = (raw_pressure + offset) / 10^decimal_places`.
- **User Restrictions**: Only slave address (0x0000), baud rate (0x0001), and offset (0x000C) can be modified without manufacturer software.

### Notes
- Data is transmitted as raw 16-bit signed integers. For floating-point values (e.g., 6.000), read decimal places and divide accordingly.
- Changes to address or baud rate take effect after the response.
- Factory reset (0x0010) may require re-detecting the sensor.

## Installation

1. **Install Arduino IDE** and add ESP32 board support (via Boards Manager, search "esp32").
2. **Install ModbusMaster Library**:
   - Open Arduino IDE, go to Sketch > Include Library > Manage Libraries.
   - Search for "ModbusMaster" and install.
3. **Connect Hardware** as described above.
4. **Upload Sketch**:
   - Open `main.ino` in the Arduino IDE.
   - Select your ESP32 board and port.
   - Upload the code.

## Usage

1. **Open Serial Monitor**:
   - Set baud rate to 115200.
   - You will see output like:
2. **Test the Sensor**:
   - Apply pressure to the sensor (e.g., blow into it).
   - The pressure value should increase (e.g., 0.22 бар).
3. **Error Handling**:
   - If an error occurs (e.g., `Ошибка Modbus: 0xE0 - Таймаут`), check wiring, power, or baud rate.

## Sketch Details (`main.ino`)

### Code Structure
- **Libraries**: Uses `ModbusMaster.h` for Modbus RTU communication.
- **Pin Definitions**:
- `RX_PIN = 16` (UART2 RX).
- `TX_PIN = 17` (UART2 TX).
- **Setup**:
- Initializes Serial (115200) and Serial2 (9600, 8N1) for Modbus.
- Configures the ModbusMaster with slave address 1.
- **Loop**:
- Reads 13 holding registers (0x0000–0x000C) every 500 ms.
- Extracts slave address, baud code, units, decimal places, raw pressure, and offset.
- Calculates pressure and prints results.
- Handles Modbus errors with codes like 0xE0 (timeout), 0x83 (illegal address).

### Functions
- `getUnitString(uint8_t unit_code)`: Converts unit code (0x0002) to a string (e.g., "бар" for 3).
- `getBaudRate(uint8_t baud_code)`: Converts baud code (0x0001) to a string (e.g., "9600" for 3).

### Improvements
- Combines all register reads into one request for efficiency.
- Checks for negative pressure values and warns if detected.
- Adapts pressure output format based on decimal places.

## Troubleshooting
- **No Data**: Verify COM port, A/B wiring, GND connection, and sensor power (12–24V).
- **Wrong Units**: Check register 0x0002 (should be 3 for bar) and adjust if needed.
- **Errors**: Use the error codes in the Serial Monitor to diagnose (e.g., 0xE0 = timeout).

## Future Enhancements
- Add support for writing to registers (e.g., change address or baud rate with function 0x06).
- Implement a PC application (e.g., in C++) for real-time pressure monitoring.
- Add calibration or logging features.

## License
This project is open-source. Feel free to modify and distribute under the MIT License.

## Acknowledgments
- Thanks to the xAI community for support and inspiration.
- Documentation based on the "Карта протокола Modbus RTU - МПД380.pdf" provided by the user.