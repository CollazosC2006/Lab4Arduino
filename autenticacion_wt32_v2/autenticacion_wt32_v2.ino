#include <Adafruit_Fingerprint.h>
#include <WiFi.h>        // Biblioteca para WiFi en ESP32
#include <HTTPClient.h>  // Biblioteca para hacer solicitudes HTTP
#include <ArduinoJson.h> // Biblioteca para manejar JSON

// Configuración del puerto serial para el sensor dactilar y la ESP32
HardwareSerial mySerial(2);  // Serial2 en ESP32 (pines IO17 para TX y IO16 para RX)
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

uint8_t fingerTemplate[512]; // Template de la huella
String backendServerURL = "http://192.168.10.20:8000/verificar_acceso/";  // Cambia a la URL de tu backend

void setup() {
  Serial.begin(9600);
  WiFi.begin("SSID", "password");  // Reemplaza con tu red WiFi

  // Esperar a la conexión WiFi
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Conectando a WiFi...");
  }
  Serial.println("Conexión WiFi establecida");
  
  // Configuración del sensor de huella
  mySerial.begin(57600, SERIAL_8N1, 5, 17);
  while (!finger.verifyPassword()) {
    Serial.println("No se encontró el sensor de huella :(");
    delay(500);
  }
  finger.getParameters();
}

void loop() {
  if (getFingerprintID()) {
    finger.LEDcontrol(FINGERPRINT_LED_FLASHING, 25, FINGERPRINT_LED_BLUE, 10);
  } else {
    finger.LEDcontrol(FINGERPRINT_LED_FLASHING, 25, FINGERPRINT_LED_RED, 10);
  }
  delay(2000);  // Espera para evitar múltiples lecturas seguidas
}

bool getFingerprintID() {
  uint8_t p = -1;
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    if (p == FINGERPRINT_OK) {
      Serial.println("Imagen de huella tomada");
      finger.LEDcontrol(FINGERPRINT_LED_ON, 0, FINGERPRINT_LED_PURPLE);
    } else if (p == FINGERPRINT_NOFINGER) {
      Serial.println(".");
    } else {
      Serial.println("Error desconocido");
      return false;
    }
  }

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK) {
    Serial.println("Error al convertir la imagen");
    return false;
  }

  p = finger.fingerSearch();
  String macAddress = WiFi.macAddress();  // Obtener la MAC
  String jsonToSend;
  
  if (p == FINGERPRINT_OK) {
    Serial.println("Huella coincidente encontrada");
    jsonToSend = "{\"access\":{\"id\":\"" + String(finger.fingerID) + "\",\"access\":\"1\",\"mac\":\"" + macAddress + "\"}}";
  } else if (p == FINGERPRINT_NOTFOUND) {
    Serial.println("No se encontró coincidencia de huella");
    jsonToSend = "{\"access\":{\"id\":\"" + String(finger.fingerID) + "\",\"access\":\"0\",\"mac\":\"" + macAddress + "\"}}";
  } else {
    Serial.println("Error desconocido en la búsqueda de huella");
    return false;
  }

  return forwardPostToBackend(jsonToSend);
}

// Función para enviar el JSON al backend
bool forwardPostToBackend(String payload) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(backendServerURL);
    http.addHeader("Content-Type", "application/json");

    int httpResponseCode = http.POST(payload);
    if (httpResponseCode == 200) {
      String response = http.getString();
      Serial.println("Código de respuesta del backend: " + String(httpResponseCode));
      Serial.println("Respuesta del backend: " + response);
      http.end();
      return true;
    } else {
      Serial.println("Error en la solicitud POST: " + String(httpResponseCode));
      http.end();
      return false;
    }
  } else {
    Serial.println("Error: No hay conexión WiFi");
    return false;
  }
}
