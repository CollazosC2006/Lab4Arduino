#include <Adafruit_Fingerprint.h>

// Definir el hardware serial para el sensor de huellas (Serial2 en ESP32)
HardwareSerial mySerial(2); // Usamos Serial2 (pines RX2 = IO16, TX2 = IO17)

Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

int getFingerprintIDez();

void setup()
{
  // Inicializamos la comunicación con el PC (Serial0) a través de USB
  Serial.begin(9600);
  while (!Serial);  
  Serial.println("Fingerprint template extractor");

  // Configuramos el Serial2 para el sensor de huellas
  mySerial.begin(57600, SERIAL_8N1, 16, 17);  // RX = IO16, TX = IO17

  // Verificamos si el sensor de huellas está conectado correctamente
  if (finger.verifyPassword()) {
    Serial.println("Found fingerprint sensor!");
  } else {
    
    while (1);
  }

  // Intentamos cargar las plantillas de huellas (IDs 1 a 10)
  for (int finger = 1; finger < 10; finger++) {
    downloadFingerprintTemplate(finger);
  }
}

uint8_t downloadFingerprintTemplate(uint16_t id)
{
  Serial.println("------------------------------------");
  Serial.print("Attempting to load #"); Serial.println(id);
  uint8_t p = finger.loadModel(id);
  switch (p) {
    case FINGERPRINT_OK:
      Serial.print("Template "); Serial.print(id); Serial.println(" loaded");
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    default:
      Serial.print("Unknown error "); Serial.println(p);
      return p;
  }

  Serial.print("Attempting to get #"); Serial.println(id);
  p = finger.getModel();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.print("Template "); Serial.print(id); Serial.println(" transferring:");
      break;
    default:
      Serial.print("Unknown error "); Serial.println(p);
      return p;
  }

  uint8_t bytesReceived[534]; // 2 paquetes de datos
  memset(bytesReceived, 0xff, 534);

  uint32_t starttime = millis();
  int i = 0;
  while (i < 534 && (millis() - starttime) < 20000) {
    if (mySerial.available()) {
      bytesReceived[i++] = mySerial.read();
    }
  }
  Serial.print(i); Serial.println(" bytes read.");
  Serial.println("Decoding packet...");

  uint8_t fingerTemplate[512]; // El template real
  memset(fingerTemplate, 0xff, 512);

  // Filtrando solo los paquetes de datos
  int uindx = 9, index = 0;
  memcpy(fingerTemplate + index, bytesReceived + uindx, 256);   // Primeros 256 bytes
  uindx += 256;
  uindx += 2;    // Saltar el checksum
  uindx += 9;    // Saltar el siguiente header
  index += 256;
  memcpy(fingerTemplate + index, bytesReceived + uindx, 256);   // Siguientes 256 bytes

  for (int i = 0; i < 512; ++i) {
    printHex(fingerTemplate[i], 2);
  }
  Serial.println("\ndone.");

  return p;
}

void printHex(int num, int precision) {
  char tmp[16];
  char format[128];

  sprintf(format, "%%.%dX", precision);
  sprintf(tmp, format, num);
  Serial.print(tmp);
}

void loop() {
  // No hay operación en el loop para este ejemplo
}
