//Inkluderer biblioteket til LCD. 
#include <LiquidCrystal.h>

//Inkuderer biblioteket til Keypad. 
#include <Keypad.h>

//Definerer hvilke pins lcden bruger på Arduino Boardet. 
LiquidCrystal lcd(2 , 3, 4, 5, 11, 12);

//Definerer hvilken pins buzzeren bruger på Arduino Boardet. 
const int buzzerPin = 22;   

//Danner et setup til keypaden. 
const byte ROWS = 4;
const byte COLS = 4; 

char hexaKeys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

//Definerer hvilke pins keypaden bruger på Arduino Boardet. 
byte rowPins[ROWS] = {A0, A1, A2, A3};
byte colPins[COLS] = {6, 8, A4, A5};

//Opretter et keypad objekt, der fortæller hvilke taster der findes, hvilke pins der er forbundet og antal af rækker og kolonner. 
Keypad customKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);

//Variabler der bruges til hvor mange muligheder man har, samt hvor mange der er brugt. 
int wrongAttempts = 0; 
const int maxAttempts = 3;

//Variablerne bruges til at styre de to primære tilstande i alarmsystemet (Kodeopsætningen & Låsning af systemet).
String input = "";
String setCode = "";
bool codeSet = false;
bool locked = false;

//Definerer hvilken pin Pir Motion Sensor bruger i Arduino Boardet.
const int pirPin = 13;

// Hvilken pin knappen er koblet til i Arduino Boardet.
const int buttonPin = 7; 

//Definerer hvilken pin LEDen bruger i Arduino Boardet. 
const int ledPinR = 10;
const int ledPinG = 9;

//lastButtonState er en variabel, der bruges til at gemme knappens tilstand, hvor den starter som low (ikke trykket).
int lastButtonState = LOW; 

//Timeren. 
unsigned long lastMotionTime = 0; //Tjekker hvor lang tid der er gået. 
const unsigned long AlarmDelay = 30000; //Gør at man har 30sek til at Indtaste koden.
bool AlarmAktiveret = false; //Sætter Alarmen til ikke at være startet. 

//Variabler der gør LED-pinen blinker, når alarmen går igang.
bool ledState = false;
unsigned long PreMillis = 0;
unsigned long CurMillis = millis();
const long BlinkInterval = 500;//Gør at BlinkInterValet er 500ms.

//Variablerne der skal bruges til at gemme sensor state.
int pirState = LOW;
int lastPirState = LOW;

//Gør at der går 2sek før der bliver skiftet tekst fra "DEAKTIVERES VIA KODE" TIL "INDTAST KODE:".
unsigned long lcdStartMillis = 0;
const long lcdDisplayTime = 2000; // 2 sekunder til start-tekst.
bool lcdStartDisplayed = false;

//Tjekker om alarmen allerede er aktiveret. 
bool alarmKanAktiveres = true;

//Alarm aktiveret af knappen, men venter på PIR-sensoren.
bool alarmVentende = false; 

//Tjekker om der en bevægelse registreret. 
bool bevaegelseRegistreret = false;

//Tjekker hvornår teksten startede, samt om teksten bliver vist lige nu. 
unsigned long lcdAktiveretMillis = 0;
bool lcdAktiveretVises = false;

//Variablerne bruges til at implementere styrer af visning af LCD, bruges istedet for delay for ikke at blokere koden.
bool startScreenDisplayed = true;
unsigned long startScreenMillis = 0;
const long startScreenTime = 2000; // 2 sekunder.

///Variablerne bruges til at implementere styrer af visning af LCD, bruges istedet for delay for ikke at blokere koden.
bool codeSetMessageDisplayed = false;
unsigned long codeSetMessageMillis = 0;
const long codeSetMessageTime = 2000; // 2 sekunder

//Variablerne bruges til at vise korte advarseler 
unsigned long displayMessageMillis = 0;
const long displayMessageTime = 2000; // 2 sekunder

//Variabler skal bruges til at danne en bib-tone, imens man indtaster koden. 
unsigned long buzzerPreviousMillis = 0;
const long BibInterval = 500; // Gør at bib intervallet er hver 500ms.
bool buzzerState = false; // Tjekker om buzzeren er Tændt eller Slukket. 
const int BibFrekvens = 500; // Frekvensen for bib-tonen er 500hz. 

bool alarmKlarTilPIR = false;//Angiver om PIR-Sensoren må registrerer. 

void setup(){
//Starter lcden og sætter den til at have 16 kolonner og 2 rækker. 
  lcd.begin(16,2);

//Sætter PIR-Sensoren som INDPUT. 
  pinMode(pirPin, INPUT);

// Knap-pin sættes til INPUT. 
  pinMode(buttonPin, INPUT); 

//Sætter LEDG-pin som OUTPUT.
  pinMode(ledPinG, OUTPUT);

//Tænder LEDG-pin ved opstart.
  digitalWrite(ledPinG, HIGH);

//Sætter LEDR-pin som OUTPUT.
  pinMode(ledPinR, OUTPUT);

//Slukker LEDR-pin ved opstart. 
  digitalWrite(ledPinR, LOW);

//Sætter BuzzerPin til OUTPUT. 
  pinMode(buzzerPin, OUTPUT);

//Slukker BuzzerPin ved opstart. 
noTone(buzzerPin);

//Start Skærmen. 
lcd.setCursor(2 ,0);
lcd.print("INDSTIL KODE");
lcd.setCursor(1 ,1);
lcd.print("4-Cifrede KODE");
startScreenMillis = millis();
}

void loop() {
unsigned long CurMillis = millis();
int buttonState = digitalRead(buttonPin);
char customKey = customKeypad.getKey();

//Tjekker om der er gået 2sek, hvorefter den viser teksten "INDTAST KODE:".
if (startScreenDisplayed && CurMillis - startScreenMillis >= startScreenTime) {
    startScreenDisplayed = false;
    lcd.clear();//Ryder skærmen. 
    lcd.setCursor(2,0);
    lcd.print("INDTAST KODE:");
    lcd.setCursor(6,1);
    input = "";
    setCode = "";
}

//Tjekker om der allerede er opsat en kode, samt bygger koden.
if (!codeSet && !locked && !startScreenDisplayed) {
    if (customKey >= '0' && customKey <= '9') {
        input += customKey;
        lcd.print("*");
    }

//Tjekker om der er indtastet 4-Cifre, hvis det er tilfældet gemmer den koden og visualiserer det med tekst.
  if (input.length() == 4) {
    setCode = input;
   codeSet = true;
    locked = true;
    input = "";
    lcd.clear();
    lcd.setCursor(4, 0);
    lcd.print("KODE GEMT");
    lcd.setCursor(1, 1);
    lcd.print("AKTIVERING KLAR");

    codeSetMessageMillis = CurMillis;
    codeSetMessageDisplayed = true;

    lcdAktiveretVises = false;//Gør at den ikke genviser teksten med at Alarmen kan aktiveres. 
    lcdStartDisplayed = false;//Gør at teksten ikke genvises med at Alarmen kan deaktiveres med kode. 
    }
}

//Tjekker om teksten er blevet vist i 2sek, hvor den så slukker lcden hvis det er korrekt.
if(codeSetMessageDisplayed && CurMillis - codeSetMessageMillis >= codeSetMessageTime) {
  lcd.clear();
  codeSetMessageDisplayed = false;

  lcd.setCursor(2,0);
  lcd.print("TRYK KNAPPEN");
  lcd.setCursor(1,1);
  lcd.print("FOR AKTIVERING");

}
 
//Låse mekanismen.
if (buttonState == HIGH && lastButtonState == LOW && alarmKanAktiveres && locked) {
// Sluk grøn LED
  digitalWrite(ledPinG, LOW);

// Sæt alarm i ventende tilstand.
  alarmVentende = true;

  alarmKlarTilPIR = true; //PIR-Sensoren kan nu registrerer bevægelse. 

// Nulstil bevægelses-registrering, så alarmen ikke starter med det samme.
  bevaegelseRegistreret = false;

//Låser knappen så den ikke kan aktiveres igen.
  alarmKanAktiveres = false;

//Det der bliver vist, når knappen bliver trykket.
  lcd.display();
  lcd.clear();
  lcd.setCursor(1, 0);
  lcd.print("ALARM AKTIVERET");
  lcd.setCursor(0, 1);
  lcd.print("----------------");

// Tænder kortvarigt den røde LED og afspiller tone for at indikere at Alarmen er aktiveret.
  digitalWrite(ledPinR, HIGH);
    
//Afspiller en tone, når Alarmen aktiveres. 
  tone(buzzerPin, 500, 222); 

  lcdAktiveretMillis = CurMillis; 
  lcdAktiveretVises = true;//Gør at den viser teksten med Alarm aktiveringen.  
}

//Slukker LCD & LED 2sek efter.
if(lcdAktiveretVises && CurMillis - lcdAktiveretMillis >= 2000) {
  lcd.clear();
  digitalWrite(ledPinR, LOW);//Slukker den røde LED. 
  noTone(buzzerPin);//Stopper tonen. 
  lcdAktiveretVises = false;
}

//Gemmer knappens siste state. 
lastButtonState = buttonState;

//Hvis den registrerer en bevægelse displayer den følgende tekst.
pirState = digitalRead(pirPin);

if (alarmVentende && alarmKlarTilPIR && pirState == HIGH && lastPirState == LOW && !AlarmAktiveret) { 
  bevaegelseRegistreret = true; //Bevægelse er nu registreret.

  lcd.clear(); 
  lcd.setCursor(1, 0); 
  lcd.print("DEAKTIVERES VIA"); 
  lcd.setCursor(1, 1); 
  lcd.print("INDTASTEDE KODE"); 

  //De styrer, hvor længe beskeden skal vises, uden at blokere resten af koden.
  lcdStartMillis = CurMillis; //Husker start tidpunktet.
  lcdStartDisplayed = true; //Fortæller at beskeden er aktiv.

  lastMotionTime = CurMillis; //Gemmer tidpunktet PIR-sensoren registrerede bevægelse.
  AlarmAktiveret = false; //Nulstiller Alarmen så den ikke starter men venter på AlarmDelay.

  buzzerPreviousMillis = CurMillis; //Gemmer tidpunktet buzzeren skal starte.
  buzzerState = true; //Sætter buzzeren tændt.
}

//Gør at programmet kun reagerer på ændringer i bevægelsen, så den ikke er konstant HIGH.
lastPirState = pirState;

//Tjekker om der er registreret bevægelse, så bib-lyden kan begynde.
if (alarmVentende && bevaegelseRegistreret && !AlarmAktiveret) { 
  if (CurMillis - buzzerPreviousMillis >= BibInterval) { 
    buzzerPreviousMillis = CurMillis; 
    buzzerState = !buzzerState; 
    if (buzzerState) { 
      tone(buzzerPin, BibFrekvens, BibInterval); 
    } 
  } 
}

//Et delay på 2sekunder til næste tekst.
if (lcdStartDisplayed && CurMillis - lcdStartMillis >= lcdDisplayTime) { 
  lcd.clear(); 
  lcd.setCursor(2, 0); 
  lcd.print("INDTAST KODE:"); 
  lcd.setCursor(6, 1); 
  lcd.print("");
  lcdStartDisplayed = false; 
}

//Deaktiveringsmekanisme.
if (locked && codeSet && !AlarmAktiveret && bevaegelseRegistreret) {

//Hvis et Ciffer fra 0 - 9 trykkes på Keypaden, gemmes det i INPUT og viser en stjerne på LCDen istedet. 
 if (customKey >= '0' && customKey <= '9') {
  input += customKey;
   if(input.length()<= 4) {
    lcd.print("*");
}
 }

//Nulstil den indtastede kode.
      if (customKey == '*') { 
      input = ""; 
      lcd.clear();
      lcd.setCursor(2,0);
      lcd.print("INDTAST KODE:");
      lcd.setCursor(6,1);
      }

//Åbningsmekanismen. 
      //Hvis de 4-Cifrede indtastet er korrekte. 
        if (input.length() == 4) {
          if (input == setCode) {
            noTone(buzzerPin);
            lcd.clear();
            lcd.setCursor(2,0);
            lcd.print("RIGTIG KODE");
            lcd.setCursor(1,1);
            lcd.print("ALARM SLUKKET");
            digitalWrite(ledPinR, LOW);//Slukker den Røde LED. 
            digitalWrite(ledPinG, HIGH);//Tænder den Grønne LED. 
            tone(buzzerPin, 700);//Afgiver en 700hz tone, når den rigtige kode er indtastet.
            
            delay(500);

            noTone(buzzerPin); //Slukker Buzzeren efter delay. 
            lcd.clear();
            locked = false;//Deaktivere Alarmensystemet. 
            codeSet = false;//Indikerer at den ikke længere kan huske den indtastede kode. 
            setCode = "";//Resetter koden. 
            alarmKanAktiveres = true;//Alarmen kan nu aktiveres igen med knappen. 
            alarmKlarTilPIR = false;//Slår PIR-Sensoren fra. 

            lcdAktiveretVises = false;//Gør at den ikke genviser teksten med at Alarmen kan aktiveres. 
            lcdStartDisplayed = false;//Gør at teksten ikke genvises med at Alarmen kan deaktiveres med kode. 
            lcdStartMillis = 0;//Nulstiller tiden. 

            lastPirState = digitalRead(pirPin);//Aflæser Pir-sensoren state. 

            alarmVentende = false; 
            bevaegelseRegistreret = false; 
            AlarmAktiveret = false;
 
            input = "";
            wrongAttempts = 0;
        
            delay(5000);//Danner et delay på 5sek, for at vise at systemet er nulstillet. 
        
            noTone(buzzerPin);
            lcd.setCursor(2,0);
            lcd.print("INDTAST KODE:");
            lcd.setCursor(6,1);
            lcd.print("");

            } else { 
           //Hvis man Indtaster koden forkert alle 3 forsøg. 
                wrongAttempts++;//Tæller antal forkert forsøg. 
                input = "";//Nulstiller inputet. 

                if (wrongAttempts >= maxAttempts) {
                noTone(buzzerPin);//Gør at bib-lyden stopper. 
            //Vises når alle forsøg er opbrugt. 
                lcd.clear();
                lcd.setCursor(2, 0);
                lcd.print("FORKERT KODE");
                lcd.setCursor(2, 1);
                lcd.print("ALARM STARTER!");
            
            //Buzzeren afspiller en tone pga man har tastet forkert. 
                tone(buzzerPin, 100, 500);//Afspiller en tone i 100hz og i 500ms).

                delay(1500);

                noTone(buzzerPin);//Slukker Buzzeren. 

            //Aktiverer Igansætnigen af Alarmen. 
                AlarmAktiveret = true;
                lcd.clear();
                lcd.setCursor(1, 0);
                lcd.print("ALARM IGANGSAT");
                lcd.setCursor(1, 1);
                lcd.print("FOR MANGE FEJL");
            
            //Tænder den Røde LED inden blinkloopet stater. 
                digitalWrite(ledPinR, HIGH);
                delay(2000); 
                
            //Viser at Alarmen er gået igang. 
                lcd.clear();
                lcd.setCursor(1,0);
                lcd.print("ALARM IGANGSAT");
                lcd.setCursor(0,1);
                lcd.print("----------------");

                return; 
                  } else {
              //Hvis man Indtaster forkert ved første og andet forsøg. 
                  lcd.clear();
                  lcd.setCursor(2, 0);
                  lcd.print("FORKERT KODE");
                  lcd.setCursor(2, 1);
                  lcd.print("INDTAST IGEN");
      
                  tone(buzzerPin, 100, 500);
                  delay(1500);
                  noTone(buzzerPin); 

              //Gør at LCDen visuelt er klar til næste forsøg. 
                  lcd.clear();
                  lcd.setCursor(2,0);
                  lcd.print("INDTAST KODE");
                  lcd.setCursor(6, 1);
                  lcd.print("");
               }
            }
          }
        }   

//Tjekker om Alarmen allerede kører, og starter kun hvis den ikke gør.
  if (alarmVentende && bevaegelseRegistreret && setCode && !AlarmAktiveret && CurMillis - lastMotionTime > AlarmDelay) {
    noTone(buzzerPin);
    AlarmAktiveret = true;
    lcd.clear();
    lcd.setCursor(1, 0);
    lcd.print("ALARM IGANGSAT!");
    lcd.setCursor(0,1);
    lcd.print("----------------");

  //Gør at LED er synkroniseret så den aktivere på det rigtige tidpunkt.
    PreMillis = CurMillis; // Nulstil timeren for at starte blink med det samme.
    ledState = HIGH;  // Sætter start tilstanden til at være tændt
    digitalWrite(ledPinR, ledState); // Gør at Leden tænder med det samme.
  }
 //Gør at Leden blinker.
  if (AlarmAktiveret) {
    if (CurMillis - PreMillis >= BlinkInterval) {
      PreMillis = CurMillis; // Gemmer tidspunktet for hvornår den sidst blinkede.

      ledState = !ledState; //Sikre sig at LEDen skrifter mellem Tændt og Slukket.

      digitalWrite(ledPinR, ledState); //Udfører skiftet på pin 10.

      if (ledState == HIGH){
        tone(buzzerPin,222, 1000);
      }
      else{
        noTone(buzzerPin);
      }
    }
  } else {
    //Gør at LEDen er slukket, når Alarmen ikke er aktiv.
    digitalWrite(ledPinR, LOW);
  }
lastPirState = pirState;//Gør at programmer kun reagere på ændringer i bevægelsen, så den ikke er konstant HIGH. 
}