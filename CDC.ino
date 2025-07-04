#define debug 0
#define LcdPresent 0

#if LcdPresent
  #include <LiquidCrystal_I2C.h>
  LiquidCrystal_I2C lcd(0x3F,8,2);
#endif

#include <SPI.h>
byte mode = 0;
byte cdNum = 0;
byte trackNum = 0;
byte cdcState = 0;
uint8_t minutes = 0;
uint8_t seconds = 0;


// Pin definitions
const int dataout = 22; // Output pin for command bit-banging

// SPI buffer and packet settings
#define PACKET_SIZE 8     // Packet size in bytes
#define BUFFER_SIZE (PACKET_SIZE * 4) // Larger buffer for better recovery
#define TIMEOUT_MS 5000   // Timeout for resync

// Command definitions (4-byte commands)
byte ScanCMD[] =        {0xCA, 0x34, 0x05, 0xFA};
byte PlayCMD[] =        {0xCA, 0x34, 0x06, 0xF9};
byte initCMD[] =        {0xCA, 0x34, 0x08, 0xF7};
byte SelectDiskCMD[] =     {0xCA, 0x34, 0x1C, 0xE3};
byte PrevTrackCMD[] =   {0xCA, 0x34, 0x1E, 0xE1};
byte NextTrackCMD[] =   {0xCA, 0x34, 0x1F, 0xE0};
byte PowerOnCMD[] =     {0xCA, 0x34, 0x27, 0xD8};
byte ACK[] =            {0xCA, 0x34, 0x28, 0xD7};
//byte ChangeDisk2[] =    {0xCA, 0x34, 0x30, 0xCE};
byte NextDiskCMD[] =    {0xCA, 0x34, 0x31, 0xCF};

const int CMDSize = 4;

// SPI buffer and state
volatile byte buffer[BUFFER_SIZE];
volatile byte writeIndex = 0;
byte readIndex = 0;
bool aligned = false;
unsigned long lastValidPacket = 0;
byte lastPacket[PACKET_SIZE];

// CDC mode definitions
typedef enum {
  CDC_MODE_IDLE = 0xF4,
  CDC_MODE_ACK = 0x94,
  CDC_MODE_BUSY = 0xB4,
  CDC_MODE_PLAY = 0x34
} cdcMode_t;

// CDC state definitions
typedef enum {
  CDC_STATE_NOT_INITIALIZED = 0x8F,
  CDC_STATE_BUSY = 0xAF,
  CDC_STATE_FAST_FW_BW = 0xBF,
  CDC_STATE_MUTE = 0xDF,
  CDC_STATE_PLAYING = 0xCF,
  CDC_STATE_INITIALIZED = 0xEF,
  CDC_STATE_SENDING_CD_INFO = 0xFF
} cdcState_t;

// === SPI Restart Function ===
void restartSPI() {
    // Disable SPI
    SPCR &= ~(1 << SPE);
    
    // Clear SPI interrupt flag
    SPSR;
    SPDR;
    
    // Reset buffer and indices
    memset(buffer, 0, BUFFER_SIZE);
    writeIndex = 0;
    readIndex = 0;
    aligned = false;
    
    // Reconfigure SPI (same settings as in setup())
    pinMode(MISO, OUTPUT);
    pinMode(MOSI, INPUT);
    pinMode(SCK, INPUT);
    SPCR &= ~(1 << CPOL); // SPI Mode 1: CPOL=0
    SPCR |= (1 << CPHA);  // CPHA=1
    SPCR &= ~(1 << DORD); // MSB-first
    SPCR |= (1 << SPE);   // Enable SPI
    SPCR |= (1 << SPIE);  // Enable SPI interrupts
    
    // Initialize SPI transfer
    SPI.transfer(0);
  #if debug
    Serial.println("SPI restarted due to sync loss");
  #endif
}

// === Command Sending Functions ===
void StartComm() {
  digitalWrite(dataout, HIGH);
  delay(9);
  digitalWrite(dataout, LOW);
  delay(5);
  digitalWrite(dataout, HIGH);
  delayMicroseconds(560);
}

void Send0() {
  digitalWrite(dataout, LOW);
  delayMicroseconds(560);
  digitalWrite(dataout, HIGH);
  delayMicroseconds(560);
}

void Send1() {
  digitalWrite(dataout, LOW);
  delayMicroseconds(1680);
  digitalWrite(dataout, HIGH);
  delayMicroseconds(560);
}

void send4CMD(uint8_t* cmd) {
  StartComm();
  for (size_t i = 0; i < CMDSize; i++) {
    for (int j = 0; j < 8; j++) {
      if (cmd[i] & (1 << j)) {
        Send1();
      } else {
        Send0();
      }
    }
  }
  digitalWrite(dataout, LOW);
}

void InitCmd() {
  send4CMD(initCMD);
  delay(135);
  send4CMD(ACK);
}

void Init() {
  Serial.println("Executing Init Command");
  InitCmd();
  delay(6500);
  InitCmd();
}

void powerOn() {
  Serial.println("Executing PowerOn Command");
  send4CMD(PowerOnCMD);
  delay(93);
  send4CMD(ACK);
  delay(93);
  send4CMD(PowerOnCMD);
  delay(93);
  send4CMD(ACK);
}

void NextDisk() {
  Serial.println("Executing ChangeDisk1 Command");
  send4CMD(NextDiskCMD);
  delay(93);
  send4CMD(ACK);
  delay(93);
  send4CMD(SelectDiskCMD);
  delay(93);
  send4CMD(ACK);
  delay(93);
}

// void SelectDisc2() {
//   Serial.println("Executing ChangeDisk2 Command");
//   send4CMD(ChangeDisk2);
//   delay(93);
//   send4CMD(ACK);
//   delay(93);
//   send4CMD(SelectDiskCMD);
//   delay(93);
//   send4CMD(ACK);
//   delay(93);
// }

void NextTrack() {
  Serial.println("Executing NextTrack Command");
  send4CMD(NextTrackCMD);
  delay(93);
  send4CMD(ACK);
  delay(93);
}

void PrevTrack() {
  Serial.println("Executing PrevTrack Command");
  send4CMD(PrevTrackCMD);
  delay(93);
  send4CMD(ACK);
  delay(93);
}

void Scan() {
  Serial.println("Executing Scan Command");
  send4CMD(ScanCMD);
  delay(93);
  send4CMD(ACK);
  delay(93);
}

void Mix() {
  Serial.println("Executing Mix Command (NextTrack)");
  NextTrack();
  send4CMD(PlayCMD);
  delay(93);
  send4CMD(ACK);
  delay(93);
}

void Play() {
  Serial.println("Executing Play Command");
  send4CMD(PlayCMD);
  delay(93);
  send4CMD(ACK);
  delay(93);
}

// === SPI Data Processing Functions ===
ISR(SPI_STC_vect) {
  buffer[writeIndex] = SPDR;
  writeIndex = (writeIndex + 1) % BUFFER_SIZE;
}

bool isValidMode(byte b) {
  return (b == CDC_MODE_IDLE || b == CDC_MODE_ACK || 
          b == CDC_MODE_BUSY || b == CDC_MODE_PLAY);
}
uint8_t decodeTime(uint8_t hexValue) {
  // Check if hexValue is in the range 0x01 to 0x59
  if (hexValue >= 0x01 && hexValue <= 0x59) {
    // Convert hex to decimal representation of the hex digits
    // E.g., 0x58 = (5 * 10) + 8 = 58 decimal
    uint8_t tens = (hexValue >> 4) & 0x0F; // Get high nibble (5 in 0x58)
    uint8_t ones = hexValue & 0x0F;       // Get low nibble (8 in 0x58)
    return (tens * 10) + ones;            // Combine as decimal (5*10 + 8 = 58)
  }
  // For values outside 0x01–0x59, return unchanged or handle as needed
  return hexValue; // Or return 0, or flag an error
}
void decodeAndPrintPacket(byte startIndex) {
    // Extract packet bytes
    byte packet[PACKET_SIZE];
    for (byte i = 0; i < PACKET_SIZE; i++) {
      packet[i] = buffer[(startIndex + i) % BUFFER_SIZE];
    }

    // Decode fields
    mode = packet[0];
    cdNum = decodeTime(~packet[1]-0x40);
    trackNum = decodeTime(255-packet[2]);
    uint16_t time = packet[3] | packet[4]; // timeMsb << 8 | timeLsb
    byte frame1 = packet[5];
    cdcState = packet[6];
    byte frame3 = packet[7];

    // Convert time (assuming frames, 75 fps)

    uint16_t totalSeconds = time;
    minutes = decodeTime(~packet[3]);
    seconds = decodeTime(~packet[4]);

  #if debug
    // Map mode to string
    String modeStr;
    switch (mode) {
      case CDC_MODE_IDLE: modeStr = "Idle"; break;
      case CDC_MODE_ACK: modeStr = "ACK"; break;
      case CDC_MODE_BUSY: modeStr = "Busy"; break;
      case CDC_MODE_PLAY: modeStr = "Play"; break;
      default: modeStr = "Unknown (0x" + String(mode, HEX) + ")";
    }
    // Map CDC state to string
    String stateStr;
    switch (cdcState) {
      case CDC_STATE_NOT_INITIALIZED: stateStr = "Not Initialized"; break;
      case CDC_STATE_BUSY: stateStr = "Busy"; break;
      case CDC_STATE_FAST_FW_BW: stateStr = "Fast Forward/Backward"; break;
      case CDC_STATE_MUTE: stateStr = "Mute"; break;
      case CDC_STATE_PLAYING: stateStr = "Playing"; break;
      case CDC_STATE_INITIALIZED: stateStr = "Initialized"; break;
      case CDC_STATE_SENDING_CD_INFO: stateStr = "Sending CD Info"; break;
      default: stateStr = "Unknown (0x" + String(cdcState, HEX) + ")";
    }

    // // Print decoded packet
      Serial.println("=== Decoded Packet ===");
      Serial.print("Mode: "); Serial.println(modeStr);
      Serial.print("CD Number: "); Serial.println(cdNum);
      Serial.print("Track Number: "); Serial.println(trackNum);
      Serial.print("Time: "); 
      Serial.print(minutes); Serial.print("m "); 
      Serial.print(seconds); Serial.println("s");
      Serial.print("Frame 1: 0x"); Serial.println(frame1, HEX);
      Serial.print("CDC State: "); Serial.println(stateStr);
      Serial.print("Frame 3: 0x"); Serial.println(frame3, HEX);
      Serial.println("====================");
  #endif
}

void printBuffer() {
  Serial.print("Buffer contents: ");
  for (byte i = 0; i < BUFFER_SIZE; i++) {
    if (buffer[i] < 0x10) Serial.print("0");
    Serial.print(buffer[i], HEX);
    Serial.print(" ");
  }
  Serial.print("(writeIndex=");
  Serial.print(writeIndex);
  Serial.print(", readIndex=");
  Serial.print(readIndex);
  Serial.println(")");
}

byte availableBytes() {
  return (writeIndex - readIndex + BUFFER_SIZE) % BUFFER_SIZE;
}

// === Menu and Control Functions ===
void displayMenu() {
  Serial.println("\n=== Command Menu ===");
  Serial.println("1. Power OFF");
  Serial.println("2. Power ON");
  // Serial.println("3. Prev CD");
  Serial.println("3. Next CD");
  Serial.println("4. Previous Track");
  Serial.println("5. Next Track");
  Serial.println("6. Scan");
  Serial.println("7. Mix");
  Serial.println("8. Play");
  Serial.println("Enter the number of the command to execute:");
}

void lcdWrite(const char *format, ...) {
  #if debug && LcdPresent
    char buffer[17]; // Buffer for 16 chars + null terminator
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args); // Format string
    va_end(args);

    // Pad buffer with spaces to ensure 16 chars
    for (int i = strlen(buffer); i < 16; i++) {
      buffer[i] = ' ';
    }
    buffer[16] = '\0'; // Null terminate

    lcd.setCursor(0, 0);
    lcd.print(buffer); // Print first 8 chars
    lcd.setCursor(0, 1);
    lcd.print(buffer+8); // Print next 8 chars
  #endif
}

void setup() {
  #if LcdPresent
    lcd.init();
    lcd.clear();
    lcdWrite("Booting!");
  #endif
    
    Serial.begin(250000); // Higher baud rate for SPI data
    pinMode(dataout, OUTPUT);
    digitalWrite(dataout, LOW);

    // Initialize SPI
    pinMode(MISO, OUTPUT);
    pinMode(MOSI, INPUT);
    pinMode(SCK, INPUT);
    SPCR &= ~(1 << CPOL); // SPI Mode 1: CPOL=0
    SPCR |= (1 << CPHA);  // CPHA=1
    SPCR &= ~(1 << DORD); // MSB-first
    SPCR |= (1 << SPE);   // Enable SPI
    SPCR |= (1 << SPIE);  // Enable SPI interrupts
    SPI.transfer(0);      // Initialize SPI transfer

    // Perform initial CD changer setup
    Serial.println("INITING!");
    powerOn();
    Serial.println("Setup complete. Use the Serial Monitor to select commands.");
    displayMenu();
}

void loop() {
  #if LcdPresent
    if(cdcState == CDC_STATE_PLAYING)
    {
      lcdWrite("CD%d Song%02d %02d:%02d",cdNum,trackNum,minutes,seconds);
    }
  #endif
    // Process SPI data
    if (availableBytes() > 0) {
      noInterrupts();
      
      if (!aligned) {
        byte searchLimit = availableBytes();
        for (byte i = 0; i < searchLimit; i++) {
          if (isValidMode(buffer[readIndex])) {
  #if debug
            Serial.print("Aligned at readIndex ");
            Serial.println(readIndex);
  #endif
            aligned = true;
            break;
          }
          readIndex = (readIndex + 1) % BUFFER_SIZE;
        }
      }
      
      if (aligned && availableBytes() >= PACKET_SIZE) {
        if (isValidMode(buffer[readIndex])) {
          bool packetChanged = false;
          for (byte i = 0; i < PACKET_SIZE; i++) {
            byte idx = (readIndex + i) % BUFFER_SIZE;
            if (buffer[idx] != lastPacket[i]) {
              packetChanged = true;
              break;
            }
          }
          if (packetChanged) {
            decodeAndPrintPacket(readIndex);
          }
          for (byte i = 0; i < PACKET_SIZE; i++) {
            lastPacket[i] = buffer[(readIndex + i) % BUFFER_SIZE];
          }
          lastValidPacket = millis();
          readIndex = (readIndex + PACKET_SIZE) % BUFFER_SIZE;
        } else {
  #if debug
          Serial.print("Lost alignment at: ");
          for (byte i = 0; i < PACKET_SIZE; i++) {
            byte idx = (readIndex + i) % BUFFER_SIZE;
            if (buffer[idx] < 0x10) Serial.print("0");
            Serial.print(buffer[idx], HEX);
            Serial.print(" ");
          }
          Serial.println();
  #endif
          restartSPI(); // Restart SPI on alignment loss
          lastValidPacket = millis();
        }
      }
      
      interrupts();
    }

    // Handle timeout for resync
    if (millis() - lastValidPacket > TIMEOUT_MS && availableBytes() > 0) {
  #if debug
      Serial.println("Timeout: Forcing resync and restarting SPI...");
  #endif
      printBuffer();
      restartSPI();
      lastValidPacket = millis();
    }

    // Check for user input
    if (Serial.available() > 0) {
      String input = Serial.readStringUntil('\n');
      input.trim();
      int choice = input.toInt();
      
      switch (choice) {
        case 1: InitCmd(); break;
        case 2: powerOn(); break;
        // case 3: SelectDisc1(); break;
        case 3: NextDisk(); break;
        case 4: PrevTrack(); break;
        case 5: NextTrack(); break;
        case 6: Scan(); break;
        case 7: Mix(); break;
        case 8: Play(); break;
        default:
          Serial.println("Invalid choice. Please enter a number between 1 and 9.");
          break;
      }
      displayMenu();
    }
}
