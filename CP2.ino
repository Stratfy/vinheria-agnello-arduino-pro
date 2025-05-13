//====================== BIBLIOTECAS ======================
#include <EEPROM.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include "DHT.h"
#include <RTClib.h>

//====================== DEFINIÇÕES GERAIS ======================
const int velocidade = 30;  // Velocidade da animação de texto
short int menuatual = 0, opcao = 0;

// Constantes para controle de navegação por setas
#define SETABAIXO 1
#define SETACIMA 2
#define SETAS 0

//====================== INICIALIZAÇÃO DE DISPOSITIVOS ======================
// Sensor DHT22
#define DHTpin 4
#define DHTmodel DHT22
DHT dht(DHTpin, DHTmodel);

// Sensor LDR
#define LDR A0

// RTC DS1307
RTC_DS1307 rtc;

// LCD I2C (endereço 0x27, 16 colunas, 2 linhas)
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Keypad 4x4
const uint8_t ROWS = 4;
const uint8_t COLS = 4;
char keys[ROWS][COLS] = {
  { '1', '2', '3', 'A' },
  { '4', '5', '6', 'B' },
  { '7', '8', '9', 'C' },
  { '*', '0', '#', 'D' }
};
uint8_t colPins[COLS] = { 9, 8, 7, 6 };     // Colunas: C1 a C4
uint8_t rowPins[ROWS] = { 13, 12, 11, 10 }; // Linhas: R1 a R4

Keypad kpd = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);


//====================== CONFIGURAÇÕES DE FÁBRICA ======================
// Endereços das variáveis salvas na EEPROM
#define CFG_INTERVALO_SCROLL_ADDR 0  // intervaloScroll ocupa 2 bytes
#define CFG_UNIDADE_TEMP_ADDR     2  // unidadeTemperatura ocupa 2 bytes
#define CFG_DISPLAY_ADDR          4  // display ocupa 2 bytes


// Valores padrão armazenados em PROGMEM
const uint16_t config_fac[] PROGMEM = {
  800,  // intervaloScroll padrão
  1,    // unidadeTemperatura padrão
  1     // display padrão
};

const uint8_t variaveismutaveis = 3; // Quantidade de variáveis configuráveis

// Variáveis configuráveis carregadas da EEPROM
uint16_t intervaloScroll;
uint16_t unidadeTemperatura;
uint16_t display;


// Lê as configurações salvas na EEPROM e atualiza as variáveis globais:
// - intervaloScroll, unidadeTemperatura e display.
void definevars(void){
  EEPROM.get(CFG_INTERVALO_SCROLL_ADDR, intervaloScroll);
  EEPROM.get(CFG_UNIDADE_TEMP_ADDR, unidadeTemperatura);
  EEPROM.get(CFG_DISPLAY_ADDR, display);
}

// Executa o setup inicial do dispositivo:
// - Se ainda não inicializado (EEPROM[100] != 1), restaura os valores de fábrica
//   da PROGMEM para a EEPROM e marca como já inicializado.
void primeirosetup(void){
    if (EEPROM.read(100) != 1) {
    restaurarConfiguracoesDeFabrica();  // Copia da PROGMEM para EEPROM
    EEPROM.write(100, 1); // Marca como "inicializado"
  }
}

//====================== TÍTULOS DOS MENUS ======================
// Cada string deve ter 16 caracteres para o display LCD
//Memória
const char texto0[] PROGMEM = "*-----Menu-----*";
const char texto1[] PROGMEM = "1. Display      ";
const char texto2[] PROGMEM = "2. Setup        ";
const char texto3[] PROGMEM = "3. Sobre        ";
const char texto4[] PROGMEM = "*----Setup----* ";
const char texto5[] PROGMEM = "1.Veloc.Txt     ";
const char texto6[] PROGMEM = "2.Unidade Temp. ";
const char texto7[] PROGMEM = "3.Display       ";
const char texto8[] PROGMEM = "4.Reset         ";
//Ponteiros para a mémoria
const char* const texto[] PROGMEM = {
  texto0, texto1, texto2, texto3, texto4, texto5, texto6, texto7, texto8
};

//====================== DESCRIÇÕES DOS MENUS ======================
//Memória
const char descricoes0[] PROGMEM = "Navegue pelas paginas e selecione no teclado - A. Subir - B. Descer - C. Voltar menu - D. Enter   ";
const char descricoes1[] PROGMEM = "Entra no modo leitura de dados e mostra na tela   ";
const char descricoes2[] PROGMEM = "Configura os parametros do dispositivo   ";
const char descricoes3[] PROGMEM = "Explica como funciona o dispositivo   ";
const char descricoes4[] PROGMEM = "Modifica as configuracoes do dispositivo aperte D para dar entrada   ";
const char descricoes5[] PROGMEM = "Altera a velocidade da rolagem do texto   ";
const char descricoes6[] PROGMEM = "Altera a unidade de medida da temperatura - 1.Celsius - 2.Fahrenheit - 3.Kelvin    ";
const char descricoes7[] PROGMEM = "Altera a disposicao dos dados lidos no display - 1. Simples - 2.Desenho - 3.SlideShow    ";
const char descricoes8[] PROGMEM = "Redefine para as configuracoes de fabrica    ";
//Ponteiros para a mémoria
const char* const descricoes[] PROGMEM = {
  descricoes0, descricoes1, descricoes2,
  descricoes3, descricoes4, descricoes5,
  descricoes6, descricoes7, descricoes8
};

//====================== CARACTERES PERSONALIZADOS ======================
//Memória
const uint8_t customChars0[] PROGMEM = {0x00};
const uint8_t customChars1[] PROGMEM = {0x10};
const uint8_t customChars2[] PROGMEM = {0x00,0x00,0x1F,0x11,0x0A,0x04,0x00,0x00};      // 2: Seta para baixo 1
const uint8_t customChars3[] PROGMEM = {0x00,0x00,0x1F,0x1F,0x0E,0x04,0x00,0x00};      // 3: Seta para baixo 2
const uint8_t customChars4[] PROGMEM = {0x00,0x00,0x04,0x0A,0x11,0x1F,0x00,0x00};      // 4: Seta para cima 1
const uint8_t customChars5[] PROGMEM = {0x00,0x00,0x04,0x0E,0x1F,0x1F,0x00,0x00};      // 5: Seta para cima 2
//Ponteiros para a mémoria
const uint8_t* const customChars[] PROGMEM = {
  customChars0, customChars1, customChars2, 
  customChars3, customChars4, customChars5, 
};


//-------------------FUNÇÕES DE EXECUÇÃO-------------------

void setup() {
  begins();
  Serial.begin(9600);
  
  primeirosetup();
  definevars();
}

 
void loop() {
  
  switch(menuatual){
    
    case 1:
      menus(1,SETAS,0); //Menu Display
      break;
    
    case 2:
      menus(2,SETAS,4); //Menu Setup
      break;
    
    case 3:
      menus(3,SETAS,0); //Menu Sobre
      break;
    
    case 4:
      menus(4,SETAS,0);//Sessão Setup
      break;
    
    case 5:
      menus(5,SETAS,11);//Velocidade de texto
      break;
    
    case 6:
      menus(6,SETAS,12); //Unidade de medida da temp
      break;
    
    case 7:
      menus(7,SETAS,13); //Muda o display
      break;

    case 8:
      menus(8,SETAS,14); //Redefine para as configurações de fábrica
      break;
    
    case 11: //Filho velocidade de texto
      intervaloScroll = modoInput(intervaloScroll, 100, 2000);
      EEPROM.put(CFG_INTERVALO_SCROLL_ADDR, intervaloScroll);
      menuatual = 5; // Volta para o menu Velocidade de texto;
      break;
    
    case 12: //Filho unidade de medida
      unidadeTemperatura = modoInput(unidadeTemperatura,1,3);
      EEPROM.put(CFG_UNIDADE_TEMP_ADDR, unidadeTemperatura);
      menuatual = 6;
      break;

    case 13://Filho do display
      display = modoInput(display,1,3);
      EEPROM.put(CFG_DISPLAY_ADDR, display);
      menuatual = 7;
      break;
    
    case 14://Ação de hardreset
      restaurarConfiguracoesDeFabrica();
      definevars();
      menuatual = 8;
      break;

    default: 
      menuatual = 0;
      menus(0,SETABAIXO,0); // Página principal do menu
  }
}

/* Função responsável pela exibição gráfica de um menu na tela LCD.
 Sintaxe: menus(indiceMenu, modoSetas, destinoAoClicar);
 - indiceMenu: número identificador do menu atual (ex: 1 para "Início").
 - modoSetas: define o comportamento das setas de navegação (SETAS, SETACIMA, SETABAIXO).
 - destinoAoClicar: número do menu para onde será redirecionado ao pressionar uma tecla de seleção.
   Use 0 se o menu não deve redirecionar.
*/
int menus(int i, int b,int filho){
  print16(i);
  do {
    opcao = descricoesFunc(i, b);

    if (opcao == 2) {
      menuatual++;
      break;
    } else if (opcao == 3) {
      menuatual--;
      break;
    } else if (opcao == 4){
      if(filho != 0){
        lcd.noBacklight();  // desliga o backlight
        delay(250);
        lcd.backlight();    // liga o backligh
        menuatual = filho;
      } 
      break;
    }else if(opcao == 5){
      menuatual = 0;
      break;
    }
  } while (opcao == 1);
}


// Inicializa os componentes do sistema:
// - Sensor DHT, LCD I2C, backlight do LCD e teclado matricial (com tempo de debounce ajustado).
void begins(void){
  dht.begin();
  lcd.init();
  lcd.backlight();
  kpd.setDebounceTime(5);
}



/* 
 Imprime texto de até 16 caracteres na linha 0 do LCD, com efeito de digitação.
- idxtexto: índice da string no array 'texto' armazenado em PROGMEM.
 Obs: a string deve ter no máximo 16 caracteres.
 */
void print16(int idxtexto){
  char buffert[17];
  strcpy_P(buffert, (PGM_P)pgm_read_word(&(texto[idxtexto])));
  
  lcd.setCursor(0,0);
  for(int i = 0; i < strlen(buffert); i++){
    lcd.setCursor(i,0);
    lcd.print(buffert[i]);
    delay(velocidade);
  } 
}



/* 
  Exibe uma descrição interativa no LCD com controle por setas.
  - idxdescricoes: índice da string no array 'descricoes' (armazenado em PROGMEM).
  - modoseta: define o comportamento das setas (SETAS, SETACIMA, SETABAIXO).
  Retorna a opção selecionada via teclado (ex: 1, 2, 3...).
 */

int descricoesFunc(int idxdescricoes,int modoseta){ 
  int cursor = 0;
  unsigned long ultimaAtualizacao = 0;
  int letras = 0;
  char textorolante[16];
  char buffer[250];
  uint8_t bufferChars[8];
  strcpy_P(buffer, (PGM_P)pgm_read_word(&(descricoes[idxdescricoes])));
  printSetas(modoseta);

  for(int i = 0; i < 15; i++){
    textorolante[i] = buffer[letras++];
  }
  
  while (letras < strlen(buffer)) {
    char tecla = kpd.getKey();
    
    if (tecla == 'B' && modoseta != SETACIMA || tecla == 'B' && modoseta == SETAS) {
      memcpy_P(bufferChars, (uint8_t*)pgm_read_word(&(customChars[3])), 8);
      lcd.createChar(3,bufferChars);
      lcd.setCursor(15,1);
      lcd.write(byte(3));
      return 2;
    } else if (tecla == 'A' && modoseta != SETABAIXO || tecla == 'A' && modoseta == SETAS) {
      memcpy_P(bufferChars, (uint8_t*)pgm_read_word(&(customChars[5])), 8);
      lcd.createChar(3,bufferChars);
      lcd.setCursor(15,0);
      lcd.write(byte(3));
      return 3;
    } else if (tecla == 'D'){
      return 4;
    } else if (tecla == 'C')return 5;

    // Verifica se já passou tempo suficiente para scroll
    if (millis() - ultimaAtualizacao > intervaloScroll) {
      ultimaAtualizacao = millis();

      for (int p = 0; p < 15; p++) {
        lcd.setCursor(p, 1);
        lcd.print(textorolante[p]);
      }

      if (letras == 15) delay(500);

      // desloca e adiciona o próximo caractere
      for (int o = 0; o < 14; o++) {
        textorolante[o] = textorolante[o + 1];
      }
      textorolante[14] = buffer[letras++];
      textorolante[15] = '\0';
    }
   
  }
  return 1;
}

// Cria os caracteres personalizados das setas e define quais serão exibidas:
// - Apenas seta para baixo, apenas para cima ou ambas, conforme o modo selecionado.
void printSetas(int modoseta){
  uint8_t buffer[8];

  // seta para baixo 1
  memcpy_P(buffer, (uint8_t*)pgm_read_word(&(customChars[2])), 8);
  lcd.createChar(0, buffer);
  
  // seta para cima 1
  memcpy_P(buffer, (uint8_t*)pgm_read_word(&(customChars[4])), 8);
  lcd.createChar(1, buffer);

  if(modoseta == 1){
    lcd.setCursor(15,1);
    lcd.write(byte(0));
  } else if(modoseta == 2){
    lcd.setCursor(15,0);
    lcd.write(byte(1));
  } else{
    lcd.setCursor(15,1);
    lcd.write(byte(0));
    lcd.setCursor(15,0);
    lcd.write(byte(1));
  }
}

// Modifica variáveis globais de controle com base na entrada do usuário via teclado.
// - valorAtual: valor inicial exibido e editável.
// - minimo / maximo: limites inferiores e superiores permitidos para o valor.
// Retorna o novo valor após confirmação pelo usuário.
int modoInput(int valorAtual, int minimo, int maximo) {
  lcd.clear();
  lcd.setCursor(1, 1);
  lcd.print(minimo);
  lcd.print("--");
  lcd.print(maximo);

  lcd.setCursor(0, 0);
  char key = '\0';
  char numChar[9] = "";
  int digitosvalidos = 0;

  while (key != 'D') {
    key = kpd.getKey();
    if (key >= '0' && key <= '9' && digitosvalidos < 8) {
      numChar[digitosvalidos] = key;
      lcd.setCursor(digitosvalidos, 0);
      lcd.print(numChar[digitosvalidos++]);
    } else if (key == 'C') {
      return valorAtual;  // cancela e mantém valor anterior
    }
  }

  numChar[digitosvalidos] = '\0';
  int num = atoi(numChar);

  if (num < minimo || num > maximo) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Valor invalido");
    delay(1000);
    return modoInput(valorAtual, minimo, maximo);  // <- agora com retorno
  } else {
    return num;
  }
}


// Restaura as configurações padrão da PROGMEM sobrescrevendo os valores atuais na EEPROM.
// Utilizada para resetar o sistema para o estado de fábrica.
void restaurarConfiguracoesDeFabrica() {
  for (int i = 0; i < variaveismutaveis; i++) {
    uint16_t valor = pgm_read_word_near(config_fac + i);
    EEPROM.put(i * 2, valor);
  }

  Serial.println("Configurações de fábrica restauradas!");
}