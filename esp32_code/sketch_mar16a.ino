#include <WiFi.h>
#include <FirebaseESP32.h>
#include <config.h>


FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

const int pinRelay1 = 32; 
const int pinRelay2 = 27; 

void tokenStatusCallback(TokenInfo info) {
    if (info.status == token_status_error) {
        Serial.printf("Error de Token: %s\n", info.error.message.c_str());
    }
}

void setup() {
  Serial.begin(115200);
  pinMode(pinRelay1, OUTPUT);
  pinMode(pinRelay2, OUTPUT);
  digitalWrite(pinRelay1, LOW);
  digitalWrite(pinRelay2, LOW);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Conectando a Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n¡Wi-Fi Conectado!");

  config.api_key = API_KEY;
  config.database_url = FIREBASE_HOST;
  config.signer.tokens.legacy_token = FIREBASE_SECRET;
  config.token_status_callback = tokenStatusCallback;

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  // CAMBIO CLAVE: Escuchar la carpeta completa para detectar ambos estados
  Serial.println("Iniciando Stream en /dispositivo1...");
  if (!Firebase.beginStream(fbdo, "/dispositivo1")) {
    Serial.printf("Error en Stream: %s\n", fbdo.errorReason().c_str());
  } else {
    Serial.println("¡Escuchando cambios para Relay 1 y 2!");
  }
}

void loop() {
  if (!Firebase.readStream(fbdo)) {
    Serial.printf("Error de lectura: %s\n", fbdo.errorReason().c_str());
  }

  if (fbdo.streamAvailable()) {
    String path = fbdo.dataPath(); // Esto devolverá "/estado" o "/estado2"
    int valor = fbdo.intData();

    if (path == "/estado") {
      digitalWrite(pinRelay1, valor == 1 ? HIGH : LOW);
      Serial.printf("Relay 1 -> %s\n", valor == 1 ? "ON" : "OFF");
    } 
    else if (path == "/estado2") {
      digitalWrite(pinRelay2, valor == 1 ? HIGH : LOW);
      Serial.printf("Relay 2 -> %s\n", valor == 1 ? "ON" : "OFF");
    }
  }
}