# CD Changer Controller

This project provides an Arduino-based solution for controlling a CD changer device using serial commands. The code sends specific 4-byte commands to the CD changer to perform actions such as initializing the device, powering it on, selecting discs, playing tracks, and more.

## Table of Contents
- [Features](#features)
- [Hardware Requirements](#hardware-requirements)
- [Software Requirements](#software-requirements)
- [Installation](#installation)
- [Usage](#usage)
- [Command Reference](#command-reference)
- [File Descriptions](#file-descriptions)
- [Contributing](#contributing)
- [License](#license)

## Features
- Initialize and de-initialize the CD changer.
- Power on the device.
- Select between different discs (e.g., Disc 1 or Disc 2).
- Navigate tracks (next/previous).
- Play, scan, and mix tracks.
- Interactive serial monitor menu for easy command execution.
- Bit-level communication protocol for sending commands to the CD changer.

## Hardware Requirements
- Arduino board (e.g., Arduino Uno, Mega, etc.). (TESTED ON MEGA2560)
- CD changer device compatible with the command protocol. (TESTED ON 1J0035111)
- Connection wire from Arduino pin 22 (data output) to the CD changer's input (brown wire Pin4 of CDC Connector).
- USB cable for Arduino programming and serial communication.

## Software Requirements
- [Arduino IDE](https://www.arduino.cc/en/software) (or compatible IDE).
- Serial monitor for interacting with the Arduino (included in Arduino IDE).

## Installation
1. **Clone the Repository**:
   ```bash
   git clone https://github.com/yourusername/cd-changer-controller.git
   ```
2. **Open the Project**:
   - Launch the Arduino IDE.
   - Open the `CDC.ino` file from the cloned repository.
3. **Connect the Arduino**:
   - Connect your Arduino board to your computer via USB.
   - Ensure the correct board and port are selected in the Arduino IDE (Tools > Board and Tools > Port).
4. **Upload the Code**:
   - Click the "Upload" button in the Arduino IDE to compile and upload the code to the Arduino.
5. **Open the Serial Monitor**:
   - In the Arduino IDE, go to Tools > Serial Monitor.
   - Set the baud rate to `9600` and ensure the line ending is set to "Newline".

## Usage
1. **Initial Setup**:
   - Upon uploading the code, the Arduino automatically runs the `Init()` and `powerOn()` functions to initialize and power on the CD changer.
   - After a 10-second delay, the serial monitor displays a command menu.
2. **Interacting with the Menu**:
   - The serial monitor shows a menu with options (1–9) corresponding to different commands.
   - Enter a number (e.g., `1` for Initialize, `9` for Play) and press Enter.
   - The Arduino executes the selected command and redisplays the menu.
3. **Example**:
   - To play a track, type `9` in the serial monitor and press Enter.
   - To switch to Disc 2, type `4` and press Enter.

## Command Reference
The following commands are supported, as defined in `commands sniff.txt` and implemented in `CDC.ino`:

| Command         | Description                              | Byte Sequence (Hex)       | Timing              |
|-----------------|------------------------------------------|---------------------------|---------------------|
| Init            | Initializes the CD changer              | `F7 08 34 CA`, `D7 28 34 CA` | 144ms, 6500ms, 144ms |
| PowerOn         | Powers on the CD changer                | `D8 27 34 CA`, `D7 28 34 CA` | 93ms intervals      |
| Change Disk 1   | Selects Disc 1                          | `CF 31 34 CA`, `D7 28 34 CA`, `E3 1C 34 CA` | 93ms intervals |
| Change Disk 2   | Selects Disc 2                          | `CE 30 34 CA`, `D7 28 34 CA`, `E3 1C 34 CA` | 93ms intervals |
| Next Track      | Plays the next track                    | `E0 1F 34 CA`, `D7 28 34 CA` | 93ms intervals      |
| Previous Track  | Plays the previous track                | `E1 1E 34 CA`, `D7 28 34 CA` | 93ms intervals      |
| Scan            | Scans tracks (untested)                 | `FA 05 34 CA`, `D7 28 34 CA` | 93ms intervals      |
| Mix             | Plays next track and plays (untested)   | `E0 1F 34 CA`, `F9 06 34 CA` | 93ms intervals      |
| Play            | Plays the current track                 | `F9 06 34 CA`, `D7 28 34 CA` | 93ms intervals      |
| DeInit          | De-initializes (same as Init)           | `F7 08 34 CA`, `D7 28 34 CA` | 93ms intervals      |

**Note**: Some commands (e.g., Scan, Mix, Next/Prev Track) are marked as untested in the code and may require further validation.

## File Descriptions
- **CDC.ino**: The main Arduino sketch that defines the command sequences, communication protocol, and serial menu interface.
- **commands sniff.txt**: A reference file containing the sniffed command sequences and their timings, used to develop the Arduino code.

## Contributing
Contributions are welcome! Please follow these steps:
1. Fork the repository.
2. Create a new branch (`git checkout -b feature/your-feature`).
3. Make your changes and commit (`git commit -m "Add your feature"`).
4. Push to the branch (`git push origin feature/your-feature`).
5. Open a pull request.

Please ensure your code follows the existing style and includes appropriate comments.

## License
This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.
