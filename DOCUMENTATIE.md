# Sistem de Rezervări Cinema — Documentație

## Ce am construit

Am pornit de la o temă simplă de rezervare cinema și am ajuns la o aplicație desktop Qt care simulează destul de fidel cum funcționează un sistem real de booking. Aplicația gestionează o rețea de 24 de cinematografe din toată România, generează automat programul de proiecții respectând reguli logice (filmele horror nu rulează dimineața, animațiile nu rulează la miezul nopții etc.), permite rezervarea în grup, comandă de mâncare și exportul rezervării în calendar.

---

## Structura claselor

### Film
Clasa de bază a aplicației. Stochează toate datele unui film — titlu, durată, gen, rating, dacă e 3D sau nu, datele despre premieră și un ID YouTube pentru trailer. Rating-ul nu e static, ci se recalculează automat din evaluările utilizatorilor.

| Atribut | Tip | Descriere |
|---|---|---|
| `titlu` | `string` | Titlul filmului |
| `este3D` | `bool` | Format 2D sau 3D |
| `pretBaza` | `double` | Prețul de bază al biletului (RON) |
| `gen` | `string` | Genul filmului |
| `durata` | `int` | Durata în minute |
| `status` | `StatusFilm` | `RULAZA_ACUM` sau `IN_CURAND` |
| `rating` | `double` | Rating /10 — actualizat din evaluările utilizatorilor |
| `trailerYtId` | `string` | ID-ul YouTube al trailerului |

### Sala
Modelează o sală fizică ca o matrice de locuri. Fiecare loc are un tip (Standard, VIP sau Cuplu) și starea lui e ținută per dată + oră — dacă sala e plină vineri la 20:00 asta nu afectează disponibilitatea sâmbătă.

| Atribut | Tip | Descriere |
|---|---|---|
| `id` | `int` | Identificator unic |
| `randuri`, `coloane` | `int` | Dimensiunile matricei |
| `ocupare` | `map<cheie, matrice>` | Starea locurilor per dată+oră |
| `categorii` | `matrice` | Tipul fiecărui loc |

Metoda `ocupaLoc` aruncă `LocOcupatException` dacă locul e deja luat și `IndexInvalidException` dacă indexul e în afara matricei — nu am lăsat erorile să treacă silențios.

### Rezervare și RezervareOnline
`Rezervare` e clasa de bază, `RezervareOnline` o extinde adăugând emailul clientului. Metoda `afiseaza()` e virtuală și suprascrisă în clasa derivată — folosit pentru polimorfism.

### Programare
Reprezintă un slot de proiecție — un film într-o sală la o oră anume, pe o anumită dată. Programările sunt generate automat de sistem, nu introduse manual.

| Atribut | Tip | Descriere |
|---|---|---|
| `filmTitlu` | `string` | Filmul programat |
| `salaId` | `int` | Sala în care rulează |
| `ora` | `string` | Ora de start |
| `oraFinal` | `string` | Ora de final (start + durată + 45 min buffer) |
| `data` | `Data` | Data proiecției |

### CinemaLocation
O sucursală a cinematografului. Are propriile săli, propriul program, propriile ore de funcționare și un multiplicator de preț. Există 3 tier-uri:

- **Premium** — acces la toate filmele inclusiv cele noi, prețuri mai mari, program extins
- **Standard** — repertoriul obișnuit
- **Basic** — nu rulează filmele cu rating foarte mare, prețuri reduse

| Atribut | Tip | Descriere |
|---|---|---|
| `id`, `nume`, `oras` | `string` | Identificare |
| `regiune` | `string` | Moldova / Transilvania / Muntenia etc. |
| `tier` | `CinemaTier` | PREMIUM / STANDARD / BASIC |
| `pretMultiplicator` | `double` | Factor de preț al locației |
| `sali` | `vector<Sala>` | Sălile din această locație |
| `program` | `vector<Programare>` | Programul generat automat |

### Cinematograf
Clasa principală care leagă totul. Implementează interfața `ICinemaService` cu metodele pur virtuale `adaugaFilm` și `adaugaSala`. Gestionează autentificarea, rezervările, generarea programului, evaluările și preferințele de loc.

### ICinemaService
Interfață abstractă — definește ce servicii trebuie să ofere orice cinematograf, indiferent de implementare:
```cpp
virtual void adaugaFilm(Film f) = 0;
virtual void adaugaSala(Sala s) = 0;
```

### PreferintaLoc
Stochează tipul și poziția preferată de loc a utilizatorului, salvată după fiecare rezervare și afișată ca hint pe harta sălii la următoarea vizită.

### FoodItem / FoodOrder
Reprezintă un produs din meniu și o comandă de food. Meniul conține 17 produse în 4 categorii: Popcorn, Băuturi, Snacks, Combos.

---

## Diagrame UML

### Diagrama claselor

```
ICinemaService (interfata)
    ^ implementeaza
Cinematograf
    +-- agregeaza --> vector<CinemaLocation>
    |                     +-- vector<Sala>
    |                     +-- vector<Programare>
    +-- agregeaza --> vector<Film>
    +-- agregeaza --> vector<Rezervare*>
    |                     +-- RezervareOnline (derivata)
    +-- agregeaza --> vector<ContUtilizator>
                          +-- map<string,int> evaluari
                          +-- PreferintaLoc preferinta

Rezervare ----are un----> Film
Rezervare ----are un loc in----> Sala
Programare ---are un----> Film
Programare ---are loc in----> Sala
```

### Fluxul de rezervare

```
Login --> Alege Locatie --> Meniu Principal
  --> Alege Film --> Detalii Film --> Alege Data
  --> Alege Programare (sala + ora) --> Selecteaza Locuri
  --> Food & Drinks (optional) --> Confirmare
  --> Export Calendar (.ics) [optional]
```

---

## Concepte POO utilizate

**Încapsulare** — toate atributele claselor sunt `private`, accesul se face exclusiv prin getteri/setteri publici.

**Moștenire** — `RezervareOnline` extinde `Rezervare` adăugând `emailClient`. Supradefinire (`override`) a metodei virtuale `afiseaza()`.

**Polimorfism** — `vector<Rezervare*>` poate conține atât obiecte `Rezervare` cât și `RezervareOnline`. Metoda `afiseaza()` virtuală este apelată polimorfic.

**Abstractizare** — `ICinemaService` definește contractul public al serviciului cinema prin metode pur virtuale.

**Tratare excepții** — 3 clase proprii de excepții:
```cpp
class LocOcupatException : public std::runtime_error { ... };
class IndexInvalidException : public std::out_of_range { ... };
class SalaNotFoundException : public std::runtime_error { ... };
```

---

## Cum funcționează prețurile

Nu am vrut un preț fix — în realitate biletele variază în funcție de mai mulți factori, și mi s-a părut mai interesant de implementat:

```
pret = pret_baza_film
     + 10 RON daca e 3D
     x tip_loc (Standard x1.0 / VIP x1.5 / Cuplu x1.8)
     x zi (weekend x1.2 / weekday x1.0)
     x locatie (Basic ~x0.85 / Standard ~x1.0 / Premium ~x1.25)
```

---

## Sistemul de programări automat

Asta e probabil cea mai complexă parte. Când utilizatorul selectează o locație, aplicația generează automat un program de proiecții pentru 7 zile, pentru fiecare sală.

Algoritmul pornește de la ora de deschidere și umple fiecare sală cu filme unul după altul, respectând:

**Reguli de gen:**

| Gen | Cel mai devreme | Cel mai târziu |
|---|---|---|
| Horror / Thriller | 18:00 | 23:59 |
| Animație / Familie | 10:00 | 20:00 |
| Comedie / Acțiune / SF | 10:00 | 23:00 |
| Drama / Altele | 12:00 | 23:00 |

**Ore de funcționare:**

| Tier | Deschide | Ultimul film |
|---|---|---|
| Basic | 12:00 | 21:00 |
| Standard | 10:00 | 22:30 |
| Premium | 10:00 | 23:30 |

**Buffer între filme:** durata filmului + 45 de minute (30 pentru reclame/trailers + 15 pentru pregătirea sălii).

**Variație între cinematografe:** fiecare locație primește un offset de 0-75 minute calculat din ID-ul său, astfel că Cinema City Suceava și Cinema City Iași rulează același film la ore ușor diferite — exact cum e în realitate.

---

## Rețeaua de cinematografe

24 de locații în 7 regiuni geografice:

| Regiune | Orașe |
|---|---|
| Moldova | Suceava (2), Iași (2), Bacău (2), Piatra Neamț (1) |
| Transilvania | Cluj-Napoca (2), Brașov (2), Sibiu (2) |
| Muntenia | București (3), Ploiești (1) |
| Oltenia | Craiova (2) |
| Dobrogea | Constanța (2) |
| Banat | Timișoara (2) |
| Crișana | Oradea (1), Arad (1) |

Când te loghezi, aplicația îți detectează automat orașul prin API-ul `ip-api.com` și îți arată cinematografele din orașul tău plus cele din cele mai apropiate 3 orașe din aceeași regiune. Dacă ești în Brașov nu îți apare Constanța.

---

## Persistarea datelor

| Fișier | Ce conține |
|---|---|
| `filme.txt` | Filmele, format pipe-separated (16 câmpuri) |
| `conturi.txt` | Username, parolă, tip cont |
| `toate_rezervarile.txt` | Log complet al tuturor rezervărilor |
| `istoric_<user>.txt` | Istoricul per utilizator |
| `ratings_<user>.txt` | Evaluările filmelor |
| `preferinte_<user>.txt` | Tipul și poziția preferată de loc |
| `bilete/Bilet_*.txt` | Biletele generate după fiecare rezervare |
| `rezervare.ics` | Exportul în calendar (generat la cerere) |

---

## Teste unitare

16 teste scrise manual în `tests/test_main.cpp`, fără biblioteci externe, acoperind Film, Sala, excepții, prețuri, Rezervare, FoodOrder, Data și getRegiune.

```bash
g++ -std=c++20 -I include tests/test_main.cpp -o tests/run_tests
./tests/run_tests
```

---

## Build și rulare

```bash
mkdir build && cd build
cmake .. && make
./CinemaBookingSystem
```

Sau deschide `CinemaBookingSystem.sln` în Visual Studio 2022 cu Qt 6.7.3. Executabilul precompilat e disponibil în Releases.

---

## Posibile îmbunătățiri

- O bază de date SQLite în loc de fișiere text — mai rapid și mai robust
- Notificări email reale pentru confirmarea rezervării
- Sistem de reduceri — card de fidelitate, reduceri studenți
- Aplicație mobilă care să consume același backend
