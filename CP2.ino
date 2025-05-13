//====================== BIBLIOTECAS ======================
#include <EEPROM.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include "DHT.h"
#include <RTClib.h>
 
//====================== DEFINIÇÕES GERAIS DO SEU PROJETO ======================
const int velocidade = 30;  // Velocidade da animação de texto (do seu sistema de menus)
short int menuatual = 0, opcao = 0;
 
// Constantes para controle de navegação por setas (do seu sistema de menus)
#define SETABAIXO 1
#define SETACIMA 2
#define SETAS 0
 
//====================== INICIALIZAÇÃO DE DISPOSITIVOS DO SEU PROJETO ======================
// Sensor DHT22
#define DHTpin 4
#define DHTmodel DHT22
DHT dht(DHTpin, DHTmodel);
 
// Sensor LDR
#define LDR A0
 
// RTC DS1307
RTC_DS1307 rtc;
 
// LCD I2C (endereço 0x27, 16 colunas, 2 linhas) - Esta instância será usada por ambos
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
 

//=======================================================================================
//====================== INÍCIO DO CÓDIGO DA ANIMAÇÃO DA ARANHA ========================
//=======================================================================================
 
// --- DEFINIÇÕES DOS SPRITES (DESENHOS) DA ARANHA (COM PREFIXO "anim_") ---
//================== SPRITES DA ARANHA (2 POSES, 4 PARTES CADA) ==================
const byte anim_aranha[2][4][8] PROGMEM = {
  { // Pose 0
    { B00000, B01001, B01001, B01001, B00101, B00011, B00111, B00111 },
    { B00111, B00111, B00011, B00101, B01001, B01001, B01001, B00000 },
    { B00000, B00100, B00100, B01000, B01000, B11100, B11111, B11100 },
    { B11100, B11111, B11100, B01000, B01000, B00100, B00100, B00000 }
  },
  { // Pose 1
    { B00000, B00101, B00101, B00101, B00011, B00101, B00111, B00111 },
    { B00111, B00111, B00101, B00011, B00101, B00101, B00101, B00000 },
    { B00000, B01000, B01000, B00100, B00100, B11100, B11111, B11100 },
    { B11100, B11111, B11100, B00100, B00100, B01000, B01000, B00000 }
  }
};

unsigned long anim_tempo_ultimo_quadro = 0;
const int anim_intervalo_principal = 320;
int anim_pose_atual_aranha = 0;

enum anim_estado_sistema { anim_entrando_e_puxando, anim_texto_centralizado_segurando, anim_soltando_fio, anim_aranha_saindo, anim_texto_final_exibido };
anim_estado_sistema anim_estado_atual = anim_entrando_e_puxando;

const char* anim_nome_empresa = "Stratfy";
const int anim_comprimento_nome_empresa = 7;
const char* anim_linha_decorativa_texto = "-------";
const int anim_comprimento_linha_decorativa = 7;
const int anim_colunas_lcd = 16;
int anim_coluna_atual_esquerda_aranha = -(anim_comprimento_nome_empresa + 3);
int anim_coluna_atual_inicio_texto;
int anim_coluna_atual_fio;
const int anim_coluna_alvo_inicio_texto = (anim_colunas_lcd - anim_comprimento_nome_empresa) / 2;
const int anim_coluna_alvo_inicio_linha_decorativa = (anim_colunas_lcd - anim_comprimento_linha_decorativa) / 2;
unsigned long anim_tempo_mudanca_estado = 0;
const int anim_duracao_estado_segurando = 1500;

void anim_carregar_sprites_pose(byte pose) {
  for (byte parte = 0; parte < 4; parte++) {
    uint8_t buffer[8];
    memcpy_P(buffer, anim_aranha[pose][parte], 8);
    lcd.createChar(parte, buffer);
  }
}

void anim_desenhar_aranha(byte col_esq) {
  if (col_esq >= 0 && col_esq < anim_colunas_lcd) {
    lcd.setCursor(col_esq, 0); lcd.write(0);
    lcd.setCursor(col_esq, 1); lcd.write(1);
  }
  if ((col_esq + 1) >= 0 && (col_esq + 1) < anim_colunas_lcd) {
    lcd.setCursor(col_esq + 1, 0); lcd.write(2);
    lcd.setCursor(col_esq + 1, 1); lcd.write(3);
  }
}

void anim_desenhar_texto() {
  for (int i = 0; i < anim_comprimento_nome_empresa; i++) {
    int col = anim_coluna_atual_inicio_texto + i;
    if (col >= 0 && col < anim_colunas_lcd)
      lcd.setCursor(col, 0), lcd.print(anim_nome_empresa[i]);
  }
  if (anim_estado_atual >= anim_texto_centralizado_segurando) {
    for (int i = 0; i < anim_comprimento_linha_decorativa; i++) {
      int col = anim_coluna_alvo_inicio_linha_decorativa + i;
      if (col >= 0 && col < anim_colunas_lcd)
        lcd.setCursor(col, 1), lcd.print(anim_linha_decorativa_texto[i]);
    }
  }
}

void anim_executar_inicializacao() {
  anim_estado_atual = anim_entrando_e_puxando;
  anim_coluna_atual_esquerda_aranha = -(anim_comprimento_nome_empresa + 3);
  anim_tempo_ultimo_quadro = millis();

  while (anim_estado_atual != anim_texto_final_exibido) {
    if (millis() - anim_tempo_ultimo_quadro >= anim_intervalo_principal) {
      lcd.clear();
      anim_tempo_ultimo_quadro = millis();
      anim_pose_atual_aranha = 1 - anim_pose_atual_aranha;
      anim_carregar_sprites_pose(anim_pose_atual_aranha);

      switch (anim_estado_atual) {
        case anim_entrando_e_puxando:
          anim_coluna_atual_esquerda_aranha++;
          anim_coluna_atual_fio = anim_coluna_atual_esquerda_aranha - 1;
          anim_coluna_atual_inicio_texto = anim_coluna_atual_fio - anim_comprimento_nome_empresa;
          if (anim_coluna_atual_inicio_texto >= anim_coluna_alvo_inicio_texto) {
            anim_coluna_atual_inicio_texto = anim_coluna_alvo_inicio_texto;
            anim_estado_atual = anim_texto_centralizado_segurando;
            anim_tempo_mudanca_estado = millis();
          }
          break;
        case anim_texto_centralizado_segurando:
          if (millis() - anim_tempo_mudanca_estado > anim_duracao_estado_segurando)
            anim_estado_atual = anim_soltando_fio;
          break;
        case anim_soltando_fio:
          anim_estado_atual = anim_aranha_saindo;
          break;
        case anim_aranha_saindo:
          anim_coluna_atual_esquerda_aranha++;
          if (anim_coluna_atual_esquerda_aranha >= anim_colunas_lcd) {
            anim_estado_atual = anim_texto_final_exibido;
            lcd.clear();
            anim_desenhar_texto();
            delay(1000);
          }
          break;
        default:
          break;
      }

      if (anim_estado_atual != anim_texto_final_exibido) {
        if (anim_coluna_atual_fio >= 0 && anim_coluna_atual_fio < anim_colunas_lcd &&
            anim_estado_atual <= anim_texto_centralizado_segurando) {
          lcd.setCursor(anim_coluna_atual_fio, 0);
          lcd.print("-");
        }
        anim_desenhar_aranha(anim_coluna_atual_esquerda_aranha);
        anim_desenhar_texto();
      }
    }
    delay(10);
  }

  lcd.clear();
}

//=======================================================================================
//====================== FIM DO CÓDIGO DA ANIMAÇÃO DA ARANHA ==========================
//=======================================================================================

//====================== CONFIGURAÇÕES DE FÁBRICA DO SEU PROJETO ======================
// Endereços das variáveis salvas na EEPROM
#define CFG_INTERVALO_SCROLL_ADDR 0  // intervaloScroll ocupa 2 bytes
#define CFG_UNIDADE_TEMP_ADDR     2  // unidadeTemperatura ocupa 2 bytes
#define CFG_DISPLAY_ADDR          4  // display ocupa 2 bytes
#define CFG_INTRO_ADDR            6  // intro
 
 
// Valores padrão armazenados em PROGMEM
const uint16_t config_fac[] PROGMEM = {
  800,  // intervaloScroll padrão
  1,    // unidadeTemperatura padrão
  1,     // display padrão
  1,    //Intro ligada padrão
};
 
const uint8_t variaveismutaveis = 4; // Quantidade de variáveis configuráveis
 
// Variáveis configuráveis carregadas da EEPROM (do seu projeto)
uint16_t intervaloScroll;
uint16_t unidadeTemperatura;
uint16_t display;
uint16_t intro;

// Lê as configurações salvas na EEPROM e atualiza as variáveis globais:
// - intervaloScroll, unidadeTemperatura e display.
void definevars(void){
  EEPROM.get(CFG_INTERVALO_SCROLL_ADDR, intervaloScroll);
  EEPROM.get(CFG_UNIDADE_TEMP_ADDR, unidadeTemperatura);
  EEPROM.get(CFG_DISPLAY_ADDR, display);
  EEPROM.get(CFG_INTRO_ADDR, intro);
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
 
// Executa o setup inicial do dispositivo:
// - Se ainda não inicializado (EEPROM[100] != 1), restaura os valores de fábrica
//   da PROGMEM para a EEPROM e marca como já inicializado.
void primeirosetup(void){
    if (EEPROM.read(100) != 1) {
    restaurarConfiguracoesDeFabrica();  // Copia da PROGMEM para EEPROM
    EEPROM.write(100, 1); // Marca como "inicializado"
  }
}
 
//====================== TÍTULOS DOS MENUS DO SEU PROJETO (ORIGINAIS) ======================
const char texto0[] PROGMEM = "*-----Menu-----*";
const char texto1[] PROGMEM = "1. Display      ";
const char texto2[] PROGMEM = "2. Setup        ";
const char texto3[] PROGMEM = "3. Sobre        ";
const char texto4[] PROGMEM = "*----Setup----* ";
const char texto5[] PROGMEM = "1.Veloc.Txt     ";
const char texto6[] PROGMEM = "2.Unidade Temp. ";
const char texto7[] PROGMEM = "3.Display       ";
const char texto8[] PROGMEM = "4.Reset         ";
const char texto9[] PROGMEM = "5.Intro         ";
//Ponteiros para a mémoria
const char* const texto[] PROGMEM = {
  texto0, texto1, texto2, texto3, texto4, texto5, texto6, texto7, texto8, texto9
};
 
//====================== DESCRIÇÕES DOS MENUS DO SEU PROJETO (ORIGINAIS) ======================
const char descricoes0[] PROGMEM = "Navegue pelas paginas e selecione no teclado - A. Subir - B. Descer - C. Voltar menu - D. Enter   ";
const char descricoes1[] PROGMEM = "Entra no modo leitura de dados e mostra na tela   ";
const char descricoes2[] PROGMEM = "Configura os parametros do dispositivo   ";
const char descricoes3[] PROGMEM = "Explica como funciona o dispositivo   ";
const char descricoes4[] PROGMEM = "Modifica as configuracoes do dispositivo aperte D para dar entrada   ";
const char descricoes5[] PROGMEM = "Altera a velocidade da rolagem do texto   ";
const char descricoes6[] PROGMEM = "Altera a unidade de medida da temperatura - 1.Celsius - 2.Fahrenheit - 3.Kelvin    ";
const char descricoes7[] PROGMEM = "Altera a disposicao dos dados lidos no display - 1. Simples - 2.Desenho - 3.SlideShow    ";
const char descricoes8[] PROGMEM = "Redefine para as configuracoes de fabrica    ";
const char descricoes9[] PROGMEM = "Liga ou desliga a intro ao ligar - 0. Desliga - 1.Liga   ";
//Ponteiros para a mémoria
const char* const descricoes[] PROGMEM = {
  descricoes0, descricoes1, descricoes2,
  descricoes3, descricoes4, descricoes5,
  descricoes6, descricoes7, descricoes8,
  descricoes9
};
 
//====================== CARACTERES PERSONALIZADOS DO SEU PROJETO (ORIGINAIS) ======================
// Estes serão recriados por printSetas e descricoesFunc conforme necessário
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
 
 
//-------------------FUNÇÕES DE EXECUÇÃO DO SEU PROJETO (ORIGINAIS)-------------------
 
// Inicializa os componentes do sistema (do seu projeto)
void begins(void){
  dht.begin();
  lcd.init();      // LCD inicializado aqui
  lcd.backlight(); // Backlight ligado aqui
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
    delay(velocidade); // Usa a variável 'velocidade' do seu projeto
  }
}
 
// Cria os caracteres personalizados das setas e define quais serão exibidas:
// - Apenas seta para baixo, apenas para cima ou ambas, conforme o modo selecionado.
void printSetas(int modoseta){
  uint8_t buffer[8]; // Variável local, sem conflito
 
  // seta para baixo 1 (usará índice 0 do LCD)
  memcpy_P(buffer, (uint8_t*)pgm_read_word(&(customChars[2])), 8);
  lcd.createChar(0, buffer);
 
  // seta para cima 1 (usará índice 1 do LCD)
  memcpy_P(buffer, (uint8_t*)pgm_read_word(&(customChars[4])), 8);
  lcd.createChar(1, buffer);
 
  if(modoseta == SETABAIXO){ // Usa a constante definida no seu projeto
    lcd.setCursor(15,1);
    lcd.write(byte(0)); // Escreve o char customizado do índice 0 (seta para baixo)
  } else if(modoseta == SETACIMA){ // Usa a constante definida no seu projeto
    lcd.setCursor(15,0);
    lcd.write(byte(1)); // Escreve o char customizado do índice 1 (seta para cima)
  } else{ // SETAS (ambas)
    lcd.setCursor(15,1);
    lcd.write(byte(0));
    lcd.setCursor(15,0);
    lcd.write(byte(1));
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
  char buffer_desc_func[250]; // Renomeado para evitar qualquer possível conflito local, embora improvável
  uint8_t buffer_chars_desc_func[8]; // Renomeado para evitar qualquer possível conflito local
  strcpy_P(buffer_desc_func, (PGM_P)pgm_read_word(&(descricoes[idxdescricoes])));
  printSetas(modoseta); // Recria os caracteres 0 e 1 para as setas
 
  for(int i = 0; i < 15; i++){
    textorolante[i] = buffer_desc_func[letras++];
  }
 
  while (letras < strlen(buffer_desc_func)) {
    char tecla = kpd.getKey();
   
    // A lógica original de criar o char 3 para a seta pressionada é mantida.
    // Isso sobrescreverá o que a aranha usou no índice 3, o que é aceitável aqui.
    if (tecla == 'B' && (modoseta != SETACIMA || modoseta == SETAS) ) {
      memcpy_P(buffer_chars_desc_func, (uint8_t*)pgm_read_word(&(customChars[3])), 8); // Seta para baixo 2
      lcd.createChar(3,buffer_chars_desc_func); // Usa índice 3
      lcd.setCursor(15,1);
      lcd.write(byte(3));
      return 2;
    } else if (tecla == 'A' && (modoseta != SETABAIXO || modoseta == SETAS) ) {
      memcpy_P(buffer_chars_desc_func, (uint8_t*)pgm_read_word(&(customChars[5])), 8); // Seta para cima 2
      lcd.createChar(3,buffer_chars_desc_func); // Usa índice 3 (sobrescreve a seta para baixo 2 se B foi pressionado antes)
      lcd.setCursor(15,0);
      lcd.write(byte(3)); // Mostra a seta para cima animada
      return 3;
    } else if (tecla == 'D'){
      return 4;
    } else if (tecla == 'C')return 5;
 
    if (millis() - ultimaAtualizacao > intervaloScroll) {
      ultimaAtualizacao = millis();
      for (int p = 0; p < 15; p++) {
        lcd.setCursor(p, 1);
        lcd.print(textorolante[p]);
      }
      for (int o = 0; o < 14; o++) {
        textorolante[o] = textorolante[o + 1];
      }
      textorolante[14] = buffer_desc_func[letras++];
      textorolante[15] = '\0';
    }
  }
  return 1;
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
    return modoInput(valorAtual, minimo, maximo);
  } else {
    return num;
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
  print16(i); // Imprime o título do menu
  do {
    opcao = descricoesFunc(i, b);
 
    if (opcao == 2) { // Opção descer
      menuatual++;
      break;
    } else if (opcao == 3) { // Opção subir
      menuatual--;
      break;
    } else if (opcao == 4){ // Opção Enter/Selecionar
      if(filho != 0){ // Se há um submenu para ir
        lcd.noBacklight();  
        delay(250);
        lcd.backlight();    
        menuatual = filho; // Define o novo menu atual
      }
      break;
    }else if(opcao == 5){ // Opção Voltar/Cancelar
      menuatual = 0; // Volta para o menu principal
      break;
    }
  } while (opcao == 1); 
  return 0;
}
 
 
//-------------------FUNÇÃO SETUP PRINCIPAL DO SEU PROJETO-------------------
void setup() {
  definevars();
  begins(); // Chama sua função original para inicializar DHT, LCD, Keypad
  if(intro)anim_executar_inicializacao();
  Serial.begin(9600); // Inicializa a comunicação serial (do seu código original)
  primeirosetup();
}
 
 
//-------------------FUNÇÃO LOOP PRINCIPAL DO SEU PROJETO-------------------
void loop() {
 
  switch(menuatual){ // Lógica de menu do seu código original
    case 1:menus(1,SETAS,0);break; //Menu Display
    case 2:menus(2,SETAS,4);break; //Menu Setup
    case 3:menus(3,SETAS,0);break; //Menu Sobre
    case 4:menus(4,SETAS,0);break;//Sessão Setup
    case 5:menus(5,SETAS,100);break;//Velocidade de texto
    case 6:menus(6,SETAS,101);break; //Unidade de medida da temp
    case 7:menus(7,SETAS,102);break; //Muda o display
    case 8:menus(8,SETAS,103);break; //Redefine para as configurações de fábrica
    case 9:menus(9,SETAS,104);break; //Liga e desliga a animação de introdução
      
    case 100: //Filho velocidade de texto
      intervaloScroll = modoInput(intervaloScroll, 100, 2000);
      EEPROM.put(CFG_INTERVALO_SCROLL_ADDR, intervaloScroll);
      menuatual = 5; // Volta para o menu Velocidade de texto;
      break;
   
    case 101: //Filho unidade de medida
      unidadeTemperatura = modoInput(unidadeTemperatura,1,3);
      EEPROM.put(CFG_UNIDADE_TEMP_ADDR, unidadeTemperatura);
      menuatual = 6;
      break;
 
    case 102://Filho do display
      display = modoInput(display,1,3);
      EEPROM.put(CFG_DISPLAY_ADDR, display);
      menuatual = 7;
      break;
   
    case 103://Ação de hardreset
      restaurarConfiguracoesDeFabrica();
      definevars();
      menuatual = 8;
      break;
    
    case 104://Filho intro
      intro = modoInput(intro,0,1);
      EEPROM.put(CFG_INTRO_ADDR, intro);
      menuatual = 9;
      break;
 
 
    default:
      menuatual = 0;
      menus(0,SETABAIXO,0); // Página principal do menu
  }
}