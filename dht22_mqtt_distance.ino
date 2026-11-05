#include <DHT.h>
#include <WiFi.h>
#include <PubSubClient.h>

// --- KONFIGURASI WIFI & MQTT ---
const char* ssid = "NAMA_WIFI_ANDA";         // Ganti dengan SSID WiFi Anda
const char* password = "PASSWORD_WIFI_ANDA"; // Ganti dengan Password WiFi Anda
const char* mqtt_server = "test.mosquitto.org";

WiFiClient espClient;
PubSubClient client(espClient);

// --- PIN & KONFIGURASI SENSOR ---
#define DHTPIN 4     
#define DHTTYPE DHT22   
DHT dht(DHTPIN, DHTTYPE);

// Pin untuk HC-SR04 (Tanpa Library)
const int trigPin = 5;
const int echoPin = 18;

unsigned long lastMsg = 0;

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Menghubungkan ke ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi terhubung!");
  Serial.print("Alamat IP: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  // Loop sampai terhubung ke broker MQTT
  while (!client.connected()) {
    Serial.print("Menghubungkan ke MQTT broker...");
    // Buat ID klien yang unik
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);
    
    if (client.connect(clientId.c_str())) {
      Serial.println("terhubung!");
    } else {
      Serial.print("gagal, rc=");
      Serial.print(client.state());
      Serial.println(" coba lagi dalam 5 detik");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  delay(2000);
  
  // Inisialisasi pin HC-SR04
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  // Inisialisasi Sensor DHT22
  dht.begin();

  // Koneksi WiFi & MQTT
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  
  Serial.println(F("Sistem siap. Memulai pembacaan sensor..."));
}

// Fungsi untuk membaca jarak dari HC-SR04 tanpa library
float bacaJarakCM() {
  // Kirim sinyal trigger 10 mikrodetik
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // Baca waktu pantulan pada pin echo (timeout 30ms / sekitar 5 meter)
  long durasi = pulseIn(echoPin, HIGH, 30000);
  
  if (durasi == 0) {
    return -1; // Menandakan di luar jangkauan atau error pembacaan
  }

  // Hitung jarak dalam cm (Kecepatan suara sekitar 0.034 cm/mikrodetik dibagi 2 perjalanan)
  float jarak = durasi * 0.034 / 2;
  return jarak;
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Jeda antar pembacaan sensor (mengikuti batasan minimum DHT22 yaitu ~2 detik)
  delay(2000);

  // 1. Baca Sensor DHT22
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (isnan(h) || isnan(t)) {
    Serial.println(F("Gagal membaca data dari sensor DHT22!"));
    return;
  }

  // 2. Baca Sensor HC-SR04
  float jarak = bacaJarakCM();

  // Tampilkan hasil pembacaan ke Serial Monitor
  Serial.print(F("Suhu: "));
  Serial.print(t);
  Serial.print(F("°C  |  Kelembapan: "));
  Serial.print(h);
  Serial.print(F("%  |  Jarak: "));
  if (jarak == -1) {
    Serial.println(F("Di luar jangkauan"));
  } else {
    Serial.print(jarak);
    Serial.println(F(" cm"));
  }

  // 3. Kirim data ke MQTT jika jarak < 5 cm
  if (jarak > 0 && jarak < 5.0) {
    Serial.println(F("--> Jarak < 5 cm terdeteksi! Mengirim data ke MQTT..."));
    
    // Format payload JSON sederhana
    String payload = "{ \"jarak_cm\": " + String(jarak) + ", \"suhu_c\": " + String(t) + ", \"kelembapan\": " + String(h) + " }";
    
    // Publikasikan ke topik MQTT (sesuaikan nama topik jika diperlukan)
    if (client.publish("esp32/sensor/peringatan_jarak", payload.c_str())) {
      Serial.println(F("Data berhasil dikirim ke test.mosquitto.org!"));
    } else {
      Serial.println(F("Gagal mengirim data MQTT."));
    }
  }
}
