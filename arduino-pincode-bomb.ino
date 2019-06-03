#include <LiquidCrystalRus.h>
#include <Keypad.h>

const byte KEYPADROWS = 4; // 4 строки
const byte KEYPADCOLS = 4; // 4 столбца

// --------------Пины--------------

// Инициализация дисплея
LiquidCrystalRus lcd(6, 7, 8, 9, 10, 11);

// Выводные диджитал пины
const int passPin = 13;
const int failPin = 49;
const int boomPin = 50;

// Пины клавиатуры (строки, колонки)
byte rowPins[KEYPADROWS] = {29, 28, 27, 26};
byte colPins[KEYPADCOLS] = {25, 24, 23, 22};

// --------------Настройки--------------

// Начало отсчёта для пинкода
unsigned long maxCountdown = 60;

// Начало отсчёта перед взрывом
const unsigned long maxBoomCountdown = 120;

// Начальные попытки
int maxTries = 3;

// Правильный пинкод
String pincode = "1234";

// ----------------------------

// Состояния выходных пинов
int passPinState = LOW;
int failPinState = LOW;
int boomPinState = LOW;

// Отсчёт, попытки, ввод
unsigned long seconds = 0;
unsigned long startSec = 0;
int tries = maxTries;
String blabla;

// Настройки клавиатуры
char keys[KEYPADROWS][KEYPADCOLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

// Создаём клавиатуру
Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, KEYPADROWS, KEYPADCOLS );

// Значек часов
byte clock[8] = {
  0b00000,
  0b00000,
  0b01110,
  0b10101,
  0b10111,
  0b10001,
  0b01110,
  0b00000
};

// Возможные состояния автомата
enum States
{
  PLAY,
  STOP,
  PASS,
  FAIL,
  BOOM,
  SETTINGS,
};

// Текущее состояние автомата
States current;

void drawSettings(int y = NULL, int x = NULL);

void setup() {
  // устанавливаем колличество колонок и строк дисплея
  lcd.begin(20, 4);

  // добавляем значёк
  lcd.createChar(0, clock);

  // set the digital pin as output:
  pinMode(passPin, OUTPUT);
  pinMode(failPin, OUTPUT);
  pinMode(boomPin, OUTPUT);

  // Рисуем экран
  notify("Время Сквончить!");

  // Выставляем текущее состояние как 'настройка'
  openSettings();
}

void loop() {
  digitalWrite(passPin, passPinState);
  digitalWrite(failPin, failPinState);
  digitalWrite(boomPin, boomPinState);

  bool charGot = false;
  switch (current)
  {
    case STOP:
      charGot = keypad.getKey();
      drawGetReadyToPlay();
      if (charGot) {
        play();
      }
      break;
    case PASS:
      drawPass();
      break;
    case FAIL:
      if (seconds > maxBoomCountdown) {
        boom();
      }
      seconds = millis() / 1000 - startSec;
      drawFail(maxBoomCountdown - seconds);
      break;
    case BOOM:
      drawBoom();
      break;
    case SETTINGS:
      charGot = inputSettings();
      drawSettings();
      if (charGot) {
        checkSettings();
      }
      break;
    case PLAY:
    default:
      charGot = inputPlay();
      if (seconds > maxCountdown || tries <= 0) {
        fail();
        return;
      }
      seconds = millis() / 1000 - startSec;
      drawCountdown(maxCountdown - seconds, tries);
      if (charGot) {
        checkPinCode();
      }
      break;
  }
}

void drawCountdown(unsigned long countdown, int tries) {
  lcd.setCursor(0, 0);
  lcd.print("Время: " + formatTime(countdown));
  lcd.setCursor(5, 1);
  lcd.print("Попыток: " + String(tries));
  lcd.setCursor(2, 2);
  lcd.print("Введите пинкод:");
  lcd.setCursor(10, 3);
  lcd.print(blabla);
}

void drawSettings(int y = NULL, int x = NULL) {
  int l = blabla.length();
  String blablaNew = blabla;
  if (l < 12) {
    for (int i = 0; i < 12 - l; i++) {
      blablaNew += "0";
    }
  }
  lcd.setCursor(15, 0);
  lcd.print("Меню");
  lcd.setCursor(0, 0);
  lcd.print("Пинкод: " + blablaNew.substring(0, 4));
  lcd.setCursor(0, 1);
  lcd.print("Время: " + blablaNew.substring(4, 6) + ':' + blablaNew.substring(6, 8) + ':' + blablaNew.substring(8, 10));
  lcd.setCursor(0, 2);
  lcd.print("Попытки: " + blablaNew.substring(10, 12));
  lcd.setCursor(0, 3);
  lcd.print("А)Норм B)Нах");
  if (x == NULL && y == NULL) {
    if (l > 9) {
      x = 2;
      y = 9 + l - 9;
    } else if (l > 4) {
      x = 1;
      y = 7 + l - 4;
    } else {
      x = 0;
      y = 8 + l;
    }
  }
  lcd.setCursor(y, x);
}

void drawGetReadyToPlay() {
  lcd.setCursor(6, 1);
  lcd.print("Начать");
  lcd.setCursor(6, 2);
  lcd.print("отсчёт");
}

void drawPass() {
  lcd.setCursor(0, 0);
  lcd.print("####################");
  lcd.setCursor(0, 1);
  lcd.print("#  Пинкод          #");
  lcd.setCursor(0, 2);
  lcd.print("#          Принят  #");
  lcd.setCursor(0, 3);
  lcd.print("####################");
}

void drawFail(unsigned long secCount) {
  lcd.setCursor(0, 0);
  lcd.print("* Вы обосрались *");
  lcd.setCursor(0, 1);
  lcd.print("* Пиздец натупает *");
  lcd.setCursor(0, 2);
  lcd.print("* через: *");
  lcd.setCursor(0, 3);
  lcd.print("* " + formatTime(secCount) +" *");
}

void drawBoom() {
  lcd.setCursor(0, 0);
  lcd.print("********************");
  lcd.setCursor(0, 1);
  lcd.print("*     Вы умерли    *");
  lcd.setCursor(0, 2);
  lcd.print("*   Приятного дня  *");
  lcd.setCursor(0, 3);
  lcd.print("********************");
}

void notify(String message) {
  lcd.clear();
  int y = int((20 - message.length()) / 2);
  lcd.setCursor(y, 1);
  lcd.print(message);
  delay(1000);
  lcd.clear();
}

void pass() {
  lcd.clear();
  lcd.noBlink();
  passPinState = HIGH;
  current = PASS;
}

void fail() {
  lcd.clear();
  lcd.noBlink();
  seconds = 0;
  startSec = millis() / 1000;
  failPinState = HIGH;
  current = FAIL;
}

void boom() {
  lcd.clear();
  lcd.noBlink();
  failPinState = LOW;
  boomPinState = HIGH;
  current = BOOM;
}

void getReadyToPlay() {
  lcd.clear();
  lcd.noBlink();
  blabla = "";
  current = STOP;
}

void play() {
  lcd.clear();
  lcd.blink();
  seconds = 0;
  startSec = millis() / 1000;
  blabla = "";
  current = PLAY;
}

void openSettings() {
  lcd.clear();
  lcd.blink();
  blabla = "";
  current = SETTINGS;
}

bool inputPlay() {
  char key = keypad.getKey();
  if (key) {
    String deleteChars = "ABCD";
    if (deleteChars.indexOf(key) != -1) {
      if (blabla.length() > 0) {
        blabla.remove(blabla.length() - 1);
        lcd.clear();
      }
    } else {
      blabla += key;
    }
    return true;
  }
  return false;
}

bool inputSettings() {
  char key = keypad.getKey();
  if (key) {
    String deleteChars = "CD";
    String confirmChars = "A";
    String clearChars = "B";
    if (deleteChars.indexOf(key) != -1) {
      if (blabla.length() > 0) {
        blabla.remove(blabla.length() - 1);
        lcd.clear();
      }
    } else if (confirmChars.indexOf(key) != -1) {
      return true;
    } else if (clearChars.indexOf(key) != -1) {
      blabla = "";
      lcd.clear();
    } else if (blabla.length() < 12) {
      blabla += key;
    }
  }
  return false;
}

void parseSettings() {
  pincode = blabla.substring(0, 4);
  unsigned long inputCountdown = 0;
  inputCountdown += blabla.substring(4, 6).toInt() * 60 * 60;
  inputCountdown += blabla.substring(6, 8).toInt() * 60;
  inputCountdown += blabla.substring(8, 10).toInt();
  maxCountdown = inputCountdown;
  tries = maxTries = blabla.substring(10, 12).toInt();

  getReadyToPlay();
}

void checkSettings() {
  if (blabla.length() < 12) {
    notify("Заполни всё");
  } else {
    parseSettings();
  }
}

void checkPinCode() {
  if (blabla.equals(pincode)) {
    pass();
  } else if (blabla.length() > pincode.length() - 1) {
    tries -= 1;
    blabla = "";
    if (tries < 1) {
      return fail();
    }
    notify("Ошибочный пинкод");
  }
}

String formatTime(unsigned long secCount) {
  int minutes = secCount / 60;
  int hours = minutes / 60;
  minutes = minutes % 60;
  secCount = secCount % 60;
  String secondsS;
  String minutesS;
  String hoursS;
  if (hours < 1) {
    hoursS = "00";
  } else {
    hoursS = String(hours);
    if (hours < 10) {
      hoursS = "0" + hoursS;
    }
  }
  if (minutes < 1) {
    minutesS = "00";
  } else {
    minutesS = String(minutes);
    if (minutes < 10) {
      minutesS = "0" + minutesS;
    }
  }
  if (secCount < 1) {
    secondsS = "00";
  } else {
    secondsS = String(secCount);
    if (secCount < 10) {
      secondsS = "0" + secondsS;
    }
  }
  return hoursS + ":" + minutesS + ":" + secondsS;
}
