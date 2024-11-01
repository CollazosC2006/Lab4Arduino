#include <Adafruit_Fingerprint.h>
#include <WiFi.h>          // Biblioteca para Wi-Fi en ESP32
#include <HTTPClient.h>    // Biblioteca para hacer solicitudes HTTP
#include <ArduinoJson.h>   // Biblioteca para manejar JSON

// Configuración Wi-Fi
const char* ssid = "X3 pro";            // Reemplaza con el nombre de tu red Wi-Fi
const char* password = "a1234567";  // Reemplaza con la contraseña de tu red Wi-Fi

// Configuramos Serial2 para el sensor dactilar y Serial0 para la comunicación con la PC
HardwareSerial mySerial(2); // Serial2 en ESP32 (pines IO17 para TX y IO16 para RX)

Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

uint8_t id = 1;
uint8_t fingerTemplate[512]; // El template real

void setup() {
  Serial.begin(9600); 
  delay(100);

  // Conexión a Wi-Fi
  Serial.print("Conectando a Wi-Fi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConectado a Wi-Fi.");
  Serial.print("Dirección IP: ");
  Serial.println(WiFi.localIP());

  // Configuración del puerto serial para el sensor dactilar
  mySerial.begin(57600, SERIAL_8N1, 16, 17); // RX = IO16, TX = IO17
  
  // Aseguramos la conexión del sensor
  while (!finger.verifyPassword()) {
    Serial.println("Did not find fingerprint sensor :(");
    delay(500);
  }
  
  finger.getParameters();
}

void loop() {
  while (!getFingerprintEnroll());
  if (downloadFingerprintTemplate(id)) {
    sendFingerprintTemplate(fingerTemplate);
  }
  delay(1000);
}

// Convierte el arreglo de huellas dactilares a una cadena hexadecimal
String fingerprintToHexString(uint8_t *data, size_t length) {
  String hexString = "";
  for (size_t i = 0; i < length; i++) {
    if (data[i] < 16) hexString += "0";
    hexString += String(data[i], HEX);
  }
  return hexString;
}

// Función para enviar el template de la huella al servidor
void sendFingerprintTemplate(uint8_t* fingerprintData) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String serverUrl = "http://192.168.6.27:8000/autenticar_huella";

    Serial.println("Convirtiendo template de huella a cadena hexadecimal...");
    String fingerprintHex = fingerprintToHexString(fingerTemplate, 512);

    // Preparar el payload en formato application/x-www-form-urlencoded
    String payload = "template=" + fingerprintHex + "&porteria=" + String(1);

    http.begin(serverUrl);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");

    Serial.println("Enviando solicitud POST al servidor...");
    int httpCode = http.POST(payload);

    if (httpCode > 0) {
      Serial.printf("Código de respuesta HTTP: %d\n", httpCode);
      String response = http.getString();

      DynamicJsonDocument doc(1024);
      deserializeJson(doc, response);
      bool isValid = doc["valid"];  // Obtener el valor de "valid" en la respuesta JSON

      // Realizar acción basada en la respuesta booleana
      if (isValid) {
        Serial.println("La huella es válida.");
        finger.LEDcontrol(FINGERPRINT_LED_FLASHING, 25, FINGERPRINT_LED_BLUE, 10);
      } else {
        Serial.println("La huella es inválida.");
        finger.LEDcontrol(FINGERPRINT_LED_FLASHING, 25, FINGERPRINT_LED_RED, 10);
      }
    } else {
      Serial.printf("Error al enviar POST: %s\n", http.errorToString(httpCode).c_str());
      finger.LEDcontrol(FINGERPRINT_LED_FLASHING, 25, FINGERPRINT_LED_RED, 10);
    }
    http.end();
  } else {
    Serial.println("No está conectado a Wi-Fi.");
  }
}

bool downloadFingerprintTemplate(uint16_t id) {
  Serial.println("------------------------------------");
  Serial.print("Attempting to load #"); Serial.println(id);
  uint8_t p = finger.loadModel(id);
  if (p != FINGERPRINT_OK) return false;

  p = finger.getModel();
  if (p != FINGERPRINT_OK) return false;

  uint8_t bytesReceived[534];
  memset(bytesReceived, 0xff, 534);

  uint32_t starttime = millis();
  int i = 0;
  while (i < 534 && (millis() - starttime) < 20000) {
    if (mySerial.available()) {
      bytesReceived[i++] = mySerial.read();
    }
  }
  Serial.printf("%d bytes read.\n", i);
  
  memset(fingerTemplate, 0xff, 512);

  int uindx = 9, index = 0;
  memcpy(fingerTemplate + index, bytesReceived + uindx, 256);
  uindx += 256 + 2 + 9;
  index += 256;
  memcpy(fingerTemplate + index, bytesReceived + uindx, 256);

  Serial.println("Done.");
  return true;
}

/*
uint8_t getFingerprintEnroll() {
  int p = finger.getImage();
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
  }
  finger.LEDcontrol(FINGERPRINT_LED_ON, 0, FINGERPRINT_LED_PURPLE);
  p = finger.image2Tz(1);
  if (p != FINGERPRINT_OK){
    finger.LEDcontrol(FINGERPRINT_LED_FLASHING, 25, FINGERPRINT_LED_RED, 10);
    return false;
  }

  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
  }

  finger.LEDcontrol(FINGERPRINT_LED_OFF, 0, FINGERPRINT_LED_PURPLE);

  p = finger.image2Tz(2);
  if (p != FINGERPRINT_OK){
    finger.LEDcontrol(FINGERPRINT_LED_FLASHING, 25, FINGERPRINT_LED_RED, 10);
    return false;
  };

  if(finger.createModel() == FINGERPRINT_OK && finger.storeModel(id) == FINGERPRINT_OK){
    return true;
  }else{
    finger.LEDcontrol(FINGERPRINT_LED_FLASHING, 25, FINGERPRINT_LED_RED, 10);
    return false;
  }

}*/

uint8_t getFingerprintEnroll() {
  int p = -1;
  Serial.print("Waiting for valid finger to enroll as #"); Serial.println(id);
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
    case FINGERPRINT_OK:
      finger.LEDcontrol(FINGERPRINT_LED_ON, 0, FINGERPRINT_LED_PURPLE);
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.print(".");
      break;
    default:
      finger.LEDcontrol(FINGERPRINT_LED_FLASHING, 25, FINGERPRINT_LED_RED, 10);
      delay(2000);
      Serial.println("Unknown error");
      break;
    }
  }

  p = finger.image2Tz(1);
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    default:
      Serial.println("Unknown error");
      finger.LEDcontrol(FINGERPRINT_LED_FLASHING, 25, FINGERPRINT_LED_RED, 10);
      delay(2000);
      return p;
  }

  Serial.println("Remove finger");
  delay(10);
  
  
  
  Serial.print("ID "); Serial.println(id);
  p = -1;
  Serial.println("Place same finger again");
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      finger.LEDcontrol(FINGERPRINT_LED_OFF, 0, FINGERPRINT_LED_PURPLE);
      break;
    default:
      Serial.println("Unknown error");
      finger.LEDcontrol(FINGERPRINT_LED_FLASHING, 25, FINGERPRINT_LED_RED, 10);
      delay(2000);
      break;
    }
  }

  p = finger.image2Tz(2);
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    default:
      Serial.println("Unknown error");
      finger.LEDcontrol(FINGERPRINT_LED_FLASHING, 25, FINGERPRINT_LED_RED, 10);
      delay(2000);
      return p;
  }

  Serial.print("Creating model for #");  Serial.println(id);

  p = finger.createModel();
  if (p == FINGERPRINT_OK) {
    Serial.println("Prints matched!");
  } else {
    Serial.println("Unknown error");
    finger.LEDcontrol(FINGERPRINT_LED_FLASHING, 25, FINGERPRINT_LED_RED, 10);
    delay(2000);
    return p;
  }

  p = finger.storeModel(id);
  if (p == FINGERPRINT_OK) {
    Serial.println("Stored!");
  }else {
    Serial.println("Unknown error");
    finger.LEDcontrol(FINGERPRINT_LED_FLASHING, 25, FINGERPRINT_LED_RED, 10);
    delay(2000);
    return p;
  }

  return true;
}
