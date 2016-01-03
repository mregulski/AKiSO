Kodowanie instrukcji
====================

`[opcode: 8bit] [int/reg: 16bit] [reg2: 16bit]`

Jeśli opcode przyjmuje jeden argument, reg2 jest pomijany.\
Argument typu int dopuszcza wartości takie jak typ short w C.\
reg i reg2 dopuszczają liczby całkowite z przedziału 0..511
(wymuszane przez RMA).

Opcody
======

Pierwsze 4 bity: rodzaj argumentu:

    0: SREG     (1 rejestr)
    1: SINT     (1 liczba 16bit [short])
    2: DREG     (2 rejestry)
    3: INT_REG  (liczba 16bit i rejestr)
    4: NONE     (brak argumentu; tylko STP)

Drugie 4 bity: numer instrukcji

    NAZWA   HEX     DEC     ROZMIAR
    -----   -----   ----    -------
    INP     0x01    1       3
    PRT     0x02    2       3
    JMP     0x10    16      3
    JEQ     0x11    17      3
    JNE     0x12    18      3
    JGE     0x13    19      3
    JNG     0x14    20      3
    JLT     0x15    21      3
    JNL     0x16    22      3
    MOV     0x20    32      5
    ADD     0x21    33      5
    SUB     0x22    34      5
    MUL     0x23    35      5
    DIV     0x24    36      5
    MOD     0x25    37      5
    CMP     0x26    38      5
    LD      0X30    48      3
    STP     0xFF    255     1

RMA
===
`/ścieżka/rma <plik>`

*   RMA dopuszcza jedną instrukcję na linię.
*   Instrukcje i argumenty mogą być rodzielanie spacjami lub przecinkami.
    Instrukcje:
    ```
    MOV 14 15
    MOV 14,15
    MOV,14,15
    MOV 14,15
    ``` 
    są traktowane tak samo.
*   Zawartość linii po pierwszym elemencie (ograniczonym białymi znakami),
    który nie jest rozpoznawalny jako część instrukcji jest traktowana
    jako komentarz. Np:
```
INP 14 to jest komentarz
       ^ start komentarza 
to też jest komentarz
MOV 14 51 12 błąd składni: za dużo argumentów
         ^ start komentarza
MOV 14 51 teraz 12 nie przeszkadza
```
*   pliki wynikowe mają rozszerzenie .aks, i zaczynają się nagłówkiem
    "AKiSO ASM" 

    `0x41 0x4b 0x69 0x53 0x4f 0x20 0x41 0x53 0x4d`
* na końcu pliku automatycznie dodawany jest `STP`

RM EMU
======

`/path/rm_emu <plik> [-v] [-d] [-s]`

    -v, --verbose: wyświetla wszystkie wykonywane instrukcje

    -d, --disassemble: przechodzi przez cały kod, nie wykonując go.
                       Przydatne do adresowania skoków.
                       Implikuje -v.

    -s, --step: wykonuje kod instrukcja po instrukcji.
                Implikuje -v.


* 512 rejestrów 32bit (C int)

    * 2 specjalne: read/write tylko przez odpowiednie instrukcje

        0: `PC` - inkrementowany przy odczycie instrukcji, ustawiany przez `Jxx`.\
           Zawiera offset, liczony od pierwszego bajta po nagłówku, z którego
           ma być odczytany następny bajt (instrukcja lub część argumentu).\
        1: `F`  - flagi ustawiane przez `CMP`, odczyt tylko przez `Jxx`.
           Są 3 flagi, ustawiane przez `CMP regA regB`:
           
           EQ = A == B ? 1 : 0
           GT = A > B  ? 1 : 0
           LT = A < B  ? 1 : 0

    * 500 zwykłych (2 - 511)

* plik wykonywalny musi się zaczynać nagłówkiem` AKiSO ASM`

* `PC` = 0 odpowiada pierwszemu bajtowi po nagłówku

* rejestry 432-490 zawierają początkowo kody ASCII odpowiednia 32 - 90\
  rejestr 491 zawiera `LF` (`0x10`)
