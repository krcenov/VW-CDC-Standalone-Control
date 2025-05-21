// Define the pin number for the pulse output
const int dataout = 22;

byte ScanCMD[] =        {0xCA, 0x34, 0x05, 0xFA}; //UNTESTED
byte PlayCMD[] =        {0xCA, 0x34, 0x06, 0xF9}; //UNTESTED
byte initCMD[] =        {0xCA, 0x34, 0x08, 0xF7};
byte SelectDisk[] =     {0xCA, 0x34, 0x1C, 0xE3};
byte PrevTrackCMD[] =   {0xCA, 0x34, 0x1E, 0xE1}; //UNTESTED
byte NextTrackCMD[] =   {0xCA, 0x34, 0x1F, 0xE0}; //UNTESTED
byte PowerOnCMD[] =     {0xCA, 0x34, 0x27, 0xD8};
byte ACK[] =            {0xCA, 0x34, 0x28, 0xD7};
byte ChangeDisk2[] =    {0xCA, 0x34, 0x30, 0xCE};
byte ChangeDisk1[] =    {0xCA, 0x34, 0x31, 0xCF};

int CMDSize = 4;

// Function to send the 4-byte command
void send4CMD(uint8_t* cmd) {
    StartComm(); // Initialize communication
    for (size_t i = 0; i < 4; i++) { // Loop through each byte
        for (int j = 0; j < 8; j++) { // Loop through each bit
            if (cmd[i] & (1 << j)) { // Check if the j-th bit is 1
                Send1(); // Send a 1 bit
            } else {
                Send0(); // Send a 0 bit
            }
        }
    }
    digitalWrite(dataout, LOW); // Ensure pin is LOW at the end
}

void Init() {
    Serial.println("Executing Init Command");
    send4CMD(initCMD);
    delay(135);
    send4CMD(ACK);
    delay(6500);
    send4CMD(initCMD);
    delay(135);
    send4CMD(ACK);
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

void SelectDisc1() {
    Serial.println("Executing ChangeDisk1 Command");
    send4CMD(ChangeDisk1);
    delay(93);
    send4CMD(ACK);
    delay(93);
    send4CMD(SelectDisk);
    delay(93);
    send4CMD(ACK);
    delay(93);
}

void SelectDisc2() {
    Serial.println("Executing ChangeDisk2 Command");
    send4CMD(ChangeDisk2);
    delay(93);
    send4CMD(ACK);
    delay(93);
    send4CMD(SelectDisk);
    delay(93);
    send4CMD(ACK);
    delay(93);
}

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

// Function to display the menu
void displayMenu() {
    Serial.println("\n=== Command Menu ===");
    Serial.println("1. Initialize");
    Serial.println("2. Power On");
    Serial.println("3. Select Disc 1");
    Serial.println("4. Select Disc 2");
    Serial.println("5. Next Track");
    Serial.println("6. Previous Track");
    Serial.println("7. Scan");
    Serial.println("8. Mix");
    Serial.println("9. Play");
    Serial.println("Enter the number of the command to execute:");
}

void setup() {
    Serial.begin(9600); // Initialize Serial communication
    pinMode(dataout, OUTPUT);
    Init();
    delay(10000);
    powerOn();
    Serial.println("Setup complete. Use the Serial Monitor to select commands.");
    displayMenu();
}

void loop() {
    // Check if data is available in the Serial buffer
    if (Serial.available() > 0) {
        // Read the input as a string and convert to an integer
        String input = Serial.readStringUntil('\n');
        input.trim(); // Remove any whitespace or newline
        int choice = input.toInt();

        // Execute the corresponding command based on user input
        switch (choice) {
            case 1:
                Init();
                break;
            case 2:
                powerOn();
                break;
            case 3:
                SelectDisc1();
                break;
            case 4:
                SelectDisc2();
                break;
            case 5:
                NextTrack();
                break;
            case 6:
                PrevTrack();
                break;
            case 7:
                Scan();
                break;
            case 8:
                Mix();
                break;
            case 9:
                Play();
                break;
            default:
                Serial.println("Invalid choice. Please enter a number between 1 and 9.");
                break;
        }
        // Display the menu again after executing the command
        displayMenu();
    }
}

void StartComm() {
    digitalWrite(dataout, HIGH);  // Set pin 22 to HIGH (5V)
    delay(9);                      // Wait for 9 milliseconds
    digitalWrite(dataout, LOW);   // Set pin 22 to LOW (0V)
    delay(5);                      // Wait for 5 milliseconds
    digitalWrite(dataout, HIGH);  // Set pin back to HIGH
    delayMicroseconds(560);        // Wait for 560 microseconds
}

void Send0() {
    digitalWrite(dataout, LOW);   // Set pin LOW
    delayMicroseconds(560);        // Wait for 560 microseconds
    digitalWrite(dataout, HIGH);  // Set pin HIGH
    delayMicroseconds(560);        // Wait for 560 microseconds
}

void Send1() {
    digitalWrite(dataout, LOW);   // Set pin LOW
    delayMicroseconds(1680);       // Wait for 1680 microseconds
    digitalWrite(dataout, HIGH);  // Set pin HIGH
    delayMicroseconds(560);        // Wait for 560 microseconds
}