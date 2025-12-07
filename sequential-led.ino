#include <Arduino_FreeRTOS.h>
#include <TM1638.h>

#define STB 11
#define CLK 12
#define DIO 13

#define BTN_SW1  0x01
#define BTN_SW2  0x02
#define BTN_SW3  0x04
#define BTN_SW4  0x08
#define BTN_SW5  0x10
#define BTN_SW6  0x20
#define BTN_SW7  0x40
#define BTN_SW8  0x80

#define DEBOUNCE_MS       50
#define TEMPO_MAXIMO      99
#define TEMPO_INICIAL     3

typedef enum {
  ESTADO_CONFIGURANDO,
  ESTADO_EXECUTANDO,
  ESTADO_PAUSADO
} EstadoSistema_t;

TM1638 tm(CLK, DIO, STB);

volatile uint8_t tempoLED[4] = {TEMPO_INICIAL, TEMPO_INICIAL, TEMPO_INICIAL, TEMPO_INICIAL};
volatile uint8_t tempoLEDBackup[4] = {TEMPO_INICIAL, TEMPO_INICIAL, TEMPO_INICIAL, TEMPO_INICIAL};
volatile uint8_t tempoRestante = 0;
volatile EstadoSistema_t estadoAtual = ESTADO_CONFIGURANDO;
volatile int8_t ledAtivo = -1;
volatile uint8_t estadoLEDs[4] = {0, 0, 0, 0};

void TaskLeituraBotoes(void *pvParameters);
void TaskControleLEDs(void *pvParameters);
void TaskAtualizaDisplay(void *pvParameters);
void TaskComunicacaoSerial(void *pvParameters);

void atualizarDisplayTempos(void);
void apagarTodosLEDs(void);
void salvarTemposBackup(void);
void restaurarTemposBackup(void);
void mostrarTempoNoDisplay(uint8_t posicaoLED, uint8_t tempo);

void setup() {
  Serial.begin(9600);
  
  tm.reset();
  tm.displaySetBrightness(PULSE14_16);
  
  atualizarDisplayTempos();
  
  xTaskCreate(TaskLeituraBotoes, "Botoes", 150, NULL, 1, NULL);
  xTaskCreate(TaskControleLEDs, "LEDs", 150, NULL, 2, NULL);
  xTaskCreate(TaskAtualizaDisplay, "Display", 150, NULL, 1, NULL);
  xTaskCreate(TaskComunicacaoSerial, "Serial", 150, NULL, 1, NULL);

  
  Serial.println(F("Tarefas criadas!"));
}

void loop() {

  
}


void TaskLeituraBotoes(void *pvParameters) {
  (void) pvParameters;
  
  uint8_t botaoAnterior = 0;
  uint8_t botaoAtual = 0;
  
  for (;;) {
    botaoAtual = tm.getButtons();
    
    if (botaoAtual != botaoAnterior) {
      vTaskDelay(pdMS_TO_TICKS(DEBOUNCE_MS));
      botaoAtual = tm.getButtons();
      
      if (botaoAtual != botaoAnterior) {
        uint8_t botoesNovos = botaoAtual & ~botaoAnterior;
        if (estadoAtual == ESTADO_CONFIGURANDO) {
          bool decrementar = (botaoAtual & BTN_SW8);
          
          if (botoesNovos & BTN_SW1) {
            if (decrementar && tempoLED[0] > 0) {
              tempoLED[0]--;
            } else if (!decrementar && tempoLED[0] < TEMPO_MAXIMO) {
              tempoLED[0]++;
            }
          }
          
          if (botoesNovos & BTN_SW2) {
            if (decrementar && tempoLED[1] > 0) {
              tempoLED[1]--;
            } else if (!decrementar && tempoLED[1] < TEMPO_MAXIMO) {
              tempoLED[1]++;
            }
          }
          if (botoesNovos & BTN_SW3) {
            if (decrementar && tempoLED[2] > 0) {
              tempoLED[2]--;
            } else if (!decrementar && tempoLED[2] < TEMPO_MAXIMO) {
              tempoLED[2]++;
            }
          }
          
          if (botoesNovos & BTN_SW4) {
            if (decrementar && tempoLED[3] > 0) {
              tempoLED[3]--;
            } else if (!decrementar && tempoLED[3] < TEMPO_MAXIMO) {
              tempoLED[3]++;
            }
          }
          
          if (botoesNovos & BTN_SW5) {
            Serial.println(F("INICIANDO"));
            salvarTemposBackup();
            ledAtivo = 0;
            tempoRestante = tempoLED[0];
            estadoAtual = ESTADO_EXECUTANDO;
          }
          
          if (botoesNovos & BTN_SW7) {
            for (int i = 0; i < 4; i++) {
              tempoLED[i] = 0;
            }
          }
        }
        else if (estadoAtual == ESTADO_EXECUTANDO) {
          if (botoesNovos & BTN_SW6) {
            Serial.println(F("PAUSADO"));
            estadoAtual = ESTADO_PAUSADO;
          }
          
          if (botoesNovos & BTN_SW7) {
            restaurarTemposBackup();
            apagarTodosLEDs();
            ledAtivo = -1;
            estadoAtual = ESTADO_CONFIGURANDO;
          }
        }
        else if (estadoAtual == ESTADO_PAUSADO) {
          if (botoesNovos & BTN_SW6) {
            Serial.println(F("CONTINUANDO"));
            estadoAtual = ESTADO_EXECUTANDO;
          }
          if (botoesNovos & BTN_SW7) {
            restaurarTemposBackup();
            apagarTodosLEDs();
            ledAtivo = -1;
            estadoAtual = ESTADO_CONFIGURANDO;
          }
        }
        
        botaoAnterior = botaoAtual;
      }
    }
    
    vTaskDelay(pdMS_TO_TICKS(20));
  }
}

void TaskControleLEDs(void *pvParameters){
  (void) pvParameters;
  
  for (;;){
    if (estadoAtual == ESTADO_EXECUTANDO) {
      if (ledAtivo >= 0 && ledAtivo < 4) {
        
        for (int i = 0; i < 4; i++) {
          estadoLEDs[i] = (i == ledAtivo) ? 1 : 0;
        }
        
        tm.writeLed(ledAtivo + 1, 1);
        
        if (tempoRestante > 0) {
          tempoRestante--;
        } else {
          tm.writeLed(ledAtivo + 1, 0);
          ledAtivo++;
          
          if (ledAtivo >= 4) {
            Serial.println(F("FIM"));
            for (int i = 0; i < 4; i++) {
              estadoLEDs[i] = 0;
            }
            apagarTodosLEDs();
            ledAtivo = -1;
            estadoAtual = ESTADO_CONFIGURANDO;
          } else {
            tempoRestante = tempoLED[ledAtivo];
          }
        }
      }
    }
    
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

void TaskAtualizaDisplay(void *pvParameters) {
  (void) pvParameters;
  
  for (;;) {
    if (estadoAtual == ESTADO_CONFIGURANDO) {
      atualizarDisplayTempos();
    } else {
      for (int i = 0; i < 4; i++){
        uint8_t tempo;
        
        if (i == ledAtivo) {
          tempo = tempoRestante;
        } else if (i < ledAtivo){
          tempo = 0;
        } else {
          tempo = tempoLED[i];
        }
        
        mostrarTempoNoDisplay(i, tempo);
      }
    }
    
    vTaskDelay(pdMS_TO_TICKS(200));
  }
}

void TaskComunicacaoSerial(void *pvParameters) {
  (void) pvParameters;
  
  for (;;) {
    if (estadoAtual == ESTADO_EXECUTANDO) {
      Serial.print(estadoLEDs[0]);
      Serial.print(F(","));
      Serial.print(estadoLEDs[1]);
      Serial.print(F(","));
      Serial.print(estadoLEDs[2]);
      Serial.print(F(","));
      Serial.println(estadoLEDs[3]);
    }
    
    
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

void mostrarTempoNoDisplay(uint8_t posicaoLED, uint8_t tempo) {
  uint8_t pos = (3 - posicaoLED) * 2;
  tm.displayVal(pos, tempo % 10);
  tm.displayVal(pos + 1, tempo / 10);
}


void atualizarDisplayTempos() {
  for (int i = 0; i < 4; i++) {
    mostrarTempoNoDisplay(i, tempoLED[i]);
  }
}



void apagarTodosLEDs() {
  for (int i = 1; i <= 4; i++) {
    tm.writeLed(i, 0);
  }
}

void salvarTemposBackup() {
  for (int i = 0; i < 4; i++) {
    tempoLEDBackup[i] = tempoLED[i];
  }
}

void restaurarTemposBackup() {
  for (int i = 0; i < 4; i++) {
    tempoLED[i] = tempoLEDBackup[i];
  }
}
