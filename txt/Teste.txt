#include <LiquidCrystal_I2C.h>

// ================= LCD =================
LiquidCrystal_I2C lcd(0x27, 16, 2);

// ================= PINAGENS =================
// Par A
const int sensorA1Pin = A0;  // A1
const int sensorA2Pin = A1;  // A2
// Par B
const int sensorB1Pin = A2;  // B1
const int sensorB2Pin = A3;  // B2

// ================= LIMIAR =================
const int THRESHOLD = 650;

// ================= ESTADO DOS SENSORES =================
bool A1Blocked = false;
bool A2Blocked = false;
bool B1Blocked = false;
bool B2Blocked = false;

void setup() {
    lcd.init();
    lcd.backlight();
    mostrarAguardando();
}

// ---------------------------------------------------------------
// Tela inicial: todos os sensores como "--"
//   A1:--  B1:--
//   A2:--  B2:--
// ---------------------------------------------------------------
void mostrarAguardando() {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("A1:--  B1:--");
    lcd.setCursor(0, 1);
    lcd.print("A2:--  B2:--");
}

// ---------------------------------------------------------------
// Atualiza o LCD no instante em que um sensor é interrompido.
// Cada sensor tem posição fixa na tela.
// ---------------------------------------------------------------
void atualizarSensor(int coluna, int linha, const char* nome, bool interrompido) {
    lcd.setCursor(coluna, linha);
    lcd.print(nome);
    lcd.print(interrompido ? ":OK" : ":--");
}

void loop() {
    // ---- A1 ----
    if (!A1Blocked && analogRead(sensorA1Pin) < THRESHOLD) {
        A1Blocked = true;
        atualizarSensor(0, 0, "A1", true);
    }

    // ---- B1 ----
    if (!B1Blocked && analogRead(sensorB1Pin) < THRESHOLD) {
        B1Blocked = true;
        atualizarSensor(8, 0, "B1", true);
    }

    // ---- A2 ----
    if (!A2Blocked && analogRead(sensorA2Pin) < THRESHOLD) {
        A2Blocked = true;
        atualizarSensor(0, 1, "A2", true);
    }

    // ---- B2 ----
    if (!B2Blocked && analogRead(sensorB2Pin) < THRESHOLD) {
        B2Blocked = true;
        atualizarSensor(8, 1, "B2", true);
    }

    // Se todos já foram interrompidos, reseta após um tempo
    if (A1Blocked && A2Blocked && B1Blocked && B2Blocked) {
        delay(4000);   // mantém o resultado na tela por 4 s
        A1Blocked = A2Blocked = B1Blocked = B2Blocked = false;
        mostrarAguardando();
    }
}
