// Скетч для ESP32: Чтение давления МПД-380 по Modbus RTU с отображением на дисплее 2004
// Библиотеки: ModbusMaster, LiquidCrystal_I2C
// Подключение: UART2 (GPIO16 RX, GPIO17 TX) через RS485-TTL, I2C (GPIO21 SDA, GPIO22 SCL)
// Адрес: 1, Baud: 9600, 8N1
// Вычисляет давление и отображает на дисплее

#include <ModbusMaster.h>
#include <LiquidCrystal_I2C.h>

// Пины
#define RX_PIN 16
#define TX_PIN 17
#define SDA_PIN 21
#define SCL_PIN 22

ModbusMaster node;

// Инициализация дисплея (адрес I2C может быть 0x27 или 0x3F, проверь с помощью I2C Scanner)
LiquidCrystal_I2C lcd(0x27, 20, 4); // 20 символов, 4 строки, адрес 0x27 (измени, если нужно)

void setup() {
  Serial.begin(115200);
  while (!Serial);

  // Инициализация UART2 для Modbus
  Serial2.begin(9600, SERIAL_8N1, RX_PIN, TX_PIN);
  node.begin(1, Serial2); // Slave адрес 1

  // Инициализация дисплея
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("МПД-380 Modbus");
  delay(2000); // Пауза для отображения приветствия

  Serial.println("МПД-380 Modbus: Чтение давления");
}

void loop() {
  uint8_t result = node.readHoldingRegisters(0x0000, 13); // Читаем 0x0000–0x000C

  lcd.clear(); // Очищаем дисплей перед новым выводом

  if (result == node.ku8MBSuccess) {
    int16_t slave_addr = node.getResponseBuffer(0);    // 0x0000
    int16_t baud_code = node.getResponseBuffer(1);     // 0x0001
    int16_t units = node.getResponseBuffer(2);         // 0x0002
    int16_t decimal_places = node.getResponseBuffer(3); // 0x0003
    int16_t raw_pressure = node.getResponseBuffer(4);  // 0x0004
    int16_t offset = node.getResponseBuffer(12);       // 0x000C

    // Расчёт давления
    float pressure_value = (float)(raw_pressure + offset) / pow(10, decimal_places);
    String unit_str = getUnitString(units);
    String baud_str = getBaudRate(baud_code);

    // Вывод на дисплей
    lcd.setCursor(0, 0);
    lcd.print("Addr: ");
    lcd.print(slave_addr);
    lcd.print("  Baud: ");
    lcd.print(baud_str);

    lcd.setCursor(0, 1);
    lcd.print("Unit: ");
    lcd.print(unit_str);
    lcd.print("  P: ");
    lcd.print(pressure_value, (decimal_places > 0) ? decimal_places : 2);

    lcd.setCursor(0, 2);
    lcd.print("Dec: ");
    lcd.print(decimal_places);
    lcd.print("  Off: ");
    lcd.print(offset);

    lcd.setCursor(0, 3);
    lcd.print("Status: OK");

    // Вывод в Serial Monitor (для отладки)
    Serial.print("Адрес: "); Serial.println(slave_addr);
    Serial.print("Скорость: "); Serial.print(baud_str); Serial.println(" бод");
    Serial.print("Единицы: "); Serial.println(unit_str);
    Serial.print("Десятичные знаки: "); Serial.println(decimal_places);
    Serial.print("Offset: "); Serial.println(offset);
    Serial.print("Давление: ");
    Serial.print(pressure_value, (decimal_places > 0) ? decimal_places : 2);
    Serial.print(" ");
    Serial.println(unit_str);
    if (raw_pressure < 0) Serial.println("Внимание: Отрицательное давление!");
    Serial.println("---");
  } else {
    // Обработка ошибок
    lcd.setCursor(0, 3);
    lcd.print("Status: Err 0x");
    lcd.print(result, HEX);

    Serial.print("Ошибка Modbus: 0x"); Serial.print(result, HEX);
    switch (result) {
      case 0xE0: Serial.println(" - Таймаут"); lcd.print(" Timeout"); break;
      case 0xE1: Serial.println(" - Неверный CRC"); lcd.print(" CRC Err"); break;
      case 0xE2: Serial.println(" - Исключение Modbus"); lcd.print(" Modbus Err"); break;
      case 0x83: Serial.println(" - Неверный адрес"); lcd.print(" Addr Err"); break;
      default: Serial.println(" - Неизвестная ошибка"); lcd.print(" Unknown"); break;
    }
  }

  delay(500); // Обновление каждые 500 мс
}

// Функция для получения строки единиц измерения
String getUnitString(uint8_t unit_code) {
  switch (unit_code) {
    case 0: return "MPa";
    case 1: return "kPa";
    case 2: return "Pa";
    case 3: return "bar";
    case 4: return "mbar";
    case 5: return "kg/cm²";
    case 6: return "Psi";
    case 7: return "mH2O";
    case 8: return "mmH2O";
    case 9: return "mA";
    default: return "unkn.";
  }
}

// Функция для получения скорости UART
String getBaudRate(uint8_t baud_code) {
  switch (baud_code) {
    case 0: return "1200";
    case 1: return "2400";
    case 2: return "4800";
    case 3: return "9600";
    case 4: return "19200";
    case 5: return "38400";
    case 6: return "57600";
    case 7: return "115200";
    default: return "unkn.";
  }
}