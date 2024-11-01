#include <Adafruit_Fingerprint.h>

// Configuración del puerto Serial para el sensor dactilar (Serial2 en ESP32)
// Pines RX = IO16, TX = IO17
HardwareSerial mySerial(2);

// Inicializa la instancia del sensor de huellas digitales
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

void setup() {
  // Inicializamos el Serial0 para la comunicación con la computadora
  Serial.begin(9600);
  while (!Serial);  // Espera hasta que el Serial esté listo
  delay(100);

  Serial.println("\n\nDeleting all fingerprint templates!");
  Serial.println("Press 'Y' key to continue");

  // Espera a que se presione la tecla 'Y' en el monitor serial para continuar
  while (1) {
    if (Serial.available() && (Serial.read() == 'Y')) {
      break;
    }
  }

  // Configuramos Serial2 para el sensor de huellas (RX = IO16, TX = IO17)
  mySerial.begin(57600, SERIAL_8N1, 16, 17);  // 57600 baudios, 8 bits, sin paridad, 1 bit de stop

  // Verifica la conexión con el sensor
  if (finger.verifyPassword()) {
    Serial.println("Found fingerprint sensor!");
  } else {
    Serial.println("Did not find fingerprint sensor :(");
    while (1);  // Si no se encuentra el sensor, se detiene aquí
  }

  // Vacía la base de datos de huellas dactilares en el sensor
  finger.emptyDatabase();

  Serial.println("Now database is empty :)");
}

void loop() {
  // No es necesario ejecutar nada en el loop, ya que el código solo vacía la base de datos una vez.
}
