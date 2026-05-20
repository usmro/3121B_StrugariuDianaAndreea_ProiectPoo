#ifndef CINEMAMODELS_H
#define CINEMAMODELS_H

#include <iostream>
#include <vector>
#include <string>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <map>
#include <algorithm>
#include <ctime>
#include <stdexcept>

// ─── Custom exceptions ────────────────────────────────────────────────────────
class LocOcupatException : public std::runtime_error {
public:
    int rand, loc;
    LocOcupatException(int r, int l)
        : std::runtime_error("Locul R" + std::to_string(r+1) +
                             " L" + std::to_string(l+1) + " este deja ocupat."),
          rand(r), loc(l) {}
};

class IndexInvalidException : public std::out_of_range {
public:
    IndexInvalidException(const std::string& context, int val)
        : std::out_of_range("Index invalid in " + context +
                            ": " + std::to_string(val)) {}
};

class SalaNotFoundException : public std::runtime_error {
public:
    explicit SalaNotFoundException(int id)
        : std::runtime_error("Sala cu ID " + std::to_string(id) + " nu a fost gasita.") {}
};

enum class CategorieLocSeat { STANDARD, VIP, CUPLU };

inline std::string numeCategorie(CategorieLocSeat c) {
    switch(c) {
        case CategorieLocSeat::VIP:   return "VIP";
        case CategorieLocSeat::CUPLU: return "CUPLU";
        default:                      return "STANDARD";
    }
}
inline double multiplicatorCategorie(CategorieLocSeat c) {
    switch(c) {
        case CategorieLocSeat::VIP:   return 1.5;
        case CategorieLocSeat::CUPLU: return 1.8;
        default:                      return 1.0;
    }
}

// ─── Food ────────────────────────────────────────────────────────────────────
struct FoodItem {
    std::string id;
    std::string emoji;
    std::string nume;
    std::string descriere;
    double      pret;
    std::string categorie; // "Popcorn","Bauturi","Snacks","Combos"
};

struct FoodOrder {
    std::map<std::string,int> items; // item id → qty
    double total(const std::vector<FoodItem>& menu) const {
        double t=0;
        for(auto& [id,qty]:items) for(auto& f:menu) if(f.id==id){ t+=f.pret*qty; break; }
        return t;
    }
    bool empty() const { for(auto& [id,qty]:items) if(qty>0) return false; return true; }
    std::string toStr(const std::vector<FoodItem>& menu) const {
        std::ostringstream oss; bool first=true;
        for(auto& [id,qty]:items){ if(qty<=0) continue;
            for(auto& f:menu) if(f.id==id){ if(!first)oss<<", "; oss<<qty<<"x "<<f.nume; first=false; break; } }
        return oss.str();
    }
};

// ─── Data ─────────────────────────────────────────────────────────────────────
struct Data {
    int zi, luna, an;
    std::string toString() const {
        std::ostringstream oss;
        oss << std::setfill('0') << std::setw(2) << zi << "/"
            << std::setw(2) << luna << "/" << an;
        return oss.str();
    }
    bool operator==(const Data& o) const { return zi==o.zi && luna==o.luna && an==o.an; }
    bool operator<(const Data& o) const {
        if (an!=o.an) return an<o.an;
        if (luna!=o.luna) return luna<o.luna;
        return zi<o.zi;
    }
};

inline Data dataAzi() {
    time_t t = time(nullptr);
    tm* lt = localtime(&t);
    return {lt->tm_mday, lt->tm_mon+1, lt->tm_year+1900};
}
inline std::vector<Data> urmatoroarele7Zile() {
    std::vector<Data> zile;
    time_t t = time(nullptr);
    for (int i = 0; i < 7; ++i) {
        time_t ti = t + i*86400;
        tm* lt = localtime(&ti);
        zile.push_back({lt->tm_mday, lt->tm_mon+1, lt->tm_year+1900});
    }
    return zile;
}
inline std::string numeZiSaptamana(const Data& d) {
    tm t{};
    t.tm_mday = d.zi; t.tm_mon = d.luna-1; t.tm_year = d.an-1900;
    mktime(&t);
    const char* zile[] = {"Dum","Lun","Mar","Mie","Joi","Vin","Sam"};
    return zile[t.tm_wday];
}

// ─── Day-based price multiplier ──────────────────────────────────────────────
// Weekend (Sâmbătă/Duminică) = ×1.2, weekday = ×1.0
inline double multiplicatorZi(const Data& d) {
    tm t{};
    t.tm_mday = d.zi; t.tm_mon = d.luna-1; t.tm_year = d.an-1900;
    mktime(&t);
    // 0=Sun, 6=Sat
    return (t.tm_wday == 0 || t.tm_wday == 6) ? 1.2 : 1.0;
}

// ─── Film ─────────────────────────────────────────────────────────────────────
enum class StatusFilm { RULAZA_ACUM, IN_CURAND };
inline std::string numeStatus(StatusFilm s) {
    return s == StatusFilm::RULAZA_ACUM ? "Ruleaza acum" : "In curand";
}

class Film {
private:
    std::string titlu;
    bool        este3D;
    double      pretBaza;
    std::string gen;
    int         durata;
    StatusFilm  status;
    double      rating       = 0.0;
    std::string descriere;
    std::string regizor;
    std::string distributie;
    std::string dataPremiera;
    std::string tara;
    std::string limba        = "EN";
    std::string clasificare  = "AG";
    std::string posterPath;
public:
    Film(std::string t, bool e3d, double p,
         std::string g    = "General", int d = 0,
         StatusFilm  s    = StatusFilm::RULAZA_ACUM,
         double      rat  = 0.0,
         std::string desc = "", std::string reg  = "",
         std::string dist = "", std::string datP = "",
         std::string tar  = "", std::string lim  = "EN",
         std::string clas = "AG", std::string post = "")
        : titlu(t), este3D(e3d), pretBaza(p), gen(g), durata(d), status(s),
          rating(rat), descriere(desc), regizor(reg), distributie(dist),
          dataPremiera(datP), tara(tar), limba(lim), clasificare(clas), posterPath(post) {}

    std::string getTitlu()        const { return titlu; }
    bool        getEste3D()       const { return este3D; }
    double      getPretBaza()     const { return pretBaza; }
    std::string getGen()          const { return gen; }
    int         getDurata()       const { return durata; }
    StatusFilm  getStatus()       const { return status; }
    double      getRating()       const { return rating; }
    std::string getDescriere()    const { return descriere; }
    std::string getRegizor()      const { return regizor; }
    std::string getDistributie()  const { return distributie; }
    std::string getDataPremiera() const { return dataPremiera; }
    std::string getTara()         const { return tara; }
    std::string getLimba()        const { return limba; }
    std::string getClasificare()  const { return clasificare; }
    std::string getPosterPath()   const { return posterPath; }

    void setTitlu(const std::string& t)        { titlu=t; }
    void setEste3D(bool v)                     { este3D=v; }
    void setPretBaza(double p)                 { pretBaza=p; }
    void setGen(const std::string& g)          { gen=g; }
    void setDurata(int d)                      { durata=d; }
    void setStatus(StatusFilm s)               { status=s; }
    void setRating(double r)                   { rating=r; }
    void setDescriere(const std::string& d)    { descriere=d; }
    void setRegizor(const std::string& r)      { regizor=r; }
    void setDistributie(const std::string& d)  { distributie=d; }
    void setDataPremiera(const std::string& d) { dataPremiera=d; }
    void setTara(const std::string& t)         { tara=t; }
    void setLimba(const std::string& l)        { limba=l; }
    void setClasificare(const std::string& c)  { clasificare=c; }
    void setPosterPath(const std::string& p)   { posterPath=p; }
};

// ─── Sala ─────────────────────────────────────────────────────────────────────
class Sala {
private:
    int         id;
    std::string nume;
    int         randuri;
    int         coloane;
    std::map<std::string, std::vector<std::vector<int>>> ocupare;
    std::vector<std::vector<CategorieLocSeat>> categorii;

    std::string cheie(const Data& d, const std::string& ora) const {
        return d.toString() + "|" + ora;
    }
    void initCategorii() {
        categorii.assign(randuri, std::vector<CategorieLocSeat>(coloane, CategorieLocSeat::STANDARD));
        if (randuri > 0)
            for (int j=0;j<coloane;++j)
                categorii[randuri-1][j] = CategorieLocSeat::VIP;
        if (coloane >= 4) {
            int mid = coloane/2;
            categorii[0][mid-1] = CategorieLocSeat::CUPLU;
            categorii[0][mid]   = CategorieLocSeat::CUPLU;
        }
    }
public:
    Sala(int _id, std::string _n, int _r, int _c)
        : id(_id), nume(_n), randuri(_r), coloane(_c) { initCategorii(); }

    int         getId()      const { return id; }
    std::string getNume()    const { return nume; }
    int         getRanduri() const { return randuri; }
    int         getColoane() const { return coloane; }

    CategorieLocSeat getCategorieLocSeat(int r, int l) const {
        if (r>=0&&r<randuri&&l>=0&&l<coloane) return categorii[r][l];
        return CategorieLocSeat::STANDARD;
    }
    void initOra(const Data& d, const std::string& ora) {
        std::string k = cheie(d,ora);
        if (!ocupare.count(k))
            ocupare[k] = std::vector<std::vector<int>>(randuri, std::vector<int>(coloane,0));
    }
    bool esteOcupat(const Data& d, const std::string& ora, int r, int l) {
        initOra(d,ora);
        if (r<0||r>=randuri||l<0||l>=coloane)
            throw IndexInvalidException("Sala::esteOcupat", r*100+l);
        return ocupare[cheie(d,ora)][r][l]==1;
    }
    // throws LocOcupatException if seat already taken, IndexInvalidException if out of bounds
    void ocupaLoc(const Data& d, const std::string& ora, int r, int l) {
        initOra(d,ora);
        if (r<0||r>=randuri||l<0||l>=coloane)
            throw IndexInvalidException("Sala::ocupaLoc", r*100+l);
        if (ocupare[cheie(d,ora)][r][l]==1)
            throw LocOcupatException(r,l);
        ocupare[cheie(d,ora)][r][l]=1;
    }
    void elibereazaLoc(const Data& d, const std::string& ora, int r, int l) {
        std::string k=cheie(d,ora);
        if (r<0||r>=randuri||l<0||l>=coloane)
            throw IndexInvalidException("Sala::elibereazaLoc", r*100+l);
        if (ocupare.count(k)) ocupare[k][r][l]=0;
    }
    int locuriLibere(const Data& d, const std::string& ora) {
        initOra(d,ora);
        int cnt=0;
        auto& mat=ocupare[cheie(d,ora)];
        for (int r=0;r<randuri;++r)
            for (int l=0;l<coloane;++l)
                if (mat[r][l]==0) ++cnt;
        return cnt;
    }
    void afiseazaHarta(const Data& d, const std::string& ora, int selR=-1, int selL=-1) {
        initOra(d,ora);
        auto& mat = ocupare[cheie(d,ora)];
        std::cout << "\n  === " << nume << " | " << d.toString() << " | ORA: " << ora << " ===\n\n";
        std::cout << "       ";
        for (int j=1;j<=coloane;++j) std::cout << std::setw(3) << j;
        std::cout << "\n       " << std::string(coloane*3+2,'-') << "\n";
        for (int i=0;i<randuri;++i) {
            std::cout << "  R" << std::setw(2) << i+1 << " |";
            for (int j=0;j<coloane;++j) {
                if (i==selR && j==selL) { std::cout << " [*"; continue; }
                if (mat[i][j]==1)        std::cout << "  X";
                else switch(categorii[i][j]) {
                    case CategorieLocSeat::VIP:   std::cout << "  V"; break;
                    case CategorieLocSeat::CUPLU: std::cout << "  C"; break;
                    default:                      std::cout << "  O"; break;
                }
            }
            std::cout << " |\n";
        }
        std::cout << "       " << std::string(coloane*3+2,'-') << "\n";
    }
};

// ─── Cinema location ─────────────────────────────────────────────────────────
struct CinemaLocation {
    std::string id;
    std::string nume;
    std::string oras;
    std::string adresa;
    std::string regiune; // "Moldova","Muntenia","Transilvania","Oltenia","Dobrogea","Banat","Crisana"
    std::vector<Sala>        sali;
    std::vector<std::string> oreDisponibile = {"11:00","14:00","17:30","20:45"};
};

// ─── Region map ──────────────────────────────────────────────────────────────
// Maps city names (lowercase) to region. Comparison is ASCII-only (no diacritics).
inline std::string getRegiune(const std::string& oras) {
    std::string o = oras;
    std::transform(o.begin(),o.end(),o.begin(),::tolower);
    auto has = [&](const char* s){ return o.find(s) != std::string::npos; };
    // Moldova
    if(has("suceava")||has("iasi")||has("bacau")||has("piatra")||
       has("neamt")||has("botosani")||has("vaslui")||has("galati")||
       has("vrancea")||has("focsani")||has("roman")||has("pascani"))
        return "Moldova";
    // Transilvania
    if(has("brasov")||has("cluj")||has("sibiu")||has("mures")||
       has("alba")||has("bistrita")||has("gheorghe")||has("miercurea")||
       has("deva")||has("hunedoara")||has("fagaras")||has("odorheiu")||
       has("reghin")||has("sighisoara"))
        return "Transilvania";
    // Muntenia
    if(has("bucuresti")||has("bucharest")||has("ploiesti")||has("pitesti")||
       has("targoviste")||has("buzau")||has("giurgiu")||has("alexandria")||
       has("gaesti")||has("ramnicu valcea"))
        return "Muntenia";
    // Oltenia
    if(has("craiova")||has("ramnicu")||has("drobeta")||has("turnu")||
       has("slatina")||has("targu jiu")||has("caracal")||has("bailesti"))
        return "Oltenia";
    // Dobrogea
    if(has("constanta")||has("tulcea")||has("mangalia")||has("medgidia")||
       has("cernavoda")||has("navodari"))
        return "Dobrogea";
    // Banat
    if(has("timisoara")||has("resita")||has("lugoj")||has("caransebes")||
       has("deta")||has("sannicolau"))
        return "Banat";
    // Crisana
    if(has("oradea")||has("arad")||has("baia mare")||has("satu mare")||
       has("zalau")||has("marghita")||has("beius"))
        return "Crisana";
    return "Muntenia"; // fallback
}

// ─── Rezervare ───────────────────────────────────────────────────────────────
class Rezervare {
protected:
    std::string titluFilm;
    std::string salaNume;
    Data        data;
    std::string ora;
    int         rand, loc;
    double      pretFinal;
    std::string categorieLoc;
    std::string username;
public:
    Rezervare(const std::string& tf, const std::string& sn, const Data& d,
              const std::string& o, int r, int l, double p,
              const std::string& cat, const std::string& usr)
        : titluFilm(tf), salaNume(sn), data(d), ora(o),
          rand(r), loc(l), pretFinal(p), categorieLoc(cat), username(usr) {}
    virtual void afiseaza() const {
        std::cout << "  Film: " << titluFilm << " | " << salaNume
                  << " | " << data.toString() << " " << ora
                  << " | R" << rand+1 << " L" << loc+1
                  << " [" << categorieLoc << "]"
                  << " | " << std::fixed << std::setprecision(2) << pretFinal << " RON\n";
    }
    std::string getTitluFilm()   const { return titluFilm; }
    std::string getSalaNume()    const { return salaNume; }
    Data        getData()        const { return data; }
    std::string getOra()         const { return ora; }
    int         getRand()        const { return rand; }
    int         getLoc()         const { return loc; }
    double      getPretFinal()   const { return pretFinal; }
    std::string getCategorie()   const { return categorieLoc; }
    std::string getUsername()    const { return username; }
    virtual ~Rezervare() {}
};

class RezervareOnline : public Rezervare {
    std::string email;
public:
    RezervareOnline(const std::string& tf, const std::string& sn, const Data& d,
                    const std::string& o, int r, int l, double p,
                    const std::string& cat, const std::string& usr, const std::string& e)
        : Rezervare(tf,sn,d,o,r,l,p,cat,usr), email(e) {}
    void afiseaza() const override {
        std::cout << "  [ONLINE] "; Rezervare::afiseaza();
        std::cout << "           Email: " << email << "\n";
    }
    std::string getEmail() const { return email; }
};

// ─── Cont ────────────────────────────────────────────────────────────────────
struct ContUtilizator {
    std::string username;
    std::string parola;
    bool        esteAdmin;
    std::vector<std::string> istoricRezervari;
};

class ICinemaService {
public:
    virtual void adaugaFilm(Film f) = 0;
    virtual void adaugaSala(Sala s) = 0;
    virtual ~ICinemaService() {}
};

#endif
