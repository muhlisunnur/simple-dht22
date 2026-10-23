#include <DHT.h>

// Tentukan pin data ESP32 yang terhubung ke sensor DHT22
#define DHTPIN 15     

// Tentukan tipe sensor: DHT22 (AM2302) atau DHT11
#define DHTTYPE DHT22   

// Inisialisasi objek DHT
DHT dht(DHTPIN, DHTTYPE);

void setup() {
  // Memulai komunikasi serial dengan kecepatan 115200 baud
  Serial.begin(115200);
  Serial.println(F("Inisialisasi Sensor DHT22..."));

  // Memulai pembacaan sensor DHT
  dht.begin();
}

void loop() {
  // Beri jeda 2 detik antar pembacaan
  delay(2000);

  // Membaca kelembaban (humidity)
  float h = dht.readHumidity();
  // Membaca suhu dalam Celsius (C)
  float t = dht.readTemperature();

  // Cek jika ada kegagalan pembacaan dari sensor
  if (isnan(h) || isnan(t)) {
    Serial.println(F("Gagal membaca dari sensor DHT22! Periksa kembali kabel atau pin."));
    return;
  }

  // Menampilkan hasil ke Serial Monitor
  Serial.print(F("Kelembaban: "));
  Serial.print(h);
  Serial.print(F("%  |  Suhu: "));
  Serial.print(t);
  Serial.println(F(" °C"));
}
