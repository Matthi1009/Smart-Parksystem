/*
 * Smart Parksystem – ESP32 Firmware
 * HTL Leoben · 2IT · Labor
 *
 * Einfahrt:  RFID-Karte (RC522) -> Schranke (Servo) öffnet
 * Ausfahrt:  Ultraschallsensor (HC-SR04) erkennt ausfahrendes Fahrzeug
 * Anzeige:   LCD 16x2 + Web-Dashboard über USB (Web Serial, 115200 Baud)
 */

#include <SPI.h>
#include <MFRC522.h>
#include <LiquidCrystal.h>
#include <ESP32Servo.h>

// ---------------------------------------------------------------- Pins
#define RST_PIN     17
#define SS_PIN      5
#define SPI_SCK     18
#define SPI_MISO    19
#define SPI_MOSI    23
#define LED_PIN     13   // rote LED: leuchtet, wenn das Parkhaus voll ist
#define SERVO_PIN   26
#define LCD_RS      21
#define LCD_E       22
#define LCD_D4      25
#define LCD_D5      27
#define LCD_D6      32
#define LCD_D7      33
#define TRIG_PIN    16
#define ECHO_PIN    34   // nur Eingang – ECHO (5 V) über Spannungsteiler anschließen!

// ---------------------------------------------------------- Konstanten
#define MAX_PLAETZE           10
#define SCHRANKE_OFFEN_ZEIT   3000  // ms, wie lange die Schranke offen bleibt
#define MELDUNG_ZEIT          2000  // ms, wie lange "VOLL" angezeigt wird
#define ULTRASCHALL_DISTANZ   10    // cm, darunter gilt ein Fahrzeug als erkannt
#define AUSFAHRT_COOLDOWN     3000  // ms Sperre nach einer erkannten Ausfahrt
#define WEB_UPDATE_INTERVALL  2000  // ms zwischen zwei Status-Meldungen
#define MESS_INTERVALL        60    // ms zwischen zwei Ultraschall-Messungen

#define SERVO_ZU    0
#define SERVO_OFFEN 90

// ------------------------------------------------------------- Objekte
MFRC522 rfid(SS_PIN, RST_PIN);
LiquidCrystal lcd(LCD_RS, LCD_E, LCD_D4, LCD_D5, LCD_D6, LCD_D7);
Servo servo;

// ------------------------------------------------------------ Zustand
int parkplaetzeBenutzt = 0;

bool schrankeOffen = false;
unsigned long schrankeGeoeffnetZeit = 0;

bool meldungAktiv = false;          // z. B. "VOLL" steht gerade am LCD
unsigned long meldungStartZeit = 0;

bool fahrzeugVorUltraschall = false; // Flankenerkennung: nur beim Eintreten zählen
unsigned long letzteAusfahrtZeit = 0;
unsigned long letzteMessZeit = 0;
unsigned long letzteSendeZeit = 0;

// ------------------------------------------------------- Hilfsfunktionen
void sendeWebStatus(const char* event) {
  // Format: STATUS:{"free":7,"total":10,"barrier":"closed","event":"..."}
  Serial.print("STATUS:{\"free\":");
  Serial.print(MAX_PLAETZE - parkplaetzeBenutzt);
  Serial.print(",\"total\":");
  Serial.print(MAX_PLAETZE);
  Serial.print(",\"barrier\":\"");
  Serial.print(schrankeOffen ? "open" : "closed");
  Serial.print("\",\"event\":\"");
  Serial.print(event);
  Serial.println("\"}");
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

void zeigeMeldung(const char* text) {
  lcd.clear();
  lcd.print(text);
  meldungAktiv = true;
  meldungStartZeit = millis();
}

long messeDistanz() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  long dauer = pulseIn(ECHO_PIN, HIGH, 20000);   // Timeout 20 ms (~3,4 m)
  return (dauer == 0) ? 999 : (dauer * 0.034 / 2);
}

// ------------------------------------------------------------- Ablauf
void pruefeEinfahrt() {
  if (schrankeOffen) return;  // während die Schranke offen ist, keine neue Karte
  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) return;

  if (parkplaetzeBenutzt < MAX_PLAETZE) {
    parkplaetzeBenutzt++;
    servo.write(SERVO_OFFEN);
    schrankeOffen = true;
    schrankeGeoeffnetZeit = millis();
    meldungAktiv = false;
    lcd.clear();
    lcd.print("ZUGRIFF OK");
    sendeWebStatus("Einfahrt gewaehrt");
  } else {
    zeigeMeldung("!!! VOLL !!!");
    sendeWebStatus("Parkhaus voll!");
  }

  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
}

void pruefeSchranke() {
  if (schrankeOffen && millis() - schrankeGeoeffnetZeit >= SCHRANKE_OFFEN_ZEIT) {
    servo.write(SERVO_ZU);
    schrankeOffen = false;
    zeigeStatus();
    sendeWebStatus("Schranke zu");
  }
}

void pruefeAusfahrt() {
  if (millis() - letzteMessZeit < MESS_INTERVALL) return;
  letzteMessZeit = millis();

  long distanz = messeDistanz();
  bool fahrzeugDa = (distanz > 0 && distanz < ULTRASCHALL_DISTANZ);

  // Nur zählen, wenn ein Fahrzeug NEU vor dem Sensor auftaucht (Flanke)
  // und die Sperrzeit seit der letzten Ausfahrt abgelaufen ist.
  if (fahrzeugDa && !fahrzeugVorUltraschall &&
      millis() - letzteAusfahrtZeit > AUSFAHRT_COOLDOWN &&
      parkplaetzeBenutzt > 0) {
    parkplaetzeBenutzt--;
    letzteAusfahrtZeit = millis();
    if (!schrankeOffen) zeigeStatus();
    sendeWebStatus("Ausfahrt erkannt");
  }

  fahrzeugVorUltraschall = fahrzeugDa;
}

void pruefeMeldung() {
  if (meldungAktiv && millis() - meldungStartZeit >= MELDUNG_ZEIT) {
    meldungAktiv = false;
    zeigeStatus();
  }
}

// ------------------------------------------------------- Setup / Loop
void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  lcd.begin(16, 2);
  SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI, SS_PIN);
  rfid.PCD_Init();

  ESP32PWM::allocateTimer(0);
  servo.setPeriodHertz(50);
  servo.attach(SERVO_PIN, 500, 2400);
  servo.write(SERVO_ZU);

  delay(1000);
  zeigeStatus();
  sendeWebStatus("System bereit");
}

void loop() {
  pruefeEinfahrt();
  pruefeSchranke();
  pruefeAusfahrt();
  pruefeMeldung();

  if (millis() - letzteSendeZeit > WEB_UPDATE_INTERVALL) {
    sendeWebStatus("Update");
    letzteSendeZeit = millis();
  }

  digitalWrite(LED_PIN, parkplaetzeBenutzt >= MAX_PLAETZE ? HIGH : LOW);
}
