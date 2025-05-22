# Arduino CD Changer Emulator for 1J0035111

This project implements an Arduino-based emulator for the 1J0035111 CD changer, communicating over a custom SPI-like protocol. The emulator sends commands to control the CD changer (e.g., play, next track, change disk) and processes responses to decode and display status information, such as mode, track number, and playback time. The project includes an Arduino sketch (`CDC.ino`) tested on the Arduino Mega 2560, a logic analyzer capture (`LOGIC 2 Capture.sal`) for protocol analysis, and a wiring diagram (`Wiring.png`).

## Project Overview

The Arduino sketch (`CDC.ino`) is designed to:
- Send 4-byte commands to the 1J0035111 CD changer using bit-banged signaling on a designated output pin (`dataout`).
- Receive and process 8-byte response packets over SPI, decoding fields like CD number, track number, playback time, and changer state.
- Provide a serial interface for user interaction via a menu-driven system.
- Handle synchronization, error detection, and SPI restarts for reliable communication.

The logic analyzer capture (`LOGIC 2 Capture.sal`) contains recordings of the clock (`clk`) and data output (`DATA out`) signals. The `Wiring.png` file illustrates the connections between the Arduino Mega 2560 and the CD changer.

## Files

- **CDC.ino**: The main Arduino sketch for the CD changer emulator. It includes:
  - Command definitions (e.g., `PlayCMD`, `NextTrackCMD`).
  - SPI communication setup and interrupt-driven data processing.
  - A serial menu for sending commands.
  - Packet decoding and status printing for CD changer responses.
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
  - Refer to `Wiring.png` in the [Connections](#connections) section for detailed wiring instructions.
- **Logic Analyzer** (optional): For analyzing `LOGIC 2 Capture.sal` (e.g., Saleae Logic 2).

## Setup Instructions

1. **Install Arduino IDE**:
   - Download and install the [Arduino IDE](https://www.arduino.cc/en/software).
   - Ensure the `SPI` library is available (included by default).

2. **Connect Hardware**:
   - Wire the Arduino Mega 2560 to the 1J0035111 CD changer as shown in `Wiring.png`:
     - Pin 22 (`dataout`) to the CD changer's command input.
     - `MOSI` (pin 51) to the CD changer's data output.
     - `SCK` (pin 52) to the CD changer's clock line.
   - Ensure the CD changer is powered and connected.

3. **Load the Sketch**:
   - Open `CDC.ino` in the Arduino IDE.
   - Select `Arduino Mega 2560` under `Tools > Board` and the correct port under `Tools > Port`.
   - Upload the sketch.

4. **Interact with the Emulator**:
   - Open the Serial Monitor (`Tools > Serial Monitor`).
   - Set the baud rate to `250000`.
   - Use the menu to send commands (e.g., `2` for Power ON, `9` for Play).
   - View decoded CD changer status packets in the Serial Monitor.

5. **Analyze the Protocol (Optional)**:
   - Install [Saleae Logic 2](https://www.saleae.com/downloads/).
   - Open `LOGIC 2 Capture.sal` to analyze clock and data signals for timing and debugging.

## Connections

The following diagram illustrates the wiring between the Arduino Mega 2560 and the 1J0035111 CD changer:

![Wiring Diagram](images/Wiring.png)

Ensure all connections match the diagram to establish proper communication. The diagram details the connections for:
- `dataout` (pin 22) to the CD changer's Remote.
- `MOSI` (pin 51) to the CD changer's CD Data.
- `SCK` (pin 52) to the CD changer's CD Clock.

## Usage

- **Serial Menu**: After initialization, the Serial Monitor displays a menu. Enter a number (1–9) to execute commands:
  - `1`: Power OFF (init command).
  - `2`: Power ON.
  - `3`: Select Disc 1.
  - `4`: Select Disc 2.
  - `5`: Previous Track.
  - `6`: Next Track.
  - `7`: Scan.
  - `8`: Mix (next track and play).
  - `9`: Play.
- **Output**: The Serial Monitor shows:
  - Command execution confirmation (e.g., "Executing Play Command").
  - Decoded packets with mode, CD number, track number, time, and CDC state.
  - Debugging info (e.g., buffer contents) on errors.
- **Logic Analyzer**: Use `LOGIC 2 Capture.sal` to:
  - Verify bit-banged command timing (e.g., `Send0`, `Send1`).
  - Analyze SPI response packets.
  - Debug synchronization issues.
- **Wiring Reference**: Consult `Wiring.png` in the [Connections](#connections) section to verify connections.

## Protocol Details

- **Command Format**:
  - 4-byte sequences sent via bit-banging on `dataout`.
  - Preceded by `StartComm` and followed by `ACK`.
  - Timing: 560µs for `Send0`, 1680µs for `Send1`.
- **Response Packets**:
  - 8-byte packets via SPI.
  - Fields: Mode (`CDC_MODE_*`), CD Number, Track Number, Time (minutes/seconds), CDC State (`CDC_STATE_*`).
  - Decoded in `decodeAndPrintPacket`.
- **SPI Configuration**:
  - Mode 1 (CPOL=0, CPHA=1).
  - MSB-first, interrupt-driven.
  - Buffer: 32 bytes (4 packets).
  - Timeout: 5000ms for resync.

## Troubleshooting

- **No Response**:
  - Verify wiring using `Wiring.png`.
  - Check SPI pins (51, 52) and power to the CD changer.
  - Use the logic analyzer to confirm command timing.
- **Garbled Output**:
  - Set Serial Monitor baud rate to `250000`.
  - Check for buffer overflows via `printBuffer`.
- **Alignment Loss**:
  - The sketch restarts SPI on alignment loss or timeout.
  - Use `LOGIC 2 Capture.sal` for diagnosis.

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