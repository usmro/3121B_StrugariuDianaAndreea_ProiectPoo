#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <cstdlib>
#include <limits>
#include "CinemaModels.h"

// ============================================================
//  COMPATIBILITATE CROSS-PLATFORM: tastatura & terminal
// ============================================================
#ifdef _WIN32
#include <conio.h>
#define CLEAR "cls"
enum Tasta { T_SUS = 1000, T_JOS, T_STANGA, T_DREAPTA, T_ENTER, T_ESC, T_BACKSPACE, T_ALTUL };
Tasta citesteInput(int& ch) {
    int c = _getch();
    if (c == 224 || c == 0) {
        int c2 = _getch();
        if (c2 == 72) return T_SUS;
        if (c2 == 80) return T_JOS;
        if (c2 == 75) return T_STANGA;
        if (c2 == 77) return T_DREAPTA;
        return T_ALTUL;
    }
    if (c == 13) return T_ENTER;
    if (c == 27) return T_ESC;
    if (c == 8)  return T_BACKSPACE;
    ch = c; return T_ALTUL;
}
#else
#include <termios.h>
#include <unistd.h>
#define CLEAR "clear"
enum Tasta { T_SUS = 1000, T_JOS, T_STANGA, T_DREAPTA, T_ENTER, T_ESC, T_BACKSPACE, T_ALTUL };

static int rawGetchar(bool blocking = true) {
    struct termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    newt.c_cc[VMIN] = blocking ? 1 : 0;
    newt.c_cc[VTIME] = blocking ? 0 : 1;
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    int c = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    return c;
}

Tasta citesteInput(int& ch) {
    int c = rawGetchar(true);
    if (c == 27) {
        int c2 = rawGetchar(false);
        if (c2 == EOF || c2 == -1) return T_ESC;
        if (c2 == 91) {
            int c3 = rawGetchar(true);
            if (c3 == 65) return T_SUS;
            if (c3 == 66) return T_JOS;
            if (c3 == 67) return T_DREAPTA;
            if (c3 == 68) return T_STANGA;
        }
        return T_ESC;
    }
    if (c == 10 || c == 13) return T_ENTER;
    if (c == 127 || c == 8) return T_BACKSPACE;
    ch = c; return T_ALTUL;
}
#endif

// ============================================================
//  CLASA PRINCIPALA CINEMATOGRAF
// ============================================================
class Cinematograf : public ICinemaService {
private:
    std::vector<Film>          filme;
    std::vector<Sala>          sali;
    std::vector<Rezervare*>    istoricRezervari;
    std::vector<ContUtilizator> utilizatori;
    ContUtilizator* utilizatorCurent = nullptr;

    std::vector<std::string> oreDisponibile = { "11:00","14:00","17:30","20:45" };

    // ---- persistenta conturi ----
    void salveazaConturiInFisier() {
        std::ofstream f("conturi.txt", std::ios::trunc);
        for (auto& u : utilizatori)
            f << u.username << "," << u.parola << "," << (u.esteAdmin ? "1" : "0") << "\n";
    }

    void incarcaConturiDinFisier() {
        std::ifstream f("conturi.txt");
        if (!f.is_open()) {
            // Creeaza admin implicit daca nu exista fisier
            utilizatori.push_back({ "admin", "admin123", true, {} });
            salveazaConturiInFisier();
            return;
        }
        std::string ln;
        while (std::getline(f, ln)) {
            if (!ln.empty() && ln.back() == '\r') ln.pop_back();
            std::stringstream ss(ln);
            std::string usr, par, adm;
            if (std::getline(ss, usr, ',') && std::getline(ss, par, ',') && std::getline(ss, adm, ','))
                utilizatori.push_back({ usr, par, adm == "1", {} });
        }
        if (utilizatori.empty()) {
            utilizatori.push_back({ "admin","admin123",true,{} });
            salveazaConturiInFisier();
        }
    }

    ContUtilizator* gasesteUtilizator(const std::string& usr) {
        for (auto& u : utilizatori) if (u.username == usr) return &u;
        return nullptr;
    }

    // ---- cod bilet unic ----
    std::string genereazaCod(int r, int l) {
        static const char ch[] = "ABCDEFGHJKLMNPQRSTUVWXYZ23456789";
        unsigned seed = (unsigned)(time(nullptr)) ^ ((r + 1) * 31u) ^ ((l + 1) * 97u);
        std::srand(seed);
        std::string cod;
        for (int i = 0;i < 4;++i) cod += ch[std::rand() % 32];
        cod += "-";
        for (int i = 0;i < 4;++i) cod += ch[std::rand() % 32];
        return cod;
    }

public:
    // ---- getteri ----
    void adaugaFilm(Film f) override { filme.push_back(f); }
    void adaugaSala(Sala s) override { sali.push_back(s); }
    const std::vector<Film>& getFilme()  const { return filme; }
    const std::vector<std::string>& getOre()    const { return oreDisponibile; }
    const std::vector<Sala>& getSali()   const { return sali; }
    ContUtilizator* getCurent() { return utilizatorCurent; }

    Sala* getSalaById(int id) {
        for (auto& s : sali) if (s.getId() == id) return &s;
        return nullptr;
    }

    // ---- incarcare date ----
    void incarcaFilme(const std::string& fisier) {
        std::ifstream f(fisier);
        if (!f.is_open()) return;
        std::string ln;
        while (std::getline(f, ln)) {
            if (!ln.empty() && ln.back() == '\r') ln.pop_back();
            std::stringstream ss(ln);
            std::string titlu, s3d, spret, gen, sdur;
            if (std::getline(ss, titlu, ',') && std::getline(ss, s3d, ',') && std::getline(ss, spret, ',')) {
                std::string g = "General", sd = "0";
                std::getline(ss, g, ',');
                std::getline(ss, sd, ',');
                try { filme.push_back(Film(titlu, s3d == "1", std::stod(spret), g, std::stoi(sd))); }
                catch (...) {}
            }
        }
    }

    void incarcaRezervari() {
        std::ifstream f("toate_rezervarile.txt");
        if (!f.is_open()) return;
        std::string ln;
        while (std::getline(f, ln)) {
            if (!ln.empty() && ln.back() == '\r') ln.pop_back();
            if (ln.find("Sala: ") == std::string::npos) continue;
            if (ln.find("Ora: ") == std::string::npos) continue;
            size_t pS = ln.find("Sala: ");
            std::string salaNume = ln.substr(pS + 6, ln.find(" |", pS + 6) - pS - 6);
            size_t pO = ln.find("Ora: ");
            std::string oraStr = ln.substr(pO + 5, 5);
            size_t pLoc = ln.find("Loc: R");
            if (pLoc == std::string::npos) continue;
            int r = 0, l = 0;
            if (sscanf(ln.c_str() + pLoc, "Loc: R%d L%d", &r, &l) == 2)
                for (auto& s : sali)
                    if (s.getNume() == salaNume)
                        s.ocupaLoc(oraStr, r - 1, l - 1);
        }
    }

    void init() {
        incarcaConturiDinFisier();
    }

    // ---- autentificare ----
    bool login(const std::string& usr, const std::string& par) {
        ContUtilizator* u = gasesteUtilizator(usr);
        if (u && u->parola == par) { utilizatorCurent = u; return true; }
        return false;
    }

    bool inregistrare(const std::string& usr, const std::string& par) {
        if (usr.empty() || par.empty()) return false;
        if (gasesteUtilizator(usr)) return false; // exista deja
        utilizatori.push_back({ usr, par, false, {} });
        salveazaConturiInFisier();
        utilizatorCurent = &utilizatori.back();
        return true;
    }

    void logout() { utilizatorCurent = nullptr; }

    // ---- admin: gestionare filme ----
    void adaugaFilmManual(const std::string& titlu, bool e3d, double pret,
        const std::string& gen, int durata) {
        filme.push_back(Film(titlu, e3d, pret, gen, durata));
        // Salveaza in filme.txt
        std::ofstream f("filme.txt", std::ios::app);
        f << titlu << "," << (e3d ? 1 : 0) << "," << std::fixed << std::setprecision(2)
            << pret << "," << gen << "," << durata << "\n";
    }

    bool stergeFilm(int index) {
        if (index < 0 || index >= (int)filme.size()) return false;
        filme.erase(filme.begin() + index);
        // Rescrie fisier
        std::ofstream f("filme.txt", std::ios::trunc);
        for (auto& fm : filme)
            f << fm.getTitlu() << "," << (fm.getEste3D() ? 1 : 0) << ","
            << std::fixed << std::setprecision(2) << fm.getPretBaza() << ","
            << fm.getGen() << "," << fm.getDurata() << "\n";
        return true;
    }

    // ---- admin: gestionare utilizatori ----
    const std::vector<ContUtilizator>& getUtilizatori() const { return utilizatori; }

    bool stergeUtilizator(const std::string& usr) {
        if (usr == "admin") return false; // nu stergem adminul
        for (auto it = utilizatori.begin(); it != utilizatori.end(); ++it) {
            if (it->username == usr) {
                utilizatori.erase(it);
                salveazaConturiInFisier();
                return true;
            }
        }
        return false;
    }

    void schimbaParola(const std::string& usr, const std::string& paraNou) {
        ContUtilizator* u = gasesteUtilizator(usr);
        if (u) { u->parola = paraNou; salveazaConturiInFisier(); }
    }

    // ---- rezervare completa ----
    void proceseazaRezervare(int idSala, int r, int l, Film* film,
        const std::string& ora, const std::string& email,
        double multExtra = 1.0) {
        Sala* s = getSalaById(idSala);
        if (!s) return;
        s->ocupaLoc(ora, r, l);

        CategorieLocSeat cat = s->getCategorieLocSeat(r, l);
        double pretFinal = (film->getPretBaza() + (film->getEste3D() ? 10.0 : 0.0))
            * multiplicatorCategorie(cat) * multExtra;
        std::string catStr = numeCategorie(cat);

        Rezervare* rez;
        if (!email.empty())
            rez = new RezervareOnline(*film, r, l, pretFinal, ora, s->getNume(), catStr, email);
        else
            rez = new Rezervare(*film, r, l, pretFinal, ora, s->getNume(), catStr);
        istoricRezervari.push_back(rez);

        // Adauga la istoricul utilizatorului curent
        if (utilizatorCurent) {
            std::ostringstream oss;
            oss << film->getTitlu() << " | " << s->getNume() << " | " << ora
                << " | R" << r + 1 << " L" << l + 1 << " [" << catStr << "]"
                << " | " << std::fixed << std::setprecision(2) << pretFinal << " RON";
            utilizatorCurent->istoricRezervari.push_back(oss.str());
        }

        // Salveaza in log
        std::ofstream f("toate_rezervarile.txt", std::ios::app);
        f << "Film: " << film->getTitlu()
            << " | Sala: " << s->getNume()
            << " | Ora: " << ora
            << " | Loc: R" << r + 1 << " L" << l + 1
            << " [" << catStr << "]"
            << " | Pret: " << std::fixed << std::setprecision(2) << pretFinal << " RON"
            << " | Client: " << (email.empty() ? (utilizatorCurent ? utilizatorCurent->username : "Offline") : email)
            << "\n";

        // Genereaza bilet
        genereazaBilet(film->getTitlu(), s->getNume(), ora, r + 1, l + 1, catStr, pretFinal);
    }

    void genereazaBilet(const std::string& filmN, const std::string& salaN,
        const std::string& ora, int r, int l,
        const std::string& catStr, double pret) {
        std::string numeFisier = "Bilet_R" + std::to_string(r) + "L" + std::to_string(l)
            + "_" + std::to_string(time(nullptr)) + ".txt";
        std::ofstream bf(numeFisier);
        std::string cod = genereazaCod(r, l);
        bf << "==========================================\n"
            << "           BILET CINEMATOGRAF            \n"
            << "==========================================\n"
            << " FILM : " << filmN << "\n"
            << " SALA : " << salaN << "\n"
            << " ORA  : " << ora << "\n"
            << " LOC  : Rand " << r << ", Loc " << l << " [" << catStr << "]\n"
            << " PRET : " << std::fixed << std::setprecision(2) << pret << " RON\n"
            << "------------------------------------------\n"
            << " COD  : " << cod << "\n"
            << "==========================================\n";
        bf.close();
        std::cout << "\n  [OK] Bilet generat: " << numeFisier << "\n";
    }

    // ---- anulare ----
    void anuleazaRezervare(int idSala, const std::string& ora, int r, int l) {
        Sala* s = getSalaById(idSala);
        if (!s || !s->esteLocOcupat(ora, r, l)) {
            std::cout << "\n  [!] Locul nu este ocupat sau sala invalida.\n";
            return;
        }
        s->elibereazaLoc(ora, r, l);
        std::ifstream fIn("toate_rezervarile.txt");
        std::vector<std::string> linii;
        std::string ln;
        std::string cheie = "Sala: " + s->getNume() + " | Ora: " + ora
            + " | Loc: R" + std::to_string(r + 1) + " L" + std::to_string(l + 1);
        bool gasit = false;
        while (std::getline(fIn, ln)) {
            if (!ln.empty() && ln.back() == '\r') ln.pop_back();
            if (!gasit && ln.find(cheie) != std::string::npos) gasit = true;
            else linii.push_back(ln);
        }
        fIn.close();
        std::ofstream fOut("toate_rezervarile.txt", std::ios::trunc);
        for (auto& lne : linii) fOut << lne << "\n";
        std::cout << (gasit ? "\n  [OK] Rezervare anulata.\n" : "\n  [!] Intrarea nu a fost gasita in log.\n");
    }

    // ---- statistici admin ----
    void afiseazaToateRezervariLog() {
        std::ifstream f("toate_rezervarile.txt");
        if (!f.is_open()) { std::cout << "  (fisier gol)\n"; return; }
        std::string ln; int nr = 0;
        while (std::getline(f, ln)) {
            if (!ln.empty() && ln.back() == '\r') ln.pop_back();
            if (!ln.empty()) { std::cout << "  " << ++nr << ". " << ln << "\n"; }
        }
        if (nr == 0) std::cout << "  (nicio rezervare)\n";
        else std::cout << "\n  Total: " << nr << " rezervari\n";
    }

    ~Cinematograf() {
        for (auto r : istoricRezervari) delete r;
    }
};

// ============================================================
//  UTILITARE UI
// ============================================================
bool contineSubsir(std::string text, std::string cautare) {
    std::transform(text.begin(), text.end(), text.begin(), ::tolower);
    std::transform(cautare.begin(), cautare.end(), cautare.begin(), ::tolower);
    return text.find(cautare) != std::string::npos;
}

void asteptaTasta(const std::string& msg = "\n  Apasa orice tasta...") {
    std::cout << msg << std::flush;
    int ch = 0; citesteInput(ch);
}

int meniuInteractiv(const std::vector<std::string>& opt, const std::string& titlu,
    const std::string& subtext = "") {
    int sel = 0;
    while (true) {
        system(CLEAR);
        std::cout << "=== " << titlu << " ===\n";
        if (!subtext.empty()) std::cout << subtext << "\n";
        std::cout << "\n";
        for (int i = 0;i < (int)opt.size();++i) {
            if (i == sel) std::cout << "  [>] " << opt[i] << "\n";
            else        std::cout << "      " << opt[i] << "\n";
        }
        std::cout << "\n  Sageti Sus/Jos + Enter | ESC = inapoi\n";
        int ch = 0; Tasta t = citesteInput(ch);
        if (t == T_SUS && sel > 0)              --sel;
        if (t == T_JOS && sel < (int)opt.size() - 1) ++sel;
        if (t == T_ENTER) return sel;
        if (t == T_ESC)   return -1;
    }
}

Film* meniuCautareLive(Cinematograf& cinema) {
    std::string input; int idx = 0;
    while (true) {
        system(CLEAR);
        std::cout << "=== CAUTARE FILM ===\n";
        std::cout << "  Scrie: " << input << "_\n";
        std::cout << std::string(52, '-') << "\n";

        std::vector<Film*> rez;
        for (auto& f : cinema.getFilme())
            if (contineSubsir(f.getTitlu(), input))
                rez.push_back(const_cast<Film*>(&f));

        if (idx >= (int)rez.size()) idx = (int)rez.size() - 1;
        if (idx < 0) idx = 0;

        for (int i = 0;i < (int)rez.size() && i < 14;++i) {
            std::string extra = "";
            if (rez[i]->getEste3D()) extra += " [3D]";
            if (rez[i]->getDurata() > 0) extra += " " + std::to_string(rez[i]->getDurata()) + "min";
            if (!rez[i]->getGen().empty()) extra += " | " + rez[i]->getGen();
            if (i == idx) std::cout << "  [>] " << rez[i]->getTitlu() << extra << "\n";
            else        std::cout << "      " << rez[i]->getTitlu() << extra << "\n";
        }
        if ((int)rez.size() > 14)
            std::cout << "  ... si " << rez.size() - 14 << " mai multe\n";
        std::cout << "\n  Enter=selecteaza | ESC=inapoi\n";

        int ch = 0; Tasta t = citesteInput(ch);
        if (t == T_SUS && idx > 0) --idx;
        else if (t == T_JOS && idx < (int)rez.size() - 1) ++idx;
        else if (t == T_ENTER) return rez.empty() ? nullptr : rez[idx];
        else if (t == T_ESC)   return nullptr;
        else if (t == T_BACKSPACE) { if (!input.empty()) input.pop_back(); idx = 0; }
        else if (t == T_ALTUL && isprint(ch)) { input += (char)ch; idx = 0; }
    }
}

// Citeste un string cu echo normal (mode cooked)
std::string citesteString(const std::string& prompt) {
    std::cout << prompt;
#ifndef _WIN32
    // Asiguram ca terminalul e in modul normal inainte de getline
    struct termios oldt;
    tcgetattr(STDIN_FILENO, &oldt);
    struct termios newt = oldt;
    newt.c_lflag |= (ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
#endif
    std::string s;
    std::getline(std::cin, s);
    if (!s.empty() && s.back() == '\r') s.pop_back();
#ifndef _WIN32
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
#endif
    return s;
}

// ============================================================
//  FLUX: LOGIN / INREGISTRARE
// ============================================================
bool menuAutentificare(Cinematograf& cinema) {
    while (true) {
        int al = meniuInteractiv({ "Autentificare","Inregistrare cont nou","Iesire" }, "CINEMA - AUTENTIFICARE");
        if (al == 2 || al == -1) return false;
        system(CLEAR);
        if (al == 0) {
            std::cout << "=== AUTENTIFICARE ===\n\n";
            std::string usr = citesteString("  Username: ");
            std::string par = citesteString("  Parola:   ");
            if (cinema.login(usr, par)) return true;
            std::cout << "\n  [!] Username sau parola incorecta.\n";
            asteptaTasta();
        }
        else {
            std::cout << "=== INREGISTRARE ===\n\n";
            std::string usr = citesteString("  Username nou: ");
            std::string par = citesteString("  Parola:       ");
            std::string par2 = citesteString("  Confirma:     ");
            if (par != par2) {
                std::cout << "\n  [!] Parolele nu coincid.\n"; asteptaTasta(); continue;
            }
            if (cinema.inregistrare(usr, par)) {
                std::cout << "\n  [OK] Cont creat! Bun venit, " << usr << "!\n";
                asteptaTasta(); return true;
            }
            else {
                std::cout << "\n  [!] Username deja existent sau date invalide.\n";
                asteptaTasta();
            }
        }
    }
}

// ============================================================
//  FLUX: REZERVARE NOUA
// ============================================================
void fluxRezervareNoua(Cinematograf& cinema) {
    Film* f = meniuCautareLive(cinema);
    if (!f) return;

    // Alege sala
    std::vector<std::string> optSali;
    for (auto& s : cinema.getSali())
        optSali.push_back(s.getNume() + " (" + std::to_string(s.getRanduri())
            + "x" + std::to_string(s.getColoane()) + ")");
    int idxS = meniuInteractiv(optSali, "ALEGE SALA");
    if (idxS == -1) return;
    int idSala = cinema.getSali()[idxS].getId();

    // Alege ora
    int idxO = meniuInteractiv(
        std::vector<std::string>(cinema.getOre().begin(), cinema.getOre().end()),
        "ALEGE ORA"
    );
    if (idxO == -1) return;
    std::string ora = cinema.getOre()[idxO];

    // Afiseaza harta
    system(CLEAR);
    Sala* sala = cinema.getSalaById(idSala);
    sala->afiseazaHarta(ora);

    std::cout << "  Film: " << f->getTitlu()
        << (f->getEste3D() ? " [3D]" : "") << "\n"
        << "  Pret baza: " << f->getPretBaza()
        << (f->getEste3D() ? " + 10 RON (3D)" : "") << " RON\n"
        << "  (VIP x1.5 | Cuplu x1.8)\n\n";

    // Citeste rand/loc
    std::string srIn = citesteString("  Rand (1-" + std::to_string(sala->getRanduri()) + "): ");
    std::string slIn = citesteString("  Loc  (1-" + std::to_string(sala->getColoane()) + "): ");
    int r = 0, l = 0;
    try { r = std::stoi(srIn); l = std::stoi(slIn); }
    catch (...) {}

    if (r<1 || r>sala->getRanduri() || l<1 || l>sala->getColoane()) {
        std::cout << "\n  [!] Rand sau loc invalid!\n"; asteptaTasta(); return;
    }
    if (sala->esteLocOcupat(ora, r - 1, l - 1)) {
        std::cout << "\n  [!] Loc ocupat! Alege altul.\n"; asteptaTasta(); return;
    }

    // Afiseaza pret final calculat
    CategorieLocSeat cat = sala->getCategorieLocSeat(r - 1, l - 1);
    double pretFinal = (f->getPretBaza() + (f->getEste3D() ? 10.0 : 0.0)) * multiplicatorCategorie(cat);
    std::cout << "\n  Categorie loc: " << numeCategorie(cat)
        << " | Pret final: " << std::fixed << std::setprecision(2) << pretFinal << " RON\n";

    std::string confirm = citesteString("  Confirmi rezervarea? (d/n): ");
    if (confirm != "d" && confirm != "D") { std::cout << "  Anulat.\n"; asteptaTasta(); return; }

    std::string email = citesteString("  Email (optional, Enter skip): ");
    while (!email.empty() && (email.front() == ' ' || email.front() == '\r')) email.erase(email.begin());
    while (!email.empty() && (email.back() == ' ' || email.back() == '\r'))   email.pop_back();

    cinema.proceseazaRezervare(idSala, r - 1, l - 1, f, ora, email);
    std::cout << "  [OK] Rezervare confirmata!\n";
    asteptaTasta();
}

// ============================================================
//  FLUX: MENIU ADMIN
// ============================================================
void meniuAdmin(Cinematograf& cinema) {
    while (true) {
        int al = meniuInteractiv({
            "Vezi toate rezervarile",
            "Adauga film",
            "Sterge film",
            "Lista utilizatori",
            "Sterge utilizator",
            "Inapoi"
            }, "PANOU ADMIN", "Conectat ca: admin");

        if (al == -1 || al == 5) return;

        system(CLEAR);

        // --- TOATE REZERVARILE ---
        if (al == 0) {
            std::cout << "=== TOATE REZERVARILE ===\n\n";
            cinema.afiseazaToateRezervariLog();
            asteptaTasta();
        }

        // --- ADAUGA FILM ---
        else if (al == 1) {
            std::cout << "=== ADAUGA FILM ===\n\n";
            std::string titlu = citesteString("  Titlu: ");
            std::string sgen = citesteString("  Gen (ex: Actiune): ");
            std::string sdur = citesteString("  Durata (minute): ");
            std::string spret = citesteString("  Pret bilet (RON): ");
            std::string s3d = citesteString("  Este 3D? (d/n): ");
            try {
                int dur = std::stoi(sdur);
                double pret = std::stod(spret);
                bool e3d = (s3d == "d" || s3d == "D");
                cinema.adaugaFilmManual(titlu, e3d, pret, sgen, dur);
                std::cout << "\n  [OK] Film adaugat: " << titlu << "\n";
            }
            catch (...) {
                std::cout << "\n  [!] Date invalide.\n";
            }
            asteptaTasta();
        }

        // --- STERGE FILM ---
        else if (al == 2) {
            system(CLEAR);
            std::cout << "=== STERGE FILM ===\n\n";
            auto& filme = cinema.getFilme();
            for (int i = 0;i < (int)filme.size();++i)
                std::cout << "  " << i + 1 << ". " << filme[i].getTitlu()
                << (filme[i].getEste3D() ? " [3D]" : "") << " | "
                << filme[i].getPretBaza() << " RON\n";
            std::string snr = citesteString("\n  Numarul filmului de sters (0=renunta): ");
            int nr = 0;
            try { nr = std::stoi(snr); }
            catch (...) {}
            if (nr > 0 && cinema.stergeFilm(nr - 1))
                std::cout << "  [OK] Film sters.\n";
            else if (nr != 0)
                std::cout << "  [!] Index invalid.\n";
            asteptaTasta();
        }

        // --- LISTA UTILIZATORI ---
        else if (al == 3) {
            std::cout << "=== UTILIZATORI INREGISTRATI ===\n\n";
            for (auto& u : cinema.getUtilizatori())
                std::cout << "  - " << u.username
                << (u.esteAdmin ? " [ADMIN]" : "") << "\n";
            asteptaTasta();
        }

        // --- STERGE UTILIZATOR ---
        else if (al == 4) {
            std::cout << "=== STERGE UTILIZATOR ===\n\n";
            for (auto& u : cinema.getUtilizatori())
                std::cout << "  - " << u.username << (u.esteAdmin ? " [ADMIN]" : "") << "\n";
            std::string usr = citesteString("\n  Username de sters: ");
            if (cinema.stergeUtilizator(usr))
                std::cout << "  [OK] Utilizator sters.\n";
            else
                std::cout << "  [!] Nu s-a putut sterge (inexistent sau admin).\n";
            asteptaTasta();
        }
    }
}

// ============================================================
//  FLUX: PROFIL UTILIZATOR
// ============================================================
void meniuProfil(Cinematograf& cinema) {
    ContUtilizator* u = cinema.getCurent();
    if (!u) return;
    while (true) {
        int al = meniuInteractiv({
            "Istoricul meu de rezervari",
            "Schimba parola",
            "Inapoi"
            }, "PROFIL", "  Utilizator: " + u->username);

        if (al == -1 || al == 2) return;
        system(CLEAR);

        if (al == 0) {
            std::cout << "=== ISTORICUL REZERVARILOR TALE ===\n\n";
            if (u->istoricRezervari.empty())
                std::cout << "  (nicio rezervare in sesiunea curenta)\n";
            else
                for (int i = 0;i < (int)u->istoricRezervari.size();++i)
                    std::cout << "  " << i + 1 << ". " << u->istoricRezervari[i] << "\n";
            asteptaTasta();
        }
        else if (al == 1) {
            std::cout << "=== SCHIMBA PAROLA ===\n\n";
            std::string veche = citesteString("  Parola actuala: ");
            if (veche != u->parola) { std::cout << "  [!] Parola incorecta.\n"; asteptaTasta(); continue; }
            std::string noua = citesteString("  Parola noua:    ");
            std::string conf = citesteString("  Confirma:       ");
            if (noua != conf) { std::cout << "  [!] Nu coincid.\n"; asteptaTasta(); continue; }
            cinema.schimbaParola(u->username, noua);
            std::cout << "  [OK] Parola schimbata.\n";
            asteptaTasta();
        }
    }
}

// ============================================================
//  MAIN
// ============================================================
int main() {
    std::srand((unsigned)time(nullptr));

    Cinematograf cinema;
    cinema.init(); // incarca conturi

    cinema.incarcaFilme("filme.txt");
    cinema.adaugaSala(Sala(1, "SALA 1 STANDARD", 5, 10));
    cinema.adaugaSala(Sala(2, "SALA 2 VIP", 3, 6));
    cinema.incarcaRezervari();

    // Autentificare obligatorie
    if (!menuAutentificare(cinema)) {
        system(CLEAR);
        std::cout << "La revedere!\n";
        return 0;
    }

    while (true) {
        ContUtilizator* u = cinema.getCurent();
        std::string infoUser = u ? ("  Conectat: " + u->username + (u->esteAdmin ? " [ADMIN]" : "")) : "";

        std::vector<std::string> optMain = {
            "Rezervare Noua",
            "Harta Sala",
            "Anulare Rezervare",
            "Profilul Meu"
        };
        if (u && u->esteAdmin) optMain.push_back("Panou Admin");
        optMain.push_back("Deconectare");

        int al = meniuInteractiv(optMain, "CINEMA - MENIU PRINCIPAL", infoUser);

        int nOpt = (int)optMain.size();
        bool eAdmin = (u && u->esteAdmin);

        if (al == -1 || al == nOpt - 1) {
            // Deconectare
            cinema.logout();
            if (!menuAutentificare(cinema)) break;
            continue;
        }

        if (al == 0) {
            // Rezervare noua
            fluxRezervareNoua(cinema);
        }
        else if (al == 1) {
            // Harta sala
            std::vector<std::string> optSali;
            for (auto& s : cinema.getSali())
                optSali.push_back(s.getNume());
            int idxS = meniuInteractiv(optSali, "ALEGE SALA");
            if (idxS == -1) continue;
            int idSala = cinema.getSali()[idxS].getId();

            int idxO = meniuInteractiv(
                std::vector<std::string>(cinema.getOre().begin(), cinema.getOre().end()),
                "ALEGE ORA"
            );
            if (idxO == -1) continue;
            system(CLEAR);
            cinema.getSalaById(idSala)->afiseazaHarta(cinema.getOre()[idxO]);
            asteptaTasta();
        }
        else if (al == 2) {
            // Anulare
            std::vector<std::string> optSali;
            for (auto& s : cinema.getSali()) optSali.push_back(s.getNume());
            int idxS = meniuInteractiv(optSali, "ALEGE SALA - ANULARE");
            if (idxS == -1) continue;
            int idSala = cinema.getSali()[idxS].getId();

            int idxO = meniuInteractiv(
                std::vector<std::string>(cinema.getOre().begin(), cinema.getOre().end()),
                "ALEGE ORA"
            );
            if (idxO == -1) continue;
            std::string ora = cinema.getOre()[idxO];

            system(CLEAR);
            cinema.getSalaById(idSala)->afiseazaHarta(ora);
            std::string sr = citesteString("  Rand: ");
            std::string sl = citesteString("  Loc:  ");
            int r = 0, l = 0;
            try { r = std::stoi(sr); l = std::stoi(sl); }
            catch (...) {}
            cinema.anuleazaRezervare(idSala, ora, r - 1, l - 1);
            asteptaTasta();
        }
        else if (al == 3) {
            // Profil
            meniuProfil(cinema);
        }
        else if (al == 4 && eAdmin) {
            // Admin panel
            meniuAdmin(cinema);
        }
    }

    system(CLEAR);
    std::cout << "La revedere!\n";
    return 0;
}