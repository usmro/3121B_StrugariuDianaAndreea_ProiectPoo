#ifndef CINEMA_H
#define CINEMA_H

#include "CinemaModels.h"
#include <sstream>
#include <cstdlib>

class Cinematograf : public ICinemaService {
private:
    std::vector<Film>           filme;
    std::vector<Sala>           sali;
    std::vector<Rezervare*>     rezervari;
    std::vector<ContUtilizator> utilizatori;
    ContUtilizator*             curent = nullptr;
    std::vector<std::string>    oreDisponibile = {"11:00","14:00","17:30","20:45"};

    
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

public:
    void adaugaFilm(Film f) override { filme.push_back(f); }
    void adaugaSala(Sala s) override { sali.push_back(s); }

    std::vector<Film>&              getFilme()      { return filme; }
    const std::vector<Film>&        getFilmeConst() const { return filme; }
    std::vector<Sala>&              getSali()       { return sali; }
    const std::vector<std::string>& getOre()        const { return oreDisponibile; }
    ContUtilizator*                 getCurent()     { return curent; }
    const std::vector<ContUtilizator>& getUtilizatori() const { return utilizatori; }

    Sala* getSalaById(int id) {
        for (auto& s : sali) if (s.getId()==id) return &s;
        return nullptr;
    }

    
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
                        gf(3),std::stoi(gf(4)),st,
                        rat,gf(7),gf(8),gf(9),gf(10),gf(11),gf(12),gf(13),gf(14)));
                } catch(...) {}
            } else {
                std::stringstream ss(ln);
                std::string titlu,s3d,spret,g="General",sd="0",ss2="0";
                if (!std::getline(ss,titlu,',')) continue;
                if (!std::getline(ss,s3d,  ',')) continue;
                if (!std::getline(ss,spret,',')) continue;
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
        std::ifstream f("data/toate_rezervarile.txt");
        if (!f.is_open()) return;
        std::string ln;
        while (std::getline(f,ln)) {
            if (!ln.empty()&&ln.back()=='\r') ln.pop_back();
            if (ln.find("Sala: ")==std::string::npos) continue;
            if (ln.find("Ora: ")==std::string::npos)  continue;
            if (ln.find("Data: ")==std::string::npos) continue;

            auto extrage = [&](const std::string& tag, const std::string& end) -> std::string {
                size_t p=ln.find(tag);
                if (p==std::string::npos) return "";
                p+=tag.size();
                size_t e=ln.find(end,p);
                return (e==std::string::npos) ? ln.substr(p) : ln.substr(p,e-p);
            };

            std::string salaNume = extrage("Sala: "," | ");
            std::string dataStr  = extrage("Data: "," | ");
            std::string oraStr   = extrage("Ora: "," | ");

            
            Data d{1,1,2025};
            if (dataStr.size()>=10) {
                try {
                    d.zi   = std::stoi(dataStr.substr(0,2));
                    d.luna = std::stoi(dataStr.substr(3,2));
                    d.an   = std::stoi(dataStr.substr(6,4));
                } catch(...) {}
            }

            size_t pLoc=ln.find("Loc: R");
            if (pLoc==std::string::npos) continue;
            int r=0,l=0;
            if (sscanf(ln.c_str()+pLoc,"Loc: R%d L%d",&r,&l)==2)
                for (auto& s : sali)
                    if (s.getNume()==salaNume)
                        s.ocupaLoc(d,oraStr,r-1,l-1);
        }
    }

    
    void initConturi() {
        std::ifstream f("data/conturi.txt");
        if (!f.is_open()) {
            utilizatori.push_back({"admin","admin123",true,{}});
            salveazaConturi();
            return;
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

    bool login(const std::string& usr, const std::string& par) {
        ContUtilizator* u=gasesteUser(usr);
        if (u&&u->parola==par) {
            curent=u;
            incarcaIstoricUser(*u);
            return true;
        }
        return false;
    }

    bool inregistrare(const std::string& usr, const std::string& par) {
        if (usr.empty()||par.empty()||gasesteUser(usr)) return false;
        utilizatori.push_back({usr,par,false,{}});
        salveazaConturi();
        curent=&utilizatori.back();
        return true;
    }

    void logout() { curent=nullptr; }

    
    void adaugaFilmManual(const std::string& titlu, bool e3d, double pret,
                          const std::string& gen, int dur, StatusFilm st,
                          double rat=0.0,
                          const std::string& desc="",  const std::string& reg="",
                          const std::string& dist="",  const std::string& datP="",
                          const std::string& tar="",   const std::string& lim="EN",
                          const std::string& clas="AG",const std::string& post="") {
        filme.push_back(Film(titlu,e3d,pret,gen,dur,st,rat,desc,reg,dist,datP,tar,lim,clas,post));
        salveazaFilme();
    }

    bool stergeFilm(int idx) {
        if (idx<0||idx>=(int)filme.size()) return false;
        filme.erase(filme.begin()+idx);
        salveazaFilme();
        return true;
    }

    bool editeazaFilm(int idx, const std::string& titlu, bool e3d, double pret,
                      const std::string& gen, int dur, StatusFilm st,
                      double rat=0.0,
                      const std::string& desc="",  const std::string& reg="",
                      const std::string& dist="",  const std::string& datP="",
                      const std::string& tar="",   const std::string& lim="EN",
                      const std::string& clas="AG",const std::string& post="") {
        if (idx<0||idx>=(int)filme.size()) return false;
        filme[idx]=Film(titlu,e3d,pret,gen,dur,st,rat,desc,reg,dist,datP,tar,lim,clas,post);
        salveazaFilme();
        return true;
    }

    
    void adaugaOra(const std::string& ora) {
        if (std::find(oreDisponibile.begin(),oreDisponibile.end(),ora)==oreDisponibile.end())
            oreDisponibile.push_back(ora);
    }
    void stergeOra(int idx) {
        if (idx>=0&&idx<(int)oreDisponibile.size())
            oreDisponibile.erase(oreDisponibile.begin()+idx);
    }

    
    bool stergeUtilizator(const std::string& usr) {
        if (usr=="admin") return false;
        for (auto it=utilizatori.begin();it!=utilizatori.end();++it)
            if (it->username==usr) { utilizatori.erase(it); salveazaConturi(); return true; }
        return false;
    }
    void schimbaParola(const std::string& usr, const std::string& p) {
        ContUtilizator* u=gasesteUser(usr);
        if (u) { u->parola=p; salveazaConturi(); }
    }

    
    void proceseazaRezervare(int idSala, const Data& data, int r, int l,
                              Film* film, const std::string& ora,
                              const std::string& email) {
        Sala* s=getSalaById(idSala);
        if (!s) return;
        s->ocupaLoc(data,ora,r,l);

        CategorieLocSeat cat=s->getCategorieLocSeat(r,l);
        double pret=(film->getPretBaza()+(film->getEste3D()?10.0:0.0))*multiplicatorCategorie(cat);
        std::string catStr=numeCategorie(cat);
        std::string usr=curent?curent->username:"Offline";

        Rezervare* rez;
        if (!email.empty())
            rez=new RezervareOnline(film->getTitlu(),s->getNume(),data,ora,r,l,pret,catStr,usr,email);
        else
            rez=new Rezervare(film->getTitlu(),s->getNume(),data,ora,r,l,pret,catStr,usr);
        rezervari.push_back(rez);

        
        if (curent) {
            std::ostringstream oss;
            oss << film->getTitlu() << " | " << s->getNume()
                << " | " << data.toString() << " " << ora
                << " | R" << r+1 << " L" << l+1 << " [" << catStr << "]"
                << " | " << std::fixed << std::setprecision(2) << pret << " RON";
            curent->istoricRezervari.push_back(oss.str());
            salveazaIstoricUser(*curent);
        }

        
        std::ofstream f("data/toate_rezervarile.txt",std::ios::app);
        f << "Film: " << film->getTitlu()
          << " | Sala: " << s->getNume()
          << " | Data: " << data.toString()
          << " | Ora: " << ora
          << " | Loc: R" << r+1 << " L" << l+1
          << " [" << catStr << "]"
          << " | Pret: " << std::fixed << std::setprecision(2) << pret << " RON"
          << " | Client: " << usr
          << (!email.empty() ? " | Email: "+email : "")
          << "\n";

        
        genereazaBilet(film->getTitlu(),s->getNume(),data,ora,r+1,l+1,catStr,pret);
    }

    void genereazaBilet(const std::string& fn, const std::string& sn,
                        const Data& d, const std::string& ora,
                        int r, int l, const std::string& cat, double pret) {
        std::string dataTag = std::string(1,d.toString()[0])+d.toString()[1]
                             +d.toString()[3]+d.toString()[4];
        std::string numeFisier="Bilet_"+dataTag
                              +"_R"+std::to_string(r)+"L"+std::to_string(l)
                              +"_"+std::to_string(time(nullptr))+".txt";
        std::string folderPath = "data/bilete/";
        std::string fullPath   = folderPath + numeFisier;
        // ensure folder exists
        #ifdef _WIN32
        std::system("if not exist \"data\\bilete\" mkdir data\\bilete >nul 2>&1");
        #else
        std::system("mkdir -p data/bilete");
        #endif
        std::ofstream bf(fullPath);
        bf << "==========================================\n"
           << "          BILET CINEMATOGRAF              \n"
           << "==========================================\n"
           << " FILM : " << fn << "\n"
           << " SALA : " << sn << "\n"
           << " DATA : " << d.toString() << "\n"
           << " ORA  : " << ora << "\n"
           << " LOC  : Rand " << r << ", Loc " << l << " [" << cat << "]\n"
           << " PRET : " << std::fixed << std::setprecision(2) << pret << " RON\n"
           << "------------------------------------------\n"
           << " COD  : " << genCod(r,l) << "\n"
           << "==========================================\n";
        bf.close();
        std::cout << "\n  [OK] Bilet generat: " << fullPath << "\n";
    }

    void anuleazaRezervare(int idSala, const Data& data, const std::string& ora, int r, int l) {
        Sala* s=getSalaById(idSala);
        if (!s||!s->esteOcupat(data,ora,r,l)) {
            std::cout << "\n  [!] Locul nu e ocupat.\n"; return;
        }
        s->elibereazaLoc(data,ora,r,l);

        std::ifstream fIn("data/toate_rezervarile.txt");
        std::vector<std::string> linii;
        std::string ln;
        std::string cheie="Sala: "+s->getNume()+" | Data: "+data.toString()
                         +" | Ora: "+ora+" | Loc: R"+std::to_string(r+1)+" L"+std::to_string(l+1);
        bool gasit=false;
        while(std::getline(fIn,ln)) {
            if(!ln.empty()&&ln.back()=='\r') ln.pop_back();
            if(!gasit&&ln.find(cheie)!=std::string::npos) gasit=true;
            else linii.push_back(ln);
        }
        fIn.close();
        std::ofstream fOut("data/toate_rezervarile.txt",std::ios::trunc);
        for (auto& lne : linii) fOut << lne << "\n";
        std::cout << (gasit?"\n  [OK] Rezervare anulata.\n":"\n  [!] Nu s-a gasit in log.\n");
    }

    
    void raportVanzari() {
        std::ifstream f("data/toate_rezervarile.txt");
        if(!f.is_open()) { std::cout << "  (gol)\n"; return; }
        std::map<std::string,int>    biletePeFilm;
        std::map<std::string,double> venitPeFilm;
        std::map<std::string,int>    biletePeSala;
        std::map<std::string,int>    biletePeOra;
        double totalVenit=0; int totalBilete=0;
        std::string ln;
        while(std::getline(f,ln)) {
            if(!ln.empty()&&ln.back()=='\r') ln.pop_back();
            if(ln.empty()) continue;
            auto extrage=[&](const std::string& tag, const std::string& end)->std::string{
                size_t p=ln.find(tag); if(p==std::string::npos) return "";
                p+=tag.size(); size_t e=ln.find(end,p);
                return (e==std::string::npos)?ln.substr(p):ln.substr(p,e-p);
            };
            std::string film=extrage("Film: "," | ");
            std::string sala=extrage("Sala: "," | ");
            std::string ora =extrage("Ora: "," | ");
            std::string spret=extrage("Pret: "," RON");
            if(film.empty()) continue;
            double pret=0;
            try { pret=std::stod(spret); } catch(...) {}
            biletePeFilm[film]++;
            venitPeFilm[film]+=pret;
            biletePeSala[sala]++;
            biletePeOra[ora]++;
            totalBilete++; totalVenit+=pret;
        }
        std::cout << "\n  === RAPORT VANZARI ===\n\n";
        std::cout << "  -- Pe film --\n";
        for (auto& [k,v] : biletePeFilm)
            std::cout << "    " << std::left << std::setw(45) << k
                      << " " << std::setw(4) << v << " bilete  |  "
                      << std::fixed << std::setprecision(2) << venitPeFilm[k] << " RON\n";
        std::cout << "\n  -- Pe sala --\n";
        for (auto& [k,v] : biletePeSala)
            std::cout << "    " << std::left << std::setw(25) << k << " " << v << " bilete\n";
        std::cout << "\n  -- Pe ora --\n";
        for (auto& [k,v] : biletePeOra)
            std::cout << "    " << k << " : " << v << " bilete\n";
        std::cout << "\n  TOTAL: " << totalBilete << " bilete | "
                  << std::fixed << std::setprecision(2) << totalVenit << " RON\n";
    }

    void afiseazaToateRezervari() {
        std::ifstream f("data/toate_rezervarile.txt");
        if(!f.is_open()) { std::cout << "  (gol)\n"; return; }
        std::string ln; int nr=0;
        while(std::getline(f,ln)) {
            if(!ln.empty()&&ln.back()=='\r') ln.pop_back();
            if(!ln.empty()) std::cout << "  " << ++nr << ". " << ln << "\n";
        }
        if(!nr) std::cout << "  (nicio rezervare)\n";
        else std::cout << "\n  Total: " << nr << " rezervari\n";
    }

    ~Cinematograf() { for(auto r:rezervari) delete r; }
};

#endif
