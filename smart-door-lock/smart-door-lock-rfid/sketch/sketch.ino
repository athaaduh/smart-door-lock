
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <Servo.h>
#include <SPI.h>
#include <MFRC522.h>

// ===============================
// KONFIGURASI KOMPONEN
// ===============================

LiquidCrystal_I2C lcd(0x27, 16, 2);
Servo doorServo;

const int SERVO_PIN = 9;
const int BUZZER_PIN = 8;

const int SERVO_LOCKED = 0;
const int SERVO_UNLOCKED = 90;

// RFID
#define RFID_SS_PIN 10
#define RFID_RST_PIN A2

MFRC522 rfid(RFID_SS_PIN, RFID_RST_PIN);

// ===============================
// KONFIGURASI KEYPAD
// ===============================

const byte ROWS = 4;
const byte COLS = 4;

char keys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

byte rowPins[ROWS] = {2, 3, 4, 5};

// D10 dan D11 dipindahkan ke A0 dan A1
byte colPins[COLS] = {6, 7, A0, A1};

Keypad keypad = Keypad(
  makeKeymap(keys),
  rowPins,
  colPins,
  ROWS,
  COLS
);

// ===============================
// DATA PENGGUNA
// ===============================

String inputPin = "";

String userNames[3] = {
  "Atha",
  "Zahira",
  "Ika"
};

String userPins[3] = {
  "1111",
  "2222",
  "0000"
};

// UID kartu RFID masing-masing pengguna
// Sesuaikan UID jika kartu yang digunakan berbeda

byte authorizedUIDs[3][4] = {
  {0x01, 0x02, 0x03, 0x04}, // Atha
  {0x11, 0x22, 0x33, 0x44}, // Zahira
  {0x55, 0x66, 0x77, 0x88}  // Ika
};

// ===============================
// SETUP
// ===============================

void setup() {

  Serial.begin(9600);

  lcd.init();
  lcd.backlight();

  doorServo.attach(SERVO_PIN);
  pinMode(BUZZER_PIN, OUTPUT);

  // Memulai komunikasi RFID
  SPI.begin();
  rfid.PCD_Init();

  lockDoor();

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Sistem Keamanan");
  lcd.setCursor(0, 1);
  lcd.print("Memulai...");

  delay(2000);

  showEnterPin();

  Serial.println("Sistem RFID aktif");
  Serial.println("Tempelkan kartu RFID");
}

// ===============================
// LOOP UTAMA
// ===============================

void loop() {

  // Memeriksa kartu RFID
  checkRFID();

  // Membaca keypad
  char key = keypad.getKey();

  if (key) {

    if (key == '#') {
      checkPin();
    }

    else if (key == '*') {
      inputPin = "";
      showEnterPin();
    }

    else if (key == 'C') {

      if (inputPin.length() > 0) {
        inputPin.remove(inputPin.length() - 1);
        updateDisplay();
      }

    }

    else if (inputPin.length() < 4) {

      if (key >= '0' && key <= '9') {
        inputPin += key;
        updateDisplay();
      }

    }
  }
}

// ===============================
// TAMPILAN LCD
// ===============================

void showEnterPin() {

  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("Masukkan PIN:");

  lcd.setCursor(0, 1);
  lcd.print("Lalu tekan #");
}

void updateDisplay() {

  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("Masukkan PIN:");

  lcd.setCursor(0, 1);

  for (int i = 0; i < inputPin.length(); i++) {
    lcd.print("*");
  }
}

// ===============================
// AUTENTIKASI PIN
// ===============================

void checkPin() {

  bool accessGranted = false;

  for (int i = 0; i < 3; i++) {

    if (inputPin == userPins[i]) {

      accessGranted = true;

      unlockDoor(userNames[i]);

      break;
    }
  }

  if (!accessGranted) {
    accessDenied();
  }

  inputPin = "";

  showEnterPin();
}

// ===============================
// AUTENTIKASI RFID
// ===============================

void checkRFID() {

  // Memeriksa apakah ada kartu baru
  if (!rfid.PICC_IsNewCardPresent()) {
    return;
  }

  // Membaca UID kartu
  if (!rfid.PICC_ReadCardSerial()) {
    return;
  }

  Serial.print("UID Kartu: ");

  for (byte i = 0; i < rfid.uid.size; i++) {

    Serial.print(rfid.uid.uidByte[i], HEX);
    Serial.print(" ");

  }

  Serial.println();

  bool accessGranted = false;

  // Memeriksa UID dengan daftar pengguna
  for (int i = 0; i < 3; i++) {

    if (rfid.uid.size == 4) {

      bool uidMatch = true;

      for (byte j = 0; j < 4; j++) {

        if (rfid.uid.uidByte[j] != authorizedUIDs[i][j]) {
          uidMatch = false;
          break;
        }

      }

      if (uidMatch) {

        accessGranted = true;

        inputPin = "";

        unlockDoor(userNames[i]);

        break;
      }
    }
  }

  // Jika kartu tidak terdaftar
  if (!accessGranted) {

    accessDenied();

  }

  // Mengakhiri komunikasi dengan kartu
  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
}

// ===============================
// AKSES DITERIMA
// ===============================

void unlockDoor(String name) {

  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("Akses diterima");

  lcd.setCursor(0, 1);
  lcd.print("Halo, ");
  lcd.print(name);

  beep(1);

  doorServo.write(SERVO_UNLOCKED);

  delay(5000);

  lockDoor();
}

// ===============================
// AKSES DITOLAK
// ===============================

void accessDenied() {

  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("Akses ditolak");

  lcd.setCursor(0, 1);
  lcd.print("PIN/Kartu salah");

  beep(3);

  delay(2000);
}

// ===============================
// KUNCI PINTU
// ===============================

void lockDoor() {

  doorServo.write(SERVO_LOCKED);

}

// ===============================
// BUZZER
// ===============================

void beep(int times) {

  for (int i = 0; i < times; i++) {

    tone(BUZZER_PIN, 1000, 400);

    delay(470);

    noTone(BUZZER_PIN);
  }
}
