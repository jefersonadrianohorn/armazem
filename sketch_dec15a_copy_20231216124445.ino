#include <WiFi.h>
#include <HTTPClient.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

const char *ssid = "Marli";
const char *password = "Chacarabeijaflor@1975";
const char *sheetMonkeyURL = "https://api.sheetmonkey.io/form/fmkWMARMZDghhGNx7HgA28";
const int relePin = 27;  // Pino ao qual o relé está conectado
int tempoPara1Kg = 2000;  // Tempo em milissegundos para 1 kg

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");

void setup() {
  Serial.begin(115200);  // Inicia a comunicação serial para monitoramento
  pinMode(relePin, OUTPUT);  // Configura o pino do relé como saída

  // Conectar-se à rede WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Conectando ao WiFi...");
  }
  Serial.println("Conectado ao WiFi!");

  // Inicializa o cliente de tempo (NTP)
  timeClient.begin();

  // Configuração do fuso horário do Brasil (GMT-3)
  timeClient.setTimeOffset(-3 * 3600);  // Ajuste de fuso horário em segundos
}

void loop() {
  // Lê a entrada do usuário e converte para um valor numérico
  int QUANTIDADE_DE_RACAO = -1;  // Inicializa com um valor inválido

  // Aguarda até que uma quantidade de ração válida seja inserida
  while (QUANTIDADE_DE_RACAO < 0) {
    Serial.print("Digite a quantidade de ração (kg): ");
    while (Serial.available() == 0) {
      // Aguarda a entrada do usuário
    }

    // Lê a entrada do usuário e converte para um valor numérico
    QUANTIDADE_DE_RACAO = Serial.parseInt();
  }

// Verifica se o valor é negativo
  if (QUANTIDADE_DE_RACAO < 0) {
    Serial.println("Valor inválido. Insira um valor positivo.");
    // Sai da função loop() imediatamente
    return;
  }


  // Restante do código continua apenas se a quantidade de ração for válida

  // Obtém a hora formatada
  timeClient.update(); // Atualiza a hora do servidor NTP
  String hora_da_dispercao = timeClient.getFormattedTime();

  // Calcula o tempo necessário com base na quantidade de kg
  int tempoNecessario = QUANTIDADE_DE_RACAO * tempoPara1Kg;

if (Serial.available() > 0) {
      String comando = Serial.readStringUntil('\n');
      if (comando == "abort") {
        Serial.println("Loop abortado devido ao comando de aborto.");
        digitalWrite(relePin, LOW);  // Desliga o relé
        return;
      }
  // Liga o relé
  digitalWrite(relePin, HIGH);
  Serial.println("Relé ligado.");

  // Contador de porcentagem
  for (int tempoDecorrido = 0; tempoDecorrido <= tempoNecessario; tempoDecorrido += 100) {
    // Ajusta os valores mínimo e máximo para evitar problemas com map
    int tempoDecorridoMapeado = map(tempoDecorrido, 0, tempoNecessario, 0, 100);
    int percentualConcluido = constrain(tempoDecorridoMapeado, 0, 100);

    Serial.print("Porcentagem concluída: ");
    Serial.print(percentualConcluido);
    Serial.println("%");

    delay(100);  // Aguarda 100 milissegundos entre as atualizações

    // Verifica se o comando de aborto foi recebido durante o loop
    
    }
  }

  // Desliga o relé
  digitalWrite(relePin, LOW);
  Serial.println("Relé desligado.");

  // Envia os dados para o Sheet Monkey via POST
  HTTPClient http;
  http.begin(sheetMonkeyURL);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");

  // Constrói o payload com a quantidade de ração e a hora atual
  String postData = "QUANTIDADE_DE_RACAO=" + String(QUANTIDADE_DE_RACAO) + " kg&hora_da_dispercao=" + hora_da_dispercao;
  
  int httpCode = http.POST(postData);

  if (httpCode > 0) {
    Serial.print("Resposta do servidor: ");
    Serial.println(http.getString());
  } else {
    Serial.println("Falha na requisição HTTP.");
  }

  http.end();

  // Aguarda um segundo antes de ler novamente
  delay(1000);
}
