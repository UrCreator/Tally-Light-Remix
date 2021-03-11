#include <M5StickC.h>
#include <WiFi.h>
#include <SkaarhojPgmspace.h>
#include <ATEMbase.h>
#include <ATEMstd.h>
#include <Adafruit_NeoPixel.h>

IPAddress switcherIp(10, 10, 0, 150);        // IP address of the ATEM switcher
ATEMstd AtemSwitcher;

// http://www.barth-dev.de/online/rgb565-color-picker/
#define GRAY  0x0020 //   8  8  8
#define GREEN 0x0200 //   0 64  0
#define RED   0xF800 // 255  0  0
#define WHITE 0xFFFF // 255 255 255

const char* ssid = "Gabo's iPhone";                  // WiFi SSID
const char* password = "Gabriel18";            // WiFi Password

int ledPin = 10; // Inboard LED
int ledPinExternal = 26; // External LED
int NUMPIXELS = 7; // How many NeoPixels are attached to the Arduino
int neoColor = 0; // Variable for the NeoPixel color
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, ledPinExternal, NEO_GRB + NEO_KHZ800);  

int cameraNumber = 1;

int PreviewTallyPrevious = 1;
int ProgramTallyPrevious = 1;
int cameraNumberPrevious = cameraNumber;

void setup() {

  Serial.begin(9600);
  pixels.begin(); // Initializes the NeoPixel library.

  // Start the Ethernet, Serial (debugging) and UDP:
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println("Connected to the WiFi network");

  // Initialize the M5StickC object
  M5.begin();

  // Change LCD rotation 
  M5.Lcd.setRotation(3);

  pinMode(ledPin, OUTPUT);  // INTERNAL LED: 1 is on Program (Tally)
  digitalWrite(ledPin, HIGH); // INTERNAL LED (Off)

  changeNeoPixel(0);  

  // Initialize a connection to the switcher:
  AtemSwitcher.begin(switcherIp);
  AtemSwitcher.serialOutput(0x80);
  AtemSwitcher.connect();
}

void loop() {

  M5.update();
  
  // if WiFi is down, turn off NeoPixels, print "Reconnecting" and try reconnecting
  while ((WiFi.status() != WL_CONNECTED)) {
  changeNeoPixel(0);
  Serial.println("Reconnecting to WiFi...");
  M5.Lcd.fillScreen(GRAY);
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.setCursor(28,33,2);
  M5.Lcd.setTextSize(1);
  M5.Lcd.print("Reconnecting...");
  WiFi.disconnect();
  WiFi.begin(ssid, password);
  delay(1000);
    if ((WiFi.status() == WL_CONNECTED)) {
      M5.Lcd.fillScreen(GREEN);
      M5.Lcd.setCursor(43,33,2);
      M5.Lcd.print("Connected!");
      delay(2000);
      break;
      }
}
 
  // Check for packets, respond to them etc. Keeping the connection alive
  AtemSwitcher.runLoop();

  int ProgramTally = AtemSwitcher.getProgramTally(cameraNumber);
  int PreviewTally = AtemSwitcher.getPreviewTally(cameraNumber);

  if ((cameraNumber != cameraNumberPrevious) || (ProgramTallyPrevious != ProgramTally) || (PreviewTallyPrevious != PreviewTally)) { // changed?

    if ((ProgramTally && !PreviewTally) || (ProgramTally && PreviewTally) ) { // only program, or program AND preview
      drawLabel(RED, BLACK, LOW, 255);
    } else if (PreviewTally && !ProgramTally) { // only preview
      drawLabel(GREEN, BLACK, HIGH, 0);
    } else if (!PreviewTally || !ProgramTally) { // neither
      drawLabel(BLACK, GRAY, HIGH, LOW);
    }

  }

  ProgramTallyPrevious = ProgramTally;
  PreviewTallyPrevious = PreviewTally;
  cameraNumberPrevious = cameraNumber;

  // Change camera number
  if (M5.BtnA.wasPressed()) {
    cameraNumber = (cameraNumber + 1) % 4;
    if(cameraNumber == 0) {
      cameraNumber = 4;
      }
  }
}

// Updates M5 based on Switcher
void drawLabel(unsigned long int screenColor, unsigned long int labelColor, bool ledValue, int neoColor) {
  changeNeoPixel(neoColor);  
  digitalWrite(ledPin, ledValue);
  M5.Lcd.fillScreen(screenColor);
  M5.Lcd.setTextColor(labelColor, screenColor);
  M5.Lcd.drawString(String(cameraNumber), 55, 2, 8);
}

// Change NeoPixel colors from off to Red
void changeNeoPixel(int neoColor){
  pixels.setPixelColor(0, pixels.Color(neoColor, 0, 0));
  pixels.setPixelColor(1, pixels.Color(neoColor, 0, 0));
  pixels.setPixelColor(2, pixels.Color(neoColor, 0, 0));
  pixels.setPixelColor(3, pixels.Color(neoColor, 0, 0));
  pixels.setPixelColor(4, pixels.Color(neoColor, 0, 0));
  pixels.setPixelColor(5, pixels.Color(neoColor, 0, 0));
  pixels.setPixelColor(6, pixels.Color(neoColor, 0, 0));
  pixels.show();
    }
