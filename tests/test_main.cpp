// ============================================================
// Teste unitare – Cinema Booking System
// Compilare: g++ -std=c++20 -I../include tests/test_main.cpp -o tests/run_tests
// Rulare:    ./tests/run_tests
// ============================================================
#define _CRT_SECURE_NO_WARNINGS
#include <cassert>
#include <iostream>
#include <stdexcept>
#include "CinemaModels.h"
#include "Cinema.h"

static int passed = 0, failed = 0;

#define TEST(name) void name()
#define RUN(name) do { \
    try { name(); std::cout << "  [PASS] " #name "\n"; ++passed; } \
    catch(const std::exception& e){ std::cout << "  [FAIL] " #name " -> " << e.what() << "\n"; ++failed; } \
    catch(...){ std::cout << "  [FAIL] " #name " -> exceptie necunoscuta\n"; ++failed; } \
} while(0)

#define ASSERT(cond) do { if(!(cond)) throw std::runtime_error("assert esuat: " #cond); } while(0)
#define ASSERT_THROWS(expr, ExcType) do { \
    bool caught = false; \
    try { expr; } catch(const ExcType&){ caught=true; } \
    if(!caught) throw std::runtime_error("Nu s-a aruncat " #ExcType); \
} while(0)
#define ASSERT_NEAR(a,b,eps) do { if(std::abs((a)-(b))>(eps)) throw std::runtime_error("Valori diferite: " + std::to_string(a) + " vs " + std::to_string(b)); } while(0)

// ── 1. Film ──────────────────────────────────────────────────────────────────
TEST(test_film_getteri) {
    Film f("Inception", true, 30.0, "SF", 148,
           StatusFilm::RULAZA_ACUM, 8.8, "Vis in vis", "Nolan", "", "2010", "SUA", "EN", "AP12");
    ASSERT(f.getTitlu() == "Inception");
    ASSERT(f.getEste3D() == true);
    ASSERT_NEAR(f.getPretBaza(), 30.0, 0.001);
    ASSERT(f.getGen() == "SF");
    ASSERT(f.getDurata() == 148);
    ASSERT(f.getStatus() == StatusFilm::RULAZA_ACUM);
    ASSERT_NEAR(f.getRating(), 8.8, 0.001);
    ASSERT(f.getRegizor() == "Nolan");
}

TEST(test_film_setteri) {
    Film f("Test", false, 20.0);
    f.setTitlu("Nou"); f.setPretBaza(45.0); f.setEste3D(true);
    ASSERT(f.getTitlu() == "Nou");
    ASSERT_NEAR(f.getPretBaza(), 45.0, 0.001);
    ASSERT(f.getEste3D() == true);
}

// ── 2. Sala ──────────────────────────────────────────────────────────────────
TEST(test_sala_rezervare_simpla) {
    Sala s(1, "Sala Test", 4, 8);
    Data d{15, 6, 2025};
    std::string ora = "20:45";
    ASSERT(s.locuriLibere(d, ora) == 32);
    s.ocupaLoc(d, ora, 0, 0);
    ASSERT(s.locuriLibere(d, ora) == 31);
    ASSERT(s.esteOcupat(d, ora, 0, 0) == true);
    ASSERT(s.esteOcupat(d, ora, 0, 1) == false);
}

TEST(test_sala_loc_ocupat_exceptie) {
    Sala s(1, "Sala Test", 4, 8);
    Data d{15, 6, 2025};
    s.ocupaLoc(d, "20:45", 1, 1);
    // A doua ocupare pe acelasi loc trebuie sa arunce LocOcupatException
    ASSERT_THROWS(s.ocupaLoc(d, "20:45", 1, 1), LocOcupatException);
}

TEST(test_sala_index_invalid_exceptie) {
    Sala s(1, "Sala Test", 4, 8);
    Data d{15, 6, 2025};
    // Rand -1 invalid
    ASSERT_THROWS(s.ocupaLoc(d, "20:45", -1, 0), IndexInvalidException);
    // Rand prea mare
    ASSERT_THROWS(s.ocupaLoc(d, "20:45", 99, 0), IndexInvalidException);
    // Coloana invalida
    ASSERT_THROWS(s.ocupaLoc(d, "20:45", 0, 99), IndexInvalidException);
}

TEST(test_sala_elibereaza) {
    Sala s(1, "Sala Test", 4, 8);
    Data d{15, 6, 2025};
    s.ocupaLoc(d, "20:45", 2, 3);
    ASSERT(s.esteOcupat(d, "20:45", 2, 3) == true);
    s.elibereazaLoc(d, "20:45", 2, 3);
    ASSERT(s.esteOcupat(d, "20:45", 2, 3) == false);
}

TEST(test_sala_categorii) {
    Sala s(1, "Test", 5, 10);
    // Ultimul rand = VIP
    ASSERT(s.getCategorieLocSeat(4, 0) == CategorieLocSeat::VIP);
    // Mijlocul primului rand = Cuplu
    ASSERT(s.getCategorieLocSeat(0, 4) == CategorieLocSeat::CUPLU);
    ASSERT(s.getCategorieLocSeat(0, 5) == CategorieLocSeat::CUPLU);
    // Rest = Standard
    ASSERT(s.getCategorieLocSeat(2, 3) == CategorieLocSeat::STANDARD);
}

// ── 3. Multiplicatori pret ────────────────────────────────────────────────────
TEST(test_multiplicator_categorie) {
    ASSERT_NEAR(multiplicatorCategorie(CategorieLocSeat::STANDARD), 1.0,  0.001);
    ASSERT_NEAR(multiplicatorCategorie(CategorieLocSeat::VIP),      1.5,  0.001);
    ASSERT_NEAR(multiplicatorCategorie(CategorieLocSeat::CUPLU),    1.8,  0.001);
}

TEST(test_pret_3d) {
    Film f2d("Film2D", false, 25.0);
    Film f3d("Film3D", true,  25.0);
    double pret2d = f2d.getPretBaza() + (f2d.getEste3D() ? 10.0 : 0.0);
    double pret3d = f3d.getPretBaza() + (f3d.getEste3D() ? 10.0 : 0.0);
    ASSERT_NEAR(pret2d, 25.0, 0.001);
    ASSERT_NEAR(pret3d, 35.0, 0.001);
}

TEST(test_multiplicator_zi_weekend) {
    // 5 iulie 2025 = Sâmbătă → weekend
    Data weekend{5, 7, 2025};
    ASSERT_NEAR(multiplicatorZi(weekend), 1.2, 0.001);
    // 7 iulie 2025 = Luni → weekday
    Data weekday{7, 7, 2025};
    ASSERT_NEAR(multiplicatorZi(weekday), 1.0, 0.001);
}

// ── 4. Rezervare ─────────────────────────────────────────────────────────────
TEST(test_rezervare_getteri) {
    Data d{10, 5, 2025};
    Rezervare r("Inception","Sala 1",d,"20:45",2,3,45.0,"VIP","ion");
    ASSERT(r.getTitluFilm() == "Inception");
    ASSERT(r.getSalaNume()  == "Sala 1");
    ASSERT(r.getRand()      == 2);
    ASSERT(r.getLoc()       == 3);
    ASSERT_NEAR(r.getPretFinal(), 45.0, 0.001);
    ASSERT(r.getCategorie() == "VIP");
    ASSERT(r.getUsername()  == "ion");
}

TEST(test_rezervare_online) {
    Data d{10, 5, 2025};
    RezervareOnline ro("Dune","Sala 2",d,"17:30",1,2,30.0,"STANDARD","maria","maria@test.ro");
    ASSERT(ro.getEmail() == "maria@test.ro");
    ASSERT(ro.getTitluFilm() == "Dune");
}

// ── 5. FoodOrder ──────────────────────────────────────────────────────────────
TEST(test_food_order) {
    std::vector<FoodItem> meniu = {
        {"POP_M_SAR","🍿","Popcorn Mare Sărat","",18.0,"Popcorn"},
        {"COLA_S",   "🥤","Coca-Cola 0.5L",   "",10.0,"Bauturi"},
    };
    FoodOrder fo;
    ASSERT(fo.empty());
    fo.items["POP_M_SAR"] = 2;
    fo.items["COLA_S"]    = 1;
    ASSERT(!fo.empty());
    ASSERT_NEAR(fo.total(meniu), 46.0, 0.001); // 2x18 + 1x10
}

// ── 6. Data ───────────────────────────────────────────────────────────────────
TEST(test_data_comparare) {
    Data d1{1, 1, 2025}, d2{2, 1, 2025}, d3{1, 1, 2025};
    ASSERT(d1 < d2);
    ASSERT(!(d2 < d1));
    ASSERT(d1 == d3);
    ASSERT(!(d1 == d2));
}

TEST(test_data_tostring) {
    Data d{5, 3, 2025};
    ASSERT(d.toString() == "05/03/2025");
}

// ── 7. getRegiune ─────────────────────────────────────────────────────────────
TEST(test_get_regiune) {
    ASSERT(getRegiune("Suceava")    == "Moldova");
    ASSERT(getRegiune("Iasi")       == "Moldova");
    ASSERT(getRegiune("Brasov")     == "Transilvania");
    ASSERT(getRegiune("Cluj-Napoca")== "Transilvania");
    ASSERT(getRegiune("Bucuresti")  == "Muntenia");
    ASSERT(getRegiune("Timisoara")  == "Banat");
    ASSERT(getRegiune("Constanta")  == "Dobrogea");
    ASSERT(getRegiune("Oradea")     == "Crisana");
    ASSERT(getRegiune("Craiova")    == "Oltenia");
}

// ─────────────────────────────────────────────────────────────────────────────
int main() {
    std::cout << "\n=== Teste unitare – Cinema Booking System ===\n\n";

    std::cout << "[ Film ]\n";
    RUN(test_film_getteri);
    RUN(test_film_setteri);

    std::cout << "\n[ Sala ]\n";
    RUN(test_sala_rezervare_simpla);
    RUN(test_sala_loc_ocupat_exceptie);
    RUN(test_sala_index_invalid_exceptie);
    RUN(test_sala_elibereaza);
    RUN(test_sala_categorii);

    std::cout << "\n[ Preturi ]\n";
    RUN(test_multiplicator_categorie);
    RUN(test_pret_3d);
    RUN(test_multiplicator_zi_weekend);

    std::cout << "\n[ Rezervare ]\n";
    RUN(test_rezervare_getteri);
    RUN(test_rezervare_online);

    std::cout << "\n[ Food ]\n";
    RUN(test_food_order);

    std::cout << "\n[ Data ]\n";
    RUN(test_data_comparare);
    RUN(test_data_tostring);

    std::cout << "\n[ Regiuni ]\n";
    RUN(test_get_regiune);

    std::cout << "\n==============================================\n";
    std::cout << "  Rezultat: " << passed << " PASS, " << failed << " FAIL\n";
    std::cout << "==============================================\n\n";
    return failed == 0 ? 0 : 1;
}
