#pragma once
#include <QMainWindow>
#include <QStackedWidget>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QListWidget>
#include <QComboBox>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QInputDialog>
#include <QFrame>
#include <QTextEdit>
#include <QScrollArea>
#include <QTabWidget>
#include <QPainter>
#include <QPainterPath>
#include <QMouseEvent>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QTimer>
#include <QFormLayout>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QDialog>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QUrlQuery>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QToolTip>
#include <QDesktopServices>
#include <QUrl>
#include "Cinema.h"
#include "config.h"



// ─── SeatMapWidget ────────────────────────────────────────────────────────────
class SeatMapWidget : public QWidget {
    Q_OBJECT
public:
    bool readOnly = false;
    explicit SeatMapWidget(QWidget* parent = nullptr);
    void load(Sala* sala, const Data& d, const std::string& ora);
    void setSelected(const std::vector<std::pair<int, int>>& seats);
    void setPretBaza(double p) { pretBaza_ = p; }
    std::vector<std::pair<int, int>> getSelected() const { return selected_; }
signals:
    void seatClicked(int row, int col);
protected:
    void paintEvent(QPaintEvent*) override;
    void mousePressEvent(QMouseEvent*) override;
    void mouseMoveEvent(QMouseEvent*) override;
    void leaveEvent(QEvent*) override;
    QSize sizeHint() const override;
private:
    Sala* sala = nullptr;
    Data        data = { 1,1,2025 };
    std::string ora;
    std::vector<std::pair<int, int>> selected_;
    int hoverRow = -1, hoverCol = -1;
    double pretBaza_ = 0.0;
    static constexpr int S = 52, G = 10, ML = 72, MT = 120;
    QRect              seatRect(int r, int c) const;
    std::pair<int, int> seatAt(QPoint p)     const;
};

// ─── MenuCard ────────────────────────────────────────────────────────────────
class MenuCard : public QFrame {
    Q_OBJECT
public:
    MenuCard(const QString& emoji, const QString& title, const QString& desc, bool gold = false, QWidget* parent = nullptr);
signals:
    void clicked();
protected:
    void mousePressEvent(QMouseEvent* e) override { if (e->button() == Qt::LeftButton) emit clicked(); QFrame::mousePressEvent(e); }
#if QT_VERSION >= QT_VERSION_CHECK(6,0,0)
    void enterEvent(QEnterEvent*) override { setStyleSheet(hov_); }
#else
    void enterEvent(QEvent*)     override { setStyleSheet(hov_); }
#endif
    void leaveEvent(QEvent*)     override { setStyleSheet(nor_); }
private:
    QString nor_, hov_;
};

// ─── StepBar ─────────────────────────────────────────────────────────────────
class StepBar : public QWidget {
    Q_OBJECT
public:
    explicit StepBar(QWidget* parent = nullptr);
    void setStep(int step);
protected:
    void paintEvent(QPaintEvent*) override;
    QSize sizeHint() const override { return { 0,52 }; }
private:
    int current_ = 0;
    static const QStringList STEPS;
};

// ─── LocationCard ─────────────────────────────────────────────────────────────
class LocationCard : public QFrame {
    Q_OBJECT
public:
    LocationCard(const CinemaLocation& loc, bool isCurrentCity, QWidget* parent = nullptr);
    QString locationId() const { return id_; }
signals:
    void clicked(const QString& id);
protected:
    void mousePressEvent(QMouseEvent* e) override { if (e->button() == Qt::LeftButton) emit clicked(id_); QFrame::mousePressEvent(e); }
#if QT_VERSION >= QT_VERSION_CHECK(6,0,0)
    void enterEvent(QEnterEvent*) override { setStyleSheet(hov_); }
#else
    void enterEvent(QEvent*)     override { setStyleSheet(hov_); }
#endif
    void leaveEvent(QEvent*)     override { setStyleSheet(nor_); }
private:
    QString id_, nor_, hov_;
};

// ─── Pages ───────────────────────────────────────────────────────────────────
enum Page {
    P_LOGIN = 0, P_LOCATION, P_MAIN,
    P_RESERVE_FILM, P_RESERVE_SALA, P_RESERVE_DATA, P_RESERVE_ORA,
    P_RESERVE_LOC, P_RESERVE_FOOD, P_RESERVE_CONFIRM,
    P_BROWSE, P_MAP, P_CANCEL, P_PROFILE, P_ADMIN,
    P_FILM_DETAIL, P_CONFIRM_DONE
};

// ─── MainWindow ──────────────────────────────────────────────────────────────
class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    // auth
    void onLogin(); void onRegister(); void onLogout();
    // location
    void onLocationDetected(); void onLocationSelected(const QString& id);
    // main menu
    void onNewReservation(); void onBrowseGenuri(); void onHartaSala();
    void onAnulareRezervare(); void onProfilMeu(); void onAdminPanel();
    // reserve flow
    void onFilmSelected(); void onProceedFromDetail();
    void onDataSelected();
    void onShowingSelected(const Programare* p);
    void onLocSelected(int row, int col);
    void onFoodConfirmed(); void onFoodSkipped();
    void onConfirmReservation(); void onCancelReservation();
    // browse
    void onGenChanged(int idx); void updateBrowseDetail(int row); void onBrowseReserve();
    // admin
    void onAdminAddFilm(); void onAdminEditFilm(); void onAdminDeleteFilm();
    void onAdminDeleteUser();
    // misc
    void refreshHeroBanner(); void refreshUpcomingStrip();
    void updateSeatLegendCounts(); void fetchPoster(Film* f);
    void showToast(const QString& msg, bool isError = true);
    void refreshMapOra();   // repopulate ora combo from schedule
    void refreshCancelOra();

private:
    Cinematograf cinema;

    // reservation state
    Film* rezFilm = nullptr;
    int         rezSalaId = -1;
    Data        rezData = { 1,1,2025 };
    std::string rezOra;
    const Programare* rezProgramare = nullptr;       // selected showing
    std::vector<std::pair<int, int>> rezLocuri;     // group of seats
    int         maxLocuri = 8;
    FoodOrder   rezFood;
    int         cancelRow = -1, cancelCol = -1;
    int         browse3D = -1;
    bool        browseActiveOnly = false;
    Page        detailFromPage = P_RESERVE_FILM;
    int         filmFilter3D = -1;

    // location detection
    QString     detectedCity;
    QNetworkAccessManager* netManager = nullptr;

    // layout
    QStackedWidget* stack;
    QWidget* navbar;
    QLabel* lblNavUser, * lblNavLocation;
    StepBar* stepBar;

    // ── login
    QLineEdit* leUser, * lePass;

    // ── location page
    QWidget* locationContainer;
    QLabel* lblDetectedCity;
    QPushButton* btnDetectLocation;

    // ── main menu
    QLabel* lblWelcome;
    MenuCard* cardAdmin;
    QLabel* lblHeroTitle, * lblHeroInfo, * lblHeroBadge;
    QFrame* heroFrame;
    QTimer* heroTimer;
    int        heroIdx = 0;
    QWidget* upcomingContainer;
    QFrame* upcomingSection;

    // ── film detail
    QLabel* detailTitle, * detailGenreRating, * detailDesc,
        * detailPoster, * detailMeta, * detailPrice;
    QPushButton* btnDetailBook;
    QLabel* detailUnavail;

    // ── reserve - film
    QLineEdit* leSearch;
    QComboBox* cbFilmGenre;
    QPushButton* btnFilm3D;
    QListWidget* lwFilms;
    QLabel* lblFilmsEmpty;
    QStackedWidget* filmListStack;
    std::vector<Film*> searchResults;

    // ── reserve - sala+ora → now "showings"
    QScrollArea* showingsScroll;
    QWidget* showingsContainer;
    QLabel* lblNoShowings;
    const Programare* hoveredProgramare = nullptr;

    // ── reserve - data
    QListWidget* lwData;

    QLabel* miniData, * miniShow, * miniLoc, * miniFood, * miniConf;
    SeatMapWidget* seatMapWidget;
    QLabel* lblSeatInfo, * lblSeatCount;
    QLabel* lblLegStd, * lblLegVip, * lblLegCpl;
    QPushButton* btnClearSeats;

    // ── reserve - food
    std::vector<QPushButton*> foodQtyBtns;
    std::vector<QLabel*>      foodQtyLabels;
    QLabel* lblFoodTotal;

    // ── reserve - confirm
    QLabel* lblConfirmDetails;
    QLineEdit* leEmail;
    QLabel* lblBookingRef;

    // ── confirm done
    QLabel* lblDoneRef, * lblDoneSummary;

    // ── browse
    QComboBox* cbGenuri;
    QListWidget* lwBrowseFilms;
    QLabel* lblBrowseDetail;
    QPushButton* btnBrowseFilter3D, * btnBrowseFilterActive;

    // ── map
    SeatMapWidget* mapWidget;
    QComboBox* cbMapSala, * cbMapData, * cbMapOra;

    // ── cancel
    SeatMapWidget* cancelWidget;
    QComboBox* cbCancelSala, * cbCancelData, * cbCancelOra;

    // ── profile
    QTextEdit* teHistory;

    // ── admin
    QTabWidget* adminTabs;
    QTextEdit* teAllRes, * teReport;
    QListWidget* lwAdminFilms, * lwAdminOre, * lwAdminUsers;

    // ── toast notification
    QLabel* toastLabel = nullptr;
    QTimer* toastTimer = nullptr;
    // ── builders
    QWidget* buildNavbar();
    QWidget* buildLoginPage();
    QWidget* buildLocationPage();
    QWidget* buildMainPage();
    QWidget* buildFilmDetailPage();
    QWidget* buildReserveFilmPage();
    QWidget* buildReserveDataPage();
    QWidget* buildReserveShowingPage();   // replaces sala+ora
    QWidget* buildReserveLocPage();
    QWidget* buildReserveFoodPage();
    QWidget* buildReserveConfirmPage();
    QWidget* buildBrowsePage();
    QWidget* buildMapPage();
    QWidget* buildCancelPage();
    QWidget* buildProfilePage();
    QWidget* buildAdminPage();
    QWidget* buildConfirmDonePage();

    // ── helpers
    void goTo(Page p);
    void refreshMainMenu();
    void refreshLocationPage(const QString& city);
    void refreshSearchResults();
    void refreshMapPage();
    void refreshCancelSeats();
    void refreshAdminFilms();
    void refreshAdminUsers();
    void refreshAdminReservations();
    void refreshAdminReport();
    void refreshBrowse();
    void refreshProfile();
    void refreshFilmMiniHeaders();
    void refreshFoodPage();
    void buildConfirmPage();
    void populateFilmDetail(Film* f);
    QFrame* filmMiniCard(QLabel*& lbl);
    bool    showFilmDialog(Film& out, const Film* existing = nullptr);
};