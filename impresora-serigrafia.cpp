// ====== Pines del driver y microstepping ======
const int dirPin  = 2;
const int stepPin = 3;
const int M0 = 6;
const int M1 = 5;
const int M2 = 4;

// ====== Pines de fin de carrera (normalmente abiertos a GND) ======
const int LEFT_ENDSTOP_PIN  = 7;  // Cambia si lo necesitas
const int RIGHT_ENDSTOP_PIN = 8;  // Cambia si lo necesitas

// ====== Parámetros del motor / movimiento ======
const int stepsPerRevolution = 200;   // Paso completo del motor (sin microstepping)
const unsigned int STEP_DELAY_US = 150;  // Velocidad (mayor = más lento)
const int BACKOFF_STEPS = 100;        // Pasos para liberar el switch al invertir
const long MAX_TRAVEL_STEPS = 100000; // Failsafe por si un switch no funciona

// ====== Ciclos de ida y vuelta ======
const int ROUND_TRIPS = 5; // Número de ciclos completos (derecha + izquierda)

// ====== Utilidades ======
inline void stepPulse(unsigned int us) {
  digitalWrite(stepPin, HIGH);
  delayMicroseconds(us);
  digitalWrite(stepPin, LOW);
  delayMicroseconds(us);
}

// Antirrebote simple: confirma lectura LOW estable
bool endstopPressed(int pin) {
  if (digitalRead(pin) == LOW) {
    delay(10); // 10 ms de antirrebote
    return (digitalRead(pin) == LOW);
  }
  return false;
}

// Mueve en una dirección hasta que se active el endstop correspondiente
// dirRight = true  -> mover hacia derecha (usa RIGHT_ENDSTOP_PIN)
// dirRight = false -> mover hacia izquierda (usa LEFT_ENDSTOP_PIN)
bool moveUntilLimit(bool dirRight) {
  digitalWrite(dirPin, dirRight ? HIGH : LOW);

  const int targetPin = dirRight ? RIGHT_ENDSTOP_PIN : LEFT_ENDSTOP_PIN;
  long steps = 0;

  while (steps < MAX_TRAVEL_STEPS) {
    if (endstopPressed(targetPin)) {
      // Llegó al extremo: aplica un pequeño backoff para soltar el switch
      digitalWrite(dirPin, dirRight ? LOW : HIGH); // invertir momentáneamente
      for (int i = 0; i < BACKOFF_STEPS; i++) stepPulse(STEP_DELAY_US);
      return true;
    }
    stepPulse(STEP_DELAY_US);
    steps++;
  }
  // Failsafe: no encontró el switch
  return false;
}

// Si al iniciar algún switch está pulsado, se aleja unos pasos para soltarlo
void releaseIfPressed() {
  if (endstopPressed(LEFT_ENDSTOP_PIN)) {
    // Aléjate hacia la derecha
    digitalWrite(dirPin, HIGH);
    for (int i = 0; i < BACKOFF_STEPS * 5; i++) stepPulse(STEP_DELAY_US);
  }
  if (endstopPressed(RIGHT_ENDSTOP_PIN)) {
    // Aléjate hacia la izquierda
    digitalWrite(dirPin, LOW);
    for (int i = 0; i < BACKOFF_STEPS * 5; i++) stepPulse(STEP_DELAY_US);
  }
}

void setup() {
  // Pines de control del driver
  pinMode(stepPin, OUTPUT);
  pinMode(dirPin,  OUTPUT);
  pinMode(M0, OUTPUT);
  pinMode(M1, OUTPUT);
  pinMode(M2, OUTPUT);

  // Fines de carrera con pull-up interno
  pinMode(LEFT_ENDSTOP_PIN, INPUT_PULLUP);
  pinMode(RIGHT_ENDSTOP_PIN, INPUT_PULLUP);

  // Configura microstepping (ajusta según tu driver)
  // A4988: LOW,LOW,HIGH = 1/16; DRV8825: HIGH,LOW,LOW = 1/2 (varía por driver)
  // Dejamos tu configuración previa: LOW, LOW, HIGH
  digitalWrite(M0, LOW);
  digitalWrite(M1, LOW);
  digitalWrite(M2, HIGH);

  // Opcional: Serial para diagnóstico
  Serial.begin(115200);
  Serial.println("Inicio: barridos con fin de carrera (NO)");

  // Asegura que no arrancamos con un switch presionado
  releaseIfPressed();
}

void loop() {
  // Ejecuta ROUND_TRIPS ciclos (derecha + izquierda)
  for (int c = 1; c <= ROUND_TRIPS; c++) {
    Serial.print("Ciclo ");
    Serial.print(c);
    Serial.println(" -> hacia la DERECHA");

    if (!moveUntilLimit(true)) {
      Serial.println("ERROR: no se detecto fin de carrera DERECHA. Revisar cableado.");
      while (1); // Detener por seguridad
    }

    delay(200); // breve pausa

    Serial.print("Ciclo ");
    Serial.print(c);
    Serial.println(" -> hacia la IZQUIERDA");

    if (!moveUntilLimit(false)) {
      Serial.println("ERROR: no se detecto fin de carrera IZQUIERDA. Revisar cableado.");
      while (1); // Detener por seguridad
    }

    delay(400); // pausa entre ciclos (opcional)
  }

  // Terminados los 5 ciclos, puedes detener o repetir.
  // Aquí detenemos indefinidamente.
  Serial.println("Listo: se completaron los ciclos de ida y vuelta.");
  while (1);
}
