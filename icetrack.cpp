#include <DHT.h>
#include <LiquidCrystal_I2C.h>

#define DHT_PIN      8
#define DHT_TYPE     DHT22
#define LDR_PIN      A0
#define LED_PIN      13

// Limites de temperatura para calculo do indice de degelo
// Referencia: IPCC AR6 (2021) - Chapter 9: Ocean, Cryosphere and Sea Level Change
#define TEMP_CRITICA  -2.0   // °C — limiar de alerta de degelo ativo
#define TEMP_MIN     -30.0   // °C — temperatura polar tipica (degelo zero)
#define TEMP_MAX      10.0   // °C — temperatura maxima da faixa polar monitorada

// Limites de albedo glacial (fracao de luz refletida, em %)
// Referencia: NSIDC / IPCC AR6 — Surface Albedo of Snow and Ice
#define ALBEDO_GELO_SUJO    30   // % — gelo antigo, com algas ou particulas de carbono
#define ALBEDO_GELO_FRESCO  90   // % — neve recente ou gelo limpo recém-formado
// Agua exposta onde havia gelo tem albedo ~6% (nao modelada aqui — fora do range do sensor)

#define INTERVALO_MS  1000

DHT dht(DHT_PIN, DHT_TYPE);
LiquidCrystal_I2C lcd(0x27, 16, 2);

unsigned long ultimaLeitura = 0;
float tempAtual    = 0.0;
float albedoPorc   = 0.0;
int   indiceDegelo = 0;
bool  alertaAtivo  = false;
bool  ledState     = false;

void setup() {
  Serial.begin(9600);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  dht.begin();
  delay(2000);
  lcd.init();
  lcd.backlight();

  lcd.setCursor(0, 0);
  lcd.print("  IceTrack v1.0 ");
  lcd.setCursor(0, 1);
  lcd.print(" Iniciando...   ");
  delay(2000);
  lcd.clear();

  Serial.println("===========================================");
  Serial.println("  IceTrack - Estacao de Monitoramento Polar");
  Serial.println("  FIAP Global Solution 2026");
  Serial.println("===========================================");
  Serial.println("Timestamp(ms) | Temp(C) | Albedo(%) | IDegelo(%) | Alerta");
  Serial.println("-------------------------------------------");
}

void loop() {
  unsigned long agora = millis();
  if (agora - ultimaLeitura >= INTERVALO_MS) {
    ultimaLeitura = agora;
    realizarLeitura();
    atualizarAlerta();
    atualizarLCD();
    enviarSerial(agora);
  }
}

void realizarLeitura() {
  float leitura = dht.readTemperature();
  if (isnan(leitura)) {
    Serial.println("[AVISO] Falha na leitura do DHT22 - usando valor anterior.");
  } else {
    tempAtual = leitura;
  }

  int ldrBruto = analogRead(LDR_PIN);
  albedoPorc = map(ldrBruto, 0, 1023, ALBEDO_GELO_SUJO, ALBEDO_GELO_FRESCO);
  indiceDegelo = calcularIndiceDegelo(tempAtual);
  alertaAtivo = (tempAtual > TEMP_CRITICA);
}

int calcularIndiceDegelo(float temp) {
  if (temp <= TEMP_MIN) return 0;
  if (temp >= TEMP_MAX) return 100;
  float proporcao = (temp - TEMP_MIN) / (TEMP_MAX - TEMP_MIN);
  return (int)(proporcao * 100.0);
}

void atualizarAlerta() {
  if (alertaAtivo) {
    ledState = !ledState;
    digitalWrite(LED_PIN, ledState);
  } else {
    ledState = false;
    digitalWrite(LED_PIN, LOW);
  }
}

void atualizarLCD() {
  lcd.clear();

  lcd.setCursor(0, 0);
  if (tempAtual >= 0) lcd.print("+");
  else lcd.print("-");
  lcd.print(abs(tempAtual), 1);
  lcd.print((char)223);
  lcd.print("C");
  lcd.setCursor(11, 0);
  if (alertaAtivo) lcd.print("ALERT");
  else lcd.print("  OK ");

  lcd.setCursor(0, 1);
  lcd.print("Dg:");
  if (indiceDegelo < 10)       lcd.print("  ");
  else if (indiceDegelo < 100) lcd.print(" ");
  lcd.print(indiceDegelo);
  lcd.print("% Al:");
  lcd.print((int)albedoPorc);
  lcd.print("%");
}

void enviarSerial(unsigned long ts) {
  Serial.print(ts);
  Serial.print(" ms    | ");
  if (tempAtual >= 0) Serial.print("+");
  Serial.print(tempAtual, 1);
  Serial.print(" C  | ");
  Serial.print((int)albedoPorc);
  Serial.print("%       | ");
  Serial.print(indiceDegelo);
  Serial.print("%          | ");
  Serial.println(alertaAtivo ? "*** ALERTA ***" : "Normal");
}