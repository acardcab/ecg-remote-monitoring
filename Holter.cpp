#include <Adafruit_ADS1X15.h>
#include <SPI.h>
#include <SD.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

Adafruit_ADS1115 ads;

#define CS_PIN 5  // Pin CS para SD

const float FSH       = 475.0f;   // frecuencia de muestreo esperada
const float HPF_FC    = 0.5f;
const float LPF_FC    = 40.0f;
const float NOTCH_F0  = 60.0f;
const float NOTCH_Q   = 30.0f;

// Filtros
float hpf_a = 0.0f;
float hpf_y1 = 0.0f, hpf_x1 = 0.0f;

float nb0, nb1, nb2, na1, na2;
float nz1 = 0.0f, nz2 = 0.0f;

float lb0, lb1, lb2, la1, la2;
float lz1 = 0.0f, lz2 = 0.0f;

// Archivo
File archivo;
String nombreArchivo;

// Buffer de escritura
const int BLOCK_SIZE = 200;
static char bufferSD[BLOCK_SIZE * 20];
static int bufIndex = 0;
static int count = 0;

// Tiempo base
static unsigned long t0 = 0;

// --- Filtros ---
static inline void calcHPF() {
  hpf_a = expf(-2.0f * M_PI * HPF_FC / FSH);
}

static inline void calcNotch() {
  float w0 = 2.0f * M_PI * (NOTCH_F0 / FSH);
  float cw = cosf(w0);
  float sw = sinf(w0);
  float alpha = sw / (2.0f * NOTCH_Q);

  float b0 = 1.0f;
  float b1 = -2.0f * cw;
  float b2 = 1.0f;
  float a0 = 1.0f + alpha;
  float a1 = -2.0f * cw;
  float a2 = 1.0f - alpha;

  nb0 = b0 / a0;
  nb1 = b1 / a0;
  nb2 = b2 / a0;
  na1 = a1 / a0;
  na2 = a2 / a0;
}

static inline void calcLPF2() {
  const float Q = 0.70710678f;
  float w0 = 2.0f * M_PI * (LPF_FC / FSH);
  float cw = cosf(w0);
  float sw = sinf(w0);
  float alpha = sw / (2.0f * Q);

  float b0 = (1.0f - cw) * 0.5f;
  float b1 = 1.0f - cw;
  float b2 = (1.0f - cw) * 0.5f;
  float a0 = 1.0f + alpha;
  float a1 = -2.0f * cw;
  float a2 = 1.0f - alpha;

  lb0 = b0 / a0;
  lb1 = b1 / a0;
  lb2 = b2 / a0;
  la1 = a1 / a0;
  la2 = a2 / a0;
}

static inline float notch60(float x) {
  float y = nb0 * x + nz1;
  nz1 = nb1 * x - na1 * y + nz2;
  nz2 = nb2 * x - na2 * y;
  return y;
}

static inline float lpf40_biquad(float x) {
  float y = lb0 * x + lz1;
  lz1 = lb1 * x - la1 * y + lz2;
  lz2 = lb2 * x - la2 * y;
  return y;
}

// Buscar nombre de archivo disponible
String nextFileName() {
  int idx = 1;
  String fname = "/ecg_data.txt";
  while (SD.exists(fname)) {
    idx++;
    fname = "/ecg_data" + String(idx) + ".txt";
  }
  return fname;
}

void setup(void) {
  Serial.begin(115200);
  Serial.println("ECG ADS1115 modo continuo + SD");

  ads.setGain(GAIN_ONE);
  ads.setDataRate(RATE_ADS1115_475SPS);

  if (!ads.begin()) {
    Serial.println("Fallo al iniciar ADS1115");
    while (1);
  }

  // --- Configurar modo continuo ---
  ads.startADCReading(ADS1X15_REG_CONFIG_MUX_SINGLE_0, true);

  if (!SD.begin(CS_PIN)) {
    Serial.println("Fallo al iniciar SD");
    while (1);
  }

  nombreArchivo = nextFileName();
  archivo = SD.open(nombreArchivo.c_str(), FILE_WRITE);
  if (archivo) {
    archivo.println("tiempo_ms,valor");
    archivo.flush();
    Serial.print("Archivo creado: ");
    Serial.println(nombreArchivo);
  } else {
    Serial.println("No se pudo crear archivo");
    while (1);
  }

  calcHPF();
  calcNotch();
  calcLPF2();
  t0 = millis();
}

void loop(void) {
  // Leer directamente el registro de conversión en modo continuo
  int16_t adc0 = ads.getLastConversionResults();
  float x = (float)adc0;

  // Filtros
  float y_hpf   = hpf_a * (hpf_y1 + x - hpf_x1);
  hpf_y1        = y_hpf;
  hpf_x1        = x;

  float y_notch = notch60(y_hpf);
  float y       = lpf40_biquad(y_notch);

  unsigned long t_now = millis() - t0;

  // Guardar en buffer
  int n = snprintf(bufferSD + bufIndex, sizeof(bufferSD) - bufIndex,
                   "%lu,%f\n", t_now, y);
  bufIndex += n;
  count++;

  if (count >= BLOCK_SIZE) {
    if (archivo) {
      archivo.write((uint8_t*)bufferSD, bufIndex);
      archivo.flush();
    }
    bufIndex = 0;
    count = 0;
  }
}
