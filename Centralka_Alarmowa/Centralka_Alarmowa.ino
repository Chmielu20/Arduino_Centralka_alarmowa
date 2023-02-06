// Filip Chmielowski
// Wrocław, 06.02.2023r.
// Centralka alarmowa

#include <Keypad.h> // Biblioteka od klawiatury numerycznej
#include <Adafruit_NeoPixel.h> // Biblioteka od linijki diod LED WS2812

#define PIR 11 // Czujnik ruchu PIR podłączony do pinu 11
#define KONTAKTRON 12 // Kontaktron podłączony do pinu 12
#define BUZZER 13 // Buzzer podłączony do pinu 13
#define BUTTON 2 // Przycisk podłączony do pinu 2
// W przypadku Arduino Uno, przerwania obsługiwane są jedynie na pinach
// nr 2 i nr 3, więc przycisk podłączony został do jednego z tych pinów.

const byte ROWS = 4; // Ilość wierszy w klawiaturze
const byte COLS = 4; // Ilość kolumn w klawiaturze

byte rowPins[ROWS] = {6, 5, 4, 3}; // Piny wierszy klawiatury
byte colPins[COLS] = {7, 8, 9, 10}; // Piny kolumn klawiatury

// Mapowanie klawiatury - przypisanie konkretnych znaków pod
// przyciski klawiatury za pomocą tablicy dwuwymiarowej:
char keys[ROWS][COLS] =
{
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

// Utworzenie nowego obiektu typu "Keypad" o nazwie "klawiatura":
Keypad klawiatura = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// Utworzenie nowego obiektu typu "Adafruit_NeoPixel" o nazwie "linijka":
Adafruit_NeoPixel linijka = Adafruit_NeoPixel(8, A0, NEO_GRB + NEO_KHZ800);

volatile int stanAlarmu = 1; // Aktualny stan systemu alarmowego
// Ważna zmienna - jej wartość będzie decydowała o wykonywanych operacjach.

int pinAlarmuPozycja = 1; // Ilość podanych poprawnych znaków z kodu PIN
char pinCyfra1 = '2'; // Cyfra pierwsza kodu PIN
char pinCyfra2 = '8'; // Cyfra druga kodu PIN
char pinCyfra3 = '0'; // Cyfra trzecia kodu PIN
char pinCyfra4 = '8'; // Cyfra czwarta kodu PIN

int ileCzasuMinelo = 0; // Zmienna zliczająca czas na wpisanie kodu PIN

void setup()
{
  pinMode(PIR, INPUT_PULLUP); // Czujnik ruchu PIR jako wejście typu 'pullup'
  pinMode(KONTAKTRON, INPUT_PULLUP); // Kontaktron jako wejście typu 'pullup'
  pinMode(BUZZER, OUTPUT); // Buzzer bez generatora jako wyjście
  pinMode(BUTTON, INPUT_PULLUP); // Przycisk jako wejście typu 'pullup'

  // Deklaracja przerwania zewnętrznego na pinie 2, gdzie podłączono przycisk.
  // Na zmianę wartości ze stanu niskiego na wysoki, wywoływana jest funkcja
  // resetująca alarm (nieprzyjmująca argumentów i niezwracająca niczego).
  attachInterrupt(digitalPinToInterrupt(BUTTON), zresetujAlarm, RISING);

  linijka.begin(); // Inicjalizacja linijki diod LED
  linijka.show();
}

void loop()
{
  char klawisz = 0; // Zmienna do przetrzymywania znaków z klawiatury
  int i = 0; // Zmienna pomocnicza do pętli  

  switch(stanAlarmu) // Wykonywanie akcji odpowiedniej dla danego stanu systemu
  {
    case 1: // CZUWANIE ALARMU
      linijka.setPixelColor(0, linijka.Color(0, 15, 0)); // Dioda 1 na zielono
      linijka.show();

      klawisz = klawiatura.getKey();

      if(klawisz == 'A') // Uzbrojenie alarmu po naciśnięciu przycisku 'A'
      {
        tone(BUZZER, 1750); // Uruchomienie buzzera na częstotliwości 1750 [Hz]
        delay(100); // Małe opóźnienie, które nie przeszkadza w pracy programu
        tone(BUZZER, 2250); // Uruchomienie buzzera na częstotliwości 2250 [Hz]
        delay(100); // Małe opóźnienie, które nie przeszkadza w pracy programu
        noTone(BUZZER); // Wyłączenie buzzera
        
        for(i = 1; i < 8; i++) // Wykonanie tej pętli zajmie około 5 sekund
        {
          linijka.setPixelColor(i, linijka.Color(0, 0, 15)); // Dioda i na niebiesko
          linijka.show();
          delay(710); // Małe opóźnienie, które nie przeszkadza w pracy programu
        }

        for(i = 1; i < 8; i++) // Wykonanie tej pętli zajmie około 5 sekund
        {
          linijka.setPixelColor(i, linijka.Color(15, 0, 0)); // Dioda i na czerwono
          linijka.show();
          delay(710); // Małe opóźnienie, które nie przeszkadza w pracy programu
        }

        tone(BUZZER, 2250); // Uruchomienie buzzera na częstotliwości 2250 [Hz]
        delay(100); // Małe opóźnienie, które nie przeszkadza w pracy programu
        tone(BUZZER, 1750); // Uruchomienie buzzera na częstotliwości 1750 [Hz]
        delay(100); // Małe opóźnienie, które nie przeszkadza w pracy programu
        noTone(BUZZER); // Wyłączenie buzzera

        wylaczDiody();
        stanAlarmu = 2; // Przejście do monitorowania pomieszczenia
      }
    break;

    case 2: // MONITOROWANIE POMIESZCZENIA
      linijka.setPixelColor(7, linijka.Color(15, 0, 0)); // Dioda 8 na czerwono
      linijka.show();
      delay(50); // Małe opóźnienie dla uwidocznienia efektów świetlnych linijki

      linijka.setPixelColor(7, linijka.Color(0, 0, 0)); // Dioda 8 wyłączona
      linijka.show();
      delay(50); // Małe opóźnienie dla uwidocznienia efektów świetlnych linijki

      if(digitalRead(PIR) == HIGH) // Jeżeli czujnik ruchu PIR wykryje ruch
      {
        stanAlarmu = 4; // Natychmiast uruchamiany jest alarm
      }
      else // Jeżeli PIR nie wykryje ruchu, a kontaktron zarejestruje otwarcie drzwi
      {
        ileCzasuMinelo = 0; // Zerowanie zmiennej odliczającej mierzony czas
        stanAlarmu = 3; // Szansa na rozbrojenie systemu alarmowego
      }
    break;

    case 3: // ROZBRAJANIE ALARMU (WPISYWANIE PINU)
      klawisz = klawiatura.getKey();

      if(klawisz) // Sprawdzenie poprawności kolejnych cyfr kodu PIN
      {
        if(pinAlarmuPozycja == 1 && klawisz == pinCyfra1) // Pierwsza cyfra PINu
        {
          pinAlarmuPozycja++; // Cyfra poprawna, można sprawdzać kolejną
          tone(BUZZER, 2000, 50); // Uruchomienie buzzera na częstotliwości 2000 [Hz]
          // Buzzer będzie wydawał dźwięk przez czas 50 ms
        }
        else if(pinAlarmuPozycja == 2 && klawisz == pinCyfra2) // Druga cyfra PINu
        {
          pinAlarmuPozycja++; // Cyfra poprawna, można sprawdzać kolejną
          tone(BUZZER, 2200, 50); // Uruchomienie buzzera na częstotliwości 2200 [Hz]
          // Buzzer będzie wydawał dźwięk przez czas 50 ms
        }
        else if(pinAlarmuPozycja == 3 && klawisz == pinCyfra3) // Trzecia cyfra PINu
        {
          pinAlarmuPozycja++; // Cyfra poprawna, można sprawdzać kolejną
          tone(BUZZER, 2400, 50); // Uruchomienie buzzera na częstotliwości 2400 [Hz]
          // Buzzer będzie wydawał dźwięk przez czas 50 ms
        }
        else if(pinAlarmuPozycja == 4 && klawisz == pinCyfra4)  // Czwarta cyfra PINu
        {
          pinAlarmuPozycja = 1; // Reset informacji o wpisywanym kodzie PIN
          tone(BUZZER, 2600, 50); // Uruchomienie buzzera na częstotliwości 2600 [Hz]
          // Buzzer będzie wydawał dźwięk przez czas 50 ms
          stanAlarmu = 1; // Wszystkie cztery cyfry kodu PIN są poprawne
        }
        else
        {
          pinAlarmuPozycja = 1; // Reset informacji o wpisywanym kodzie PIN
          stanAlarmu = 4; // Błąd w kodzie PIN - włącz alarm
        }
      }

      delay(100); // Opóźnienie całego programu o 0,1 sekundy
      ileCzasuMinelo++; // Zinkrementowanie zmiennej mierzącej czas

      if(ileCzasuMinelo >= 75) // Jeżeli zmierzony czas jest większy od 7,5 sekundy
      {
        stanAlarmu = 4; // Włączenie alarmu
      }
    break;

    case 4: // SYGNALIZACJA ALARMU
      for(i = 0; i < 8; i++) // Ustawienie wszystkich 8 diod LED w linijce
      {
        linijka.setPixelColor(i, linijka.Color(255, 0, 0)); // Dioda i na czerwono
      }
      linijka.show();
      tone(BUZZER, 4250); // Uruchomienie buzzera na częstotliwości 4250 [Hz]
      delay(100); // Małe opóźnienie, aby efekt wizualny był widoczny

      for(i = 0; i < 8; i++) // Ustawienie wszystkich 8 diod LED w linijce
      {
        linijka.setPixelColor(i, linijka.Color(0, 0, 255)); // Dioda i na niebiesko
      }
      linijka.show();
      tone(BUZZER, 3750); // Uruchomienie buzzera na częstotliwości 3750 [Hz]
      delay(100); // Małe opóźnienie, aby efekt wizualny był widoczny
    break;
  }
}

// Funkcja resetująca alarm, który następnie przechodzi do stanu czuwania
void zresetujAlarm()
{
  wylaczDiody();
  noTone(BUZZER); // Wyłączenie buzzera
  delay(10); // Małe opóźnienie, aby diody i buzzer zdążyły się wyłączyć
  stanAlarmu = 1; // Przejście do czuwania alarmu
}

// Funkcja wyłączająca wszystkie diody w linijce diod LED WS2812
void wylaczDiody()
{
  for(int i = 0; i < 8; i++) // Ustawienie wszystkich 8 diod LED w linijce
  {
    linijka.setPixelColor(i, linijka.Color(0, 0, 0)); // Dioda i wyłączona
  }

  linijka.show();
}

/*
STEROWANIE UKŁADEM CENTRALKI ALARMOWEJ:
-> Jeżeli świeci się pojedyncza zielona dioda w pasku diod LED, należy
   nacisnąć przycisk 'A', aby uzbroić alarm.
-> Po uzbrojeniu alarmu, można włączyć go od razu, powodując ruch przed
   czujnikiem ruchu PIR, lub można włączyć go z opóźnieniem około 5 sekund,
   rozwierając kontaktron i czekając lub wpisując błędny kod PIN.
-> Po rozwarciu kontaktronu, należy wpisać poprawny kod PIN (2808), aby
   dezaktywować alarm i przejść do jego czuwania (czekania na uzbrojenie).
-> Jeżeli alarm zostanie włączony, można go wyłączyć i przejść do trybu
   czuwania poprzez naciśnięcie (osobnego) przycisku.
*/
