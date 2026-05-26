#include "mainwindow.h"
#include <QSvgRenderer>
#include <QPainter>
#include <sstream>
#include <iomanip>
#include <fstream>
#ifdef Q_OS_WIN
#include <windows.h>
#include <dwmapi.h>
#endif

#define C_BG    "#0a0a0f"
#define C_SURF  "#111116"
#define C_CARD  "#18181f"
#define C_RAISE "#1e1e28"
#define C_BOR   "#2a2a38"
#define C_BOR2  "#1e1e2a"
#define C_RED   "#e50914"
#define C_RHOV  "#ff2233"
#define C_GOLD  "#f5a623"
#define C_PURP  "#8b5cf6"
#define C_GRN   "#22c55e"
#define C_BLUE  "#38bdf8"
#define C_TXT   "#f0f0f5"
#define C_MUT   "#8b8fa8"
#define C_DIM   "#50536a"

static QPixmap svgIcon(const QString& paths, int size = 32, const QString& color = "#e50914") {
    QString svg = QString(
        "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"%1\" height=\"%1\" viewBox=\"0 0 24 24\""
        " fill=\"none\" stroke=\"%2\" stroke-width=\"1.8\" stroke-linecap=\"round\" stroke-linejoin=\"round\">%3</svg>"
    ).arg(size).arg(color).arg(paths);
    QSvgRenderer r(svg.toUtf8());
    QPixmap px(size * 2, size * 2); px.fill(Qt::transparent);
    QPainter p(&px); p.setRenderHint(QPainter::Antialiasing);
    r.render(&p); return px.scaled(size, size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
}
static QPixmap iconTicket(int s = 32) { return svgIcon("<path d=\"M2 9a3 3 0 0 1 0 6v2a2 2 0 0 0 2 2h16a2 2 0 0 0 2-2v-2a3 3 0 0 1 0-6V7a2 2 0 0 0-2-2H4a2 2 0 0 0-2 2v2z\"/><path d=\"M13 5v2M13 17v2M13 11v2\"/>", s); }
static QPixmap iconSearch(int s = 32) { return svgIcon("<circle cx=\"11\" cy=\"11\" r=\"8\"/><path d=\"m21 21-4.35-4.35\"/>", s); }
static QPixmap iconGrid(int s = 32) { return svgIcon("<rect x=\"3\" y=\"3\" width=\"7\" height=\"7\"/><rect x=\"14\" y=\"3\" width=\"7\" height=\"7\"/><rect x=\"3\" y=\"14\" width=\"7\" height=\"7\"/><rect x=\"14\" y=\"14\" width=\"7\" height=\"7\"/>", s); }
static QPixmap iconXCircle(int s = 32) { return svgIcon("<circle cx=\"12\" cy=\"12\" r=\"10\"/><path d=\"m15 9-6 6M9 9l6 6\"/>", s); }
static QPixmap iconUser(int s = 32) { return svgIcon("<path d=\"M20 21v-2a4 4 0 0 0-4-4H8a4 4 0 0 0-4 4v2\"/><circle cx=\"12\" cy=\"7\" r=\"4\"/>", s); }
static QPixmap iconSettings(int s = 32) { return svgIcon("<circle cx=\"12\" cy=\"12\" r=\"3\"/><path d=\"M19.4 15a1.65 1.65 0 0 0 .33 1.82l.06.06a2 2 0 0 1-2.83 2.83l-.06-.06a1.65 1.65 0 0 0-1.82-.33 1.65 1.65 0 0 0-1 1.51V21a2 2 0 0 1-4 0v-.09A1.65 1.65 0 0 0 9 19.4a1.65 1.65 0 0 0-1.82.33l-.06.06a2 2 0 0 1-2.83-2.83l.06-.06A1.65 1.65 0 0 0 4.68 15a1.65 1.65 0 0 0-1.51-1H3a2 2 0 0 1 0-4h.09A1.65 1.65 0 0 0 4.6 9a1.65 1.65 0 0 0-.33-1.82l-.06-.06a2 2 0 0 1 2.83-2.83l.06.06A1.65 1.65 0 0 0 9 4.68a1.65 1.65 0 0 0 1-1.51V3a2 2 0 0 1 4 0v.09a1.65 1.65 0 0 0 1 1.51 1.65 1.65 0 0 0 1.82-.33l.06-.06a2 2 0 0 1 2.83 2.83l-.06.06A1.65 1.65 0 0 0 19.4 9a1.65 1.65 0 0 0 1.51 1H21a2 2 0 0 1 0 4h-.09a1.65 1.65 0 0 0-1.51 1z\"/>", s); }
static QPixmap iconMap(int s = 32) { return svgIcon("<polygon points=\"1 6 1 22 8 18 16 22 23 18 23 2 16 6 8 2 1 6\"/><line x1=\"8\" x2=\"8\" y1=\"2\" y2=\"18\"/><line x1=\"16\" x2=\"16\" y1=\"6\" y2=\"22\"/>", s); }
static QPixmap iconFilm(int s = 32, const QString& c = "#e50914") { return svgIcon("<rect x=\"2\" y=\"2\" width=\"20\" height=\"20\" rx=\"2.18\"/><line x1=\"7\" x2=\"7\" y1=\"2\" y2=\"22\"/><line x1=\"17\" x2=\"17\" y1=\"2\" y2=\"22\"/><line x1=\"2\" x2=\"22\" y1=\"12\" y2=\"12\"/><line x1=\"2\" x2=\"7\" y1=\"7\" y2=\"7\"/><line x1=\"17\" x2=\"22\" y1=\"7\" y2=\"7\"/><line x1=\"17\" x2=\"22\" y1=\"17\" y2=\"17\"/><line x1=\"2\" x2=\"7\" y1=\"17\" y2=\"17\"/>", s, c); }
static QPixmap iconPin(int s = 16, const QString& c = "#4ade80") { return svgIcon("<path d=\"M12 2C8.13 2 5 5.13 5 9c0 5.25 7 13 7 13s7-7.75 7-13c0-3.87-3.13-7-7-7z\"/><circle cx=\"12\" cy=\"9\" r=\"2.5\"/>", s, c); }


static QString inputSS() {
    return "QLineEdit{background:" C_RAISE ";color:" C_TXT ";border:1.5px solid " C_BOR ";"
        "border-radius:8px;padding:11px 16px;font-size:14px;}"
        "QLineEdit:focus{border-color:" C_RED ";}"
        "QLineEdit:hover{border-color:#3a3a50;}";
}
static QString listSS() {
    return "QListWidget{background:" C_BG ";border:none;outline:none;}"
        "QListWidget::item{background:" C_CARD ";border:1.5px solid " C_BOR2 ";"
        "border-radius:10px;color:" C_TXT ";margin:3px 0;padding:13px 18px;min-height:44px;}"
        "QListWidget::item:selected{background:#0d1a2e;border:1.5px solid " C_RED ";}"
        "QListWidget::item:hover:!selected{background:#1c1c27;border-color:" C_BOR ";}";
}
static QString comboSS() {
    return "QComboBox{background:" C_RAISE ";color:" C_TXT ";border:1.5px solid " C_BOR ";"
        "border-radius:8px;padding:8px 14px;font-size:13px;}"
        "QComboBox:hover{border-color:#3a3a50;}"
        "QComboBox::drop-down{border:none;width:26px;}"
        "QComboBox QAbstractItemView{background:" C_RAISE ";color:" C_TXT ";"
        "selection-background-color:" C_RED ";border:1px solid " C_BOR ";outline:none;}";
}
static QPushButton* redBtn(const QString& t, int h = 46) {
    auto* b = new QPushButton(t); b->setFixedHeight(h); b->setCursor(Qt::PointingHandCursor);
    b->setStyleSheet("QPushButton{background:" C_RED ";color:white;border:none;border-radius:8px;"
        "padding:0 28px;font-size:14px;font-weight:700;}"
        "QPushButton:hover{background:" C_RHOV ";}"
        "QPushButton:pressed{background:#a0000e;}");
    return b;
}
static QPushButton* ghostBtn(const QString& t, int h = 42) {
    auto* b = new QPushButton(t); b->setFixedHeight(h); b->setCursor(Qt::PointingHandCursor);
    b->setStyleSheet("QPushButton{background:transparent;color:" C_MUT ";border:1.5px solid " C_BOR ";"
        "border-radius:8px;padding:0 20px;font-size:13px;}"
        "QPushButton:hover{color:" C_TXT ";border-color:#555570;background:" C_RAISE ";}");
    return b;
}
static QLabel* secLbl(const QString& t) {
    auto* l = new QLabel(t);
    l->setStyleSheet("font-size:11px;font-weight:700;color:" C_DIM ";letter-spacing:2px;background:transparent;border:none;padding:0;");
    return l;
}
static QFrame* hLine() {
    auto* f = new QFrame; f->setFrameShape(QFrame::HLine);
    f->setStyleSheet("background:" C_BOR2 ";max-height:1px;border:none;"); return f;
}
static QLabel* comboLbl(const QString& t) {
    auto* l = new QLabel(t); l->setStyleSheet("color:" C_MUT ";font-size:13px;"); return l;
}


SeatMapWidget::SeatMapWidget(QWidget* p) :QWidget(p) { setMouseTracking(true);setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred); }
void SeatMapWidget::load(Sala* s, const Data& d, const std::string& o) { sala = s;data = d;ora = o;selected_.clear();hoverRow = hoverCol = -1;updateGeometry();update(); }
void SeatMapWidget::setSelected(const std::vector<std::pair<int, int>>& seats) { selected_ = seats;update(); }
QRect SeatMapWidget::seatRect(int r, int c) const { return{ ML + c * (S + G),MT + r * (S + G),S,S }; }
std::pair<int, int> SeatMapWidget::seatAt(QPoint p) const {
    if (!sala) return{ -1,-1 };
    for (int r = 0;r < sala->getRanduri();r++) for (int c = 0;c < sala->getColoane();c++) if (seatRect(r, c).contains(p)) return{ r,c };
    return{ -1,-1 };
}
QSize SeatMapWidget::sizeHint() const {
    if (!sala) return{ 420,300 };
    return{ ML + sala->getColoane() * (S + G) - G + 24,MT + sala->getRanduri() * (S + G) - G + 28 };
}
void SeatMapWidget::leaveEvent(QEvent*) { hoverRow = hoverCol = -1;setCursor(Qt::ArrowCursor);update(); }
void SeatMapWidget::mousePressEvent(QMouseEvent* e) {
    if (!readOnly) { auto [r, c] = seatAt(e->pos());if (r >= 0)emit seatClicked(r, c); }
}
void SeatMapWidget::mouseMoveEvent(QMouseEvent* e) {
    auto [r, c] = seatAt(e->pos());
    if (r != hoverRow || c != hoverCol) { hoverRow = r;hoverCol = c;setCursor((r >= 0 && !readOnly) ? Qt::PointingHandCursor : Qt::ArrowCursor);update(); }
    if (r >= 0 && sala) {
        bool occ = sala->esteOcupat(data, ora, r, c);
        CategorieLocSeat cat = sala->getCategorieLocSeat(r, c);
        QString tip = occ ? "Loc ocupat" :
            QString::fromStdString(numeCategorie(cat)) + " · " + QString::number((int)(pretBaza_ * multiplicatorCategorie(cat))) + " RON";
        QToolTip::showText(QCursor::pos(), tip, this);
    }
    else QToolTip::hideText();
}
void SeatMapWidget::paintEvent(QPaintEvent*) {
    QPainter p(this); p.setRenderHint(QPainter::Antialiasing);
    p.fillRect(rect(), QColor(C_BG));
    if (!sala) { p.setPen(QColor(C_DIM));p.setFont(QFont("Segoe UI", 11));p.drawText(rect(), Qt::AlignCenter, "Selectează sala și ora");return; }
    int cols = sala->getColoane();
    int sx1 = ML - 14, sx2 = ML + cols * (S + G) - G + 14, sy = MT - 80;
    QPainterPath arc; arc.moveTo(sx1, sy + 36); arc.quadTo((sx1 + sx2) / 2, sy, sx2, sy + 36);
    for (int i = 5;i >= 0;i--) { QPen gp(QColor(229, 9, 20, 10 + i * 12), 3.0 + i * 2.5, Qt::SolidLine, Qt::RoundCap);p.setPen(gp);p.drawPath(arc); }
    p.setPen(QPen(QColor(C_RED), 2.5, Qt::SolidLine, Qt::RoundCap)); p.drawPath(arc);
    p.setPen(QColor(C_DIM)); p.setFont(QFont("Arial", 10, QFont::Bold));
    p.drawText(QRect(sx1, sy + 36, sx2 - sx1, 22), Qt::AlignHCenter, "— ECRAN —");
    p.setPen(QColor(C_DIM)); p.setFont(QFont("Segoe UI", 9));
    for (int c = 0;c < cols;c++) { QRect sr = seatRect(0, c);p.drawText(QRect(sr.x(), MT - 26, S, 20), Qt::AlignHCenter | Qt::AlignVCenter, QString::number(c + 1)); }
    for (int r = 0;r < sala->getRanduri();r++) {
        QRect first = seatRect(r, 0);
        p.setPen(QColor(C_DIM)); p.setFont(QFont("Segoe UI", 9));
        p.drawText(QRect(0, first.y(), ML - 12, S), Qt::AlignRight | Qt::AlignVCenter, QString("R%1").arg(r + 1));
        for (int c = 0;c < cols;c++) {
            bool occ = sala->esteOcupat(data, ora, r, c);
            bool isSel = false;
            for (auto& [sr, sc] : selected_) if (sr == r && sc == c) { isSel = true;break; }
            CategorieLocSeat cat = sala->getCategorieLocSeat(r, c);


            bool isPref = preferinta_.setat && cat == preferinta_.tip
                && !occ && !isSel;
            bool hov = (r == hoverRow && c == hoverCol) && !readOnly;
            QColor fill, border, txtC; QString lbl;
            if (isSel) { fill = QColor("#0c3d5c");border = QColor(C_BLUE);txtC = QColor(C_BLUE);lbl = "★"; }
            else if (occ) { fill = QColor("#161620");border = QColor("#252535");txtC = QColor("#3a3a55");lbl = "✕"; }
            else if (isPref && !hov) {


                switch (cat) {
                case CategorieLocSeat::VIP:fill = QColor("#2e2008");border = QColor("#c8961e");txtC = QColor(C_GOLD);lbl = "V";break;
                case CategorieLocSeat::CUPLU:fill = QColor("#1e0e3a");border = QColor("#7c3aed");txtC = QColor(C_PURP);lbl = "C";break;
                default:fill = QColor("#0a2210");border = QColor("#15803d");txtC = QColor(C_GRN);break;
                }
            }
            else if (hov) {
                switch (cat) {
                case CategorieLocSeat::VIP:fill = QColor("#4a380a");border = QColor(C_GOLD);txtC = QColor(C_GOLD);break;
                case CategorieLocSeat::CUPLU:fill = QColor("#321866");border = QColor(C_PURP);txtC = QColor(C_PURP);break;
                default:fill = QColor("#133a22");border = QColor(C_GRN);txtC = QColor(C_GRN);break;
                }
            }
            else {
                switch (cat) {
                case CategorieLocSeat::VIP:fill = QColor("#2e1f08");border = QColor("#b07818");txtC = QColor(C_GOLD);lbl = "V";break;
                case CategorieLocSeat::CUPLU:fill = QColor("#200e42");border = QColor("#5b21b6");txtC = QColor(C_PURP);lbl = "C";break;
                default:fill = QColor("#0d2618");border = QColor("#166534");txtC = QColor(C_GRN);break;
                }
            }
            QRect sr = seatRect(r, c);
            p.setBrush(fill); p.setPen(QPen(border, isSel ? 1.8 : 1.2)); p.drawRoundedRect(sr.adjusted(1, 1, -1, -1), 5, 5);
            if (!lbl.isEmpty()) { p.setPen(txtC);p.setFont(QFont("Segoe UI", 9, isSel ? QFont::Bold : QFont::Normal));p.drawText(sr, Qt::AlignCenter, lbl); }
        }
    }
}


MenuCard::MenuCard(const QPixmap& icon, const QString& title, const QString& desc, bool gold, QWidget* parent)
    :QFrame(parent) {
    setMinimumSize(180, 120);setMaximumSize(380, 260);setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setCursor(Qt::PointingHandCursor);
    if (gold) {
        nor_ = "QFrame{background:#16120a;border:1.5px solid #3a2a0a;border-left:3px solid " C_GOLD ";border-radius:0 14px 14px 0;}";
        hov_ = "QFrame{background:#1e180d;border:1.5px solid " C_GOLD ";border-left:3px solid " C_GOLD ";border-radius:0 14px 14px 0;}";
    }
    else {
        nor_ = "QFrame{background:" C_CARD ";border:1.5px solid " C_BOR2 ";border-left:3px solid " C_RED ";border-radius:0 14px 14px 0;}";
        hov_ = "QFrame{background:" C_RAISE ";border:1.5px solid " C_RED ";border-left:3px solid " C_RED ";border-radius:0 14px 14px 0;}";
    }
    setStyleSheet(nor_);
    auto* vb = new QVBoxLayout(this);vb->setContentsMargins(20, 20, 20, 16);vb->setSpacing(6);vb->setAlignment(Qt::AlignVCenter);
    // SVG icon
    auto* ico = new QLabel;ico->setPixmap(icon);ico->setFixedSize(32, 32);
    ico->setStyleSheet("background:transparent;border:none;");
    ico->setAttribute(Qt::WA_TransparentForMouseEvents);vb->addWidget(ico);
    auto mk = [](const QString& t, const QString& ss) {auto* l = new QLabel(t);l->setStyleSheet(ss);l->setAttribute(Qt::WA_TransparentForMouseEvents);return l;};
    vb->addWidget(mk(title, gold ? "font-size:15px;font-weight:700;color:" C_GOLD ";background:transparent;border:none;"
        : "font-size:15px;font-weight:700;color:" C_TXT ";background:transparent;border:none;"));
    auto* d = mk(desc, "font-size:11px;color:" C_MUT ";background:transparent;border:none;");
    d->setWordWrap(true);vb->addWidget(d);
}


const QStringList StepBar::STEPS = { "Film","Data","Programare","Locuri","Food","Confirmare" };
StepBar::StepBar(QWidget* p) :QWidget(p) { setFixedHeight(52);setStyleSheet("background:#0d0d16;border-bottom:1px solid " C_BOR2 ";"); }
void StepBar::setStep(int s) { current_ = s;update(); }
void StepBar::paintEvent(QPaintEvent*) {
    QPainter p(this);p.setRenderHint(QPainter::Antialiasing);
    p.fillRect(rect(), QColor("#0d0d16"));
    int n = STEPS.size(), w = width(), cy = height() / 2 - 4, spc = w / n;
    for (int i = 0;i < n;i++) {
        int x = spc / 2 + i * spc;
        if (i < n - 1) { int nx = spc / 2 + (i + 1) * spc;p.setPen(QPen(i < current_ ? QColor(C_GRN) : QColor(C_BOR), 2));p.drawLine(x + 11, cy, nx - 11, cy); }
        QColor cc;int cr;
        if (i < current_) { cc = QColor(C_GRN);cr = 8; }
        else if (i == current_) { cc = QColor(C_RED);cr = 10; }
        else { cc = QColor(C_BOR);cr = 8; }
        p.setBrush(cc);p.setPen(Qt::NoPen);p.drawEllipse(QPoint(x, cy), cr, cr);
        if (i < current_) { p.setBrush(Qt::white);p.drawEllipse(QPoint(x, cy), 3, 3); }
        p.setPen(i == current_ ? QColor(Qt::white) : i < current_ ? QColor(C_GRN) : QColor(C_DIM));
        p.setFont(QFont("Segoe UI", i == current_ ? 8 : 7, i == current_ ? QFont::Bold : QFont::Normal));
        p.drawText(QRect(x - 36, cy + 13, 72, 16), Qt::AlignHCenter, STEPS[i]);
    }
}


LocationCard::LocationCard(const CinemaLocation& loc, bool isCurrentCity, QWidget* parent)
    :QFrame(parent), id_(QString::fromStdString(loc.id)) {
    setFixedHeight(116);setCursor(Qt::PointingHandCursor);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    if (isCurrentCity) {
        nor_ = "QFrame{background:#0d1a10;border:1.5px solid #166534;border-radius:12px;}";
        hov_ = "QFrame{background:#0f2214;border:2px solid " C_GRN ";border-radius:12px;}";
    }
    else {
        nor_ = "QFrame{background:" C_CARD ";border:1.5px solid " C_BOR2 ";border-radius:12px;}";
        hov_ = "QFrame{background:" C_RAISE ";border:2px solid " C_RED ";border-radius:12px;}";
    }
    setStyleSheet(nor_);
    auto* hb = new QHBoxLayout(this);hb->setContentsMargins(18, 14, 18, 14);hb->setSpacing(14);


    auto* ico = new QLabel(isCurrentCity ? "📍" : "🎬");
    ico->setStyleSheet("font-size:26px;background:transparent;border:none;");
    ico->setAttribute(Qt::WA_TransparentForMouseEvents);hb->addWidget(ico);


    auto* tv = new QVBoxLayout;tv->setSpacing(3);
    auto* n = new QLabel(QString::fromStdString(loc.nume));
    n->setStyleSheet(isCurrentCity
        ? "font-size:15px;font-weight:700;color:" C_GRN ";background:transparent;border:none;"
        : "font-size:15px;font-weight:700;color:" C_TXT ";background:transparent;border:none;");
    n->setAttribute(Qt::WA_TransparentForMouseEvents);tv->addWidget(n);
    auto* a = new QLabel(QString::fromStdString(loc.oras) + " · " + QString::fromStdString(loc.adresa));
    a->setStyleSheet("font-size:12px;color:" C_MUT ";background:transparent;border:none;");
    a->setAttribute(Qt::WA_TransparentForMouseEvents);tv->addWidget(a);
    QString hallInfo = QString::number(loc.sali.size()) + " " + (loc.sali.size() == 1 ? "sală" : "săli");
    auto* hi = new QLabel(hallInfo);
    hi->setStyleSheet("font-size:11px;color:" C_DIM ";background:transparent;border:none;");
    hi->setAttribute(Qt::WA_TransparentForMouseEvents);tv->addWidget(hi);
    hb->addLayout(tv, 1);


    auto* rv = new QVBoxLayout;rv->setSpacing(4);rv->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    if (isCurrentCity) {
        auto* badge = new QLabel("Orașul tău");
        badge->setStyleSheet("background:#166534;color:#4ade80;border-radius:6px;padding:3px 10px;"
            "font-size:11px;font-weight:700;");
        badge->setAttribute(Qt::WA_TransparentForMouseEvents);rv->addWidget(badge);
    }


    QString tierTxt, tierSS;
    switch (loc.tier) {
    case CinemaTier::PREMIUM:
        tierTxt = "⭐ Premium"; tierSS = "background:#2d1f08;color:#f5a623;border-radius:6px;padding:3px 10px;font-size:11px;font-weight:700;"; break;
    case CinemaTier::BASIC:
        tierTxt = "🎟 Basic"; tierSS = "background:#0f172a;color:#64748b;border-radius:6px;padding:3px 10px;font-size:11px;font-weight:700;"; break;
    default:
        tierTxt = "🎬 Standard"; tierSS = "background:#111827;color:#6b7280;border-radius:6px;padding:3px 10px;font-size:11px;font-weight:700;"; break;
    }
    auto* tierBadge = new QLabel(tierTxt);
    tierBadge->setStyleSheet(tierSS);
    tierBadge->setAttribute(Qt::WA_TransparentForMouseEvents);rv->addWidget(tierBadge);


    if (loc.pretMultiplicator != 1.0) {
        QString hint = loc.pretMultiplicator > 1.0
            ? QString("+%1%").arg((int)((loc.pretMultiplicator - 1.0) * 100))
            : QString("-%1%").arg((int)((1.0 - loc.pretMultiplicator) * 100));
        auto* pml = new QLabel(hint + " față de standard");
        pml->setStyleSheet("font-size:10px;color:#50536a;background:transparent;border:none;");
        pml->setAttribute(Qt::WA_TransparentForMouseEvents);rv->addWidget(pml);
    }
    hb->addLayout(rv);
}


QFrame* MainWindow::filmMiniCard(QLabel*& lbl) {
    auto* f = new QFrame;f->setFixedHeight(42);
    f->setStyleSheet("QFrame{background:#111118;border-left:3px solid " C_RED ";border-radius:0;}");
    auto* hb = new QHBoxLayout(f);hb->setContentsMargins(14, 0, 16, 0);hb->setSpacing(8);
    auto* ico = new QLabel("🎬");ico->setStyleSheet("font-size:14px;background:transparent;border:none;");
    lbl = new QLabel("—");lbl->setStyleSheet("color:" C_MUT ";font-size:13px;background:transparent;border:none;");
    hb->addWidget(ico);hb->addWidget(lbl, 1);return f;
}


void MainWindow::goTo(Page p) {
    navbar->setVisible(p != P_LOGIN && p != P_LOCATION);
    bool inRes = (p == P_FILM_DETAIL) || (p >= P_RESERVE_DATA && p <= P_RESERVE_CONFIRM && p != P_RESERVE_ORA);
    stepBar->setVisible(inRes);
    if (p == P_FILM_DETAIL)         stepBar->setStep(0);
    else if (p == P_RESERVE_DATA)   stepBar->setStep(1);
    else if (p == P_RESERVE_SALA)   stepBar->setStep(2);
    else if (p == P_RESERVE_LOC)    stepBar->setStep(3);
    else if (p == P_RESERVE_FOOD)   stepBar->setStep(4);
    else if (p == P_RESERVE_CONFIRM)stepBar->setStep(5);
    if (p == P_MAIN) {
        if (heroTimer && !heroTimer->isActive()) heroTimer->start();
    }
    else {
        if (heroTimer && heroTimer->isActive()) heroTimer->stop();
    }
    stack->setCurrentIndex(p);
    auto* eff = new QGraphicsOpacityEffect(stack);stack->setGraphicsEffect(eff);
    auto* anim = new QPropertyAnimation(eff, "opacity", this);
    anim->setDuration(160);anim->setStartValue(0.0);anim->setEndValue(1.0);
    anim->setEasingCurve(QEasingCurve::OutCubic);
    connect(anim, &QPropertyAnimation::finished, this, [this] {stack->setGraphicsEffect(nullptr);});
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}


MainWindow::MainWindow(QWidget* parent) :QMainWindow(parent) {
    setWindowTitle(QString("Cinema Lumi") + QChar(0x00E8) + QString("re"));setMinimumSize(980, 700);
    {
        QString svgStr = "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"64\" height=\"64\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"#e50914\" stroke-width=\"1.8\" stroke-linecap=\"round\" stroke-linejoin=\"round\">"
            "<rect x=\"2\" y=\"2\" width=\"20\" height=\"20\" rx=\"2.18\"/>"
            "<line x1=\"7\" x2=\"7\" y1=\"2\" y2=\"22\"/>"
            "<line x1=\"17\" x2=\"17\" y1=\"2\" y2=\"22\"/>"
            "<line x1=\"2\" x2=\"22\" y1=\"12\" y2=\"12\"/>"
            "<line x1=\"2\" x2=\"7\" y1=\"7\" y2=\"7\"/>"
            "<line x1=\"17\" x2=\"22\" y1=\"7\" y2=\"7\"/>"
            "<line x1=\"17\" x2=\"22\" y1=\"17\" y2=\"17\"/>"
            "<line x1=\"2\" x2=\"7\" y1=\"17\" y2=\"17\"/>"
            "</svg>";
        QSvgRenderer rend(svgStr.toUtf8());
        QPixmap ico(64, 64); ico.fill(Qt::transparent);
        QPainter ip(&ico); rend.render(&ip); ip.end();
        setWindowIcon(QIcon(ico));
    }
#ifdef Q_OS_WIN
    {
        HWND hwnd = (HWND)winId();
        BOOL dark = TRUE;
        DwmSetWindowAttribute(hwnd, 20, &dark, sizeof(dark));
    }
#endif
    QPalette pal;
    pal.setColor(QPalette::Window, QColor(C_BG));pal.setColor(QPalette::WindowText, Qt::white);
    pal.setColor(QPalette::Base, QColor(C_SURF));pal.setColor(QPalette::Text, Qt::white);
    pal.setColor(QPalette::Button, QColor(C_RAISE));pal.setColor(QPalette::ButtonText, Qt::white);
    pal.setColor(QPalette::Highlight, QColor(C_RED));pal.setColor(QPalette::HighlightedText, Qt::white);
    setPalette(pal);
    netManager = new QNetworkAccessManager(this);
    std::srand((unsigned)time(nullptr));
    cinema.init();

    auto* central = new QWidget;central->setStyleSheet("background:" C_BG ";");
    auto* vb = new QVBoxLayout(central);vb->setContentsMargins(0, 0, 0, 0);vb->setSpacing(0);
    navbar = buildNavbar();vb->addWidget(navbar);
    stepBar = new StepBar;stepBar->hide();vb->addWidget(stepBar);
    stack = new QStackedWidget;
    stack->addWidget(buildLoginPage());

    stack->addWidget(buildLocationPage());

    stack->addWidget(buildMainPage());

    stack->addWidget(buildReserveFilmPage());

    stack->addWidget(buildReserveShowingPage());

    stack->addWidget(buildReserveDataPage());

    stack->addWidget(new QWidget());

    stack->addWidget(buildReserveLocPage());

    stack->addWidget(buildReserveFoodPage());

    stack->addWidget(buildReserveConfirmPage());

    stack->addWidget(buildBrowsePage());

    stack->addWidget(buildMapPage());

    stack->addWidget(buildCancelPage());

    stack->addWidget(buildProfilePage());

    stack->addWidget(buildAdminPage());

    stack->addWidget(buildFilmDetailPage());

    stack->addWidget(buildConfirmDonePage());

    vb->addWidget(stack);setCentralWidget(central);
    goTo(P_LOGIN);
}
MainWindow::~MainWindow() {}


static QWidget* pageWrap(QVBoxLayout*& out) {
    auto* w = new QWidget;w->setStyleSheet("background:" C_BG ";");
    out = new QVBoxLayout(w);out->setContentsMargins(40, 24, 40, 28);out->setSpacing(0);return w;
}
static void addHeader(QVBoxLayout* vb, const QString& title, const QString& sub, QPushButton* bk) {
    auto* row = new QHBoxLayout;row->setSpacing(14);row->addWidget(bk);
    auto* col = new QVBoxLayout;col->setSpacing(2);
    auto* ttl = new QLabel(title);ttl->setStyleSheet("font-size:22px;font-weight:700;color:" C_TXT ";");
    col->addWidget(ttl);
    if (!sub.isEmpty()) { auto* s = new QLabel(sub);s->setStyleSheet("font-size:12px;color:" C_MUT ";");col->addWidget(s); }
    row->addLayout(col);row->addStretch();
    vb->addLayout(row);vb->addSpacing(4);vb->addWidget(hLine());vb->addSpacing(16);
}


QWidget* MainWindow::buildNavbar() {
    auto* w = new QWidget;w->setFixedHeight(58);
    w->setStyleSheet("background:#0d0d16;border-bottom:1px solid " C_BOR2 ";");
    auto* hb = new QHBoxLayout(w);hb->setContentsMargins(28, 0, 24, 0);hb->setSpacing(0);
    auto* logoIco = new QLabel;logoIco->setPixmap(iconFilm(20, "#e50914"));
    logoIco->setStyleSheet("background:transparent;border:none;");
    auto* logoTxt = new QLabel(QString("LUMI") + QChar(0x00C8) + QString("RE"));
    logoTxt->setStyleSheet("font-size:17px;font-weight:800;color:#e50914;letter-spacing:3px;background:transparent;");
    hb->addWidget(logoIco);hb->addSpacing(6);hb->addWidget(logoTxt);hb->addSpacing(16);
    lblNavLocation = new QLabel;
    lblNavLocation->setStyleSheet("color:#166534;font-size:12px;background:#0d1a10;border:1px solid #166534;"
        "border-radius:6px;padding:2px 10px;");
    hb->addWidget(lblNavLocation);hb->addStretch();
    lblNavUser = new QLabel;lblNavUser->setStyleSheet("color:" C_MUT ";font-size:13px;margin-right:16px;");
    hb->addWidget(lblNavUser);


    auto* btnLoc = ghostBtn("📍 Schimbă locația", 34);btnLoc->setFixedWidth(160);
    connect(btnLoc, &QPushButton::clicked, this, [this] {


        cinema.setLocatieCurenta("");
        onLocationDetected();
        goTo(P_LOCATION);
        });
    hb->addWidget(btnLoc);hb->addSpacing(8);
    auto* bo = ghostBtn("Deconectare", 34);bo->setFixedWidth(120);
    connect(bo, &QPushButton::clicked, this, &MainWindow::onLogout);hb->addWidget(bo);
    return w;
}


QWidget* MainWindow::buildLoginPage() {
    auto* w = new QWidget;w->setStyleSheet("background:" C_BG ";");
    auto* vb = new QVBoxLayout(w);vb->setAlignment(Qt::AlignCenter);vb->setSpacing(0);
    auto mk = [](const QString& t, const QString& ss) {auto* l = new QLabel(t);l->setStyleSheet(ss);l->setAlignment(Qt::AlignCenter);return l;};
    {
        auto* icoLbl = new QLabel;
        icoLbl->setPixmap(iconFilm(64, "#e50914"));
        icoLbl->setAlignment(Qt::AlignCenter);
        icoLbl->setStyleSheet("background:transparent;border:none;");
        vb->addWidget(icoLbl);
    }
    vb->addWidget(mk(QString("LUMI") + QChar(0x00C8) + QString("RE"), "font-size:32px;font-weight:800;color:" C_RED ";letter-spacing:6px;margin-bottom:4px;"));
    vb->addWidget(mk("Rezervă-ți locul. Trăiește povestea.", "font-size:13px;color:" C_DIM ";margin-bottom:36px;"));

    auto* card = new QFrame;card->setFixedWidth(430);
    card->setStyleSheet("QFrame{background:" C_CARD ";border-radius:18px;border:1px solid " C_BOR2 ";} QLabel{background:transparent;border:none;padding:0;}");
    auto* cardStack = new QStackedWidget(card);
    auto* cardLayout = new QVBoxLayout(card);cardLayout->setContentsMargins(0, 0, 0, 0);cardLayout->addWidget(cardStack);

    // ── Login panel ──────────────────────────────────────────────────────────
    auto* loginPanel = new QWidget;
    auto* fl = new QVBoxLayout(loginPanel);fl->setContentsMargins(38, 36, 38, 36);fl->setSpacing(14);
    auto* ct = new QLabel("Bun venit înapoi");ct->setStyleSheet("font-size:21px;font-weight:700;color:" C_TXT ";margin-bottom:6px;background:transparent;border:none;padding:0;");fl->addWidget(ct);
    auto* cs = new QLabel("Intră în cont pentru a face rezervări");cs->setStyleSheet("font-size:12px;color:" C_MUT ";margin-bottom:10px;background:transparent;border:none;padding:0;");fl->addWidget(cs);
    fl->addWidget(secLbl("USERNAME"));
    leUser = new QLineEdit;leUser->setPlaceholderText("ex: ion_popescu");leUser->setStyleSheet(inputSS());leUser->setFixedHeight(48);fl->addWidget(leUser);
    fl->addWidget(secLbl("PAROLĂ"));
    lePass = new QLineEdit;lePass->setPlaceholderText("••••••••");lePass->setEchoMode(QLineEdit::Password);lePass->setStyleSheet(inputSS());lePass->setFixedHeight(48);fl->addWidget(lePass);
    fl->addSpacing(6);
    auto* bL = redBtn("Intră în cont", 50);fl->addWidget(bL);
    auto* dr = new QHBoxLayout;
    auto mkL = [] {auto* f = new QFrame;f->setFrameShape(QFrame::HLine);f->setStyleSheet("background:#2a2a35;max-height:1px;border:none;");return f;};
    auto* orL = new QLabel("  sau  ");orL->setStyleSheet("color:" C_DIM ";font-size:12px;");
    dr->addWidget(mkL(), 1);dr->addWidget(orL);dr->addWidget(mkL(), 1);fl->addLayout(dr);
    auto* bToReg = ghostBtn("Creează cont nou", 46);fl->addWidget(bToReg);
    cardStack->addWidget(loginPanel);

    // ── Register panel ───────────────────────────────────────────────────────
    auto* regPanel = new QWidget;regPanel->setStyleSheet("background:transparent;");
    auto* fr = new QVBoxLayout(regPanel);fr->setContentsMargins(38, 36, 38, 36);fr->setSpacing(14);
    auto* rt = new QLabel("Creează cont nou");rt->setStyleSheet("font-size:21px;font-weight:700;color:" C_TXT ";margin-bottom:6px;background:transparent;border:none;padding:0;");fr->addWidget(rt);
    auto* rs = new QLabel("Completează datele pentru a te înregistra");rs->setStyleSheet("font-size:12px;color:" C_MUT ";margin-bottom:10px;background:transparent;border:none;padding:0;");fr->addWidget(rs);
    fr->addWidget(secLbl("USERNAME"));
    auto* leRegUser = new QLineEdit;leRegUser->setPlaceholderText("ex: ion_popescu");leRegUser->setStyleSheet(inputSS());leRegUser->setFixedHeight(48);fr->addWidget(leRegUser);
    fr->addWidget(secLbl("PAROLĂ"));
    auto* leRegPass = new QLineEdit;leRegPass->setPlaceholderText("••••••••");leRegPass->setEchoMode(QLineEdit::Password);leRegPass->setStyleSheet(inputSS());leRegPass->setFixedHeight(48);fr->addWidget(leRegPass);
    fr->addWidget(secLbl("CONFIRMĂ PAROLA"));
    auto* leRegConf = new QLineEdit;leRegConf->setPlaceholderText("••••••••");leRegConf->setEchoMode(QLineEdit::Password);leRegConf->setStyleSheet(inputSS());leRegConf->setFixedHeight(48);fr->addWidget(leRegConf);
    // Error label
    auto* regErr = new QLabel;regErr->setStyleSheet("font-size:12px;color:#f87171;background:transparent;");regErr->setVisible(false);fr->addWidget(regErr);
    fr->addSpacing(4);
    auto* bReg = redBtn("Creează cont", 50);fr->addWidget(bReg);
    auto* bBack = ghostBtn("← Înapoi la autentificare", 42);fr->addWidget(bBack);
    cardStack->addWidget(regPanel);

    vb->addWidget(card, 0, Qt::AlignCenter);

    // ── Connections ──────────────────────────────────────────────────────────
    connect(lePass, &QLineEdit::returnPressed, this, &MainWindow::onLogin);
    connect(bL, &QPushButton::clicked, this, &MainWindow::onLogin);

    connect(bToReg, &QPushButton::clicked, this, [cardStack, leRegUser, leRegPass, leRegConf, regErr] {
        leRegUser->clear();leRegPass->clear();leRegConf->clear();
        regErr->setVisible(false);
        cardStack->setCurrentIndex(1);
        });
    connect(bBack, &QPushButton::clicked, this, [cardStack] { cardStack->setCurrentIndex(0); });

    connect(bReg, &QPushButton::clicked, this, [this, leRegUser, leRegPass, leRegConf, regErr, cardStack] {
        QString usr = leRegUser->text().trimmed();
        QString par = leRegPass->text();
        QString conf = leRegConf->text();
        if (usr.isEmpty()) { regErr->setText("Username-ul nu poate fi gol."); regErr->setVisible(true); return; }
        if (par.length() < 4) { regErr->setText("Parola trebuie să aibă cel puțin 4 caractere."); regErr->setVisible(true); return; }
        if (par != conf) { regErr->setText("Parolele nu coincid."); regErr->setVisible(true); return; }
        if (cinema.inregistrare(usr.toStdString(), par.toStdString())) {
            cardStack->setCurrentIndex(0);
            leUser->setText(usr);
            showToast("Cont creat! Bun venit, " + usr + "!", false);
            onLocationDetected(); goTo(P_LOCATION);
        }
        else {
            regErr->setText("Username-ul este deja folosit.");
            regErr->setVisible(true);
        }
        });
    return w;
}


QWidget* MainWindow::buildLocationPage() {
    auto* w = new QWidget;w->setStyleSheet("background:" C_BG ";");
    auto* vb = new QVBoxLayout(w);vb->setContentsMargins(60, 40, 60, 40);vb->setSpacing(0);



    auto* ttl = new QLabel("Alege Cinema-ul");
    ttl->setStyleSheet("font-size:28px;font-weight:800;color:" C_TXT ";");vb->addWidget(ttl);
    lblDetectedCity = new QLabel("Se detectează locația...");
    lblDetectedCity->setStyleSheet("font-size:14px;color:" C_MUT ";margin-top:6px;margin-bottom:20px;");
    vb->addWidget(lblDetectedCity);vb->addWidget(hLine());vb->addSpacing(20);



    auto* scroll = new QScrollArea;scroll->setWidgetResizable(true);
    scroll->setStyleSheet("QScrollArea{background:" C_BG ";border:none;}");
    auto* inner = new QWidget;inner->setStyleSheet("background:" C_BG ";");
    locationContainer = inner;
    auto* innerVb = new QVBoxLayout(inner);innerVb->setContentsMargins(0, 0, 0, 0);innerVb->setSpacing(10);
    scroll->setWidget(inner);vb->addWidget(scroll, 1);



    vb->addSpacing(16);
    btnDetectLocation = ghostBtn("Redetectează locația", 40);btnDetectLocation->setFixedWidth(220);
    connect(btnDetectLocation, &QPushButton::clicked, this, &MainWindow::onLocationDetected);
    vb->addWidget(btnDetectLocation, 0, Qt::AlignCenter);
    return w;
}


QWidget* MainWindow::buildMainPage() {
    auto* w = new QWidget;w->setStyleSheet("background:" C_BG ";");
    auto* vb = new QVBoxLayout(w);vb->setContentsMargins(40, 28, 40, 28);vb->setSpacing(0);
    lblWelcome = new QLabel("Bun venit!");
    lblWelcome->setStyleSheet("font-size:27px;font-weight:700;color:" C_TXT ";");lblWelcome->setTextFormat(Qt::RichText);
    vb->addWidget(lblWelcome);
    auto* sub = new QLabel("Ce ai vrea să faci astăzi?");
    sub->setStyleSheet("font-size:13px;color:" C_MUT ";margin-top:4px;margin-bottom:20px;");vb->addWidget(sub);



    heroFrame = new QFrame;heroFrame->setFixedHeight(162);
    heroFrame->setStyleSheet("QFrame{background:qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 #1a0505,stop:1 #0d0d16);"
        "border:1px solid " C_BOR2 ";border-radius:14px;}");
    auto* hHB = new QHBoxLayout(heroFrame);hHB->setContentsMargins(28, 20, 28, 20);hHB->setSpacing(0);
    auto* hLeft = new QVBoxLayout;hLeft->setSpacing(8);
    lblHeroBadge = new QLabel("ACUM ÎN CINEMA");
    lblHeroBadge->setStyleSheet("font-size:10px;font-weight:700;color:" C_RED ";letter-spacing:2px;background:#1f0505;border:1px solid #5a0a0a;border-radius:4px;padding:3px 8px;");
    hLeft->addWidget(lblHeroBadge, 0, Qt::AlignLeft);
    lblHeroTitle = new QLabel("—");lblHeroTitle->setStyleSheet("font-size:26px;font-weight:800;color:white;");lblHeroTitle->setWordWrap(true);hLeft->addWidget(lblHeroTitle);
    lblHeroInfo = new QLabel("—");lblHeroInfo->setStyleSheet("font-size:13px;color:" C_MUT ";");hLeft->addWidget(lblHeroInfo);
    hLeft->addStretch();
    auto* bHero = new QPushButton("Rezervă acum");bHero->setFixedHeight(38);bHero->setFixedWidth(170);
    bHero->setCursor(Qt::PointingHandCursor);
    bHero->setStyleSheet("QPushButton{background:" C_RED ";color:white;border:none;border-radius:7px;font-size:13px;font-weight:700;}QPushButton:hover{background:" C_RHOV ";}");
    connect(bHero, &QPushButton::clicked, this, &MainWindow::onNewReservation);
    hLeft->addWidget(bHero, 0, Qt::AlignLeft);hHB->addLayout(hLeft, 1);


    lblHeroPoster = new QLabel;lblHeroPoster->setFixedSize(82, 122);
    lblHeroPoster->setAlignment(Qt::AlignCenter);
    lblHeroPoster->setStyleSheet("background:#1a0505;border-radius:8px;border:1px solid #3a1010;font-size:24px;color:#3a1010;");
    lblHeroPoster->setText("🎬");
    hHB->addSpacing(20);hHB->addWidget(lblHeroPoster, 0, Qt::AlignVCenter);
    vb->addWidget(heroFrame);vb->addSpacing(20);



    auto* grid = new QGridLayout;grid->setSpacing(16);
    grid->setColumnStretch(0, 1);grid->setColumnStretch(1, 1);grid->setColumnStretch(2, 1);
    struct E { QPixmap ico;QString t, d;void(MainWindow::* s)(); };
    std::vector<E> entries = {
        {iconTicket(),"Rezervare Nouă","Alege film, sală și loc",&MainWindow::onNewReservation},
        {iconSearch(),"Browse Genuri","Descoperă după gen",&MainWindow::onBrowseGenuri},
        {iconGrid(),  "Hartă Sală","Verifică disponibilitatea",&MainWindow::onHartaSala},
        {iconXCircle(),"Anulare","Anulează o rezervare",&MainWindow::onAnulareRezervare},
        {iconUser(),  "Profilul Meu","Istoric și setări",&MainWindow::onProfilMeu},
    };
    int col = 0, row = 0;
    for (auto& e : entries) { auto* c = new MenuCard(e.ico, e.t, e.d, false, w);connect(c, &MenuCard::clicked, this, e.s);grid->addWidget(c, row, col);if (++col == 3) { col = 0;row++; } }
    cardAdmin = new MenuCard(iconSettings(), "Panou Admin", "Gestionare sistem", true, w);
    connect(cardAdmin, &MenuCard::clicked, this, &MainWindow::onAdminPanel);
    cardAdmin->hide();grid->addWidget(cardAdmin, row, col);
    vb->addLayout(grid, 1);



    upcomingSection = new QFrame;upcomingSection->setStyleSheet("QFrame{background:transparent;}");
    auto* usVb = new QVBoxLayout(upcomingSection);usVb->setContentsMargins(0, 16, 0, 0);usVb->setSpacing(8);
    auto* usTitle = new QLabel("ÎN CURÂND");usTitle->setStyleSheet("font-size:11px;font-weight:700;color:" C_DIM ";letter-spacing:2px;");usVb->addWidget(usTitle);
    auto* usScroll = new QScrollArea;usScroll->setFixedHeight(108);usScroll->setWidgetResizable(true);
    usScroll->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    usScroll->setStyleSheet("QScrollArea{background:transparent;border:none;}");
    upcomingContainer = new QWidget;upcomingContainer->setStyleSheet("background:transparent;");
    usScroll->setWidget(upcomingContainer);usVb->addWidget(usScroll);
    vb->addWidget(upcomingSection, 0);

    heroTimer = new QTimer(this);heroTimer->setInterval(4000);
    connect(heroTimer, &QTimer::timeout, this, &MainWindow::refreshHeroBanner);
    return w;
}


QWidget* MainWindow::buildFilmDetailPage() {
    QVBoxLayout* vb;auto* w = pageWrap(vb);
    auto* bk = ghostBtn("← Înapoi", 36);
    connect(bk, &QPushButton::clicked, this, [this] {goTo(detailFromPage);});
    auto* bkRow = new QHBoxLayout;bkRow->addWidget(bk);bkRow->addStretch();
    vb->addLayout(bkRow);vb->addSpacing(8);
    detailTitle = new QLabel("—");detailTitle->setStyleSheet("font-size:28px;font-weight:800;color:" C_TXT ";");detailTitle->setWordWrap(true);vb->addWidget(detailTitle);
    detailGenreRating = new QLabel;detailGenreRating->setStyleSheet("font-size:13px;color:" C_MUT ";margin-bottom:12px;");vb->addWidget(detailGenreRating);


    auto* bTrailer = ghostBtn("Trailer", 34);bTrailer->setFixedWidth(120);
    connect(bTrailer, &QPushButton::clicked, this, [this]() {
        if (!rezFilm) return;
        QString ytId = QString::fromStdString(rezFilm->getTrailerYtId());
        QString url = ytId.isEmpty()
            ? "https://www.youtube.com/results?search_query="
            + QUrl::toPercentEncoding(QString::fromStdString(rezFilm->getTitlu()) + " trailer oficial")
            : "https://www.youtube.com/watch?v=" + ytId;
        QDesktopServices::openUrl(QUrl(url));
        });
    auto* trRow = new QHBoxLayout;trRow->addWidget(bTrailer);trRow->addStretch();
    vb->addLayout(trRow);vb->addSpacing(10);
    vb->addWidget(hLine());vb->addSpacing(16);
    auto* split = new QHBoxLayout;split->setSpacing(32);split->setAlignment(Qt::AlignTop);
    auto* leftVb = new QVBoxLayout;leftVb->setSpacing(16);
    detailDesc = new QLabel;detailDesc->setWordWrap(true);detailDesc->setStyleSheet("font-size:14px;color:#c8c8d8;line-height:1.7;");detailDesc->setTextFormat(Qt::PlainText);leftVb->addWidget(detailDesc);
    detailMeta = new QLabel;detailMeta->setTextFormat(Qt::RichText);
    detailMeta->setStyleSheet("font-size:13px;color:" C_TXT ";line-height:1.9;background:" C_CARD ";border:1px solid " C_BOR2 ";border-radius:12px;padding:18px 22px;");
    leftVb->addWidget(detailMeta);
    auto* bookRow = new QHBoxLayout;bookRow->setSpacing(16);
    detailPrice = new QLabel;detailPrice->setStyleSheet("font-size:22px;font-weight:800;color:" C_RED ";");bookRow->addWidget(detailPrice);
    btnDetailBook = redBtn("Rezervă acum", 50);btnDetailBook->setFixedWidth(200);
    connect(btnDetailBook, &QPushButton::clicked, this, &MainWindow::onProceedFromDetail);bookRow->addWidget(btnDetailBook);
    detailUnavail = new QLabel;detailUnavail->setStyleSheet("font-size:13px;color:" C_GOLD ";");bookRow->addWidget(detailUnavail);bookRow->addStretch();
    leftVb->addLayout(bookRow);split->addLayout(leftVb, 3);
    auto* rightVb = new QVBoxLayout;rightVb->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
    detailPoster = new QLabel;detailPoster->setFixedSize(220, 330);detailPoster->setAlignment(Qt::AlignCenter);
    detailPoster->setStyleSheet("background:" C_CARD ";border:1px solid " C_BOR2 ";border-radius:12px;font-size:48px;color:" C_DIM ";");
    rightVb->addWidget(detailPoster);split->addLayout(rightVb, 1);
    auto* scrollArea = new QScrollArea;scrollArea->setWidgetResizable(true);scrollArea->setStyleSheet("QScrollArea{background:" C_BG ";border:none;}");
    auto* inner = new QWidget;inner->setStyleSheet("background:" C_BG ";");
    auto* innerVb = new QVBoxLayout(inner);innerVb->setContentsMargins(0, 0, 0, 0);innerVb->addLayout(split);innerVb->addStretch();
    scrollArea->setWidget(inner);vb->addWidget(scrollArea, 1);
    return w;
}


QWidget* MainWindow::buildReserveFilmPage() {
    QVBoxLayout* vb;auto* w = pageWrap(vb);
    auto* bk = ghostBtn("← Înapoi", 36);addHeader(vb, "Alege Filmul", "Caută sau filtrează titluri", bk);
    auto* fRow = new QHBoxLayout;fRow->setSpacing(10);
    leSearch = new QLineEdit;leSearch->setPlaceholderText("🔍  Caută titlu…");leSearch->setStyleSheet(inputSS());leSearch->setFixedHeight(42);
    cbFilmGenre = new QComboBox;cbFilmGenre->setMinimumWidth(140);cbFilmGenre->setStyleSheet(comboSS());
    btnFilm3D = ghostBtn("Format: Toate", 38);btnFilm3D->setFixedWidth(130);
    fRow->addWidget(leSearch, 1);fRow->addWidget(cbFilmGenre);fRow->addWidget(btnFilm3D);
    vb->addLayout(fRow);vb->addSpacing(10);
    filmListStack = new QStackedWidget;
    lwFilms = new QListWidget;lwFilms->setStyleSheet(listSS());lwFilms->setFont(QFont("Segoe UI", 13));
    filmListStack->addWidget(lwFilms);
    auto* emptyW = new QWidget;emptyW->setStyleSheet("background:" C_BG ";");
    auto* eVb = new QVBoxLayout(emptyW);eVb->setAlignment(Qt::AlignCenter);
    auto* eIco = new QLabel("🎬");eIco->setStyleSheet("font-size:48px;");eIco->setAlignment(Qt::AlignCenter);
    lblFilmsEmpty = new QLabel("Niciun film găsit");lblFilmsEmpty->setStyleSheet("font-size:16px;font-weight:600;color:" C_MUT ";margin-top:8px;");lblFilmsEmpty->setAlignment(Qt::AlignCenter);
    auto* eSub = new QLabel("Modifică filtrele sau termenul de căutare");eSub->setStyleSheet("font-size:12px;color:" C_DIM ";");eSub->setAlignment(Qt::AlignCenter);
    eVb->addWidget(eIco);eVb->addWidget(lblFilmsEmpty);eVb->addWidget(eSub);
    filmListStack->addWidget(emptyW);vb->addWidget(filmListStack, 1);
    auto* btmRow = new QHBoxLayout;btmRow->addStretch();
    auto* bGo = redBtn("Continuă  →");bGo->setFixedWidth(160);btmRow->addWidget(bGo);
    vb->addSpacing(14);vb->addLayout(btmRow);
    connect(leSearch, &QLineEdit::textChanged, this, &MainWindow::refreshSearchResults);
    connect(cbFilmGenre, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::refreshSearchResults);
    connect(btnFilm3D, &QPushButton::clicked, this, [this] {filmFilter3D = (filmFilter3D + 2) % 3 - 1;btnFilm3D->setText(filmFilter3D == -1 ? "Format: Toate" : filmFilter3D == 0 ? "Format: 2D" : "Format: 3D");refreshSearchResults();});
    connect(bGo, &QPushButton::clicked, this, &MainWindow::onFilmSelected);
    connect(lwFilms, &QListWidget::itemDoubleClicked, this, &MainWindow::onFilmSelected);
    connect(bk, &QPushButton::clicked, this, [this] {goTo(P_MAIN);});
    return w;
}


QWidget* MainWindow::buildReserveShowingPage() {
    QVBoxLayout* vb;auto* w = pageWrap(vb);
    auto* bk = ghostBtn("← Înapoi", 36);
    addHeader(vb, "Alege Programarea", "Selectează ora și sala pentru filmul ales", bk);


    auto* miniW = new QFrame;miniW->setFixedHeight(62);
    miniW->setStyleSheet("QFrame{background:#111118;border-left:3px solid #e50914;border-radius:0;}");
    auto* mhb = new QHBoxLayout(miniW);mhb->setContentsMargins(14, 0, 16, 0);mhb->setSpacing(8);
    auto* mico = new QLabel("🎬");mico->setStyleSheet("font-size:14px;background:transparent;border:none;");
    miniShow = new QLabel("—");
    miniShow->setStyleSheet("color:#8b8fa8;font-size:13px;background:transparent;border:none;");
    miniShow->setWordWrap(true);
    miniShow->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    mhb->addWidget(mico);mhb->addWidget(miniShow, 1);
    vb->addWidget(miniW);vb->addSpacing(10);



    showingsScroll = new QScrollArea;showingsScroll->setWidgetResizable(true);
    showingsScroll->setStyleSheet("QScrollArea{background:#0a0a0f;border:none;}");
    showingsContainer = new QWidget;showingsContainer->setStyleSheet("background:#0a0a0f;");
    showingsScroll->setWidget(showingsContainer);
    vb->addWidget(showingsScroll, 1);

    lblNoShowings = new QLabel("Nu există programări pentru acest film\nîn această zi la locația selectată.");
    lblNoShowings->setAlignment(Qt::AlignCenter);
    lblNoShowings->setStyleSheet("font-size:15px;color:#8b8fa8;margin:40px;");
    lblNoShowings->hide();vb->addWidget(lblNoShowings);

    connect(bk, &QPushButton::clicked, this, [this] {goTo(P_RESERVE_DATA);});
    return w;
}


QWidget* MainWindow::buildReserveDataPage() {
    QVBoxLayout* vb;auto* w = pageWrap(vb);
    auto* bk = ghostBtn("← Înapoi", 36);addHeader(vb, "Alege Data", "Disponibilitate 7 zile", bk);


    auto* miniW = new QFrame;miniW->setFixedHeight(62);
    miniW->setStyleSheet("QFrame{background:#111118;border-left:3px solid #e50914;border-radius:0;}");
    auto* mhb = new QHBoxLayout(miniW);mhb->setContentsMargins(14, 0, 16, 0);mhb->setSpacing(8);
    auto* mico = new QLabel("🎬");mico->setStyleSheet("font-size:14px;background:transparent;border:none;");
    miniData = new QLabel("—");
    miniData->setStyleSheet("color:#8b8fa8;font-size:13px;background:transparent;border:none;");
    miniData->setWordWrap(true);miniData->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    mhb->addWidget(mico);mhb->addWidget(miniData, 1);vb->addWidget(miniW);vb->addSpacing(12);
    lwData = new QListWidget;lwData->setStyleSheet(listSS());lwData->setFont(QFont("Segoe UI", 14));lwData->setSpacing(2);
    auto zile = urmatoroarele7Zile();Data azi = dataAzi();
    for (auto& z : zile) { QString l = QString::fromStdString(numeZiSaptamana(z)) + "  –  " + QString::fromStdString(z.toString());if (z == azi)l += "   ★ Azi";lwData->addItem(l); }
    vb->addWidget(lwData, 1);
    auto* row = new QHBoxLayout;row->addStretch();auto* bGo = redBtn("Continuă  →");bGo->setFixedWidth(160);row->addWidget(bGo);
    vb->addSpacing(14);vb->addLayout(row);
    connect(bGo, &QPushButton::clicked, this, &MainWindow::onDataSelected);
    connect(lwData, &QListWidget::itemDoubleClicked, this, &MainWindow::onDataSelected);
    connect(bk, &QPushButton::clicked, this, [this] {goTo(P_FILM_DETAIL);});return w;
}


QWidget* MainWindow::buildReserveLocPage() {
    QVBoxLayout* vb;auto* w = pageWrap(vb);
    auto* bk = ghostBtn("← Înapoi", 36);addHeader(vb, "Alege Locurile", "Apasă pe locuri libere · max 8 locuri per rezervare", bk);


    auto* miniW2 = new QFrame;miniW2->setFixedHeight(62);
    miniW2->setStyleSheet("QFrame{background:#111118;border-left:3px solid #e50914;border-radius:0;}");
    auto* mhb2 = new QHBoxLayout(miniW2);mhb2->setContentsMargins(14, 0, 16, 0);mhb2->setSpacing(8);
    auto* mico2 = new QLabel("🎬");mico2->setStyleSheet("font-size:14px;background:transparent;border:none;");
    miniLoc = new QLabel("—");miniLoc->setStyleSheet("color:#8b8fa8;font-size:13px;background:transparent;border:none;");
    miniLoc->setWordWrap(true);miniLoc->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    mhb2->addWidget(mico2);mhb2->addWidget(miniLoc, 1);vb->addWidget(miniW2);vb->addSpacing(10);



    auto* legRow = new QHBoxLayout;legRow->setSpacing(20);
    auto mkLeg = [&](const QString& c, const QString& t, QLabel*& cnt)->QWidget* {
        auto* hb = new QHBoxLayout;hb->setSpacing(7);
        auto* sq = new QLabel;sq->setFixedSize(14, 14);sq->setStyleSheet(QString("background:%1;border-radius:3px;").arg(c));
        auto* lb = new QLabel(t);lb->setStyleSheet("color:" C_MUT ";font-size:12px;");
        cnt = new QLabel;cnt->setStyleSheet("color:" C_DIM ";font-size:11px;");
        hb->addWidget(sq);hb->addWidget(lb);hb->addWidget(cnt);
        auto* cw = new QWidget;cw->setLayout(hb);return cw;
        };
    legRow->addWidget(mkLeg(C_GRN, "Standard", lblLegStd));
    legRow->addWidget(mkLeg(C_GOLD, "VIP (×1.5)", lblLegVip));
    legRow->addWidget(mkLeg(C_PURP, "Cuplu (×1.8)", lblLegCpl));
    legRow->addWidget(mkLeg("#252535", "Ocupat", *(new QLabel*)));
    legRow->addWidget(mkLeg(C_BLUE, "Selectat", *(new QLabel*)));
    legRow->addStretch();
    vb->addLayout(legRow);vb->addSpacing(10);

    auto* scroll = new QScrollArea;scroll->setWidgetResizable(true);scroll->setStyleSheet("QScrollArea{background:" C_BG ";border:none;}");
    auto* inner = new QWidget;inner->setStyleSheet("background:" C_BG ";");
    auto* iVb = new QVBoxLayout(inner);iVb->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    seatMapWidget = new SeatMapWidget;iVb->addWidget(seatMapWidget, 0, Qt::AlignHCenter);
    scroll->setWidget(inner);vb->addWidget(scroll, 1);



    auto* statusRow = new QHBoxLayout;statusRow->setSpacing(16);
    lblSeatInfo = new QLabel("Apasă pe un loc pentru a-l selecta");lblSeatInfo->setStyleSheet("color:" C_MUT ";font-size:13px;");
    lblSeatCount = new QLabel("0 / 8 locuri selectate");lblSeatCount->setStyleSheet("color:" C_BLUE ";font-size:13px;font-weight:700;");
    btnClearSeats = ghostBtn("✕ Șterge selecția", 36);btnClearSeats->setFixedWidth(160);
    connect(btnClearSeats, &QPushButton::clicked, this, [this] {
        rezLocuri.clear();seatMapWidget->setSelected({});
        lblSeatCount->setText("0 / 8 locuri selectate");
        lblSeatInfo->setText("Apasă pe un loc pentru a-l selecta");
        updateSeatLegendCounts();
        });
    statusRow->addWidget(lblSeatInfo, 1);statusRow->addWidget(lblSeatCount);statusRow->addWidget(btnClearSeats);
    vb->addSpacing(8);vb->addLayout(statusRow);

    auto* btm = new QHBoxLayout;btm->addStretch();
    auto* bCont = redBtn("Continuă  →");bCont->setFixedWidth(210);btm->addWidget(bCont);
    vb->addSpacing(10);vb->addLayout(btm);

    connect(seatMapWidget, &SeatMapWidget::seatClicked, this, &MainWindow::onLocSelected);
    connect(bCont, &QPushButton::clicked, this, [this] {
        if (rezLocuri.empty()) { showToast("Selecteaza cel putin un loc!");return; }


        Sala* sala = cinema.getSalaById(rezSalaId);
        if (!sala)return;


        rezFood = FoodOrder{};
        refreshFoodPage();
        goTo(P_RESERVE_FOOD);
        });
    connect(bk, &QPushButton::clicked, this, [this] {goTo(P_RESERVE_ORA);});return w;
}


QWidget* MainWindow::buildReserveFoodPage() {
    QVBoxLayout* vb;auto* w = pageWrap(vb);
    auto* bk = ghostBtn("← Înapoi", 36);addHeader(vb, "Adaugă Food & Drinks", "Opțional · poți sări peste acest pas", bk);
    auto* mwF = new QFrame;mwF->setFixedHeight(62);
    mwF->setStyleSheet("QFrame{background:#111118;border-left:3px solid #e50914;border-radius:0;}");
    auto* mhF = new QHBoxLayout(mwF);mhF->setContentsMargins(14, 0, 16, 0);mhF->setSpacing(8);
    auto* miF = new QLabel("\xF0\x9F\x8E\xAC");miF->setStyleSheet("font-size:14px;background:transparent;border:none;");
    miniFood = new QLabel("—");miniFood->setStyleSheet("color:#8b8fa8;font-size:13px;background:transparent;border:none;");
    miniFood->setWordWrap(true);miniFood->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    mhF->addWidget(miF);mhF->addWidget(miniFood, 1);vb->addWidget(mwF);vb->addSpacing(12);

    auto* scroll = new QScrollArea;scroll->setWidgetResizable(true);scroll->setStyleSheet("QScrollArea{background:" C_BG ";border:none;}");
    auto* inner = new QWidget;inner->setStyleSheet("background:" C_BG ";");
    auto* innerVb = new QVBoxLayout(inner);innerVb->setContentsMargins(0, 0, 0, 0);innerVb->setSpacing(0);



    const std::vector<std::pair<QString, QString>> cats = {
        {"Popcorn","🍿"},{"Bauturi","🥤"},{"Snacks","🌮"},{"Combos","🎁"}
    };
    const auto& meniu = cinema.getMeniu();

    for (auto& [cat, ico] : cats) {


        auto* catHdr = new QLabel(ico + " " + cat);
        catHdr->setStyleSheet("font-size:13px;font-weight:700;color:" C_GOLD ";letter-spacing:1px;margin-top:16px;margin-bottom:8px;");
        innerVb->addWidget(catHdr);

        for (int i = 0;i < (int)meniu.size();i++) {
            const auto& item = meniu[i];
            if (QString::fromStdString(item.categorie) != cat) continue;

            auto* card = new QFrame;
            card->setStyleSheet("QFrame{background:" C_CARD ";border:1px solid " C_BOR2 ";border-radius:10px;}");
            auto* hb = new QHBoxLayout(card);hb->setContentsMargins(16, 12, 16, 12);hb->setSpacing(12);

            auto* emoL = new QLabel(QString::fromStdString(item.emoji));
            emoL->setStyleSheet("font-size:24px;background:transparent;border:none;");hb->addWidget(emoL);

            auto* tv = new QVBoxLayout;tv->setSpacing(2);
            auto* nl = new QLabel(QString::fromStdString(item.nume));
            nl->setStyleSheet("font-size:14px;font-weight:600;color:" C_TXT ";background:transparent;border:none;");tv->addWidget(nl);
            auto* dl = new QLabel(QString::fromStdString(item.descriere));
            dl->setStyleSheet("font-size:12px;color:" C_MUT ";background:transparent;border:none;");tv->addWidget(dl);
            hb->addLayout(tv, 1);

            auto* pl = new QLabel(QString::number(item.pret, 'f', 0) + " RON");
            pl->setStyleSheet("font-size:15px;font-weight:700;color:" C_RED ";background:transparent;border:none;min-width:60px;");
            pl->setAlignment(Qt::AlignRight | Qt::AlignVCenter);hb->addWidget(pl);



            auto* qRow = new QHBoxLayout;qRow->setSpacing(6);
            auto* bMinus = new QPushButton("−");bMinus->setFixedSize(32, 32);
            bMinus->setStyleSheet("QPushButton{background:" C_RAISE ";color:" C_TXT ";border:1px solid " C_BOR ";border-radius:6px;font-size:16px;font-weight:700;}"
                "QPushButton:hover{background:#28283a;}");
            bMinus->setCursor(Qt::PointingHandCursor);

            auto* qLbl = new QLabel("0");
            qLbl->setFixedWidth(28);qLbl->setAlignment(Qt::AlignCenter);
            qLbl->setStyleSheet("font-size:15px;font-weight:700;color:" C_TXT ";background:transparent;border:none;");
            foodQtyLabels.push_back(qLbl);

            auto* bPlus = new QPushButton("+");bPlus->setFixedSize(32, 32);
            bPlus->setStyleSheet("QPushButton{background:" C_RED ";color:white;border:none;border-radius:6px;font-size:16px;font-weight:700;}"
                "QPushButton:hover{background:" C_RHOV ";}");
            bPlus->setCursor(Qt::PointingHandCursor);
            foodQtyBtns.push_back(bPlus);

            std::string itemId = item.id;
            connect(bMinus, &QPushButton::clicked, this, [this, itemId, qLbl] {
                int cur = rezFood.items[itemId];
                if (cur > 0) { rezFood.items[itemId] = cur - 1;qLbl->setText(QString::number(cur - 1)); }
                if (lblFoodTotal) lblFoodTotal->setText("Total food: " + QString::number(rezFood.total(cinema.getMeniu()), 'f', 0) + " RON");
                });
            connect(bPlus, &QPushButton::clicked, this, [this, itemId, qLbl] {
                int cur = rezFood.items.count(itemId) ? rezFood.items[itemId] : 0;
                rezFood.items[itemId] = cur + 1;qLbl->setText(QString::number(cur + 1));
                if (lblFoodTotal) lblFoodTotal->setText("Total food: " + QString::number(rezFood.total(cinema.getMeniu()), 'f', 0) + " RON");
                });

            qRow->addWidget(bMinus);qRow->addWidget(qLbl);qRow->addWidget(bPlus);hb->addLayout(qRow);
            innerVb->addWidget(card);innerVb->addSpacing(6);
        }
    }
    innerVb->addStretch();
    scroll->setWidget(inner);vb->addWidget(scroll, 1);



    vb->addSpacing(12);vb->addWidget(hLine());vb->addSpacing(10);
    auto* btm = new QHBoxLayout;btm->setSpacing(12);
    lblFoodTotal = new QLabel("Total food: 0 RON");
    lblFoodTotal->setStyleSheet("font-size:15px;font-weight:700;color:" C_GOLD ";");
    btm->addWidget(lblFoodTotal);btm->addStretch();
    auto* bSkip = ghostBtn("Sari peste →", 46);bSkip->setFixedWidth(150);
    auto* bAdd = redBtn("Adaugă și continuă →", 46);bAdd->setFixedWidth(220);
    connect(bSkip, &QPushButton::clicked, this, &MainWindow::onFoodSkipped);
    connect(bAdd, &QPushButton::clicked, this, &MainWindow::onFoodConfirmed);
    btm->addWidget(bSkip);btm->addWidget(bAdd);vb->addLayout(btm);
    connect(bk, &QPushButton::clicked, this, [this] {goTo(P_RESERVE_LOC);});return w;
}


QWidget* MainWindow::buildReserveConfirmPage() {
    QVBoxLayout* vb;auto* w = pageWrap(vb);
    auto* bk = ghostBtn("← Înapoi", 36);addHeader(vb, "Sumar Rezervare", "Verifică detaliile și confirmă", bk);
    auto* mwC = new QFrame;mwC->setFixedHeight(62);
    mwC->setStyleSheet("QFrame{background:#111118;border-left:3px solid #e50914;border-radius:0;}");
    auto* mhC = new QHBoxLayout(mwC);mhC->setContentsMargins(14, 0, 16, 0);mhC->setSpacing(8);
    auto* miC = new QLabel("\xF0\x9F\x8E\xAC");miC->setStyleSheet("font-size:14px;background:transparent;border:none;");
    miniConf = new QLabel("—");miniConf->setStyleSheet("color:#8b8fa8;font-size:13px;background:transparent;border:none;");
    miniConf->setWordWrap(true);miniConf->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    mhC->addWidget(miC);mhC->addWidget(miniConf, 1);vb->addWidget(mwC);vb->addSpacing(12);
    lblConfirmDetails = new QLabel;lblConfirmDetails->setTextFormat(Qt::RichText);
    lblConfirmDetails->setStyleSheet("background:" C_CARD ";color:" C_TXT ";border:1px solid " C_BOR2 ";border-radius:14px;padding:28px 32px;");
    vb->addWidget(lblConfirmDetails);
    lblBookingRef = new QLabel;lblBookingRef->setTextFormat(Qt::RichText);
    lblBookingRef->setStyleSheet("font-size:13px;color:" C_MUT ";margin-top:8px;");vb->addWidget(lblBookingRef);
    vb->addSpacing(16);vb->addWidget(secLbl("EMAIL (OPȚIONAL)"));vb->addSpacing(6);
    leEmail = new QLineEdit;leEmail->setPlaceholderText("adresa@exemplu.com");leEmail->setStyleSheet(inputSS());leEmail->setFixedHeight(46);vb->addWidget(leEmail);
    vb->addStretch();
    auto* row = new QHBoxLayout;row->addStretch();
    auto* bConf = new QPushButton("✔   Confirmă Rezervarea");bConf->setFixedHeight(50);bConf->setFixedWidth(240);bConf->setCursor(Qt::PointingHandCursor);
    bConf->setStyleSheet("QPushButton{background:#16a34a;color:white;border:none;border-radius:10px;font-size:14px;font-weight:700;}QPushButton:hover{background:#15803d;}");
    row->addWidget(bConf);vb->addLayout(row);
    connect(bConf, &QPushButton::clicked, this, &MainWindow::onConfirmReservation);
    connect(bk, &QPushButton::clicked, this, [this] {goTo(P_RESERVE_FOOD);});return w;
}


QWidget* MainWindow::buildBrowsePage() {
    QVBoxLayout* vb;auto* w = pageWrap(vb);
    auto* bk = ghostBtn("← Înapoi", 36);addHeader(vb, "Browse după Gen", "Explorează filmele din colecție", bk);
    auto* tr = new QHBoxLayout;tr->setSpacing(10);
    cbGenuri = new QComboBox;cbGenuri->setMinimumWidth(160);cbGenuri->setStyleSheet(comboSS());
    btnBrowseFilter3D = ghostBtn("Format: Toate", 34);btnBrowseFilter3D->setFixedWidth(130);
    btnBrowseFilterActive = ghostBtn("Status: Toate", 34);btnBrowseFilterActive->setFixedWidth(140);
    tr->addWidget(comboLbl("Gen:"));tr->addWidget(cbGenuri);tr->addSpacing(8);
    tr->addWidget(btnBrowseFilter3D);tr->addWidget(btnBrowseFilterActive);tr->addStretch();
    vb->addLayout(tr);vb->addSpacing(14);
    auto* split = new QHBoxLayout;split->setSpacing(18);
    auto* browseListStack = new QStackedWidget;
    lwBrowseFilms = new QListWidget;lwBrowseFilms->setStyleSheet(listSS());lwBrowseFilms->setFont(QFont("Segoe UI", 13));
    browseListStack->addWidget(lwBrowseFilms);
    auto* browseEmpty = new QWidget;browseEmpty->setStyleSheet("background:" C_BG ";");
    auto* beVb = new QVBoxLayout(browseEmpty);beVb->setAlignment(Qt::AlignCenter);
    auto* beIco = new QLabel("🎭");beIco->setStyleSheet("font-size:48px;");beIco->setAlignment(Qt::AlignCenter);
    auto* beLbl = new QLabel("Niciun film în acest gen");beLbl->setStyleSheet("font-size:15px;font-weight:600;color:" C_MUT ";");beLbl->setAlignment(Qt::AlignCenter);
    auto* beSub = new QLabel("Încearcă alt gen sau modifică filtrele");beSub->setStyleSheet("font-size:12px;color:" C_DIM ";");beSub->setAlignment(Qt::AlignCenter);
    beVb->addWidget(beIco);beVb->addWidget(beLbl);beVb->addWidget(beSub);
    browseListStack->addWidget(browseEmpty);
    connect(lwBrowseFilms, &QListWidget::itemSelectionChanged, this, [browseListStack, this] {browseListStack->setCurrentIndex(lwBrowseFilms->count() == 0 ? 1 : 0);});
    split->addWidget(browseListStack, 1);
    auto* dc = new QFrame;dc->setFixedWidth(300);dc->setStyleSheet("QFrame{background:" C_CARD ";border:1px solid " C_BOR2 ";border-radius:14px;}");
    auto* dcVb = new QVBoxLayout(dc);dcVb->setContentsMargins(24, 24, 24, 24);dcVb->setSpacing(0);
    lblBrowseDetail = new QLabel("Selectează un film");lblBrowseDetail->setStyleSheet("color:" C_TXT ";font-size:13px;line-height:1.8;");lblBrowseDetail->setTextFormat(Qt::RichText);lblBrowseDetail->setAlignment(Qt::AlignTop);lblBrowseDetail->setWordWrap(true);
    dcVb->addWidget(lblBrowseDetail, 1);dcVb->addSpacing(16);
    auto* bRes = redBtn("🎟  Rezervă", 44);connect(bRes, &QPushButton::clicked, this, &MainWindow::onBrowseReserve);
    auto* bDet = ghostBtn("ℹ  Detalii", 40);
    connect(bDet, &QPushButton::clicked, this, [this] {
        int row = lwBrowseFilms->currentRow();if (row < 0)return;
        std::string gen = cbGenuri->currentText().toStdString();
        std::vector<Film*>vis;
        for (auto& f : cinema.getFilme()) { if (f.getGen() != gen)continue;if (browseActiveOnly && f.getStatus() != StatusFilm::RULAZA_ACUM)continue;if (browse3D == 0 && f.getEste3D())continue;if (browse3D == 1 && !f.getEste3D())continue;vis.push_back(const_cast<Film*>(&f)); }
        if (row < (int)vis.size()) { detailFromPage = P_BROWSE;populateFilmDetail(vis[row]);goTo(P_FILM_DETAIL); }
        });
    dcVb->addWidget(bRes);dcVb->addSpacing(6);dcVb->addWidget(bDet);split->addWidget(dc);vb->addLayout(split, 1);
    connect(cbGenuri, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onGenChanged);
    connect(lwBrowseFilms, &QListWidget::currentRowChanged, this, &MainWindow::updateBrowseDetail);
    connect(btnBrowseFilter3D, &QPushButton::clicked, this, [this] {browse3D = (browse3D + 2) % 3 - 1;btnBrowseFilter3D->setText(browse3D == -1 ? "Format: Toate" : browse3D == 0 ? "Format: 2D" : "Format: 3D");refreshBrowse();});
    connect(btnBrowseFilterActive, &QPushButton::clicked, this, [this] {browseActiveOnly = !browseActiveOnly;btnBrowseFilterActive->setText(browseActiveOnly ? "Status: Rulează" : "Status: Toate");refreshBrowse();});
    connect(bk, &QPushButton::clicked, this, [this] {goTo(P_MAIN);});return w;
}


QWidget* MainWindow::buildMapPage() {
    QVBoxLayout* vb;auto* w = pageWrap(vb);
    auto* bk = ghostBtn("← Înapoi", 36);addHeader(vb, "Hartă Sală", "Disponibilitate în timp real", bk);
    auto* sr = new QHBoxLayout;sr->setSpacing(10);
    cbMapSala = new QComboBox;cbMapData = new QComboBox;cbMapOra = new QComboBox;
    for (auto* cb : { cbMapSala,cbMapData,cbMapOra })cb->setStyleSheet(comboSS());
    sr->addWidget(comboLbl("Sală:"));sr->addWidget(cbMapSala);sr->addSpacing(8);
    sr->addWidget(comboLbl("Data:"));sr->addWidget(cbMapData);sr->addSpacing(8);
    sr->addWidget(comboLbl("Ora:"));sr->addWidget(cbMapOra);sr->addSpacing(12);
    auto* br = new QPushButton("🔄");br->setFixedHeight(36);br->setFixedWidth(48);
    br->setStyleSheet("QPushButton{background:" C_RAISE ";color:" C_TXT ";border:1px solid " C_BOR ";border-radius:7px;font-size:16px;}QPushButton:hover{background:#28283a;}");
    br->setCursor(Qt::PointingHandCursor);sr->addWidget(br);sr->addStretch();vb->addLayout(sr);vb->addSpacing(16);
    auto* scroll = new QScrollArea;scroll->setWidgetResizable(true);scroll->setStyleSheet("QScrollArea{background:" C_BG ";border:none;}");
    auto* inner = new QWidget;inner->setStyleSheet("background:" C_BG ";");
    auto* iVb = new QVBoxLayout(inner);iVb->setAlignment(Qt::AlignCenter);
    mapWidget = new SeatMapWidget;mapWidget->readOnly = true;iVb->addWidget(mapWidget, 0, Qt::AlignHCenter);
    scroll->setWidget(inner);vb->addWidget(scroll, 1);


    connect(cbMapSala, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::refreshMapOra);
    connect(cbMapData, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::refreshMapOra);
    connect(cbMapOra, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::refreshMapPage);
    connect(bk, &QPushButton::clicked, this, [this] {goTo(P_MAIN);});return w;
}


QWidget* MainWindow::buildCancelPage() {
    QVBoxLayout* vb;auto* w = pageWrap(vb);
    auto* bk = ghostBtn("← Înapoi", 36);addHeader(vb, "Anulare Rezervare", "Apasă pe un loc ocupat (✕) pentru a-l anula", bk);
    auto* sr = new QHBoxLayout;sr->setSpacing(10);
    cbCancelSala = new QComboBox;cbCancelData = new QComboBox;cbCancelOra = new QComboBox;
    for (auto* cb : { cbCancelSala,cbCancelData,cbCancelOra })cb->setStyleSheet(comboSS());
    sr->addWidget(comboLbl("Sală:"));sr->addWidget(cbCancelSala);sr->addSpacing(8);
    sr->addWidget(comboLbl("Data:"));sr->addWidget(cbCancelData);sr->addSpacing(8);
    sr->addWidget(comboLbl("Ora:"));sr->addWidget(cbCancelOra);sr->addStretch();vb->addLayout(sr);vb->addSpacing(16);
    auto* scroll = new QScrollArea;scroll->setWidgetResizable(true);scroll->setStyleSheet("QScrollArea{background:" C_BG ";border:none;}");
    auto* inner = new QWidget;inner->setStyleSheet("background:" C_BG ";");
    auto* iVb = new QVBoxLayout(inner);iVb->setAlignment(Qt::AlignCenter);
    cancelWidget = new SeatMapWidget;iVb->addWidget(cancelWidget, 0, Qt::AlignHCenter);
    scroll->setWidget(inner);vb->addWidget(scroll, 1);
    auto* btm = new QHBoxLayout;btm->addStretch();
    auto* bCan = new QPushButton("✕   Anulează Rezervarea Selectată");bCan->setFixedHeight(46);bCan->setCursor(Qt::PointingHandCursor);
    bCan->setStyleSheet("QPushButton{background:#7f1d1d;color:white;border:none;border-radius:8px;padding:0 24px;font-size:14px;font-weight:700;}QPushButton:hover{background:#991b1b;}");
    btm->addWidget(bCan);vb->addSpacing(12);vb->addLayout(btm);
    connect(cbCancelSala, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::refreshCancelOra);
    connect(cbCancelData, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::refreshCancelOra);
    connect(cbCancelOra, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::refreshCancelSeats);
    connect(cancelWidget, &SeatMapWidget::seatClicked, this, [this](int r, int c) {cancelRow = r;cancelCol = c;cancelWidget->setSelected({ {r,c} });});
    connect(bCan, &QPushButton::clicked, this, &MainWindow::onCancelReservation);
    connect(bk, &QPushButton::clicked, this, [this] {goTo(P_MAIN);});return w;
}


QWidget* MainWindow::buildProfilePage() {
    QVBoxLayout* vb;auto* w = pageWrap(vb);
    auto* bk = ghostBtn("← Înapoi", 36);addHeader(vb, "Profilul Meu", "Istoricul rezervărilor și evaluări filme", bk);



    auto* scroll = new QScrollArea;scroll->setWidgetResizable(true);
    scroll->setStyleSheet("QScrollArea{background:" C_BG ";border:none;}");
    profileContainer = new QWidget;profileContainer->setStyleSheet("background:" C_BG ";");
    scroll->setWidget(profileContainer);
    vb->addWidget(scroll, 1);

    vb->addSpacing(10);vb->addWidget(hLine());vb->addSpacing(8);
    auto* bPw = ghostBtn("Schimbă Parola", 38);bPw->setFixedWidth(180);
    connect(bPw, &QPushButton::clicked, this, [this] {
        auto* u = cinema.getCurent();if (!u)return;
        bool ok;
        QString old = QInputDialog::getText(this, "Parola curenta", "Parola actuala:", QLineEdit::Password, "", &ok);
        if (!ok || old.toStdString() != u->parola) { showToast("Parola incorecta!");return; }
        QString nw = QInputDialog::getText(this, "Parola noua", "Parola noua:", QLineEdit::Password, "", &ok);if (!ok || nw.isEmpty())return;
        QString cf = QInputDialog::getText(this, "Confirmare", "Confirma parola:", QLineEdit::Password, "", &ok);if (!ok || nw != cf) { showToast("Parolele nu coincid!");return; }
        cinema.schimbaParola(u->username, nw.toStdString());u->parola = nw.toStdString();showToast("Parola schimbata!", false);
        });
    vb->addWidget(bPw, 0, Qt::AlignLeft);
    connect(bk, &QPushButton::clicked, this, [this] {goTo(P_MAIN);});return w;
}


QWidget* MainWindow::buildAdminPage() {
    QVBoxLayout* vb;auto* w = pageWrap(vb);
    auto* bk = ghostBtn("← Înapoi", 36);addHeader(vb, "Panou Admin", "Gestionare filme, ore, utilizatori și rezervări", bk);


    auto* statsRow = new QHBoxLayout;statsRow->setSpacing(12);


    auto* statsWidget = new QWidget;statsWidget->setObjectName("adminStatsWidget");
    statsWidget->setFixedHeight(90);
    statsWidget->setLayout(statsRow);
    vb->addWidget(statsWidget);vb->addSpacing(12);
    adminTabs = new QTabWidget;
    adminTabs->setStyleSheet(
        "QTabWidget::pane{border:1px solid " C_BOR2 ";background:" C_CARD ";border-radius:10px;}"
        "QTabBar::tab{background:" C_RAISE ";color:" C_MUT ";padding:9px 18px;font-size:13px;border-top-left-radius:8px;border-top-right-radius:8px;margin-right:3px;}"
        "QTabBar::tab:selected{background:" C_RED ";color:white;font-weight:700;}"
        "QTabBar::tab:hover:!selected{background:#28283a;color:" C_TXT ";}");
    teAllRes = new QTextEdit;teAllRes->setReadOnly(true);teAllRes->setStyleSheet("background:" C_BG ";color:" C_TXT ";font-family:'Consolas','Courier New';font-size:12px;border:none;padding:12px;");
    adminTabs->addTab(teAllRes, "📋  Rezervări");
    teReport = new QTextEdit;teReport->setReadOnly(true);teReport->setStyleSheet("background:" C_BG ";color:" C_TXT ";font-family:'Consolas','Courier New';font-size:12px;border:none;padding:12px;");
    adminTabs->addTab(teReport, "📊  Vânzări");
    auto* ft = new QWidget;ft->setStyleSheet("background:" C_CARD ";");
    auto* ftV = new QVBoxLayout(ft);ftV->setContentsMargins(16, 16, 16, 16);ftV->setSpacing(10);
    lwAdminFilms = new QListWidget;lwAdminFilms->setStyleSheet(listSS());ftV->addWidget(lwAdminFilms, 1);
    auto* fb = new QHBoxLayout;fb->setSpacing(8);
    auto* bA = redBtn("Adaugă", 38);auto* bE = ghostBtn("Editează", 38);auto* bD = ghostBtn("Șterge", 38);
    bD->setStyleSheet(bD->styleSheet().replace(C_MUT, "#f87171").replace(C_BOR, "#5a1a1a"));
    fb->addWidget(bA);fb->addWidget(bE);fb->addWidget(bD);fb->addStretch();ftV->addLayout(fb);
    adminTabs->addTab(ft, "🎬  Filme");

    auto* ut = new QWidget;ut->setStyleSheet("background:" C_CARD ";");
    auto* uV = new QVBoxLayout(ut);uV->setContentsMargins(16, 16, 16, 16);uV->setSpacing(10);
    lwAdminUsers = new QListWidget;lwAdminUsers->setStyleSheet(listSS());uV->addWidget(lwAdminUsers, 1);
    auto* bDU = ghostBtn("Șterge Utilizatorul Selectat", 38);
    bDU->setStyleSheet(bDU->styleSheet().replace(C_MUT, "#f87171").replace(C_BOR, "#5a1a1a"));
    uV->addWidget(bDU, 0, Qt::AlignLeft);adminTabs->addTab(ut, "👥  Utilizatori");
    vb->addWidget(adminTabs, 1);
    connect(adminTabs, &QTabWidget::currentChanged, this, [this](int i) {
        if (i == 0)refreshAdminReservations();else if (i == 1)refreshAdminReport();
        else if (i == 2)refreshAdminFilms();else if (i == 3)refreshAdminUsers();});
    connect(bA, &QPushButton::clicked, this, &MainWindow::onAdminAddFilm);
    connect(bE, &QPushButton::clicked, this, &MainWindow::onAdminEditFilm);
    connect(bD, &QPushButton::clicked, this, &MainWindow::onAdminDeleteFilm);
    connect(bDU, &QPushButton::clicked, this, &MainWindow::onAdminDeleteUser);
    connect(bk, &QPushButton::clicked, this, [this] {goTo(P_MAIN);});return w;
}


QWidget* MainWindow::buildConfirmDonePage() {
    auto* w = new QWidget;w->setStyleSheet("background:" C_BG ";");
    auto* vb = new QVBoxLayout(w);vb->setAlignment(Qt::AlignCenter);vb->setSpacing(0);vb->setContentsMargins(40, 40, 40, 40);
    auto* chk = new QLabel("✓");chk->setFixedSize(88, 88);chk->setAlignment(Qt::AlignCenter);
    chk->setStyleSheet("font-size:44px;font-weight:900;color:white;background:#16a34a;border-radius:44px;");
    vb->addWidget(chk, 0, Qt::AlignHCenter);vb->addSpacing(18);
    auto* ttl = new QLabel("Rezervare Confirmată!");ttl->setStyleSheet("font-size:28px;font-weight:800;color:" C_TXT ";");ttl->setAlignment(Qt::AlignCenter);vb->addWidget(ttl);
    vb->addSpacing(8);
    auto* sub = new QLabel("Biletul tău a fost rezervat cu succes.");sub->setStyleSheet("font-size:14px;color:" C_MUT ";");sub->setAlignment(Qt::AlignCenter);vb->addWidget(sub);
    vb->addSpacing(24);
    auto* refBox = new QFrame;refBox->setFixedWidth(460);refBox->setStyleSheet("QFrame{background:#0d1f0d;border:1.5px solid #166534;border-radius:12px;}");
    auto* rbVb = new QVBoxLayout(refBox);rbVb->setContentsMargins(24, 18, 24, 18);rbVb->setSpacing(4);
    auto* refLbl = new QLabel("REFERINȚĂ REZERVARE");refLbl->setStyleSheet("font-size:10px;font-weight:700;color:#4ade80;letter-spacing:2px;");refLbl->setAlignment(Qt::AlignCenter);rbVb->addWidget(refLbl);
    lblDoneRef = new QLabel("—");lblDoneRef->setStyleSheet("font-size:26px;font-weight:800;color:#22c55e;letter-spacing:4px;");lblDoneRef->setAlignment(Qt::AlignCenter);rbVb->addWidget(lblDoneRef);
    vb->addWidget(refBox, 0, Qt::AlignHCenter);vb->addSpacing(20);
    lblDoneSummary = new QLabel;lblDoneSummary->setTextFormat(Qt::RichText);lblDoneSummary->setFixedWidth(460);
    lblDoneSummary->setStyleSheet("background:" C_CARD ";color:" C_TXT ";border:1px solid " C_BOR2 ";border-radius:12px;padding:20px 24px;font-size:13px;");
    lblDoneSummary->setAlignment(Qt::AlignLeft | Qt::AlignTop);lblDoneSummary->setWordWrap(true);
    vb->addWidget(lblDoneSummary, 0, Qt::AlignHCenter);vb->addSpacing(28);
    auto* btnRow2 = new QHBoxLayout;btnRow2->setSpacing(12);
    btnExportCal = new QPushButton("Adaugă în Calendar");
    btnExportCal->setFixedHeight(46);btnExportCal->setFixedWidth(220);
    btnExportCal->setCursor(Qt::PointingHandCursor);
    btnExportCal->setStyleSheet("QPushButton{background:#1e3a5f;color:#38bdf8;border:1.5px solid #1d4ed8;"
        "border-radius:8px;font-size:13px;font-weight:700;}"
        "QPushButton:hover{background:#1e40af;color:white;}");
    connect(btnExportCal, &QPushButton::clicked, this, &MainWindow::onExportCalendar);
    auto* bHome = redBtn("Înapoi la Meniu Principal", 50);bHome->setFixedWidth(240);
    connect(bHome, &QPushButton::clicked, this, [this] {goTo(P_MAIN);});
    btnRow2->addWidget(btnExportCal);btnRow2->addWidget(bHome);
    vb->addLayout(btnRow2, Qt::AlignHCenter);return w;
}