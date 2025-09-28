// Скетч для ESP32: Чтение давления МПД-380 по Modbus RTU (регистр 0x0004)
// Библиотека: ModbusMaster
// Подключение: UART2 (GPIO16 RX, GPIO17 TX) через RS485-TTL преобразователь
// Адрес: 1, Baud: 9600, 8N1
// Вычисляет давление в барах с decimal и offset

#include <ModbusMaster.h>

#define RX_PIN 16
#define TX_PIN 17

ModbusMaster node;

void setup() {
  Serial.begin(115200);
  while (!Serial);
  Serial2.begin(9600, SERIAL_8N1, RX_PIN, TX_PIN);
  node.begin(1, Serial2); // Slave адрес 1
  Serial.println("МПД-380 Modbus: Чтение давления");
}

void loop() {
  uint8_t result = node.readHoldingRegisters(0x0000, 13); // Читаем 0x0000–0x000C

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

    // Вывод данных
    Serial.print("Адрес: "); Serial.println(slave_addr);
    Serial.print("Скорость: "); Serial.print(getBaudRate(baud_code)); Serial.println(" бод");
    Serial.print("Единицы: "); Serial.println(unit_str);
    Serial.print("Десятичные знаки: "); Serial.println(decimal_places);
    Serial.print("Offset: "); Serial.println(offset);
    Serial.print("Давление: ");
    Serial.print(pressure_value, (decimal_places > 0) ? decimal_places : 2); // Адаптивный вывод
    Serial.print(" ");
    Serial.println(unit_str);
    if (raw_pressure < 0) Serial.println("Внимание: Отрицательное давление!");
    Serial.println("---");
  } else {
    Serial.print("Ошибка Modbus: 0x"); Serial.println(result, HEX);
    switch (result) {
      case 0xE0: Serial.println(" - Таймаут"); break;
      case 0xE1: Serial.println(" - Неверный CRC"); break;
      case 0xE2: Serial.println(" - Исключение Modbus"); break;
      case 0x83: Serial.println(" - Неверный адрес или регистр"); break;
      default: Serial.println(" - Неизвестная ошибка"); break;
    }
  }

  delay(500); // Уменьшил для более частого обновления
}

// Функция для получения строки единиц измерения
String getUnitString(uint8_t unit_code) {
  switch (unit_code) {
    case 0: return "МПа";
    case 1: return "кПа";
    case 2: return "Па";
    case 3: return "бар";
    case 4: return "мбар";
    case 5: return "кг/см²";
    case 6: return "Psi";
    case 7: return "мН₂О";
    case 8: return "ммН₂О";
    case 9: return "мА";
    default: return "неизв.";
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
    default: return "неизв.";
  }
}