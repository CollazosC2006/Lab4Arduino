#include <Adafruit_Fingerprint.h>
#include <WiFi.h>        // Biblioteca para WiFi en 
#include <ETH.h>   // Biblioteca para Ethernet en ESP32

#include <HTTPClient.h>  // Biblioteca para hacer solicitudes HTTP
#include <ArduinoJson.h> // Biblioteca para manejar JSON

// Configuración del puerto serial para el sensor dactilar y la ESP32
HardwareSerial mySerial(2);  // Serial2 en ESP32 (pines IO17 para TX y IO16 para RX)
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

#define ETH_ADDR        1
#define ETH_POWER_PIN   16
#define ETH_POWER_PIN_ALTERNATIVE 16 
#define ETH_MDC_PIN    23
#define ETH_MDIO_PIN   18
#define ETH_TYPE       ETH_PHY_LAN8720
#define ETH_CLK_MODE    ETH_CLOCK_GPIO0_IN 

uint8_t fingerTemplate[512]; // Template de la huella
String backendServerURL = "http://192.168.10.20:8000/verificar_acceso/";  // Cambia a la URL de tu backend

void WiFiEvent(arduino_event_id_t event) {
  switch (event) {
    case ARDUINO_EVENT_ETH_START:
      Serial.println("Ethernet iniciado");
      break;
    case ARDUINO_EVENT_ETH_CONNECTED:
      Serial.println("Ethernet conectado");
      break;
    case ARDUINO_EVENT_ETH_GOT_IP:
      Serial.print("Dirección IP: ");
      Serial.println(ETH.localIP());
      break;
    case ARDUINO_EVENT_ETH_DISCONNECTED:
      Serial.println("Ethernet desconectado");
      break;
    default:
      break;
  }
}

void setup() {
  Serial.begin(9600);

  // Esperar a la conexión WiFi
  ETH.begin(ETH_TYPE, ETH_ADDR, ETH_MDC_PIN, ETH_MDIO_PIN, ETH_POWER_PIN, ETH_CLK_MODE);

  // Registrar el evento para el estado de Ethernet
  WiFi.onEvent(WiFiEvent);
  
  // Configuración del sensor de huella
  mySerial.begin(57600, SERIAL_8N1, 5, 17);
  while (!finger.verifyPassword()) {
    Serial.println("No se encontró el sensor de huella :(");
    delay(500);
  }
  finger.getParameters();
  Serial.println(ETH.localIP());
  Serial.println(ETH.localIP());
  Serial.println(ETH.localIP());

}

void loop() {
  if (getFingerprintID()) {
    finger.LEDcontrol(FINGERPRINT_LED_FLASHING, 25, FINGERPRINT_LED_BLUE, 10);
  } else {
    finger.LEDcontrol(FINGERPRINT_LED_FLASHING, 25, FINGERPRINT_LED_RED, 10);
  }
  delay(1000);  // Espera para evitar múltiples lecturas seguidas
}

bool getFingerprintID() {
  uint8_t p = -1;
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    if (p == FINGERPRINT_OK) {
      Serial.println("Imagen de huella tomada");
      finger.LEDcontrol(FINGERPRINT_LED_ON, 0, FINGERPRINT_LED_PURPLE);
    } else if (p == FINGERPRINT_NOFINGER) {
      Serial.print(".");
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
  String macAddress = "AA:C7:69:E4:12:9B";  // Obtener la MAC
  String jsonToSend;
  
  if (p == FINGERPRINT_OK) {
    Serial.println("Huella coincidente encontrada");
    jsonToSend = "{\"id\":\"" + String(finger.fingerID) + "\",\"access\":\"1\",\"mac\":\"" + macAddress + "\"}";
    Serial.println(jsonToSend);
  } else if (p == FINGERPRINT_NOTFOUND) {
    Serial.println("No se encontró coincidencia de huella");
    jsonToSend = "{\"id\":\"" + String(99999) + "\",\"access\":\"0\",\"mac\":\"" + macAddress + "\"}";
    Serial.println(jsonToSend);
  } else {
    Serial.println("Error desconocido en la búsqueda de huella");
    return false;
  }

  return forwardPostToBackend(jsonToSend);
}

// Función para enviar el JSON al backend
bool forwardPostToBackend(String payload) {
  if (ETH.linkUp()) {
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
