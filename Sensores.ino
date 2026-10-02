#include <Adafruit_LiquidCrystal.h>

Adafruit_LiquidCrystal lcd_1(0);

// Par A
const int sensorA1Pin = A0;
const int sensorA2Pin = A1;
// Par B
const int sensorB1Pin = A2;
const int sensorB2Pin = A3;

// Timestamps de cada sensor (0 = ainda não acionado)
unsigned long timeA1 = 0;
unsigned long timeA2 = 0;
unsigned long timeB1 = 0;
unsigned long timeB2 = 0;

// Distâncias (cm)
const float distance_A = 15.0; // entre A1 e A2
const float distance_B = 15.0; // entre B1 e B2

// Limiar do sensor
const int THRESHOLD = 650;
const unsigned long DEBOUNCE_MS = 50;

int lastTriggered = -1;
unsigned long lastTriggerTime = 0;

void setup() {
    lcd_1.begin(16, 2);
    lcd_1.setCursor(0, 0);
    lcd_1.print("Aguardando...");
}

// Registra o tempo do sensor se ainda não foi acionado
void tryRegister(int sensorId, int pin, unsigned long &timestamp) {
    if (timestamp != 0) return;
    if (analogRead(pin) < THRESHOLD) {
        unsigned long now = millis();
        if (sensorId == lastTriggered && (now - lastTriggerTime) < DEBOUNCE_MS) return;
        timestamp = now;
        lastTriggered = sensorId;
        lastTriggerTime = now;
    }
}

void loop() {
    tryRegister(1, sensorA1Pin, timeA1);
    tryRegister(2, sensorA2Pin, timeA2);
    tryRegister(3, sensorB1Pin, timeB1);
    tryRegister(4, sensorB2Pin, timeB2);

    bool parA_completo = (timeA1 != 0 && timeA2 != 0);
    bool parB_completo = (timeB1 != 0 && timeB2 != 0);

    if (parA_completo && parB_completo) {
        executarTesteCompleto();
        resetar();
    }
    else if (parA_completo && timeB1 == 0 && timeB2 == 0) {
        executarTestePar("A", timeA1, timeA2, distance_A);
        resetar();
    }
    else if (parB_completo && timeA1 == 0 && timeA2 == 0) {
        executarTestePar("B", timeB1, timeB2, distance_B);
        resetar();
    }
}

// ---------- Teste de um único par (bidirecional) ----------
void executarTestePar(const char* nomePar, unsigned long t1, unsigned long t2, float distancia) {
    bool sentidoNormal = (t2 > t1);
    unsigned long tIni = sentidoNormal ? t1 : t2;
    unsigned long tFim = sentidoNormal ? t2 : t1;

    float tempo = (tFim - tIni) / 1000.0;
    float vel = distancia / tempo;

    lcd_1.setCursor(0, 0);
    lcd_1.print("                ");
    lcd_1.setCursor(0, 1);
    lcd_1.print("                ");

    lcd_1.setCursor(0, 0);
    lcd_1.print("Par ");
    lcd_1.print(nomePar);
    lcd_1.print(sentidoNormal ? " ->" : " <-");

    lcd_1.setCursor(0, 1);
    lcd_1.print("t:");
    lcd_1.print(tempo, 2);
    lcd_1.print("s");
    delay(3000);

    lcd_1.setCursor(0, 0);
    lcd_1.print("                ");
    lcd_1.setCursor(0, 1);
    lcd_1.print("                ");
    lcd_1.setCursor(0, 0);
    lcd_1.print("Par ");
    lcd_1.print(nomePar);
    lcd_1.setCursor(0, 1);
    lcd_1.print("v:");
    lcd_1.print(vel, 1);
    lcd_1.print(" cm/s");
    delay(3000);
}

// ---------- Teste completo (bidirecional) ----------
void executarTesteCompleto() {
    // Ordena os 4 eventos por tempo
    struct Evento { unsigned long t; int id; };
    Evento ev[4] = {
        {timeA1, 1}, {timeA2, 2},
        {timeB1, 3}, {timeB2, 4}
    };
    for (int i = 0; i < 3; i++) {
        for (int j = i + 1; j < 4; j++) {
            if (ev[j].t < ev[i].t) {
                Evento tmp = ev[i]; ev[i] = ev[j]; ev[j] = tmp;
            }
        }
    }

    // Sentido global: qual par veio primeiro
    unsigned long primeiroA = min(timeA1, timeA2);
    unsigned long primeiroB = min(timeB1, timeB2);
    bool sentidoAB = (primeiroA < primeiroB);

    // Tempo e velocidade média DENTRO do par A
    unsigned long tA_ini = min(timeA1, timeA2);
    unsigned long tA_fim = max(timeA1, timeA2);
    float tempoA = (tA_fim - tA_ini) / 1000.0;
    float velA = distance_A / tempoA;

    // Tempo e velocidade média DENTRO do par B
    unsigned long tB_ini = min(timeB1, timeB2);
    unsigned long tB_fim = max(timeB1, timeB2);
    float tempoB = (tB_fim - tB_ini) / 1000.0;
    float velB = distance_B / tempoB;

    // Tempo total do trajeto (do primeiro ao último sensor acionado)
    unsigned long tTotalIni = ev[0].t;
    unsigned long tTotalFim = ev[3].t;
    float tempoTotal = (tTotalFim - tTotalIni) / 1000.0;

    // Aceleração média geral do sistema 
    float velInicial = sentidoAB ? velA : velB;
    float velFinal   = sentidoAB ? velB : velA;
    float acelGeral  = (velFinal - velInicial) / tempoTotal;

    // -------- Tela 1: sentido + tempo total --------
    lcd_1.setCursor(0, 0);
    lcd_1.print("                ");
    lcd_1.setCursor(0, 1);
    lcd_1.print("                ");

    lcd_1.setCursor(0, 0);
    lcd_1.print("Sentido: ");
    lcd_1.print(sentidoAB ? "A->B" : "B->A");

    lcd_1.setCursor(0, 1);
    lcd_1.print("Total:");
    lcd_1.print(tempoTotal, 2);
    lcd_1.print("s");
    delay(3000);

    // -------- Tela 2: velocidades médias --------
    lcd_1.setCursor(0, 0);
    lcd_1.print("                ");
    lcd_1.setCursor(0, 1);
    lcd_1.print("                ");
    lcd_1.setCursor(0, 0);
    lcd_1.print("vA:");
    lcd_1.print(velA, 1);
    lcd_1.setCursor(0, 1);
    lcd_1.print("vB:");
    lcd_1.print(velB, 1);
    lcd_1.print(" cm/s");
    delay(3000);

    // -------- Tela 3: aceleração média geral --------
    lcd_1.setCursor(0, 0);
    lcd_1.print("                ");
    lcd_1.setCursor(0, 1);
    lcd_1.print("                ");
    lcd_1.setCursor(0, 0);
    lcd_1.print("a geral:");
    lcd_1.setCursor(0, 1);
    lcd_1.print(acelGeral, 2);
    lcd_1.print(" cm/s2");
    delay(3000);
}

// Zera todos os timestamps para o próximo teste
void resetar() {
    timeA1 = 0;
    timeA2 = 0;
    timeB1 = 0;
    timeB2 = 0;
    lastTriggered = -1;

    lcd_1.setCursor(0, 0);
    lcd_1.print("                ");
    lcd_1.setCursor(0, 1);
    lcd_1.print("                ");
    lcd_1.setCursor(0, 0);
    lcd_1.print("Aguardando...");
    delay(2000);
}
