//czesc 1/5
#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QProcess>
#include <QLabel>
#include <QMenu>
#include <QTextEdit>
#include <QProgressBar>
#include <QMessageBox>
#include <QDir>
#include <QFile>
#include <QSettings>
#include <QSplitter>
#include <QRegularExpression>
#include <QColor>
#include <QSet>
#include <QTimer>
#include <QDateTime>
#include <QThread>
#include <QScrollBar>
#include <QDialog>
#include <QFileInfo>
#include <QFrame>
#include <QCloseEvent>
#include <ctime>
#include <cstdlib>

class GentooManager : public QWidget {
public:
    GentooManager(QWidget *parent = nullptr) : QWidget(parent) {
        QSettings settings("GentooManager", "GentooManager");
        terminal = settings.value("terminal", "kitty").toString();
        isOverlayMode = false;

        setWindowTitle("Portager - Koszmar & Adaptive Collaborator");
        resize(1150, 850);
        setStyleSheet("background-color: #121212; color: #e0e0e0;");

        auto *mainLayout = new QVBoxLayout(this);

        // --- TOP BAR ---
        auto *topLayout = new QHBoxLayout();
        statsLabel = new QLabel("Initializing...");
        statsLabel->setVisible(false);

        syncBtn = new QPushButton("🔄 Sync");
        globalUpdateBtn = new QPushButton("🚀 Upgrade");
        globalUpdateBtn->setEnabled(false);
        topCleanBtn = new QPushButton("🧹 Purge");
        etcUpBtn = new QPushButton("🔧 EtcUp");
        ovlBtn = new QPushButton("🌐 Overlays");
        aboutBtn = new QPushButton("ℹ️ About");

        topLayout->addWidget(statsLabel);
        topLayout->addWidget(syncBtn);
        topLayout->addWidget(globalUpdateBtn);
        topLayout->addWidget(topCleanBtn);
        topLayout->addWidget(etcUpBtn);
        topLayout->addWidget(ovlBtn);
        topLayout->addWidget(aboutBtn);
        mainLayout->addLayout(topLayout);

        searchBar = new QLineEdit();
        searchBar->setPlaceholderText("Search packages...");
        searchBar->setStyleSheet("padding: 10px; background: #1e1e1e; border: 1px solid #00ffcc; color: white;");
        mainLayout->addWidget(searchBar);

        auto *splitter = new QSplitter(Qt::Vertical);
        auto *listWithLegendWidget = new QWidget();
        auto *listWithLegendLayout = new QHBoxLayout(listWithLegendWidget);
        listWithLegendLayout->setContentsMargins(0,0,0,0);

        pkgList = new QListWidget();
        QString imgPath = "/usr/share/portager/background.png";
        pkgList->setStyleSheet(QString("QListWidget { background-image: url('%1'); background-repeat: no-repeat; background-position: center; background-attachment: fixed; background-color: #121212; color: #e0e0e0; border: 1px solid #333; }").arg(imgPath));
        pkgList->setSelectionMode(QAbstractItemView::ExtendedSelection);

        // --- KLUCZOWA POPRAWKA DLA MENU KONTEKSTOWEGO ---
        pkgList->setContextMenuPolicy(Qt::CustomContextMenu);

        legendContainer = new QWidget();
        legendContainer->setFixedWidth(250);
        legendContainer->setStyleSheet("background: #1a1a1a; border-left: 1px solid #333;");
        auto *legendLayout = new QVBoxLayout(legendContainer);
        legendLayout->setAlignment(Qt::AlignTop);

        legendHeader = new QLabel("PKG INFO");
        legendHeader->setStyleSheet("color: #00ffcc; font-weight: bold; font-size: 11px; letter-spacing: 2px; margin-bottom: 8px;");
        legendHeader->setAlignment(Qt::AlignCenter);
        legendLayout->addWidget(legendHeader);

        legendItemsWidget = new QWidget();
        legendItemsLayout = new QVBoxLayout(legendItemsWidget);
        legendItemsLayout->setContentsMargins(5,0,5,0);
        legendLayout->addWidget(legendItemsWidget);
        updateLegend(false);

        legendLayout->addStretch();
        listWithLegendLayout->addWidget(pkgList);
        listWithLegendLayout->addWidget(legendContainer);
        splitter->addWidget(listWithLegendWidget);

        auto *consoleWrapper = new QWidget();
        auto *consoleVLayout = new QVBoxLayout(consoleWrapper);
        consoleVLayout->setContentsMargins(0,0,0,0);
        consoleVLayout->setSpacing(0);

        editorTools = new QWidget();
        auto *toolsLayout = new QHBoxLayout(editorTools);
        toolsLayout->setContentsMargins(5,5,5,5);
        cancelUseBtn = new QPushButton("❌ CANCEL");
        cancelUseBtn->setStyleSheet("background-color: #c62828; color: white; font-weight: bold; padding: 5px;");
        saveFlagsBtn = new QPushButton("💾 SAVE & APPLY CHANGES");
        saveFlagsBtn->setStyleSheet("background-color: #2e7d32; color: white; font-weight: bold; padding: 5px;");
        toolsLayout->addWidget(new QLabel("<b style='color:#00ffcc;'>EDITOR MODE</b>"));
        toolsLayout->addStretch();
        toolsLayout->addWidget(cancelUseBtn);
        toolsLayout->addWidget(saveFlagsBtn);
        editorTools->setVisible(false);
        consoleVLayout->addWidget(editorTools);

        consoleOutput = new QTextEdit();
        consoleOutput->setReadOnly(true);
        consoleOutput->setStyleSheet("background-color: #000; color: #00ff00; font-family: monospace; border: 1px solid #333;");
        consoleVLayout->addWidget(consoleOutput);

        auto *bottomInputWrapper = new QWidget();
        auto *bottomInputLayout = new QHBoxLayout(bottomInputWrapper);
        bottomInputLayout->setContentsMargins(0,0,0,0);
        bottomInputLayout->setSpacing(0);

        consoleInput = new QLineEdit();
        consoleInput->setPlaceholderText("Type response (e.g. 1, y, n) and press Enter...");
        consoleInput->setStyleSheet("background-color: #050505; color: #00ffcc; border: 1px solid #333; font-family: monospace; padding: 5px; height: 30px;");
        bottomInputLayout->addWidget(consoleInput);

        progressBar = new QProgressBar();
        progressBar->setVisible(false);
        progressBar->setTextVisible(false);
        progressBar->setStyleSheet("QProgressBar { background-color: #050505; border: 1px solid #333; height: 30px; } QProgressBar::chunk { background-color: #00ffcc; }");
        bottomInputLayout->addWidget(progressBar);

        consoleVLayout->addWidget(bottomInputWrapper);
        splitter->addWidget(consoleWrapper);
        splitter->setStretchFactor(0, 1);
        splitter->setStretchFactor(1, 1);
        mainLayout->addWidget(splitter);

        confirmWidget = new QWidget();
        auto *confirmLayout = new QHBoxLayout(confirmWidget);
        confirmBtn = new QPushButton("Confirm");
        cancelBtn = new QPushButton("Cancel");
        confirmBtn->setStyleSheet("background-color: #2e7d32; color: white; font-weight: bold; padding: 10px;");
        cancelBtn->setStyleSheet("background-color: #c62828; color: white; font-weight: bold; padding: 10px;");
        confirmLayout->addWidget(confirmBtn); confirmLayout->addWidget(cancelBtn);
        confirmWidget->setVisible(false);
        mainLayout->addWidget(confirmWidget);

        auto *btnLayout = new QHBoxLayout();
        selectionLabel = new QLabel("Selected: 0");
        selectionLabel->setStyleSheet("color: #00ffcc; font-weight: bold; margin-right: 10px;");
        installBtn = new QPushButton("➕ Install");
        reinstallBtn = new QPushButton("🚀 Rebuild");
        useBtn = new QPushButton("⚙️ UseFlags");
        uninstallBtn = new QPushButton("🗑️ Remove");
        infoBtn = new QPushButton("🔍 Inspect");

        btnLayout->addWidget(selectionLabel);
        btnLayout->addWidget(installBtn);
        btnLayout->addWidget(reinstallBtn);
        btnLayout->addWidget(useBtn);
        btnLayout->addWidget(uninstallBtn);
        btnLayout->addWidget(infoBtn);
        mainLayout->addLayout(btnLayout);

        installProcess = new QProcess(this);

//czesc 2/5

        // --- SIGNALS ---
        connect(aboutBtn, &QPushButton::clicked, [this]() {
            QDialog *aboutDialog = new QDialog(this);
            aboutDialog->setWindowTitle("About Portager");
            aboutDialog->setStyleSheet("background-color: #1a1a1a; color: #e0e0e0;");
            aboutDialog->setMinimumWidth(400);
            auto *aboutLayout = new QVBoxLayout(aboutDialog);
            QLabel *titleLabel = new QLabel("<h2>Portager v2.5</h2><p>You are Terminal-free ;)</p>");
            titleLabel->setAlignment(Qt::AlignCenter);
            aboutLayout->addWidget(titleLabel);
            QLabel *authorLabel = new QLabel("<p style='font-size: 13px;'>Author: <b>Koszmar</b></p>");
            authorLabel->setAlignment(Qt::AlignCenter);
            aboutLayout->addWidget(authorLabel);
            QLabel *warnLabel = new QLabel("<p style='color:#ff3333; font-weight: bold; font-size: 13px;'>🚩 ATTENTION: This is a development version!<br>Use this application at your own risk. The author takes no responsibility for any eventual data loss or system damage.</p>");
            warnLabel->setWordWrap(true);
            warnLabel->setAlignment(Qt::AlignCenter);
            warnLabel->setStyleSheet("border: 1px solid #ff3333; padding: 8px; background: #2a1010; border-radius: 5px;");
            aboutLayout->addWidget(warnLabel);
            QLabel *donateLabel = new QLabel("<p style='margin-top: 10px;'>If you like this app, you can support the author's work:<br><a style='color: #00ffcc;' href='https://buycoffee.to/koszmar'>buycoffee.to/koszmar</a></p>");
            donateLabel->setOpenExternalLinks(true);
            donateLabel->setAlignment(Qt::AlignCenter);
            aboutLayout->addWidget(donateLabel);

            QString qrPath = "/usr/share/portager/qrcode.png";
            if (QFile::exists(qrPath)) {
                QLabel *qrLabel = new QLabel();
                qrLabel->setPixmap(QPixmap(qrPath).scaled(180, 180, Qt::KeepAspectRatio, Qt::SmoothTransformation));
                qrLabel->setAlignment(Qt::AlignCenter);
                aboutLayout->addWidget(qrLabel);
            }

            auto *closeBtn = new QPushButton("Close");
            closeBtn->setStyleSheet("background-color: #333; color: white; padding: 5px;");
            connect(closeBtn, &QPushButton::clicked, aboutDialog, &QDialog::accept);
            aboutLayout->addWidget(closeBtn);
            aboutDialog->exec();
        });

        aboutPulseTimer = new QTimer(this);
        connect(aboutPulseTimer, &QTimer::timeout, [this]() {
            static bool glitch = false;
            if (glitch) {
                this->aboutBtn->setText("ℹ️ About");
                this->aboutBtn->setStyleSheet("");
                aboutPulseTimer->start(6000 + (std::rand() % 9000));
            } else {
                this->aboutBtn->setText("⚠️ !!! ABOUT !!!");
                this->aboutBtn->setStyleSheet("border: 2px solid #00ffcc; color: #00ffcc; font-weight: bold;");
                aboutPulseTimer->start(500);
            }
            glitch = !glitch;
        });
        std::srand(std::time(nullptr));
        aboutPulseTimer->start(6000);

        connect(consoleInput, &QLineEdit::returnPressed, [this]() {
            if (installProcess->state() == QProcess::Running) {
                installProcess->write((consoleInput->text() + "\n").toLocal8Bit());
                consoleOutput->append("<span style='color:#555;'> &gt; " + consoleInput->text() + "</span>");
            }
            consoleInput->clear();
        });

        connect(saveFlagsBtn, &QPushButton::clicked, this, &GentooManager::saveUseFlags);
        connect(cancelUseBtn, &QPushButton::clicked, [this]() {
            consoleOutput->clear(); consoleOutput->setReadOnly(true);
            editorTools->setVisible(false); consoleInput->setVisible(true);
            setInterfaceEnabled(true);
        });

        connect(ovlBtn, &QPushButton::clicked, this, &GentooManager::toggleOverlayMode);
        connect(infoBtn, &QPushButton::clicked, this, &GentooManager::handleInfo);
        connect(etcUpBtn, &QPushButton::clicked, [this]() {
            consoleOutput->clear();
            consoleOutput->append("<b style='color:#fb8c00;'>[System] Starting interactive etc-update...</b>");
            installProcess->start("script", {"-q", "-c", "sudo etc-update", "/dev/null"});
        });

        connect(pkgList, &QListWidget::itemSelectionChanged, [this]() {
            auto sel = pkgList->selectedItems();
            selectionLabel->setText(QString("Selected: %1").arg(sel.size()));
            if (!isOverlayMode) {
                if (sel.size() == 1) {
                    int type = sel.first()->data(Qt::UserRole).toInt();
                    installBtn->setEnabled(type == 999); reinstallBtn->setEnabled(type != 999);
                    uninstallBtn->setEnabled(type != 999); useBtn->setEnabled(true); infoBtn->setEnabled(true);
                } else {
                    installBtn->setEnabled(false); reinstallBtn->setEnabled(sel.size() > 1);
                    uninstallBtn->setEnabled(sel.size() > 1); useBtn->setEnabled(false); infoBtn->setEnabled(false);
                }
            } else { installBtn->setEnabled(sel.size() == 1); }
        });

        connect(searchBar, &QLineEdit::textChanged, this, &GentooManager::handleSearch);
        connect(syncBtn, &QPushButton::clicked, this, &GentooManager::handleSync);
        connect(globalUpdateBtn, &QPushButton::clicked, [this]() { startEmerge("-avuDU --with-bdeps=y @world"); });
        connect(topCleanBtn, &QPushButton::clicked, [this]() { startEmerge("--depclean -p"); });
        connect(installBtn, &QPushButton::clicked, [this]() { if (!isOverlayMode) startEmerge("-av"); else handleOverlayAction(); });
        connect(reinstallBtn, &QPushButton::clicked, [this]() { startEmerge("-av --oneshot"); });
        connect(uninstallBtn, &QPushButton::clicked, [this]() { handleSecureRemove(); });
        connect(useBtn, &QPushButton::clicked, this, &GentooManager::handleUseEdit);
        connect(pkgList, &QListWidget::customContextMenuRequested, [this](const QPoint &pos) { this->showContextMenu(pos); });

        connect(installProcess, &QProcess::readyReadStandardOutput, this, &GentooManager::readInstallOutput);
        connect(installProcess, &QProcess::readyReadStandardError, this, &GentooManager::readInstallOutput);

        connect(installProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), [this](int exitCode) {
            progressBar->setVisible(false); consoleInput->setVisible(true); setInterfaceEnabled(true);
            if (exitCode == 0) { loadLocalPackages(); checkUpdates(); }
        });

        connect(confirmBtn, &QPushButton::clicked, [this]() { confirmWidget->setVisible(false); installProcess->write("y\n"); });
        connect(cancelBtn, &QPushButton::clicked, [this]() { confirmWidget->setVisible(false); if (installProcess->state() == QProcess::Running) installProcess->write("n\n"); });

        // START: Dane lokalne, blokada i skanowanie
        loadLocalPackages();
        setInterfaceEnabled(false);
        checkUpdates();
    }

private:
    QString terminal; QListWidget *pkgList; QLineEdit *searchBar; QTextEdit *consoleOutput;
    QLineEdit *consoleInput; QLabel *statsLabel, *selectionLabel, *legendHeader;
    QProgressBar *progressBar;
    QPushButton *syncBtn, *globalUpdateBtn, *topCleanBtn, *etcUpBtn, *ovlBtn, *aboutBtn;
    QPushButton *installBtn, *reinstallBtn, *uninstallBtn, *useBtn, *infoBtn, *saveFlagsBtn, *cancelUseBtn;
    QPushButton *confirmBtn, *cancelBtn; QWidget *confirmWidget, *legendItemsWidget, *legendContainer, *editorTools;
    QVBoxLayout *legendItemsLayout;
    QStringList localPkgs, updatablePkgs;
    QProcess *installProcess;
    bool isOverlayMode;
    QString kernelVer, gccVer, clangVer;
    int totalAvailable = 0;
    QTimer *aboutPulseTimer;

       void updateLegend(bool overlayMode) {
        QLayoutItem *child;
        while ((child = legendItemsLayout->takeAt(0)) != nullptr) { if (child->widget()) delete child->widget(); delete child; }

        auto addLeg = [&](QString c, QString t, bool bold = false) {
            auto *h = new QHBoxLayout();
            auto *b = new QLabel(); b->setFixedSize(12,12);
            if (!c.isEmpty()) b->setStyleSheet(QString("background-color: %1; border-radius: 2px;").arg(c));
            auto *l = new QLabel(t);
            l->setStyleSheet(QString("color: #ccc; font-size: 13px; %1").arg(bold ? "font-weight: bold; color: #00ffcc;" : ""));
            h->addWidget(b); h->addWidget(l); h->addStretch();
            legendItemsLayout->addLayout(h);
        };

        if (!overlayMode) {
            addLeg("#00ff00", QString("Updates: %1").arg(updatablePkgs.size()), true);
            addLeg("#eeeeee", QString("Installed: %1").arg(localPkgs.size()));
            addLeg("#666666", QString("Available: %1").arg(totalAvailable));

            auto *sep = new QFrame(); sep->setFrameShape(QFrame::HLine); sep->setStyleSheet("background: #333; margin: 10px 0;");
            legendItemsLayout->addWidget(sep);

            addLeg("", "SYSTEM INFO:", true);
            addLeg("", "Kernel: " + kernelVer);
            addLeg("", "GCC: " + gccVer);
            addLeg("", "Clang: " + clangVer);

            // --- TUTAJ DODAJEMY KOD QR DO PANELU BOCZNEGO ---
            QString qrPath = "/usr/share/portager/qrcode.png";
            if (QFile::exists(qrPath)) {
                auto *sep2 = new QFrame(); sep2->setFrameShape(QFrame::HLine); sep2->setStyleSheet("background: #333; margin: 15px 0 5px 0;");
                legendItemsLayout->addWidget(sep2);

                QLabel *qrLabel = new QLabel();
                qrLabel->setPixmap(QPixmap(qrPath).scaled(140, 140, Qt::KeepAspectRatio, Qt::SmoothTransformation));
                qrLabel->setAlignment(Qt::AlignCenter);
                qrLabel->setStyleSheet("background: white; padding: 5px; border-radius: 5px; margin: 5px;");
                legendItemsLayout->addWidget(qrLabel);

                QLabel *qrText = new QLabel("Support Koszmar");
                qrText->setStyleSheet("color: #00ffcc; font-size: 10px; font-weight: bold;");
                qrText->setAlignment(Qt::AlignCenter);
                legendItemsLayout->addWidget(qrText);
            }
        } else {
            addLeg("#00ffcc", "Active Repo", true);
            addLeg("#888888", "Disabled Repo");
        }
    }


//czesc 3/5

    void saveUseFlags() {
        QString content = consoleOutput->toPlainText();
        QString priorityFile = "/etc/portage/package.use/zzz_portager_use";
        QProcess p; p.start("bash", {"-c", QString("sudo tee %1").arg(priorityFile)});
        p.write(content.toLocal8Bit()); p.closeWriteChannel(); p.waitForFinished();

        consoleOutput->setReadOnly(true);
        consoleOutput->setStyleSheet("background-color: #000; color: #00ff00; font-family: monospace; border: 1px solid #333;");
        editorTools->setVisible(false);
        consoleOutput->clear();
        consoleOutput->append("<b style='color:#00ffcc;'>[System] USE Flags saved. Re-scanning system dependencies...</b>");

        // Blokujemy interfejs i wymuszamy pełny skan Emerge, by wykryć zmiany (newuse)
        setInterfaceEnabled(false);
        checkUpdates();
    }

    void handleSecureRemove() {
        auto sel = pkgList->selectedItems(); if (sel.isEmpty()) return;
        QString pkgs = "";
        for (auto *it : sel) pkgs += it->text().split(' ').first() + "\n";
        QMessageBox warn; warn.setWindowTitle("CRITICAL WARNING"); warn.setIcon(QMessageBox::Critical);
        warn.setText("<b style='color:red;'>ARE YOU ABSOLUTELY SURE?</b>");
        warn.setInformativeText("You are about to UNMERGE (force remove):\n\n" + pkgs + "\nThis action can potentially BROKE YOUR SYSTEM dependencies!");
        warn.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        if (warn.exec() == QMessageBox::Yes) startEmerge("-avC");
    }

    void toggleOverlayMode() {
        isOverlayMode = !isOverlayMode;
        ovlBtn->setText(isOverlayMode ? "🔙 Back" : "🌐 Overlays");
        updateLegend(isOverlayMode);
        if (isOverlayMode) {
            pkgList->clear();
            QProcess p; p.start("eselect", {"--colour=no", "repository", "list"}); p.waitForFinished();
            QString out = QString::fromLocal8Bit(p.readAllStandardOutput());
            for (const QString &line : out.split('\n', Qt::SkipEmptyParts)) {
                if (!line.contains("[") || line.contains("(#)")) continue;
                auto *it = new QListWidgetItem(line.trimmed());
                bool isEn = line.contains("*");
                QString name = line.trimmed().split(QRegularExpression("\\s+"), Qt::SkipEmptyParts).value(1);
                it->setForeground(QColor(isEn ? "#00ffcc" : "#888888"));
                it->setData(Qt::UserRole, isEn ? 1 : 0); it->setData(Qt::UserRole + 1, name);
                pkgList->addItem(it);
            }
        } else { handleSearch(searchBar->text()); }
    }

    void handleOverlayAction() {
        auto sel = pkgList->selectedItems(); if (sel.isEmpty()) return;
        QString name = sel.first()->data(Qt::UserRole + 1).toString();
        bool isEn = sel.first()->data(Qt::UserRole).toInt() == 1;
        QString act = isEn ? "remove" : "enable";
        QString cmd = (act == "enable") ? QString("sudo eselect repository enable \"%1\" && sudo emaint sync -r \"%1\"").arg(name) : QString("sudo eselect repository remove \"%1\"").arg(name);
        consoleOutput->clear(); consoleOutput->append("<b style='color:#5e35b1;'>[System] Repository " + act + ": " + name + "...</b>");
        installProcess->start("script", {"-q", "-c", cmd, "/dev/null"});
        toggleOverlayMode();
    }

    void updateStats() {
        QProcess kP; kP.start("uname", {"-r"}); kP.waitForFinished();
        kernelVer = QString::fromLocal8Bit(kP.readAllStandardOutput()).trimmed();

        QProcess gP; gP.start("gcc", {"-dumpfullversion"}); gP.waitForFinished();
        gccVer = QString::fromLocal8Bit(gP.readAllStandardOutput()).trimmed();
        if(gccVer.isEmpty()) {
            gP.start("gcc", {"--version"}); gP.waitForFinished();
            gccVer = QString::fromLocal8Bit(gP.readAllStandardOutput()).split(' ').value(2);
        }

        QProcess cP; cP.start("clang", {"--version"}); cP.waitForFinished();
        QString cOut = QString::fromLocal8Bit(cP.readAllStandardOutput());
        QRegularExpression cReg("version\\s+([0-9.]+)");
        QRegularExpressionMatch cMatch = cReg.match(cOut);
        clangVer = cMatch.hasMatch() ? cMatch.captured(1) : "N/A";

        QProcess eP;
        eP.start("bash", {"-c", "eix --pure-packages --format '<category>/<name>\n' | wc -l"});
        eP.waitForFinished();
        totalAvailable = QString::fromLocal8Bit(eP.readAllStandardOutput()).trimmed().toInt();

        if (totalAvailable == 0) {
            QProcess pP;
            pP.start("bash", {"-c", "ls -R /var/db/repos/gentoo | grep '/' | wc -l"});
            pP.waitForFinished();
            totalAvailable = QString::fromLocal8Bit(pP.readAllStandardOutput()).trimmed().toInt();
        }

        updateLegend(isOverlayMode);
    }

    void loadLocalPackages() {
        QProcess p; p.start("qlist", {"-I", "-v"}); p.waitForFinished();
        localPkgs = QString::fromLocal8Bit(p.readAllStandardOutput()).split('\n', Qt::SkipEmptyParts);
        updateStats();
        handleSearch("");
    }

    void handleSearch(const QString &t) {
        if (isOverlayMode) return;
        pkgList->setUpdatesEnabled(false); pkgList->clear();

        QStringList matchedLocal;
        for (const QString &p : localPkgs) {
            if (t.isEmpty() || p.contains(t, Qt::CaseInsensitive)) matchedLocal << p;
        }
        refreshList(matchedLocal);

        if (t.length() >= 3) {
            QProcess eix; eix.start("eix", {"--pure-packages", "--format", "<category>/<name>\n", "--substring", t});
            eix.waitForFinished();
            QStringList eixOut = QString::fromLocal8Bit(eix.readAllStandardOutput()).split('\n', Qt::SkipEmptyParts);
            for (QString res : eixOut) {
                res = res.trimmed(); bool found = false;
                for(int i=0; i<pkgList->count(); ++i) {
                    if(pkgList->item(i)->text().contains(res)) { found = true; break; }
                }
                if (!found) {
                    auto *it = new QListWidgetItem(res + " (REPO)"); it->setForeground(QColor("#666666"));
                    QFont font = it->font(); font.setItalic(true); it->setFont(font); it->setData(Qt::UserRole, 999); pkgList->addItem(it);
                }
            }
        }
        pkgList->setUpdatesEnabled(true);
    }

    void refreshList(const QStringList &l) {
        for (const QString &p : l) {
            bool hasUpdate = false;
            QString pFull = p.trimmed();
            QString pNameOnly = pFull;
            pNameOnly = pNameOnly.remove(QRegularExpression("-[0-9].*$"));

            for (const QString &up : updatablePkgs) {
                QString upClean = up.trimmed();
                if (upClean == pNameOnly || upClean.contains(pNameOnly) || pNameOnly.contains(upClean)) {
                    hasUpdate = true;
                    break;
                }
            }

            auto *it = new QListWidgetItem(pFull + (hasUpdate ? " [UPDATE]" : ""));
            it->setForeground(QColor(hasUpdate ? "#00ff00" : "#eeeeee"));
            it->setData(Qt::UserRole, hasUpdate ? 888 : 0);
            pkgList->addItem(it);
        }
    }

//czesc 4/5

    void checkUpdates() {
        globalUpdateBtn->setEnabled(false);
        topCleanBtn->setEnabled(false);
        updatablePkgs.clear();

        // Informujemy użytkownika o rozpoczęciu rzetelnego skanowania
        consoleOutput->clear();
        consoleOutput->append("<b style='color:#fb8c00;'>[System] Analyzing dependency tree via Emerge...</b>");
        consoleOutput->append("<i style='color:#888;'>Checking --update --newuse --deep @world. Interface is locked for safety.</i>");

        // Blokujemy interfejs na czas skanowania (zostawiamy tylko About)
        setInterfaceEnabled(false);

        QProcess *p = new QProcess(this);
        connect(p, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), [this, p]() {
            QString output = QString::fromLocal8Bit(p->readAllStandardOutput());
            QStringList lines = output.split('\n', Qt::SkipEmptyParts);
            updatablePkgs.clear();

            // PRECYZYJNY REGEX: Szukamy tylko linii zaczynających się od [ebuild U ]
            // Ignorujemy pakiety do przebudowy (R), bo ich wersja się nie zmienia.
            QRegularExpression updateRegex("^\\[ebuild\\s+U.*?\\]\\s+([^\\s]+)");

            for (const QString &line : lines) {
                QRegularExpressionMatch match = updateRegex.match(line.trimmed());
                if (match.hasMatch()) {
                    QString pkgFull = match.captured(1);
                    // Czyścimy nazwę z wersji dla poprawnego dopasowania na liście
                    QString pkgClean = pkgFull.remove(QRegularExpression("-[0-9].*$"));
                    if (!updatablePkgs.contains(pkgClean)) {
                        updatablePkgs.append(pkgClean);
                    }
                }
            }

            // ODBLOKOWANIE: Przywracamy przyciski i informujemy o wyniku
            setInterfaceEnabled(true);
            globalUpdateBtn->setEnabled(!updatablePkgs.isEmpty());
            topCleanBtn->setEnabled(true);

            consoleOutput->append(QString("<b style='color:#00ff00;'>[System] Analysis complete. %1 actual updates found.</b>").arg(updatablePkgs.size()));

            // Odświeżamy widok listy i statystyki w panelu bocznym
            handleSearch(searchBar->text());
            updateStats();
            p->deleteLater();
        });

        // Prawdziwy skan Gentoo: sprawdzamy aktualizacje, zmiany w flagach USE i głębokie zależności
        p->start("emerge", {"-p", "--color=n", "--update", "--deep", "--newuse", "--with-bdeps=y", "@world"});
    }

    void handleSync() {
        syncBtn->setEnabled(false);
        setInterfaceEnabled(false); // Blokada interfejsu na czas synchronizacji
        consoleOutput->clear();
        progressBar->setVisible(true);
        progressBar->setRange(0, 0); // Tryb zajętości (pulse)

        consoleOutput->append("<b style='color:#00ffcc;'>[System] Synchronizing all repositories (emaint sync)...</b>");

        // Po zakończeniu sync, proces 'finished' w części 2/5 automatycznie wywoła checkUpdates
        installProcess->start("script", {"-q", "-c", "sudo emaint sync -a && sudo eix-update", "/dev/null"});
    }

    void startEmerge(QString f) {
        QString p = "";
        // Jeśli nie jest to operacja na całym świecie (@world) lub depclean, zbieramy wybrane pakiety
        if (!f.contains("@world") && !f.contains("--depclean")) {
            auto sel = pkgList->selectedItems();
            if (sel.isEmpty()) return;
            for (auto *it : sel) {
                // Wyciągamy czysty atom pakietu bez dopisków [UPDATE] czy (REPO)
                QString cleanAtom = it->text().remove("[UPDATE]").remove("(REPO)").trimmed().remove(QRegularExpression("-[0-9].*$"));
                p += cleanAtom + " ";
            }
        }

        consoleOutput->clear();
        confirmWidget->setVisible(false);
        setInterfaceEnabled(false); // Blokada na czas trwania operacji emerge

        progressBar->setVisible(true);
        progressBar->setRange(0, 0);

        // Flagi automatyzujące unmaskowanie i zapisywanie zmian w configach, o które prosiłeś
        QString autoUnmaskFlags = "--autounmask=y --autounmask-write=y --autounmask-continue=y";

        consoleOutput->append("<b style='color:#00ffcc;'>[System] Running: emerge " + f + " " + p + "</b>");
        installProcess->start("script", {"-q", "-c", "sudo emerge --color=y " + autoUnmaskFlags + " " + f + " " + p, "/dev/null"});
    }

//czesc 5/5

    void setInterfaceEnabled(bool enable) {
        syncBtn->setEnabled(enable);
        etcUpBtn->setEnabled(enable);
        ovlBtn->setEnabled(enable);
        searchBar->setEnabled(enable);
        pkgList->setEnabled(enable);
        installBtn->setEnabled(enable);
        reinstallBtn->setEnabled(enable);
        useBtn->setEnabled(enable);
        uninstallBtn->setEnabled(enable);
        infoBtn->setEnabled(enable);
        // aboutBtn zawsze aktywny dla bezpieczeństwa i informacji
    }

    void handleUseEdit() {
        auto sel = pkgList->selectedItems(); if (sel.isEmpty()) return;
        setInterfaceEnabled(false);
        QString pkgFull = sel.first()->text().remove("[UPDATE]").remove("(REPO)").trimmed().split(' ').first();
        QString pkgAtom = pkgFull.remove(QRegularExpression("-[0-9].*$"));

        consoleOutput->clear(); consoleOutput->setReadOnly(false);
        consoleOutput->setStyleSheet("background-color: #000; color: #00ff00; font-family: monospace; border: 2px solid #004400;");
        editorTools->setVisible(true); consoleInput->setVisible(false);

        QProcess qp; qp.start("equery", {"-q", "uses", pkgAtom}); qp.waitForFinished();
        QString out = QString::fromLocal8Bit(qp.readAllStandardOutput());
        QStringList lines = out.split('\n', Qt::SkipEmptyParts);

        consoleOutput->append("# --- CURRENT USE FLAGS FOR: " + pkgAtom + " ---");
        for (const QString &line : lines) {
            if (line.startsWith("+") || line.startsWith("-")) consoleOutput->append("#" + line);
        }
        consoleOutput->append("\n# ADD YOUR CHANGES BELOW (format: category/package flag1 -flag2):");

        QFile file("/etc/portage/package.use/zzz_portager_use");
        bool found = false;
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&file);
            while (!in.atEnd()) {
                QString line = in.readLine();
                if (line.contains(pkgAtom) && !line.startsWith("#")) { consoleOutput->append(line); found = true; }
            }
            file.close();
        }
        if (!found) consoleOutput->append(pkgAtom + " ");
        consoleOutput->moveCursor(QTextCursor::End);
    }

    void readInstallOutput() {
        QByteArray d = installProcess->readAllStandardOutput() + installProcess->readAllStandardError();
        if (d.isEmpty()) return;

        QString t = QString::fromLocal8Bit(d);

        // Automatyczna obsługa unmasków i zmian w configach (etc-update)
        if (t.contains("Autounmask changes successfully written") || t.contains("configuration files in need of updating")) {
            QProcess::execute("bash", {"-c", "sudo etc-update --automode -5"});
            consoleOutput->append("<b style='color:#00ffcc;'>[Automation] Config files updated automatically.</b>");
        }

        // OPTYMALIZACJA MATRIX: Brak undo i czysty tekst
        consoleOutput->setUndoRedoEnabled(false);
        t.replace(QRegularExpression("\x1B\\][0-9];.*?\x07|\x1B\\[[0-9;]*[a-zA-Z]"), "");
        t.remove('\r');

        consoleOutput->moveCursor(QTextCursor::End);
        consoleOutput->insertPlainText(t);

        // SZTYWNY LIMIT 100 LINII - zapobiega zamarzaniu przy qtwebkit/llvm/rust
        if (consoleOutput->document()->blockCount() > 100) {
            QTextCursor cursor = consoleOutput->textCursor();
            cursor.movePosition(QTextCursor::Start);
            cursor.movePosition(QTextCursor::Down, QTextCursor::KeepAnchor, consoleOutput->document()->blockCount() - 100);
            cursor.removeSelectedText();
        }

        if (t.contains("[Yes/No]")) confirmWidget->setVisible(true);
        consoleOutput->ensureCursorVisible();
    }

    void handleInfo() {
        auto sel = pkgList->selectedItems(); if (sel.isEmpty()) return;
        QString pP = sel.first()->text().remove("[UPDATE]").remove("(REPO)").trimmed().split(' ').first().remove(QRegularExpression("-[0-9].*$"));
        consoleOutput->clear();
        QProcess eP; eP.start("eix", {"--pure-packages", "--exact", pP}); eP.waitForFinished();
        QString out = QString::fromLocal8Bit(eP.readAllStandardOutput());
        if (out.isEmpty()) { // Jeśli brak dokładnego dopasowania, szukaj substringa
             eP.start("eix", {"--pure-packages", pP}); eP.waitForFinished();
             out = QString::fromLocal8Bit(eP.readAllStandardOutput());
        }
        consoleOutput->append("<b style='color:#00ffcc;'>--- INSPECT: " + pP + " ---</b><br>");
        consoleOutput->append(out);
    }

    void showContextMenu(const QPoint &pos) {
        if (editorTools->isVisible()) return; // Blokada menu w trybie edycji flag
        auto sel = pkgList->selectedItems(); if (sel.isEmpty()) return;

        QMenu m(this);
        m.setStyleSheet("QMenu { background:#222; color:#fff; } QMenu::item:selected { background:#00ffcc; color:#000; }");

        if (isOverlayMode) {
            m.addAction("🌐 Toggle Repository", [this]() { handleOverlayAction(); });
        } else {
            int type = sel.first()->data(Qt::UserRole).toInt();
            QString pClean = sel.first()->text().remove("[UPDATE]").remove("(REPO)").trimmed().split(' ').first().remove(QRegularExpression("-[0-9].*$"));

            if (type == 999) {
                m.addAction("➕ Install", [this]() { startEmerge("-av"); });
            } else if (type == 888) {
                m.addAction("🚀 Upgrade", [this]() { startEmerge("-avuND"); });
                m.addAction("🔄 Rebuild", [this]() { startEmerge("-av --oneshot"); });
            } else {
                m.addAction("🔄 Rebuild", [this]() { startEmerge("-av --oneshot"); });
            }

            m.addSeparator();
            m.addAction("🔍 Inspect", [this]() { handleInfo(); });
            m.addAction("⚙️ UseFlags", [this]() { handleUseEdit(); });

            m.addSeparator();
            m.addAction("🔓 Unmask (~amd64)", [this, pClean]() {
                QProcess::execute("bash", {"-c", QString("echo '%1 ~amd64' | sudo tee -a /etc/portage/package.accept_keywords/zzz_portager_keywords").arg(pClean)});
                consoleOutput->append("<b style='color:#00ff00;'>[System] Unmasked " + pClean + ". Scanning...</b>");
                checkUpdates();
            });

            m.addSeparator();
            m.addAction("🗑️ Remove", [this]() { handleSecureRemove(); });
        }
        m.exec(pkgList->mapToGlobal(pos));
    }
};

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    GentooManager w;
    w.show();
    return a.exec();
}
