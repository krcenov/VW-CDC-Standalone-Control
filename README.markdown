# Arduino Multimedia Emulator for CD Changer 1J0035111

This project implements an Arduino-based emulator for the 1J0035111 CD changer, communicating over a custom SPI-like protocol. The emulator sends commands to control the CD changer (e.g., play, next track, change disk) and processes responses to decode and display status information, such as mode, track number, and playback time. The project includes an Arduino sketch (`CDC.ino`) tested on the Arduino Mega 2560, a logic analyzer capture (`LOGIC 2 Capture.sal`) for protocol analysis, and a wiring diagram (`Wiring.png`).

## Project Overview

The Arduino sketch (`CDC.ino`) is designed to:
- Send 4-byte commands to the 1J0035111 CD changer using bit-banged signaling on pin 22 (`dataout`).
- Receive and process 8-byte response packets over SPI, decoding fields like CD number, track number, playback time, and changer state.
- Provide a serial interface for user interaction via a menu-driven system (baud rate: 250000).
- Handle synchronization, error detection, and SPI restarts for reliable communication.
- Support optional LCD display output (disabled by default) for status display.

The logic analyzer capture (`LOGIC 2 Capture.sal`) contains recordings of the clock (`clk`) and data output (`DATA out`) signals. The `Wiring.png` file illustrates the connections between the Arduino Mega 2560 and the CD changer.

## Files

- **CDC.ino**: The main Arduino sketch for the CD changer emulator. It includes:
  - Command definitions (e.g., `PlayCMD`, `NextTrackCMD`, `PowerOnCMD`).
  - SPI communication setup (Mode 1, MSB-first, interrupt-driven).
  - A serial menu for sending commands (options 1–9).
  - Packet decoding and status printing for CD changer responses.
  - Optional LCD support via `LiquidCrystal_I2C` (disabled by default).
- **LOGIC 2 Capture.sal**: A Saleae Logic 2 capture file with clock and data output signals, for protocol analysis using Saleae Logic software.
- **Wiring.png**: A diagram showing the wiring connections between the Arduino Mega 2560 and the 1J0035111 CD changer.

## Hardware Requirements

- **Arduino Mega 2560**: Tested and confirmed compatible.
- **CD Changer**: Model 1J0035111.
- **Wiring**:
  - `dataout` (pin 22): Connect to the CD changer's command input.
  - SPI pins:
    - `MOSI` (pin 51): Input to Arduino from CD changer.
    - `SCK` (pin 52): Clock input from CD changer.
    - `MISO` (pin 50): Configured as output (not used for data).
  - Refer to `Wiring.png` for detailed connection instructions.
- **Logic Analyzer** (optional): For analyzing `LOGIC 2 Capture.sal` (e.g., Saleae Logic 2).
- **LCD Display** (optional): I2C LCD (address 0x3F, 8x2) for status display, enabled by setting `LcdPresent` to 1.

## Connections

The following diagram illustrates the wiring between the Arduino Mega 2560 and the 1J0035111 CD changer:

![Wiring Diagram](images/Wiring.png)

Ensure all connections match the diagram to establish proper communication. The diagram details the connections for:
- `dataout` (pin 22) to the CD changer's Remote.
- `MOSI` (pin 51) to the CD changer's CD Data.
- `SCK` (pin 52) to the CD changer's CD Clock.

## Setup Instructions

1. **Install Arduino IDE**:
   - Download and install the [Arduino IDE](https://www.arduino.cc/en/software).
   - Ensure the `SPI` library is available (included by default).
   - If using LCD, install the `LiquidCrystal_I2C` library via the Library Manager.

2. **Connect Hardware**:
   - Wire the Arduino Mega 2560 to the 1J0035111 CD changer as shown in `Wiring.png`:
     - Pin 22 (`dataout`) to the CD changer's command input.
     - `MOSI` (pin 51) to the CD changer's data output.
     - `SCK` (pin 52) to the CD changer's clock line.
     - `MISO` (pin 50) configured as output (no connection needed).
   - Ensure the CD changer is powered and connected.
   - If using LCD, connect to I2C pins (SDA: pin 20, SCL: pin 21) and set `LcdPresent` to 1 in `CDC.ino`.

3. **Load the Sketch**:
   - Open `CDC.ino` in the Arduino IDE.
   - Select `Arduino Mega 2560` under `Tools > Board` and the correct port under `Tools > Port`.
   - Upload the sketch.

4. **Interact with the Emulator**:
   - Open the Serial Monitor (`Tools > Serial Monitor`).
   - Set the baud rate to `250000`.
   - Use the menu to send commands (e.g., `2` for Power ON, `9` for Play).
   - View decoded CD changer status packets in the Serial Monitor (or on LCD if enabled).

5. **Analyze the Protocol (Optional)**:
   - Install [Saleae Logic 2](https://www.saleae.com/downloads/).
   - Open `LOGIC 2 Capture.sal` to analyze clock and data signals for timing and debugging.

## Usage

- **Serial Menu**: After initialization, the Serial Monitor displays a menu. Enter a number (1–9) to execute commands:
  - `1`: Power OFF (init command).
  - `2`: Power ON (`PowerOnCMD` sent twice with `ACK`).
  - `3`: NextDisk (`NextDiskCMD` and `SelectDiskCMD`).
  - `4`: Previous Track (`PrevTrackCMD`).
  - `5`: Next Track (`NextTrackCMD`).
  - `6`: Scan (`ScanCMD`).
  - `7`: Mix (next track and play, `NextTrackCMD` + `PlayCMD`).
  - `8`: Play (`PlayCMD`).
- **Output**: The Serial Monitor shows:
  - Command execution confirmation (e.g., "Executing Play Command").
  - Decoded packets with mode (`CDC_MODE_*`), CD number, track number, time (minutes/seconds), and CDC state (`CDC_STATE_*`) when `debug` is enabled.
  - Debugging info (e.g., buffer contents) on errors when `debug` is enabled.
  - If LCD is enabled (`LcdPresent=1`), displays "CD# Song## MM:SS" during playback.
- **Logic Analyzer**: Use `LOGIC 2 Capture.sal` to:
  - Verify bit-banged command timing (e.g., `Send0`: 560µs, `Send1`: 1680µs).
  - Analyze SPI response packets.
  - Debug synchronization issues.
- **Wiring Reference**: Refer to `Wiring.png` in the [Connections](#connections) section for detailed wiring instructions.

## Protocol Details

- **Command Format**:
  - 4-byte sequences sent via bit-banging on `dataout` (pin 22).
  - Preceded by `StartComm` (HIGH 9ms, LOW 5ms, HIGH 560µs) and followed by `ACK`.
  - Timing: 560µs for `Send0`, 1680µs for `Send1`.
  - Example commands: `PlayCMD` = `{0xCA, 0x34, 0x06, 0xF9}`, `NextTrackCMD` = `{0xCA, 0x34, 0x1F, 0xE0}`.
- **Response Packets**:
  - 8-byte packets via SPI.
  - Fields: Mode (`CDC_MODE_*`), CD Number, Track Number, Time (minutes/seconds), CDC State (`CDC_STATE_*`), and additional frames.
  - Time decoded from inverted bytes (e.g., `~packet[3]` for minutes).
  - Decoded in `decodeAndPrintPacket`.
- **SPI Configuration**:
  - Mode 1 (CPOL=0, CPHA=1).
  - MSB-first, interrupt-driven (`SPI_STC_vect`).
  - Buffer: 32 bytes (4 packets).
  - Timeout: 5000ms for resync.
  - Automatic SPI restart on alignment loss or timeout.

## Troubleshooting

- **No Response**:
  - Verify wiring using `Wiring.png`.
  - Check SPI pins (50, 51, 52) and power to the CD changer.
  - Use the logic analyzer to confirm command timing (`LOGIC 2 Capture.sal`).
- **Garbled Output**:
  - Set Serial Monitor baud rate to `250000`.
  - Enable `debug` mode (`debug=1`) and check for buffer overflows via `printBuffer`.
- **Alignment Loss**:
  - The sketch restarts SPI on alignment loss or timeout (5000ms).
  - Enable `debug` mode to log alignment issues.
  - Use `LOGIC 2 Capture.sal` for diagnosis.
- **LCD Issues**:
  - Ensure `LiquidCrystal_I2C` library is installed.
  - Verify I2C address (0x3F) and connections (SDA, SCL).
  - Set `LcdPresent=1` in `CDC.ino`.

## Contributing

Contributions are welcome! To contribute:
1. Fork the repository.
2. Create a feature branch (`git checkout -b feature/YourFeature`).
3. Commit changes (`git commit -m "Add YourFeature"`).
4. Push to the branch (`git push origin feature/YourFeature`).
5. Open a pull request.

Include:
- A description of changes.
- Updates to `LOGIC 2 Capture.sal` or `Wiring.png` if applicable.
- Validation steps for new features.

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.

## Acknowledgments

- Tested with the 1J0035111 CD changer.
- Developed for the Arduino Mega 2560.
- Uses Saleae Logic 2 for protocol analysis.
- Optional LCD support based on `LiquidCrystal_I2C` library.