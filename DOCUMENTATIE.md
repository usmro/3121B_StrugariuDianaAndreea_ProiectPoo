# Documentație – Sistem de Rezervări Cinema

## 1. Descriere generală

Aplicație desktop Qt C++ pentru gestionarea rezervărilor de bilete la cinematograf.
Suportă multiple locații, rezervare în grup, comandă food & drinks și detecție automată a locației.

---

## 2. Descrierea claselor

### `Film`
Reprezintă un film din repertoriul cinematografului.

| Atribut | Tip | Descriere |
|---|---|---|
| `titlu` | `string` | Titlul filmului |
| `este3D` | `bool` | Format 2D sau 3D |
| `pretBaza` | `double` | Prețul de bază al biletului (RON) |
| `gen` | `string` | Genul filmului (Acțiune, SF, etc.) |
| `durata` | `int` | Durata în minute |
| `status` | `StatusFilm` | `RULAZA_ACUM` sau `IN_CURAND` |
| `rating` | `double` | Rating /10 |
| `descriere` | `string` | Sinopsis |
| `regizor` | `string` | Regizorul filmului |

### `Sala`
Reprezintă o sală fizică dintr-un cinematograf, cu o matrice de locuri.

| Atribut | Tip | Descriere |
|---|---|---|
| `id` | `int` | Identificator unic |
| `nume` | `string` | Numele sălii |
| `randuri` | `int` | Numărul de rânduri |
| `coloane` | `int` | Numărul de coloane |
| `ocupare` | `map<cheie, matrice>` | Starea locurilor per dată+oră |
| `categorii` | `matrice` | Tipul fiecărui loc (Standard/VIP/Cuplu) |

**Metode cheie:**
- `ocupaLoc(data, ora, r, l)` — rezervă un loc; aruncă excepție dacă e ocupat
- `elibereazaLoc(data, ora, r, l)` — eliberează un loc
- `locuriLibere(data, ora)` — returnează numărul de locuri disponibile
- `esteOcupat(data, ora, r, l)` — verifică dacă un loc e ocupat

### `Rezervare`
Înregistrează o rezervare a unui loc de un utilizator.

| Atribut | Tip | Descriere |
|---|---|---|
| `titluFilm` | `string` | Filmul rezervat |
| `salaNume` | `string` | Sala în care se face rezervarea |
| `data` | `Data` | Data proiecției |
| `ora` | `string` | Ora proiecției |
| `rand`, `loc` | `int` | Poziția locului |
| `pretFinal` | `double` | Prețul calculat |
| `username` | `string` | Utilizatorul care a rezervat |

### `RezervareOnline` *(extinde Rezervare)*
Adaugă atributul `emailClient` pentru confirmarea prin email.

### `Cinematograf` *(implementează ICinemaService)*
Clasa principală de business. Agregă locații, filme, rezervări și conturi.

**Responsabilități:**
- Gestionare filme (CRUD)
- Gestionare locații cu sălile lor
- Procesare rezervări cu calcul de preț
- Autentificare utilizatori
- Persistare date în fișiere text

### `CinemaLocation`
Reprezintă o sucursală a cinematografului (ex: Cinema City Suceava).

| Atribut | Tip | Descriere |
|---|---|---|
| `id` | `string` | Cod unic (ex: SUC1) |
| `oras` | `string` | Orașul |
| `regiune` | `string` | Regiunea geografică |
| `sali` | `vector<Sala>` | Sălile din această locație |

### `ICinemaService` *(interfață)*
Interfață abstractă cu metode pure virtuale:
```cpp
virtual void adaugaFilm(Film f) = 0;
virtual void adaugaSala(Sala s) = 0;
```

### `FoodItem` / `FoodOrder`
Reprezintă un produs din meniu și o comandă de food pentru o rezervare.

---

## 3. Diagrame UML

### Diagrama claselor (simplificată)

```
ICinemaService (interfata)
    ↑ implementeaza
Cinematograf
    ├── agregare ──→ vector<CinemaLocation>
    │                   └── vector<Sala>
    ├── agregare ──→ vector<Film>
    ├── agregare ──→ vector<Rezervare*>
    │                   └── RezervareOnline (derivata)
    └── agregare ──→ vector<ContUtilizator>

Rezervare ──── are un ────→ Film
Rezervare ──── are un loc in ─→ Sala
```

### Fluxul de rezervare

```
Login → Alege Locație → Meniu Principal
  → Alege Film → Detalii Film → Alege Sală
  → Alege Data → Alege Ora → Selectează Locuri (grup)
  → Food & Drinks (opțional) → Confirmare → Bilet generat
```

---

## 4. Concepte POO utilizate

### Încapsulare
Toate atributele claselor (`Film`, `Sala`, `Rezervare`) sunt `private`, accesul se face exclusiv prin getteri/setteri publici.

### Moștenire
`RezervareOnline` extinde `Rezervare` adăugând `emailClient`. Supradefiniție (`override`) a metodei virtuale `afiseaza()`.

### Polimorfism
`vector<Rezervare*>` poate conține atât obiecte `Rezervare` cât și `RezervareOnline`. Metoda `afiseaza()` virtuală este apelată polimorfic.

### Abstractizare
`ICinemaService` definește contractul public al serviciului cinema prin metode pur virtuale. `Cinematograf` oferă implementarea concretă.

### Tratare excepții
```cpp
// Aruncat de Sala::ocupaLoc când locul e deja rezervat
class LocOcupatException : public std::runtime_error { ... };

// Aruncat la index în afara matricei de locuri
class IndexInvalidException : public std::out_of_range { ... };

// Aruncat de Cinematograf::getSalaById pentru ID invalid
class SalaNotFoundException : public std::runtime_error { ... };
```

---

## 5. Calculul prețului biletului

Prețul final ține cont de trei factori:

```
pret_final = pret_baza
           + (este3D ? +10 RON : 0)          // suprataxa 3D
           × multiplicator_categorie           // Standard×1.0, VIP×1.5, Cuplu×1.8
           × multiplicator_zi                  // Weekend×1.2, Weekday×1.0
```

**Exemple:**
| Scenariu | Calcul | Total |
|---|---|---|
| 2D, Standard, weekday | 25 × 1.0 × 1.0 | 25 RON |
| 3D, Standard, weekday | (25+10) × 1.0 × 1.0 | 35 RON |
| 2D, VIP, weekend | 25 × 1.5 × 1.2 | 45 RON |
| 3D, Cuplu, weekend | (25+10) × 1.8 × 1.2 | 75.6 RON |

---

## 6. Persistarea datelor

Toate datele sunt salvate în fișiere text în directorul `data/`:

| Fișier | Conținut |
|---|---|
| `data/filme.txt` | Filmele, format pipe-separated |
| `data/conturi.txt` | Conturile utilizatorilor |
| `data/toate_rezervarile.txt` | Log complet al rezervărilor |
| `data/istoric_<username>.txt` | Istoricul per utilizator |
| `data/bilete/Bilet_*.txt` | Biletele generate |

---

## 7. Build și rulare

### Qt (aplicație desktop)
```bash
# Linux / macOS
mkdir build && cd build
cmake .. && make
./CinemaBookingSystem
```

### Teste unitare (fără Qt)
```bash
g++ -std=c++20 -I include tests/test_main.cpp -o tests/run_tests
./tests/run_tests
```

---

## 8. Posibile îmbunătățiri

- **Export .ics** — adăugare rezervare în Google Calendar / Outlook
- **Rating utilizatori** — notare filme după vizionare
- **Prețuri dinamice** — variație în funcție de cât de plină e sala
- **Notificări email** — trimitere automată a biletului
- **Bază de date SQL** — înlocuirea fișierelor text cu SQLite
- **API REST** — backend accesibil din aplicații mobile
