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
#include "Cinema.h"
#include "config.h"

// ─────────────────────────────────────────────────────────────────────────────
// SeatMapWidget
// ─────────────────────────────────────────────────────────────────────────────
class SeatMapWidget : public QWidget {
    Q_OBJECT
public:
    bool readOnly = false;
    explicit SeatMapWidget(QWidget* parent = nullptr);
    void load(Sala* sala, const Data& d, const std::string& ora);
    void setSelected(int r, int c);
    std::pair<int,int> getSelected() const { return {selRow, selCol}; }
signals:
    void seatClicked(int row, int col);
protected:
    void paintEvent(QPaintEvent*) override;
    void mousePressEvent(QMouseEvent*) override;
    void mouseMoveEvent(QMouseEvent*) override;
    void leaveEvent(QEvent*) override;
    QSize sizeHint() const override;
private:
    Sala*       sala = nullptr;
    Data        data = {1,1,2025};
    std::string ora;
    int selRow=-1,selCol=-1,hoverRow=-1,hoverCol=-1;
    static constexpr int S=40,G=8,ML=58,MT=76;
    double             pretBaza_ = 0.0;
    QRect              seatRect(int r,int c) const;
    std::pair<int,int> seatAt(QPoint p)     const;
public:
    void setPretBaza(double p){ pretBaza_=p; }
};

// ─────────────────────────────────────────────────────────────────────────────
// MenuCard
// ─────────────────────────────────────────────────────────────────────────────
class MenuCard : public QFrame {
    Q_OBJECT
public:
    MenuCard(const QString& emoji, const QString& title,
             const QString& desc, bool gold=false, QWidget* parent=nullptr);
signals:
    void clicked();
protected:
    void mousePressEvent(QMouseEvent* e) override { if(e->button()==Qt::LeftButton) emit clicked(); QFrame::mousePressEvent(e); }
#if QT_VERSION >= QT_VERSION_CHECK(6,0,0)
    void enterEvent(QEnterEvent*) override { setStyleSheet(hov_); }
#else
    void enterEvent(QEvent*)     override { setStyleSheet(hov_); }
#endif
    void leaveEvent(QEvent*)     override { setStyleSheet(nor_); }
private:
    QString nor_, hov_;
};

// ─────────────────────────────────────────────────────────────────────────────
// StepBar  –  reservation progress indicator
// ─────────────────────────────────────────────────────────────────────────────
class StepBar : public QWidget {
    Q_OBJECT
public:
    explicit StepBar(QWidget* parent = nullptr);
    void setStep(int step);   // 0 = Film detail, 1-5 = Sala…Confirmare
protected:
    void paintEvent(QPaintEvent*) override;
    QSize sizeHint() const override { return {0, 52}; }
private:
    int current_ = 0;
    static const QStringList STEPS;
};

// ─────────────────────────────────────────────────────────────────────────────
// Page indices
// ─────────────────────────────────────────────────────────────────────────────
enum Page { P_LOGIN=0, P_MAIN, P_RESERVE_FILM, P_RESERVE_SALA,
            P_RESERVE_DATA, P_RESERVE_ORA, P_RESERVE_LOC,
            P_RESERVE_CONFIRM, P_BROWSE, P_MAP, P_CANCEL,
            P_PROFILE, P_ADMIN, P_FILM_DETAIL, P_CONFIRM_DONE };

// ─────────────────────────────────────────────────────────────────────────────
// MainWindow
// ─────────────────────────────────────────────────────────────────────────────
class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    void onLogin();
    void onRegister();
    void onNewReservation();
    void onBrowseGenuri();
    void onHartaSala();
    void onAnulareRezervare();
    void onProfilMeu();
    void onAdminPanel();
    void onLogout();
    void onFilmSelected();          // goes to P_FILM_DETAIL
    void onProceedFromDetail();     // goes to P_RESERVE_SALA
    void onSalaSelected();
    void onDataSelected();
    void onOraSelected();
    void onLocSelected(int row, int col);
    void onConfirmReservation();
    void onCancelReservation();
    void onGenChanged(int idx);
    void updateBrowseDetail(int row);
    void onBrowseReserve();
    void onAdminAddFilm();
    void onAdminEditFilm();
    void onAdminDeleteFilm();
    void onAdminAddOra();
    void onAdminDeleteOra();
    void onAdminDeleteUser();
    void fetchPoster(Film* f);
    void refreshHeroBanner();
    void refreshUpcomingStrip();
    void updateSeatLegendCounts();

private:
    Cinematograf cinema;

    // reservation state
    Film*       rezFilm=nullptr; int rezSalaId=-1;
    Data        rezData={1,1,2025}; std::string rezOra;
    int         rezRow=-1,rezCol=-1,cancelRow=-1,cancelCol=-1;
    int         browse3D=-1; bool browseActiveOnly=false;
    Page        detailFromPage=P_RESERVE_FILM;

    // film search filters
    int     filmFilter3D=-1;
    QString filmFilterGenre;

    // layout
    QStackedWidget* stack;
    QWidget*        navbar;
    QLabel*         lblNavUser;
    StepBar*        stepBar;

    // login
    QLineEdit *leUser, *lePass;

    // main menu
    QLabel*   lblWelcome;
    MenuCard* cardAdmin;
    // hero banner
    QLabel   *lblHeroTitle, *lblHeroInfo, *lblHeroBadge;
    QFrame   *heroFrame;
    QTimer   *heroTimer;
    int       heroIdx = 0;
    // upcoming strip
    QWidget  *upcomingContainer;
    QFrame   *upcomingSection;

    // film detail page
    QLabel      *detailTitle, *detailGenreRating, *detailDesc,
                *detailPoster, *detailMeta, *detailPrice;
    QPushButton *btnDetailBook;
    QLabel      *detailUnavail;

    // reserve – film
    QLineEdit*   leSearch;
    QComboBox*   cbFilmGenre;
    QPushButton* btnFilm3D;
    QListWidget* lwFilms;
    QLabel*      lblFilmsEmpty;
    QStackedWidget* filmListStack; // 0=list, 1=empty state
    std::vector<Film*> searchResults;

    // reserve – sala / data / ora
    QLabel*      miniSala,  *miniData, *miniOra, *miniLoc, *miniConf;
    QListWidget *lwSali, *lwData, *lwOra;

    // reserve – seat
    SeatMapWidget* seatMapWidget;
    QLabel*        lblSeatInfo;
    QLabel        *lblLegStd, *lblLegVip, *lblLegCpl;

    // reserve – confirm
    QLabel*    lblConfirmDetails;
    QLineEdit* leEmail;
    QLabel*    lblBookingRef;

    // browse
    QComboBox*   cbGenuri;
    QListWidget* lwBrowseFilms;
    QLabel*      lblBrowseDetail;
    QPushButton *btnBrowseFilter3D, *btnBrowseFilterActive;

    // map
    SeatMapWidget* mapWidget;
    QComboBox     *cbMapSala,*cbMapData,*cbMapOra;

    // cancel
    SeatMapWidget* cancelWidget;
    QComboBox     *cbCancelSala,*cbCancelData,*cbCancelOra;

    // network
    QNetworkAccessManager* netManager = nullptr;

    // confirm done page
    QLabel *lblDoneRef, *lblDoneSummary;

    // profile
    QTextEdit* teHistory;

    // admin
    QTabWidget*  adminTabs;
    QTextEdit   *teAllRes,*teReport;
    QListWidget *lwAdminFilms,*lwAdminOre,*lwAdminUsers;

    // builders
    QWidget* buildNavbar();
    QWidget* buildLoginPage();
    QWidget* buildMainPage();
    QWidget* buildFilmDetailPage();
    QWidget* buildReserveFilmPage();
    QWidget* buildReserveSalaPage();
    QWidget* buildReserveDataPage();
    QWidget* buildReserveOraPage();
    QWidget* buildReserveLocPage();
    QWidget* buildReserveConfirmPage();
    QWidget* buildBrowsePage();
    QWidget* buildMapPage();
    QWidget* buildCancelPage();
    QWidget* buildProfilePage();
    QWidget* buildAdminPage();
    QWidget* buildConfirmDonePage();

    // helpers
    void goTo(Page p);
    void refreshMainMenu();
    void refreshSearchResults();
    void refreshMapPage();
    void refreshCancelSeats();
    void refreshAdminFilms();
    void refreshAdminOre();
    void refreshAdminUsers();
    void refreshAdminReservations();
    void refreshAdminReport();
    void refreshBrowse();
    void refreshProfile();
    void refreshFilmMiniHeaders();
    void populateFilmDetail(Film* f);
    QFrame* filmMiniCard(QLabel*& lbl);   // factory used per-page
    bool    showFilmDialog(Film& out, const Film* existing=nullptr);
};
