#include <Keypad.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>

const byte ROWS = 4;
const byte COLS = 4;

char keys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

byte rowPins[ROWS] = {2, 3, 4, 5};
byte colPins[COLS] = {6, 7, 8, 9};

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

LiquidCrystal_I2C lcd(0x27, 16, 2);
Servo doorServo;

const int servoPin = 10;
const int buzzerPin = 11;
const int ledPin = 12;
const int buttonPin = 13;

const String password = "4616";
String inputPIN = "";

bool pintuTerbuka = false;
unsigned long waktuBuka = 0;
const unsigned long durasiBuka = 5000;

void bunyiBerhasil() {
  tone(buzzerPin, 1047, 150);
  delay(180);

  tone(buzzerPin, 1319, 150);
  delay(180);

  tone(buzzerPin, 1568, 150);
  delay(180);

  tone(buzzerPin, 2093, 350);
  delay(400);

  noTone(buzzerPin);
}

void bunyiGagal() {
  for (int i = 0; i < 3; i++) {

    // Sirine naik
    for (int freq = 600; freq <= 1500; freq += 20) {
      tone(buzzerPin, freq);
      delay(10);
    }

    // Sirine turun
    for (int freq = 1500; freq >= 600; freq -= 20) {
      tone(buzzerPin, freq);
      delay(10);
    }
  }

  noTone(buzzerPin);
}

void tampilkanAwal() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("SALAM DULU:");
  lcd.setCursor(0, 1);
}

void kunciPintu() {
  doorServo.write(0);
  digitalWrite(ledPin, LOW);
  pintuTerbuka = false;

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("SALAM DULU");
  delay(1200);

  inputPIN = "";
  tampilkanAwal();
}

void bukaPintu() {
  doorServo.write(90);
  digitalWrite(ledPin, HIGH);
  pintuTerbuka = true;
  waktuBuka = millis();

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("ASIK ORDAl");
  lcd.setCursor(0, 1);
  lcd.print("SOK MASUK WEH");

  bunyiBerhasil();
}

void setup() {
  pinMode(ledPin, OUTPUT);
  pinMode(buzzerPin, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);

  doorServo.attach(servoPin);
  doorServo.write(0);

  lcd.init();
  lcd.backlight();

  tampilkanAwal();
}

void loop() {
  char key = keypad.getKey();

  // Tombol dari dalam rumah
  if (digitalRead(buttonPin) == LOW && !pintuTerbuka) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("TOMBOL DALAM");
    lcd.setCursor(0, 1);
    lcd.print("MEMBUKA PINTU");
    bukaPintu();
    delay(300);
  }

  // Membaca keypad
  if (key) {
    if (key >= '0' && key <= '9') {
      if (inputPIN.length() < 8) {
        inputPIN += key;
        lcd.print("*");
      }
    }

    if (key == '*') {
      inputPIN = "";
      tampilkanAwal();
    }

    if (key == '#') {
      if (inputPIN == password) {
        bukaPintu();
      } else {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("SALAH KOCAk!");
        lcd.setCursor(0, 1);
        lcd.print("BALIK DAH");
        bunyiGagal();
        delay(1500);

        inputPIN = "";
        tampilkanAwal();
      }
    }
  }

  // Kunci kembali otomatis setelah 5 detik
  if (pintuTerbuka && millis() - waktuBuka >= durasiBuka) {
    kunciPintu();
  }
}