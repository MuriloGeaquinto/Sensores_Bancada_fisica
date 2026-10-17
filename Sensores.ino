#include <LiquidCrystal_I2C.h>

// ================= LCD =================
LiquidCrystal_I2C lcd(0x27, 16, 2);

// ================= PINAGENS =================
// Par A
const int sensorA1Pin = A0;  // ponta "início" (A0 -> A3)
const int sensorA2Pin = A1;
// Par B
const int sensorB1Pin = A2;
const int sensorB2Pin = A3;  // ponta "fim" (A3 -> A0)

// ================= DISTÂNCIAS =================
const float distance_A = 8.0; // cm entre A1 e A2
const float distance_B = 8.0; // cm entre B1 e B2

// ================= LIMIAR =================
const int THRESHOLD = 650;

// ================= TEMPOS POR SENSOR INDIVIDUAL =================
unsigned long tA0 = 0, tA1 = 0, tA2 = 0, tA3 = 0;
bool A0Blocked = false, A1Blocked = false;
bool A2Blocked = false, A3Blocked = false;

// ================= RESULTADOS =================
float velA = 0;
float velB = 0;
float tempoTotal = 0;
float aceleracao = 0;

void setup() {
    lcd.init();
    lcd.backlight();
    mostrarAguardando();
}

// ---------------------------------------------------------------
// Lê cada sensor individualmente e guarda o timestamp do 1º disparo
// Retorna true quando TODOS os 4 já foram interrompidos
// ---------------------------------------------------------------
bool lerTodosSensores() {
    if (!A0Blocked && analogRead(sensorA1Pin) < THRESHOLD) {
        A0Blocked = true; tA0 = millis();
    }
    if (!A1Blocked && analogRead(sensorA2Pin) < THRESHOLD) {
        A1Blocked = true; tA1 = millis();
    }
    if (!A2Blocked && analogRead(sensorB1Pin) < THRESHOLD) {
        A2Blocked = true; tA2 = millis();
    }
    if (!A3Blocked && analogRead(sensorB2Pin) < THRESHOLD) {
        A3Blocked = true; tA3 = millis();
    }

    return (A0Blocked && A1Blocked && A2Blocked && A3Blocked);
}

// ---------------------------------------------------------------
// Calcula velocidades, tempo total e aceleração
// ---------------------------------------------------------------
void calcularTudo() {
    // --- Velocidade do Par A (entre A0 e A1) ---
    unsigned long aIni = min(tA0, tA1);
    unsigned long aFim = max(tA0, tA1);
    float tempoA = (aFim - aIni) / 1000.0;
    velA = (tempoA > 0) ? (distance_A / tempoA) : 0;

    // --- Velocidade do Par B (entre A2 e A3) ---
    unsigned long bIni = min(tA2, tA3);
    unsigned long bFim = max(tA2, tA3);
    float tempoB = (bFim - bIni) / 1000.0;
    velB = (tempoB > 0) ? (distance_B / tempoB) : 0;

    // --- Tempo total (do 1º sensor ao último) ---
    unsigned long tIniTotal = min(min(tA0, tA1), min(tA2, tA3));
    unsigned long tFimTotal = max(max(tA0, tA1), max(tA2, tA3));
    tempoTotal = (tFimTotal - tIniTotal) / 1000.0;

    // Aceleração: se A0 disparou antes (A0 -> A3), usa (velB - velA);
    // caso contrário (A3 -> A0), usa (velA - velB).
    if (tempoTotal > 0) {
        aceleracao = (tA0 < tA3) ? ((velB - velA) / tempoTotal)
                                 : ((velA - velB) / tempoTotal);
    } else {
        aceleracao = 0;
    }
}

// ---------------------------------------------------------------
// Tela "Aguardando..."
// ---------------------------------------------------------------
void mostrarAguardando() {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Aguardando...");
}

// ---------------------------------------------------------------
// Resultados
// ---------------------------------------------------------------
void exibirResultado() {
    // Tela 1: Velocidade em A
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Velocidade em A:");
    lcd.setCursor(0, 1);
    lcd.print(velA, 1);
    lcd.print(" cm/s");
    delay(5500);

    // Tela 2: Velocidade em B
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Velocidade em B:");
    lcd.setCursor(0, 1);
    lcd.print(velB, 1);
    lcd.print(" cm/s");
    delay(5500);

    // Tela 3: Aceleração geral
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Acel. geral:");
    lcd.setCursor(0, 1);
    lcd.print(aceleracao, 2);
    lcd.print(" cm/s2");
    delay(5500);

    // Tela 4: Tempo total
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Tempo total:");
    lcd.setCursor(0, 1);
    lcd.print(tempoTotal, 2);
    lcd.print(" s");
    delay(5500);
}

// ---------------------------------------------------------------
// Reinicia tudo
// ---------------------------------------------------------------
void resetar() {
    A0Blocked = A1Blocked = A2Blocked = A3Blocked = false;
    tA0 = tA1 = tA2 = tA3 = 0;
    velA = velB = 0;
    tempoTotal = 0;
    aceleracao = 0;
}

void loop() {
    // Aguarda todos os 4 sensores serem interrompidos
    if (lerTodosSensores()) {
        calcularTudo();
        exibirResultado();
        resetar();
        mostrarAguardando();
    }
}
