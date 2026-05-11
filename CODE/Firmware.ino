#include <SPI.h>
#include <MFRC522.h>
#include <LiquidCrystal.h>
#include <ESP32Servo.h>

// Pin-Definitionen (bleiben gleich)
#define RST_PIN     17
#define SS_PIN      5
#define LED_PIN     13
#define SERVO_PIN   26
#define LCD_RS      21
#define LCD_E       22
#define LCD_D4      25
#define LCD_D5      27
#define LCD_D6      32
#define LCD_D7      33
#define TRIG_PIN    16
#define ECHO_PIN    34

// Konstanten
#define MAX_PLAETZE 10
#define SCHRANKE_OFFEN_ZEIT 3000
#define ULTRASCHALL_DISTANZ 10
#define AUSFAHRT_COOLDOWN 3000  // NEU: 3 Sekunden Sperre nach einer Ausfahrt

// Objekte
MFRC522 rfid(SS_PIN, RST_PIN);
LiquidCrystal lcd(LCD_RS, LCD_E, LCD_D4, LCD_D5, LCD_D6, LCD_D7);
Servo servo;

// Variablen
int parkplaetzeBenutzt = 0;
bool fahrzeugVorUltraschall = false;
unsigned long letzteSendeZeit = 0;
unsigned long letzteAusfahrtZeit = 0; // NEU: Speichert den Zeitpunkt der letzten Ausfahrt

void sendeWebStatus(String event) {
  Serial.print("STATUS:{\"free\":");
  Serial.print(MAX_PLAETZE - parkplaetzeBenutzt);
  Serial.print(",\"total\":");
  Serial.print(MAX_PLAETZE);
  Serial.print(",\"barrier\":\"");
  Serial.print(servo.read() > 45 ? "open" : "closed");
  Serial.print("\",\"event\":\"");
  Serial.print(event);
  Serial.println("\"}");
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  
  lcd.begin(16, 2);
  SPI.begin(18, 19, 23, 5);
  rfid.PCD_Init();
  
  ESP32PWM::allocateTimer(0);
  servo.setPeriodHertz(50);
  servo.attach(SERVO_PIN, 500, 2400);
  servo.write(0);
  
  delay(1000);
  zeigeStatus();
  sendeWebStatus("System bereit");
}

void loop() {
  // 1. RFID (Einfahrt) - BLEIBT UNVERÄNDERT
  if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
    if (parkplaetzeBenutzt < MAX_PLAETZE) {
      lcd.clear();
      lcd.print("ZUGRIFF OK");
      servo.write(90);
      parkplaetzeBenutzt++;
      sendeWebStatus("Einfahrt gewaehrt");
      delay(SCHRANKE_OFFEN_ZEIT);
      servo.write(0);
      sendeWebStatus("Schranke zu");
    } else {
      sendeWebStatus("Parkhaus voll!");
      lcd.clear();
      lcd.print("!!! VOLL !!!");
      delay(2000);
    }
    rfid.PICC_HaltA();
    rfid.PCD_StopCrypto1();
    zeigeStatus();
  }

  // 2. Ultraschall (Ausfahrt) - NUR HIER WURDE DER COOLDOWN EINGEBAUT
  long distanz = messeDistanz();
  
  // Prüfe: Distanz OK UND Sperrzeit abgelaufen?
  if (distanz > 0 && distanz < ULTRASCHALL_DISTANZ) {
    if (millis() - letzteAusfahrtZeit > AUSFAHRT_COOLDOWN) {
      if (parkplaetzeBenutzt > 0) {
        parkplaetzeBenutzt--;
        letzteAusfahrtZeit = millis(); // Zeitstempel für Sperre setzen
        sendeWebStatus("Ausfahrt erkannt");
        zeigeStatus();
      }
    }
  }

  // 3. Web-Update alle 2 Sek - BLEIBT UNVERÄNDERT
  if (millis() - letzteSendeZeit > 2000) {
    sendeWebStatus("Update");
    letzteSendeZeit = millis();
  }

  digitalWrite(LED_PIN, parkplaetzeBenutzt >= MAX_PLAETZE ? HIGH : LOW);
}

long messeDistanz() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  long dauer = pulseIn(ECHO_PIN, HIGH, 20000);
  return (dauer == 0) ? 999 : (dauer * 0.034 / 2);
}

void zeigeStatus() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Status: ");
  lcd.print(parkplaetzeBenutzt);
  lcd.print("/");
  lcd.print(MAX_PLAETZE);
  lcd.setCursor(0, 1);
  lcd.print("Frei: ");
  lcd.print(MAX_PLAETZE - parkplaetzeBenutzt);
}