#ifndef CINEMA_H
#define CINEMA_H

#include "CinemaModels.h"
#include <sstream>
#include <cstdlib>

class Cinematograf : public ICinemaService {
private:
    std::vector<Film>           filme;
    std::vector<CinemaLocation> locatii;     // all cinema branches
    CinemaLocation*             locatieCurenta = nullptr;
    std::vector<Rezervare*>     rezervari;
    std::vector<ContUtilizator> utilizatori;
    ContUtilizator*             curent = nullptr;
    std::vector<FoodItem>       meniu;

    void salveazaConturi() {
        std::ofstream f("data/conturi.txt", std::ios::trunc);
        for (auto& u : utilizatori)
            f << u.username << "," << u.parola << "," << (u.esteAdmin?1:0) << "\n";
    }
    ContUtilizator* gasesteUser(const std::string& usr) {
        for (auto& u : utilizatori) if (u.username==usr) return &u;
        return nullptr;
    }
    std::string fisierIstoric(const std::string& usr) {
        return "data/istoric_" + usr + ".txt";
    }
    void salveazaIstoricUser(const ContUtilizator& u) {
        std::ofstream f(fisierIstoric(u.username), std::ios::trunc);
        for (auto& r : u.istoricRezervari) f << r << "\n";
    }
    void incarcaIstoricUser(ContUtilizator& u) {
        std::ifstream f(fisierIstoric(u.username));
        if (!f.is_open()) return;
        std::string ln;
        while (std::getline(f, ln)) {
            if (!ln.empty()&&ln.back()=='\r') ln.pop_back();
            if (!ln.empty()) u.istoricRezervari.push_back(ln);
        }
    }
    std::string genCod(int r, int l) {
        static const char ch[]="ABCDEFGHJKLMNPQRSTUVWXYZ23456789";
        unsigned seed=(unsigned)(time(nullptr))^((r+1)*31u)^((l+1)*97u);
        std::srand(seed);
        std::string cod;
        for(int i=0;i<4;++i) cod+=ch[std::rand()%32];
        cod+="-";
        for(int i=0;i<4;++i) cod+=ch[std::rand()%32];
        return cod;
    }

    void initLocatii() {
        // ── MOLDOVA ───────────────────────────────────────────────────────────
        {
            CinemaLocation l; l.id="SUC1"; l.oras="Suceava"; l.regiune="Moldova";
            l.nume="Cinema City Suceava"; l.adresa="Str. Curtea Domneasca 7, Iulius Mall";
            l.sali.push_back(Sala(1,"Sala 1 Standard",5,10));
            l.sali.push_back(Sala(2,"Sala 2 VIP",3,8));
            l.sali.push_back(Sala(3,"Sala 3 IMAX",6,12));
            locatii.push_back(l);
        }
        {
            CinemaLocation l; l.id="SUC2"; l.oras="Suceava"; l.regiune="Moldova";
            l.nume="Arta Cinema Suceava"; l.adresa="Bd. Ana Ipatescu 5";
            l.sali.push_back(Sala(1,"Sala 1",4,9));
            l.sali.push_back(Sala(2,"Sala 2",3,7));
            locatii.push_back(l);
        }
        {
            CinemaLocation l; l.id="IAS1"; l.oras="Iasi"; l.regiune="Moldova";
            l.nume="Cinema City Palas"; l.adresa="Str. Palat 1C, Palas Mall";
            l.sali.push_back(Sala(1,"Sala 1 Standard",5,10));
            l.sali.push_back(Sala(2,"Sala 2 VIP",3,8));
            l.sali.push_back(Sala(3,"Sala 3 IMAX",7,14));
            locatii.push_back(l);
        }
        {
            CinemaLocation l; l.id="IAS2"; l.oras="Iasi"; l.regiune="Moldova";
            l.nume="Arta Cinema Iasi"; l.adresa="Str. Arcu 10";
            l.sali.push_back(Sala(1,"Sala 1",4,9));
            locatii.push_back(l);
        }
        {
            CinemaLocation l; l.id="BAC1"; l.oras="Bacau"; l.regiune="Moldova";
            l.nume="Cinema City Bacau"; l.adresa="Calea Republicii 164, Arena Mall";
            l.sali.push_back(Sala(1,"Sala 1 Standard",5,10));
            l.sali.push_back(Sala(2,"Sala 2 VIP",3,8));
            locatii.push_back(l);
        }
        {
            CinemaLocation l; l.id="BAC2"; l.oras="Bacau"; l.regiune="Moldova";
            l.nume="Millenium Cinema Bacau"; l.adresa="Str. Mihai Viteazu 3";
            l.sali.push_back(Sala(1,"Sala 1",4,8));
            locatii.push_back(l);
        }
        {
            CinemaLocation l; l.id="PN1"; l.oras="Piatra Neamt"; l.regiune="Moldova";
            l.nume="Cinema Dacia"; l.adresa="Bd. Decebal 12";
            l.sali.push_back(Sala(1,"Sala 1",4,8));
            l.sali.push_back(Sala(2,"Sala 2",3,7));
            locatii.push_back(l);
        }
        // ── TRANSILVANIA ──────────────────────────────────────────────────────
        {
            CinemaLocation l; l.id="CLJ1"; l.oras="Cluj-Napoca"; l.regiune="Transilvania";
            l.nume="Cinema City Iulius"; l.adresa="Str. Alexandru Vaida Voevod 53B";
            l.sali.push_back(Sala(1,"Sala 1 Standard",5,10));
            l.sali.push_back(Sala(2,"Sala 2 VIP",3,8));
            l.sali.push_back(Sala(3,"Sala 3 IMAX",8,14));
            locatii.push_back(l);
        }
        {
            CinemaLocation l; l.id="CLJ2"; l.oras="Cluj-Napoca"; l.regiune="Transilvania";
            l.nume="Victoria Cinema Cluj"; l.adresa="Calea Dorobantilor 2";
            l.sali.push_back(Sala(1,"Sala 1",5,9));
            l.sali.push_back(Sala(2,"Sala 2",4,8));
            locatii.push_back(l);
        }
        {
            CinemaLocation l; l.id="BV1"; l.oras="Brasov"; l.regiune="Transilvania";
            l.nume="Cinema City Coresi"; l.adresa="Calea Feldioarei 23, Coresi Mall";
            l.sali.push_back(Sala(1,"Sala 1 Standard",5,10));
            l.sali.push_back(Sala(2,"Sala 2 VIP",3,8));
            locatii.push_back(l);
        }
        {
            CinemaLocation l; l.id="BV2"; l.oras="Brasov"; l.regiune="Transilvania";
            l.nume="Redison Cinema Brasov"; l.adresa="Str. Republicii 62";
            l.sali.push_back(Sala(1,"Sala 1",4,8));
            locatii.push_back(l);
        }
        {
            CinemaLocation l; l.id="SB1"; l.oras="Sibiu"; l.regiune="Transilvania";
            l.nume="Cinema City Sibiu"; l.adresa="Str. Doamna Stanca 1, Shopping City";
            l.sali.push_back(Sala(1,"Sala 1 Standard",5,10));
            l.sali.push_back(Sala(2,"Sala 2 VIP",3,8));
            locatii.push_back(l);
        }
        {
            CinemaLocation l; l.id="SB2"; l.oras="Sibiu"; l.regiune="Transilvania";
            l.nume="Cinema Habitus Sibiu"; l.adresa="Str. Iosif Vulcan 2";
            l.sali.push_back(Sala(1,"Sala 1",4,8));
            locatii.push_back(l);
        }
        // ── MUNTENIA ──────────────────────────────────────────────────────────
        {
            CinemaLocation l; l.id="BUC1"; l.oras="Bucuresti"; l.regiune="Muntenia";
            l.nume="Cinema City Baneasa"; l.adresa="Bd. Bucuresti-Ploiesti 42D";
            l.sali.push_back(Sala(1,"Sala 1 Standard",6,12));
            l.sali.push_back(Sala(2,"Sala 2 VIP",4,8));
            l.sali.push_back(Sala(3,"Sala 3 IMAX",8,16));
            locatii.push_back(l);
        }
        {
            CinemaLocation l; l.id="BUC2"; l.oras="Bucuresti"; l.regiune="Muntenia";
            l.nume="Cinema Pro Bucuresti"; l.adresa="Str. Ion Brezoianu 23";
            l.sali.push_back(Sala(1,"Sala 1",5,10));
            l.sali.push_back(Sala(2,"Sala 2",4,8));
            locatii.push_back(l);
        }
        {
            CinemaLocation l; l.id="BUC3"; l.oras="Bucuresti"; l.regiune="Muntenia";
            l.nume="Grand Cinema Digiplex"; l.adresa="Calea Victoriei 155";
            l.sali.push_back(Sala(1,"Sala 1 Standard",5,10));
            l.sali.push_back(Sala(2,"Sala 2 VIP",3,8));
            locatii.push_back(l);
        }
        {
            CinemaLocation l; l.id="PLO1"; l.oras="Ploiesti"; l.regiune="Muntenia";
            l.nume="Cinema City Ploiesti"; l.adresa="Bd. Republicii 1, Winmarkt";
            l.sali.push_back(Sala(1,"Sala 1 Standard",5,10));
            l.sali.push_back(Sala(2,"Sala 2 VIP",3,8));
            locatii.push_back(l);
        }
        // ── OLTENIA ───────────────────────────────────────────────────────────
        {
            CinemaLocation l; l.id="CRV1"; l.oras="Craiova"; l.regiune="Oltenia";
            l.nume="Cinema City Craiova"; l.adresa="Calea Severinului 10";
            l.sali.push_back(Sala(1,"Sala 1 Standard",5,10));
            l.sali.push_back(Sala(2,"Sala 2 VIP",3,8));
            locatii.push_back(l);
        }
        {
            CinemaLocation l; l.id="CRV2"; l.oras="Craiova"; l.regiune="Oltenia";
            l.nume="Grand Cinema Craiova"; l.adresa="Str. A.I. Cuza 8";
            l.sali.push_back(Sala(1,"Sala 1",4,9));
            locatii.push_back(l);
        }
        // ── DOBROGEA ──────────────────────────────────────────────────────────
        {
            CinemaLocation l; l.id="CT1"; l.oras="Constanta"; l.regiune="Dobrogea";
            l.nume="Cinema City Constanta"; l.adresa="Bd. Alexandru Lapusneanu 116C";
            l.sali.push_back(Sala(1,"Sala 1 Standard",5,10));
            l.sali.push_back(Sala(2,"Sala 2 VIP",3,8));
            locatii.push_back(l);
        }
        {
            CinemaLocation l; l.id="CT2"; l.oras="Constanta"; l.regiune="Dobrogea";
            l.nume="Everything Cinema"; l.adresa="Str. Sulmona 1";
            l.sali.push_back(Sala(1,"Sala 1",4,8));
            locatii.push_back(l);
        }
        // ── BANAT ─────────────────────────────────────────────────────────────
        {
            CinemaLocation l; l.id="TM1"; l.oras="Timisoara"; l.regiune="Banat";
            l.nume="Cinema City Iulius Timisoara"; l.adresa="Str. Aristide Demetriade 1";
            l.sali.push_back(Sala(1,"Sala 1 Standard",5,10));
            l.sali.push_back(Sala(2,"Sala 2 VIP",3,8));
            l.sali.push_back(Sala(3,"Sala 3 IMAX",7,14));
            locatii.push_back(l);
        }
        {
            CinemaLocation l; l.id="TM2"; l.oras="Timisoara"; l.regiune="Banat";
            l.nume="Studio Cinema Timisoara"; l.adresa="Str. Marasesti 4";
            l.sali.push_back(Sala(1,"Sala 1",4,8));
            locatii.push_back(l);
        }
        // ── CRISANA ───────────────────────────────────────────────────────────
        {
            CinemaLocation l; l.id="OR1"; l.oras="Oradea"; l.regiune="Crisana";
            l.nume="Cinema City Oradea"; l.adresa="Calea Aradului 70, Lotus Center";
            l.sali.push_back(Sala(1,"Sala 1 Standard",5,10));
            l.sali.push_back(Sala(2,"Sala 2 VIP",3,8));
            locatii.push_back(l);
        }
        {
            CinemaLocation l; l.id="AR1"; l.oras="Arad"; l.regiune="Crisana";
            l.nume="Cinema City Arad"; l.adresa="Calea Aurel Vlaicu 10, Atrium Mall";
            l.sali.push_back(Sala(1,"Sala 1 Standard",5,10));
            l.sali.push_back(Sala(2,"Sala 2 VIP",3,8));
            locatii.push_back(l);
        }
    }

    void initMeniu() {
        meniu = {
            // Popcorn
            {"POP_S_SAR","🍿","Popcorn Mic Sărat","Popcorn proaspăt, porție mică",12.0,"Popcorn"},
            {"POP_M_SAR","🍿","Popcorn Mare Sărat","Popcorn proaspăt, porție mare",18.0,"Popcorn"},
            {"POP_S_DUL","🍬","Popcorn Mic Dulce","Caramel cremos, porție mică",13.0,"Popcorn"},
            {"POP_M_DUL","🍬","Popcorn Mare Dulce","Caramel cremos, porție mare",19.0,"Popcorn"},
            // Băuturi
            {"COLA_S","🥤","Coca-Cola 0.5L","Rece și carbogazoasă",10.0,"Bauturi"},
            {"COLA_L","🥤","Coca-Cola 1L","Rece și carbogazoasă, porție mare",14.0,"Bauturi"},
            {"SPRITE","🥤","Sprite 0.5L","Răcoritoare cu lămâie",10.0,"Bauturi"},
            {"FANTA","🥤","Fanta 0.5L","Aromă de portocale",10.0,"Bauturi"},
            {"APA","💧","Apă plată 500ml","Apă minerală naturală",7.0,"Bauturi"},
            {"ICE_TEA","🍵","Ice Tea 0.5L","Ceai rece cu piersici",10.0,"Bauturi"},
            // Snacks
            {"NACHOS","🌮","Nachos cu sos","Nachos crocante cu sos cheddar",16.0,"Snacks"},
            {"HOT_DOG","🌭","Hot Dog clasic","Cârnat în chiflă cu muștar și ketchup",18.0,"Snacks"},
            {"CHOCO","🍫","Ciocolată Milka","Tabletă 100g",9.0,"Snacks"},
            {"GUMMY","🍬","Gummy Bears","Jeleuri asortate 150g",8.0,"Snacks"},
            // Combos
            {"COMBO1","🎁","Combo Clasic","Popcorn mare + Coca-Cola 0.5L",25.0,"Combos"},
            {"COMBO2","🎁","Combo Duo","2x Popcorn mic + 2x Coca-Cola 0.5L",42.0,"Combos"},
            {"COMBO3","🎁","Combo Premium","Popcorn mare + Nachos + 2x Coca-Cola",48.0,"Combos"},
        };
    }

public:
    void adaugaFilm(Film f) override { filme.push_back(f); }
    void adaugaSala(Sala s) override {
        if(locatieCurenta) locatieCurenta->sali.push_back(s);
    }

    std::vector<Film>&              getFilme()          { return filme; }
    const std::vector<Film>&        getFilmeConst()     const { return filme; }
    std::vector<CinemaLocation>&    getLocatii()        { return locatii; }
    CinemaLocation*                 getLocatieCurenta() { return locatieCurenta; }
    const std::vector<FoodItem>&    getMeniu()          const { return meniu; }
    ContUtilizator*                 getCurent()         { return curent; }
    const std::vector<ContUtilizator>& getUtilizatori() const { return utilizatori; }

    std::vector<Sala>* getSaliCurente() {
        return locatieCurenta ? &locatieCurenta->sali : nullptr;
    }
    const std::vector<std::string>* getOreCurente() const {
        return locatieCurenta ? &locatieCurenta->oreDisponibile : nullptr;
    }

    void setLocatieCurenta(const std::string& id) {
        for(auto& l:locatii) if(l.id==id){ locatieCurenta=&l; return; }
    }

    // throws SalaNotFoundException if id not found in current location
    Sala* getSalaById(int id) {
        if(!locatieCurenta) throw SalaNotFoundException(id);
        for (auto& s : locatieCurenta->sali) if (s.getId()==id) return &s;
        throw SalaNotFoundException(id);
    }
    // safe version for UI use (returns nullptr instead of throwing)
    Sala* getSalaByIdSafe(int id) noexcept {
        if(!locatieCurenta) return nullptr;
        for (auto& s : locatieCurenta->sali) if (s.getId()==id) return &s;
        return nullptr;
    }

    // Get locations for a detected city + nearby cities in same region
    std::vector<CinemaLocation*> getLocatiiPentruOras(const std::string& orasDetectat) {
        std::string regiune = getRegiune(orasDetectat);
        std::string orasLower = orasDetectat;
        std::transform(orasLower.begin(),orasLower.end(),orasLower.begin(),::tolower);

        // Collect cities in region
        std::vector<std::string> oraseInRegiune;
        std::map<std::string,std::vector<CinemaLocation*>> peOras;
        for(auto& l:locatii){
            if(l.regiune!=regiune) continue;
            std::string ol=l.oras;
            std::transform(ol.begin(),ol.end(),ol.begin(),::tolower);
            if(std::find(oraseInRegiune.begin(),oraseInRegiune.end(),l.oras)==oraseInRegiune.end())
                oraseInRegiune.push_back(l.oras);
            peOras[l.oras].push_back(&l);
        }

        // Put detected city first
        std::vector<std::string> ordine;
        for(auto& o:oraseInRegiune){
            std::string ol=o;
            std::transform(ol.begin(),ol.end(),ol.begin(),::tolower);
            if(ol.find(orasLower)!=std::string::npos||orasLower.find(ol)!=std::string::npos)
                ordine.insert(ordine.begin(),o);
            else ordine.push_back(o);
        }
        // Cap at 4 cities (detected + 3 nearby)
        if(ordine.size()>4) ordine.resize(4);

        std::vector<CinemaLocation*> result;
        for(auto& o:ordine) for(auto* p:peOras[o]) result.push_back(p);
        return result;
    }

    // ── File I/O ──────────────────────────────────────────────────────────────
    void incarcaFilme(const std::string& fisier) {
        std::ifstream f(fisier);
        if (!f.is_open()) return;
        std::string ln;
        while (std::getline(f,ln)) {
            if (!ln.empty()&&ln.back()=='\r') ln.pop_back();
            if (ln.empty()) continue;
            if (ln.find('|') != std::string::npos) {
                std::vector<std::string> p;
                std::stringstream ss(ln); std::string tok;
                while (std::getline(ss,tok,'|')) p.push_back(tok);
                if (p.size()<6) continue;
                auto gf=[&](int i)->std::string{return i<(int)p.size()?p[i]:"";};
                try {
                    StatusFilm st=gf(5)=="1"?StatusFilm::IN_CURAND:StatusFilm::RULAZA_ACUM;
                    double rat=0.0; try{rat=std::stod(gf(6));}catch(...){}
                    filme.push_back(Film(gf(0),gf(1)=="1",std::stod(gf(2)),
                        gf(3),std::stoi(gf(4)),st,rat,gf(7),gf(8),gf(9),gf(10),gf(11),gf(12),gf(13),gf(14)));
                } catch(...) {}
            } else {
                std::stringstream ss(ln);
                std::string titlu,s3d,spret,g="General",sd="0",ss2="0";
                if (!std::getline(ss,titlu,',')) continue;
                if (!std::getline(ss,s3d,','))   continue;
                if (!std::getline(ss,spret,','))  continue;
                std::getline(ss,g,','); std::getline(ss,sd,','); std::getline(ss,ss2,',');
                try {
                    StatusFilm st=ss2=="1"?StatusFilm::IN_CURAND:StatusFilm::RULAZA_ACUM;
                    filme.push_back(Film(titlu,s3d=="1",std::stod(spret),g,std::stoi(sd),st));
                } catch(...) {}
            }
        }
    }

    void salveazaFilme() {
        std::ofstream f("data/filme.txt", std::ios::trunc);
        for (auto& fm : filme)
            f << fm.getTitlu()       << "|" << (fm.getEste3D()?1:0)    << "|"
              << std::fixed << std::setprecision(2) << fm.getPretBaza() << "|"
              << fm.getGen()         << "|" << fm.getDurata()           << "|"
              << (fm.getStatus()==StatusFilm::IN_CURAND?1:0)            << "|"
              << std::fixed << std::setprecision(1) << fm.getRating()   << "|"
              << fm.getDescriere()   << "|" << fm.getRegizor()          << "|"
              << fm.getDistributie() << "|" << fm.getDataPremiera()     << "|"
              << fm.getTara()        << "|" << fm.getLimba()            << "|"
              << fm.getClasificare() << "|" << fm.getPosterPath()       << "\n";
    }

    void incarcaRezervari() {
        if(!locatieCurenta) return;
        std::ifstream f("data/toate_rezervarile.txt");
        if (!f.is_open()) return;
        std::string ln;
        while (std::getline(f,ln)) {
            if (!ln.empty()&&ln.back()=='\r') ln.pop_back();
            if (ln.find("Sala: ")==std::string::npos) continue;
            if (ln.find("Ora: ")==std::string::npos)  continue;
            if (ln.find("Data: ")==std::string::npos) continue;
            auto extrage = [&](const std::string& tag, const std::string& end) -> std::string {
                size_t p=ln.find(tag); if (p==std::string::npos) return "";
                p+=tag.size(); size_t e=ln.find(end,p);
                return (e==std::string::npos) ? ln.substr(p) : ln.substr(p,e-p);
            };
            std::string salaNume = extrage("Sala: "," | ");
            std::string dataStr  = extrage("Data: "," | ");
            std::string oraStr   = extrage("Ora: "," | ");
            Data d{1,1,2025};
            if (dataStr.size()>=10) {
                try { d.zi=std::stoi(dataStr.substr(0,2)); d.luna=std::stoi(dataStr.substr(3,2)); d.an=std::stoi(dataStr.substr(6,4)); } catch(...) {}
            }
            size_t pLoc=ln.find("Loc: R"); if (pLoc==std::string::npos) continue;
            int r=0,l=0;
            if (sscanf(ln.c_str()+pLoc,"Loc: R%d L%d",&r,&l)==2)
                for (auto& s : locatieCurenta->sali)
                    if (s.getNume()==salaNume)
                        s.ocupaLoc(d,oraStr,r-1,l-1);
        }
    }

    void initConturi() {
        std::ifstream f("data/conturi.txt");
        if (!f.is_open()) {
            utilizatori.push_back({"admin","admin123",true,{}});
            salveazaConturi(); return;
        }
        std::string ln;
        while (std::getline(f,ln)) {
            if (!ln.empty()&&ln.back()=='\r') ln.pop_back();
            std::stringstream ss(ln);
            std::string usr,par,adm;
            if (std::getline(ss,usr,',')&&std::getline(ss,par,',')&&std::getline(ss,adm,','))
                utilizatori.push_back({usr,par,adm=="1",{}});
        }
        if (utilizatori.empty()) {
            utilizatori.push_back({"admin","admin123",true,{}});
            salveazaConturi();
        }
    }

    void init() {
        initLocatii();
        initMeniu();
        initConturi();
        incarcaFilme("data/filme.txt");
    }

    // ── Auth ──────────────────────────────────────────────────────────────────
    bool login(const std::string& usr, const std::string& par) {
        ContUtilizator* u=gasesteUser(usr);
        if (!u||u->parola!=par) return false;
        curent=u; incarcaIstoricUser(*curent); return true;
    }
    void logout() { curent=nullptr; locatieCurenta=nullptr; }
    bool inregistrare(const std::string& usr, const std::string& par) {
        if (gasesteUser(usr)) return false;
        utilizatori.push_back({usr,par,false,{}});
        salveazaConturi(); curent=&utilizatori.back(); return true;
    }

    // ── Film admin ────────────────────────────────────────────────────────────
    void stergeFilm(int idx) {
        if(idx>=0&&idx<(int)filme.size()){ filme.erase(filme.begin()+idx); salveazaFilme(); }
    }
    void adaugaFilmManual(const std::string& titlu,bool e3d,double pret,
                          const std::string& gen,int dur,StatusFilm st,
                          double rat=0.0,const std::string& desc="",const std::string& reg="",
                          const std::string& dist="",const std::string& datP="",
                          const std::string& tar="",const std::string& lim="EN",
                          const std::string& clas="AG",const std::string& post="") {
        filme.push_back(Film(titlu,e3d,pret,gen,dur,st,rat,desc,reg,dist,datP,tar,lim,clas,post));
        salveazaFilme();
    }
    bool editeazaFilm(int idx,const std::string& titlu,bool e3d,double pret,
                      const std::string& gen,int dur,StatusFilm st,
                      double rat=0.0,const std::string& desc="",const std::string& reg="",
                      const std::string& dist="",const std::string& datP="",
                      const std::string& tar="",const std::string& lim="EN",
                      const std::string& clas="AG",const std::string& post="") {
        if(idx<0||idx>=(int)filme.size()) return false;
        filme[idx]=Film(titlu,e3d,pret,gen,dur,st,rat,desc,reg,dist,datP,tar,lim,clas,post);
        salveazaFilme(); return true;
    }

    // ── Ore ───────────────────────────────────────────────────────────────────
    void adaugaOra(const std::string& ora) {
        if(!locatieCurenta) return;
        auto& ore=locatieCurenta->oreDisponibile;
        if(std::find(ore.begin(),ore.end(),ora)==ore.end()) ore.push_back(ora);
    }
    void stergeOra(int idx) {
        if(!locatieCurenta) return;
        auto& ore=locatieCurenta->oreDisponibile;
        if(idx>=0&&idx<(int)ore.size()) ore.erase(ore.begin()+idx);
    }

    // ── Users ─────────────────────────────────────────────────────────────────
    std::vector<ContUtilizator>& getUtilizatori() { return utilizatori; }
    bool stergeUtilizator(const std::string& usr) {
        if(usr=="admin") return false;
        for(auto it=utilizatori.begin();it!=utilizatori.end();++it)
            if(it->username==usr){ utilizatori.erase(it); salveazaConturi(); return true; }
        return false;
    }
    void schimbaParola(const std::string& usr,const std::string& p) {
        ContUtilizator* u=gasesteUser(usr);
        if(u){ u->parola=p; salveazaConturi(); }
    }

    // ── Booking ───────────────────────────────────────────────────────────────
    void proceseazaRezervare(int idSala,const Data& data,
                             const std::vector<std::pair<int,int>>& locuri,
                             Film* film,const std::string& ora,
                             const std::string& email,
                             const FoodOrder& food) {
        Sala* s=getSalaById(idSala); // throws SalaNotFoundException if missing
        double foodTotal=food.total(meniu);
        std::string foodStr=food.toStr(meniu);
        double ziMult=multiplicatorZi(data); // weekend: x1.2, weekday: x1.0

        for(auto& [r,l]:locuri){
            // throws LocOcupatException or IndexInvalidException
            s->ocupaLoc(data,ora,r,l);
            CategorieLocSeat cat=s->getCategorieLocSeat(r,l);
            // price = (base + 3D surcharge) x seat category x day of week
            double pret=(film->getPretBaza()+(film->getEste3D()?10.0:0.0))
                        *multiplicatorCategorie(cat)*ziMult;
            // food cost split evenly
            double pretTotal=pret+(locuri.size()>0?foodTotal/(int)locuri.size():0);
            std::string catStr=numeCategorie(cat);
            std::string usr=curent?curent->username:"Offline";
            std::string locatieName=locatieCurenta?locatieCurenta->nume:"";

            Rezervare* rez;
            if(!email.empty())
                rez=new RezervareOnline(film->getTitlu(),s->getNume(),data,ora,r,l,pretTotal,catStr,usr,email);
            else
                rez=new Rezervare(film->getTitlu(),s->getNume(),data,ora,r,l,pretTotal,catStr,usr);
            rezervari.push_back(rez);

            if(curent){
                std::ostringstream oss;
                oss<<film->getTitlu()<<" | "<<locatieName<<" | "<<s->getNume()
                   <<" | "<<data.toString()<<" "<<ora
                   <<" | R"<<r+1<<" L"<<l+1<<" ["<<catStr<<"]"
                   <<" | "<<std::fixed<<std::setprecision(2)<<pret<<" RON";
                if(ziMult>1.0) oss<<" [Weekend +20%]";
                if(!foodStr.empty()) oss<<" + "<<foodStr;
                curent->istoricRezervari.push_back(oss.str());
                salveazaIstoricUser(*curent);
            }

            std::ofstream f("data/toate_rezervarile.txt",std::ios::app);
            f<<"Film: "<<film->getTitlu()
             <<" | Locatie: "<<locatieName
             <<" | Sala: "<<s->getNume()
             <<" | Data: "<<data.toString()
             <<" | Ora: "<<ora
             <<" | Loc: R"<<r+1<<" L"<<l+1
             <<" ["<<catStr<<"]"
             <<" | Pret: "<<std::fixed<<std::setprecision(2)<<pret<<" RON"
             <<" | Client: "<<usr
             <<(ziMult>1.0?" [Weekend +20%]":"")
             <<(!email.empty()?" | Email: "+email:"")
             <<(!foodStr.empty()?" | Food: "+foodStr:"")
             <<"\n";

            genereazaBilet(film->getTitlu(),locatieName,s->getNume(),
                           data,ora,r+1,l+1,catStr,pret,foodStr,foodTotal,ziMult>1.0);
        }
    }

    void genereazaBilet(const std::string& fn,const std::string& loc,const std::string& sn,
                        const Data& d,const std::string& ora,
                        int r,int l,const std::string& cat,double pret,
                        const std::string& food,double foodPret,bool weekend=false) {
        std::string dataTag=std::string(1,d.toString()[0])+d.toString()[1]
                           +d.toString()[3]+d.toString()[4];
        std::string numeFisier="Bilet_"+dataTag+"_R"+std::to_string(r)+"L"+std::to_string(l)
                              +"_"+std::to_string(time(nullptr))+".txt";
        std::string fullPath="data/bilete/"+numeFisier;
        #ifdef _WIN32
        std::system("if not exist \"data\\bilete\" mkdir data\\bilete >nul 2>&1");
        #else
        std::system("mkdir -p data/bilete");
        #endif
        std::ofstream bf(fullPath);
        bf<<"==========================================\n"
          <<"          BILET CINEMATOGRAF              \n"
          <<"==========================================\n"
          <<" FILM     : "<<fn<<"\n"
          <<" LOCATIE  : "<<loc<<"\n"
          <<" SALA     : "<<sn<<"\n"
          <<" DATA     : "<<d.toString()<<"\n"
          <<" ORA      : "<<ora<<"\n"
          <<" LOC      : Rand "<<r<<", Loc "<<l<<" ["<<cat<<"]\n"
          <<" BILET    : "<<std::fixed<<std::setprecision(2)<<pret<<" RON"
          <<(weekend?" [Weekend +20%]":"")<<"\n";
        if(!food.empty())
            bf<<" FOOD     : "<<food<<" ("<<std::fixed<<std::setprecision(2)<<foodPret<<" RON)\n"
              <<" TOTAL    : "<<std::fixed<<std::setprecision(2)<<pret+foodPret<<" RON\n";
        bf<<"------------------------------------------\n"
          <<" COD      : "<<genCod(r,l)<<"\n"
          <<"==========================================\n";
        bf.close();
    }

    void anuleazaRezervare(int idSala,const Data& data,const std::string& ora,int r,int l) {
        Sala* s=getSalaById(idSala); if(!s) return;
        s->elibereazaLoc(data,ora,r,l);
        std::ifstream fIn("data/toate_rezervarile.txt");
        std::vector<std::string> linii; std::string ln;
        std::string cheie="Sala: "+s->getNume()+" | Data: "+data.toString()
                         +" | Ora: "+ora+" | Loc: R"+std::to_string(r+1)+" L"+std::to_string(l+1);
        bool gasit=false;
        while(std::getline(fIn,ln)){
            if(!ln.empty()&&ln.back()=='\r') ln.pop_back();
            if(!gasit&&ln.find(cheie)!=std::string::npos) gasit=true;
            else linii.push_back(ln);
        }
        fIn.close();
        std::ofstream fOut("data/toate_rezervarile.txt",std::ios::trunc);
        for(auto& lne:linii) fOut<<lne<<"\n";
    }

    ~Cinematograf() { for(auto r:rezervari) delete r; }
};

#endif
