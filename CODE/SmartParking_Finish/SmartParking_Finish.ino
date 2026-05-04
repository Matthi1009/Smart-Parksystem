#include <SPI.h>
#include <MFRC522.h>
#include <ESP32Servo.h>
#include <LiquidCrystal.h>

// --- PIN DEFINITIONEN ---
// LCD Anschlüsse: RS, E, D4, D5, D6, D7
const int rs = 22, en = 21, d4 = 27, d5 = 25, d6 = 32, d7 = 33;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

// Sensoren & Aktoren
#define PIN_LED       13
#define PIN_TRIG      16
#define PIN_ECHO      34
#define PIN_SERVO     26

// RFID Anschlüsse
#define SS_PIN        5
#define RST_PIN       17

// --- VARIABLEN & EINSTELLUNGEN ---
const int MAX_SPOTS = 5;
int parkCount = 0;
const int WINKEL_ZU = 0;
const int WINKEL_AUF = 90;

MFRC522 rfid(SS_PIN, RST_PIN);
Servo schranke;

unsigned long letzteAusfahrtZeit = 0;

// --- FUNKTIONEN ---

void aktualisiereAnzeige() {
  // Ausgabe auf LCD
  lcd.clear();
  lcd.setCursor(0, 0);
  if (parkCount >= MAX_SPOTS) {
    lcd.print("Parkhaus VOLL");
    digitalWrite(PIN_LED, HIGH); // LED an wenn voll
  } else {
    lcd.print("Frei: ");
    lcd.print(MAX_SPOTS - parkCount);
    digitalWrite(PIN_LED, LOW); // LED aus wenn Plätze frei
  }
  lcd.setCursor(0, 1);
  lcd.print("Belegt: ");
  lcd.print(parkCount);

  // Ausgabe in der Konsole (Serial Monitor)
  Serial.println("---------------------------");
  Serial.print("Status: ");
  Serial.println(parkCount >= MAX_SPOTS ? "VOLL" : "PLÄTZE FREI");
  Serial.print("Belegt: "); Serial.print(parkCount);
  Serial.print(" / Frei: "); Serial.println(MAX_SPOTS - parkCount);
}

float messeDistanz() {
  digitalWrite(PIN_TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(PIN_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_TRIG, LOW);
  
  long dauer = pulseIn(PIN_ECHO, HIGH, 25000); 
  if (dauer == 0) return 999.0;
  return (dauer * 0.0343) / 2;
}

// --- SETUP ---

void setup() {
  // Konsole starten
  Serial.begin(115200);
  Serial.println("System wird gestartet...");

  // Pins konfigurieren
  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);
  
  // LCD initialisieren
  lcd.begin(16, 2);
  lcd.print("System Start...");

  // Servo initialisieren
  schranke.attach(PIN_SERVO);
  schranke.write(WINKEL_ZU);

  // RFID initialisieren
  SPI.begin();
  rfid.PCD_Init();

  delay(1000);
  aktualisiereAnzeige();
  Serial.println("System bereit.");
}

// --- HAUPTSCHLEIFE ---

void loop() {
  
  // 1. EINFAHRT (RFID)
  if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
    Serial.println("RFID Karte erkannt!");
    
    if (parkCount < MAX_SPOTS) {
      //DigitalWrite(PIN_LED, LOW);
      Serial.println("Zutritt gewährt. Schranke öffnet.");
      lcd.clear();
      lcd.print("Willkommen!");
      
      schranke.write(WINKEL_AUF);
      delay(3000); // Zeit zum Durchfahren
      schranke.write(WINKEL_ZU);
      
      parkCount++;
      aktualisiereAnzeige();
    } else {
      Serial.println("Zutritt verweigert: Parkhaus voll.");
      lcd.clear();
      lcd.print("Leider voll!");
      delay(2000);
      aktualisiereAnzeige();
      //DigitalWrite(PIN_LED, HIGH);
    }
    
    rfid.PICC_HaltA();
    rfid.PCD_StopCrypto1();
  }

  // 2. AUSFAHRT (Ultraschall)
  float distanz = messeDistanz();
  
  // Wenn ein Auto näher als 15cm am Sensor ist
  if (distanz < 15.0 && distanz > 1.0) {
    // Sperre von 3 Sek, damit ein Auto nicht mehrfach zählt
    if (millis() - letzteAusfahrtZeit > 3000) {
      if (parkCount > 0) {
        parkCount--;
        letzteAusfahrtZeit = millis();
        
        Serial.println("Auto ausgefahren.");
        lcd.clear();
        lcd.print("Gute Fahrt!");
        delay(2000);
        aktualisiereAnzeige();
      }
    }
  }

  delay(100); 
}