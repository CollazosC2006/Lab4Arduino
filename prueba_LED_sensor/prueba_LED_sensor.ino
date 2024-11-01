#include <Adafruit_Fingerprint.h>

// Utilizamos Serial2 para la WT32-ETH01
/*
#define RX_PIN 5  // Pin RX donde se conecta el TX del sensor de huellas
#define TX_PIN 17  // Pin TX donde se conecta el RX del sensor de huellas*/

//ESP32
#define RX_PIN 16  // Pin RX donde se conecta el TX del sensor de huellas
#define TX_PIN 17  // Pin TX donde se conecta el RX del sensor de huellas

// Inicializar el puerto Serial2 para la WT32-ETH01
HardwareSerial mySerial(2);  // Serial2 es el hardware UART en WT32-ETH01

// Crear el objeto del sensor de huellas
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

void setup()
{
  // Iniciar la comunicación con el monitor serie
  Serial.begin(115200);


  Serial.println("\n\nAdafruit Fingerprint Sensor Test");
  while (!Serial);
  delay(100);
  Serial.println("\n\nAdafruit Fingerprint Sensor Test");

  // Configurar el puerto serial del sensor de huellas
  mySerial.begin(57600, SERIAL_8N1, RX_PIN, TX_PIN);  // Configuramos la UART con los pines definidos
  delay(100);

  // Verificar la contraseña del sensor
  if (finger.verifyPassword()) {
    Serial.println("Sensor de huellas detectado!");
  } else {
    Serial.println("No se encontró el sensor de huellas :(");
    while (1) { delay(1); }
  }

  // Leer parámetros del sensor
  Serial.println(F("Leyendo parámetros del sensor"));
  finger.getParameters();
  Serial.print(F("Estado: 0x")); Serial.println(finger.status_reg, HEX);
  Serial.print(F("ID del sistema: 0x")); Serial.println(finger.system_id, HEX);
  Serial.print(F("Capacidad: ")); Serial.println(finger.capacity);
  Serial.print(F("Nivel de seguridad: ")); Serial.println(finger.security_level);
  Serial.print(F("Dirección del dispositivo: ")); Serial.println(finger.device_addr, HEX);
  Serial.print(F("Tamaño de paquetes: ")); Serial.println(finger.packet_len);
  Serial.print(F("Tasa de baudios: ")); Serial.println(finger.baud_rate);
}

void loop()
{
  // Pruebas de control de LED
  finger.LEDcontrol(FINGERPRINT_LED_ON, 0, FINGERPRINT_LED_RED);
  delay(250);
  finger.LEDcontrol(FINGERPRINT_LED_ON, 0, FINGERPRINT_LED_BLUE);
  delay(250);
  finger.LEDcontrol(FINGERPRINT_LED_ON, 0, FINGERPRINT_LED_PURPLE);
  delay(250);

  // Parpadear LED rojo
  finger.LEDcontrol(FINGERPRINT_LED_FLASHING, 25, FINGERPRINT_LED_RED, 10);
  delay(2000);

  // Respiración del LED azul
  finger.LEDcontrol(FINGERPRINT_LED_BREATHING, 100, FINGERPRINT_LED_BLUE);
  delay(3000);
  
  finger.LEDcontrol(FINGERPRINT_LED_GRADUAL_ON, 200, FINGERPRINT_LED_PURPLE);
  delay(2000);
  finger.LEDcontrol(FINGERPRINT_LED_GRADUAL_OFF, 200, FINGERPRINT_LED_PURPLE);
  delay(2000);
}
