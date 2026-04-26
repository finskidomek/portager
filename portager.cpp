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
#include <QMouseEvent>
#include <ctime>
#include <cstdlib>

class GentooManager : public QWidget {
    // --- DEKLARACJE ZMIENNYCH (Przeniesione tutaj, by konstruktor je widział) ---
private:
    QString terminal; QListWidget *pkgList; QLineEdit *searchBar; QTextEdit *consoleOutput;
    QLabel *statsLabel, *selectionLabel, *legendHeader;
    QPushButton *syncBtn, *globalUpdateBtn, *topCleanBtn, *etcUpBtn, *ovlBtn, *aboutBtn;
    QPushButton *installBtn, *reinstallBtn, *uninstallBtn, *useBtn, *infoBtn, *saveFlagsBtn, *cancelUseBtn;
    QPushButton *confirmBtn, *cancelBtn; QWidget *confirmWidget, *legendItemsWidget, *legendContainer, *editorTools;
    QVBoxLayout *legendItemsLayout;
    QStringList localPkgs, updatablePkgs;
    QProcess *installProcess;

    bool isOverlayMode = false;
    bool isPurgeMode = false;
    QString currentAction = "IDLE";
    QString kernelVer;
    QString gccVer;
    QString clangVer;
    int totalAvailable = 0;
    QTimer *aboutPulseTimer;

public:
    GentooManager(QWidget *parent = nullptr) : QWidget(parent) {
        QSettings settings("GentooManager", "GentooManager");
        terminal = settings.value("terminal", "kitty").toString();
        isOverlayMode = false;
        isPurgeMode = false;

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

        // --- MAIN AREA ---
        auto *mainSplitter = new QSplitter(Qt::Horizontal);
        auto *centerContainer = new QWidget();
        auto *centerLayout = new QVBoxLayout(centerContainer);
        centerLayout->setContentsMargins(0,0,0,0);

        pkgList = new QListWidget();
        QString imgPath = "/usr/share/portager/background.png";
        pkgList->setStyleSheet(QString("QListWidget { background-image: url('%1'); background-repeat: no-repeat; background-position: center; background-attachment: fixed; background-color: #121212; color: #e0e0e0; border: 1px solid #333; }").arg(imgPath));
        pkgList->setSelectionMode(QAbstractItemView::ExtendedSelection);
        pkgList->setContextMenuPolicy(Qt::CustomContextMenu);

        pkgList->viewport()->installEventFilter(this);
        centerLayout->addWidget(pkgList);

        consoleOutput = new QTextEdit();
        consoleOutput->setReadOnly(true);
        consoleOutput->setStyleSheet("background-color: #000; color: #00ff00; font-family: monospace; border: 1px solid #333;");
        consoleOutput->setVisible(false);
        centerLayout->addWidget(consoleOutput);

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
        centerLayout->addWidget(editorTools);

        confirmWidget = new QWidget();
        auto *confirmLayout = new QHBoxLayout(confirmWidget);
        confirmLayout->setContentsMargins(5,5,5,5);
        confirmBtn = new QPushButton("✔️ Confirm (Yes)");
        cancelBtn = new QPushButton("✖️ Cancel (No)");
        confirmBtn->setStyleSheet("background-color: #2e7d32; color: white; font-weight: bold; padding: 8px;");
        cancelBtn->setStyleSheet("background-color: #c62828; color: white; font-weight: bold; padding: 8px;");
        confirmLayout->addWidget(confirmBtn); confirmLayout->addWidget(cancelBtn);
        confirmWidget->setVisible(false);
        centerLayout->addWidget(confirmWidget);

        mainSplitter->addWidget(centerContainer);

        legendContainer = new QWidget();
        legendContainer->setFixedWidth(250);
        legendContainer->setStyleSheet("background: #1a1a1a; border-left: 1px solid #333;");
        auto *legendLayout = new QVBoxLayout(legendContainer);
        legendLayout->setAlignment(Qt::AlignTop);
        legendHeader = new QLabel("PKG INFO");
        legendHeader->setStyleSheet("color: #00ffcc; font-weight: bold; font-size: 11px; margin-bottom: 8px;");
        legendHeader->setAlignment(Qt::AlignCenter);
        legendLayout->addWidget(legendHeader);
        legendItemsWidget = new QWidget();
        legendItemsLayout = new QVBoxLayout(legendItemsWidget);
        legendItemsLayout->setContentsMargins(5,0,5,0);
        legendLayout->addWidget(legendItemsWidget);
        legendLayout->addStretch();
        mainSplitter->addWidget(legendContainer);
        mainLayout->addWidget(mainSplitter);

        auto *btnLayout = new QHBoxLayout();
        selectionLabel = new QLabel("Selected: 0");
        selectionLabel->setStyleSheet("color: #00ffcc; font-weight: bold;");
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

        std::srand(static_cast<unsigned int>(std::time(nullptr)));
        aboutPulseTimer->start(6000);

        connect(saveFlagsBtn, &QPushButton::clicked, this, &GentooManager::saveUseFlags);
        connect(cancelUseBtn, &QPushButton::clicked, [this]() {
            consoleOutput->clear(); consoleOutput->setReadOnly(true);
            editorTools->setVisible(false);
            consoleOutput->setVisible(false); pkgList->setVisible(true);
            currentAction = "IDLE";
            updateLegend(isOverlayMode);
            setInterfaceEnabled(true);
        });

        connect(ovlBtn, &QPushButton::clicked, this, &GentooManager::toggleOverlayMode);
        connect(infoBtn, &QPushButton::clicked, this, &GentooManager::handleInfo);
        connect(etcUpBtn, &QPushButton::clicked, [this]() {
            pkgList->setVisible(false);
            consoleOutput->setVisible(true);
            consoleOutput->clear();
            currentAction = "ETC-UPDATE";
            updateLegend(isOverlayMode);
            consoleOutput->append("<b style='color:#fb8c00;'>[System] Starting interactive etc-update...</b>");
            installProcess->start("script", {"-q", "-c", "sudo etc-update", "/dev/null"});
        });

        connect(installProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), [this](int exitCode) {
            confirmWidget->setVisible(false);
            if (currentAction == "ETC-UPDATE" || currentAction == "FINISHED") {
                return;
            }
            currentAction = "IDLE";
            if (exitCode == 0 && !isPurgeMode) {
                consoleOutput->setVisible(false);
                pkgList->setVisible(true);
                loadLocalPackages();
                checkUpdates();
            } else if (isPurgeMode) {
                consoleOutput->append("<i style='color:#777;'>[System] Operation ended.</i>");
            }
            updateLegend(isOverlayMode);
            setInterfaceEnabled(true);
        });

        connect(searchBar, &QLineEdit::textChanged, this, &GentooManager::handleSearch);
        connect(syncBtn, &QPushButton::clicked, this, &GentooManager::handleSync);
        connect(globalUpdateBtn, &QPushButton::clicked, [this]() { startEmerge("-avuDU --with-bdeps=y @world"); });
        connect(topCleanBtn, &QPushButton::clicked, this, &GentooManager::handlePurge);
        connect(installBtn, &QPushButton::clicked, [this]() { if (!isOverlayMode) startEmerge("-av"); else handleOverlayAction(); });
        connect(reinstallBtn, &QPushButton::clicked, [this]() { startEmerge("-av --oneshot"); });
        connect(uninstallBtn, &QPushButton::clicked, [this]() { handleSecureRemove(); });
        connect(useBtn, &QPushButton::clicked, this, &GentooManager::handleUseEdit);
        connect(pkgList, &QListWidget::customContextMenuRequested, [this](const QPoint &pos) { this->showContextMenu(pos); });

        connect(installProcess, &QProcess::readyReadStandardOutput, this, &GentooManager::readInstallOutput);
        connect(installProcess, &QProcess::readyReadStandardError, this, &GentooManager::readInstallOutput);

        connect(confirmBtn, &QPushButton::clicked, [this]() {
            if (isPurgeMode) onConfirmPurge();
            else { installProcess->write("y\n"); confirmWidget->setVisible(false); }
        });
        connect(cancelBtn, &QPushButton::clicked, [this]() {
            if (isPurgeMode) onCancelPurge();
            else if (installProcess->state() == QProcess::Running) { installProcess->write("n\n"); confirmWidget->setVisible(false); }
        });

        loadLocalPackages();
        setInterfaceEnabled(false);
        checkUpdates();
    }

    bool eventFilter(QObject *obj, QEvent *event) override {
        if (isPurgeMode && obj == pkgList->viewport() && event->type() == QEvent::MouseButtonPress) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
            QListWidgetItem *item = pkgList->itemAt(mouseEvent->pos());
            if (item) {
                item->setCheckState(item->checkState() == Qt::Checked ? Qt::Unchecked : Qt::Checked);
                return true;
            }
        }
        return QWidget::eventFilter(obj, event);
    }

    void updateLegend(bool overlayMode) {
        QLayoutItem *child;
        while ((child = legendItemsLayout->takeAt(0)) != nullptr) { if (child->widget()) delete child->widget(); delete child; }
        auto addLeg = [&](QString c, QString t, bool bold = false) {
            auto *h = new QLabel(QString("<span style='color:%1; font-weight:%2;'>%3</span>").arg(c, bold ? "bold" : "normal", t));
            legendItemsLayout->addWidget(h);
        };
        addLeg("#00ffcc", "STATUS: " + currentAction, true);
        legendItemsLayout->addSpacing(10);
        addLeg("#888", "OS: Gentoo Linux");
        addLeg("#888", "Kernel: " + kernelVer);
        addLeg("#888", "GCC: " + gccVer);
        addLeg("#888", "Clang: " + clangVer);
        addLeg("#00ffcc", QString("Installed: %1").arg(localPkgs.size()), true);
        addLeg("#fb8c00", QString("Updatable: %1").arg(updatablePkgs.size()), true);
        addLeg("#555", QString("Repo size: %1").arg(totalAvailable));
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
        if (warn.exec() == QMessageBox::Yes) {
            pkgList->setVisible(false);
            consoleOutput->setVisible(true);
            startEmerge("-avC");
        }
    }

    void toggleOverlayMode() {
        isOverlayMode = !isOverlayMode;
        ovlBtn->setText(isOverlayMode ? "🔙 Back" : "🌐 Overlays");
        updateLegend(isOverlayMode);
        consoleOutput->setVisible(false);
        editorTools->setVisible(false);
        pkgList->setVisible(true);

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

        pkgList->setVisible(false);
        consoleOutput->setVisible(true);
        consoleOutput->clear();
        consoleOutput->append("<b style='color:#5e35b1;'>[System] Repository " + act + ": " + name + "...</b>");
        installProcess->start("script", {"-q", "-c", cmd, "/dev/null"});
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
        if (isOverlayMode || isPurgeMode) return;
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
        QStringList updates;
        QStringList others;
        for (const QString &p : l) {
            bool hasUpdate = false;
            QString pNameOnly = p.trimmed().remove(QRegularExpression("-[0-9].*$"));
            for (const QString &up : updatablePkgs) {
                if (up.trimmed() == pNameOnly || up.contains(pNameOnly) || pNameOnly.contains(up.trimmed())) {
                    hasUpdate = true;
                    break;
                }
            }
            if (hasUpdate) updates << (p.trimmed() + " [UPDATE]");
            else others << p.trimmed();
        }
        updates.sort(Qt::CaseInsensitive); others.sort(Qt::CaseInsensitive);
        for (const QString &p : updates) {
            auto *it = new QListWidgetItem(p);
            it->setForeground(QColor("#00ff00"));
            it->setData(Qt::UserRole, 888);
            pkgList->addItem(it);
        }
        for (const QString &p : others) {
            auto *it = new QListWidgetItem(p);
            it->setForeground(QColor("#eeeeee"));
            it->setData(Qt::UserRole, 0);
            pkgList->addItem(it);
        }
    }

// --- CZĘŚĆ 4/5: Logika aktualizacji, synchronizacji i operacji Emerge ---

  void checkUpdates() {
        if (isPurgeMode) return;
        currentAction = "ANALYZING";
        updateLegend(isOverlayMode);

        globalUpdateBtn->setEnabled(false);
        topCleanBtn->setEnabled(false);
        updatablePkgs.clear();

        consoleOutput->clear();
        consoleOutput->append("<b style='color:#fb8c00;'>[System] Analyzing dependency tree via Emerge...</b>");
        consoleOutput->append("<i style='color:#888;'>Checking --update --newuse --deep @world. Interface is locked for safety.</i>");

        setInterfaceEnabled(false);

        QProcess *p = new QProcess(this);
        connect(p, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), [this, p]() {
            QString output = QString::fromLocal8Bit(p->readAllStandardOutput());
            QStringList lines = output.split('\n', Qt::SkipEmptyParts);
            updatablePkgs.clear();

            QRegularExpression updateRegex("^\\[ebuild\\s+U.*?\\]\\s+([^\\s]+)");

            for (const QString &line : lines) {
                QRegularExpressionMatch match = updateRegex.match(line.trimmed());
                if (match.hasMatch()) {
                    QString pkgFull = match.captured(1);
                    QString pkgClean = pkgFull.remove(QRegularExpression("-[0-9].*$"));
                    if (!updatablePkgs.contains(pkgClean)) {
                        updatablePkgs.append(pkgClean);
                    }
                }
            }

            setInterfaceEnabled(true);
            globalUpdateBtn->setEnabled(!updatablePkgs.isEmpty());
            topCleanBtn->setEnabled(true);

            currentAction = "IDLE";
            updateLegend(isOverlayMode);

            consoleOutput->append(QString("<b style='color:#00ff00;'>[System] Analysis complete. %1 actual updates found.</b>").arg(updatablePkgs.size()));

            handleSearch(searchBar->text());
            updateStats();
            p->deleteLater();
        });

        p->start("emerge", {"-p", "--color=n", "--update", "--deep", "--newuse", "--with-bdeps=y", "@world"});
    }

    void handleSync() {
        syncBtn->setEnabled(false);
        setInterfaceEnabled(false);

        currentAction = "SYNCING";
        updateLegend(isOverlayMode);

        pkgList->setVisible(false);
        consoleOutput->setVisible(true);
        consoleOutput->clear();

        consoleOutput->append("<b style='color:#00ffcc;'>[System] Synchronizing all repositories (emaint sync)...</b>");

        installProcess->start("script", {"-q", "-c", "sudo emaint sync -a && sudo eix-update", "/dev/null"});
    }

    void startEmerge(QString f) {
        QString p = "";
        if (!f.contains("@world") && !f.contains("--depclean")) {
            auto sel = pkgList->selectedItems();
            if (sel.isEmpty()) return;
            for (auto *it : sel) {
                QString cleanAtom = it->text().remove("[UPDATE]").remove("(REPO)").trimmed().remove(QRegularExpression("-[0-9].*$"));
                p += cleanAtom + " ";
            }
        }

        consoleOutput->clear();
        setInterfaceEnabled(false);

        if (f.contains("-avC")) currentAction = "REMOVING";
        else if (f.contains("@world")) currentAction = "UPGRADING";
        else currentAction = "EMERGE";

        updateLegend(isOverlayMode);

        pkgList->setVisible(false);
        consoleOutput->setVisible(true);

        confirmWidget->setVisible(false);

        QString autoUnmaskFlags = "--autounmask=y --autounmask-write=y --autounmask-continue=y";

        consoleOutput->append("<b style='color:#00ffcc;'>[System] Running: emerge " + f + " " + p + "</b>");
        installProcess->start("script", {"-q", "-c", "sudo emerge --color=y " + autoUnmaskFlags + " " + f + " " + p, "/dev/null"});
    }

// --- CZĘŚĆ 5/5: Logika interfejsu, Interaktywny Purge i domknięcie klasy ---

    void setInterfaceEnabled(bool enable) {
        syncBtn->setEnabled(enable);
        globalUpdateBtn->setEnabled(enable);
        topCleanBtn->setEnabled(enable);
        etcUpBtn->setEnabled(enable);
        ovlBtn->setEnabled(enable);
        searchBar->setEnabled(enable);
        pkgList->setEnabled(enable);
        installBtn->setEnabled(enable);
        reinstallBtn->setEnabled(enable);
        useBtn->setEnabled(enable);
        uninstallBtn->setEnabled(enable);
        infoBtn->setEnabled(enable);
    }

    void handlePurge() {
        isPurgeMode = true;
        currentAction = "PURGE SCAN";
        updateLegend(isOverlayMode);
        consoleOutput->clear();
        consoleOutput->append("<b style='color:#fb8c00;'>[System] Scanning for orphaned packages (interactively)...</b>");
        setInterfaceEnabled(false);

        QProcess *p = new QProcess(this);
        connect(p, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), [this, p]() {
            QString out = QString::fromLocal8Bit(p->readAllStandardOutput());
            pkgList->clear();
            pkgList->setSelectionMode(QAbstractItemView::NoSelection);

            QStringList lines = out.split('\n', Qt::SkipEmptyParts);
            QString allSelectedLine = "";
            for (const QString &line : lines) {
                if (line.contains("All selected packages:")) {
                    allSelectedLine = line.mid(line.indexOf(":") + 1).trimmed();
                    break;
                }
            }

            if (!allSelectedLine.isEmpty()) {
                QStringList atoms = allSelectedLine.split(' ', Qt::SkipEmptyParts);
                for (QString atom : atoms) {
                    atom = atom.remove("=").trimmed();
                    if (!atom.isEmpty()) {
                        auto *it = new QListWidgetItem(atom);
                        it->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
                        it->setCheckState(Qt::Checked);
                        it->setForeground(QColor("#ff5555"));
                        pkgList->addItem(it);
                    }
                }
            }

            if (pkgList->count() > 0) {
                pkgList->setVisible(true);
                consoleOutput->setVisible(false);
                confirmWidget->setVisible(true);
                confirmWidget->raise();
                currentAction = "CONFIRM PURGE";
                updateLegend(isOverlayMode);
                selectionLabel->setText("Select to purge:");
            } else {
                QMessageBox::information(this, "Purge", "No orphaned packages found.");
                onCancelPurge();
            }
            p->deleteLater();
        });
        p->start("emerge", {"--depclean", "-p", "--color=n"});
    }

    void onConfirmPurge() {
        QString toRemove = "";
        QString toKeep = "";
        for (int i = 0; i < pkgList->count(); ++i) {
            if (pkgList->item(i)->checkState() == Qt::Checked) toRemove += " =" + pkgList->item(i)->text();
            else toKeep += " =" + pkgList->item(i)->text();
        }

        confirmWidget->setVisible(false);
        pkgList->setVisible(false);
        consoleOutput->setVisible(true);
        consoleOutput->clear();
        pkgList->setSelectionMode(QAbstractItemView::ExtendedSelection);

        if (!toKeep.isEmpty()) {
            consoleOutput->append("<b style='color:#00ffcc;'>[System] Saving kept packages to @world...</b>");
            QProcess::execute("bash", {"-c", "sudo emerge --noreplace " + toKeep});
        }

        if (toRemove.isEmpty()) {
            onCancelPurge();
            return;
        }

        currentAction = "PURGING...";
        updateLegend(isOverlayMode);
        consoleOutput->append("<b style='color:#fb8c00;'>[System] Removing selected packages...</b>");
        installProcess->start("script", {"-q", "-c", "sudo emerge --depclean " + toRemove, "/dev/null"});
        isPurgeMode = false;
    }

    void onCancelPurge() {
        confirmWidget->setVisible(false);
        isPurgeMode = false;
        currentAction = "IDLE";
        setInterfaceEnabled(true);
        pkgList->setVisible(true);
        consoleOutput->setVisible(false);
        pkgList->setSelectionMode(QAbstractItemView::ExtendedSelection);
        updateLegend(isOverlayMode);
        loadLocalPackages();
    }

    void handleUseEdit() {
        auto sel = pkgList->selectedItems(); if (sel.isEmpty()) return;
        setInterfaceEnabled(false);
        currentAction = "EDITING USE";
        updateLegend(isOverlayMode);
        QString pkgFull = sel.first()->text().remove("[UPDATE]").remove("(REPO)").trimmed().split(' ').first();
        QString pkgAtom = pkgFull.contains("/") ? pkgFull.remove(QRegularExpression("-[0-9].*$")) : pkgFull;

        pkgList->setVisible(false);
        consoleOutput->setVisible(true);
        consoleOutput->clear();
        consoleOutput->setReadOnly(false);
        consoleOutput->setStyleSheet("background-color: #000; color: #00ff00; font-family: monospace; border: 2px solid #004400;");
        editorTools->setVisible(true);

        QProcess qp; qp.start("equery", {"-q", "uses", pkgAtom}); qp.waitForFinished();
        QString out = QString::fromLocal8Bit(qp.readAllStandardOutput());
        QStringList lines = out.split('\n', Qt::SkipEmptyParts);

        consoleOutput->append("# --- CURRENT USE FLAGS FOR: " + pkgAtom + " ---");
        for (const QString &line : lines) if (line.startsWith("+") || line.startsWith("-")) consoleOutput->append("#" + line);
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

        // Pokazujemy przyciski Confirm/Cancel jeśli Emerge o coś pyta
        if (t.contains("Would you like to merge") || t.contains("Do you want to unmerge") || t.contains("]?")) {
            confirmWidget->setVisible(true);
            confirmWidget->raise();
        }

        // POPRAWKA ETC-UPDATE: Czytelny komunikat i stabilny powrót
        if (currentAction == "ETC-UPDATE" && (t.contains("Exiting") || t.contains("Nothing left to do"))) {
            // Zmieniamy akcję na FINISHED, żeby Timer nie odpalał się kilka razy
            currentAction = "FINISHED";
            updateLegend(isOverlayMode);

            consoleOutput->clear(); // Czyścimy szum terminala
            consoleOutput->append("<br><br><center><b style='color:#00ff00; font-size:16px;'>✅ CONFIGURATION UPDATED SUCCESSFULLY!</b><br>");
            consoleOutput->append("<span style='color:#888;'>Returning to package list in 5 seconds...</span></center>");

            QTimer::singleShot(5000, this, [this](){ onCancelPurge(); });
            return; // Kończymy przetwarzanie tego konkretnego ciągu danych
        }

        // Automatyka unmaskingu
        if (t.contains("Autounmask changes") || t.contains("configuration files in need of updating")) {
            QProcess::execute("bash", {"-c", "sudo etc-update --automode -5"});
            consoleOutput->append("<b style='color:#00ffcc;'>[Automation] Config files updated.</b>");
        }

        consoleOutput->setUndoRedoEnabled(false);
        t.replace(QRegularExpression("\x1B\\][0-9];.*?\x07|\x1B\\[[0-9;]*[a-zA-Z]"), "");
        t.remove('\r');
        consoleOutput->moveCursor(QTextCursor::End);
        consoleOutput->insertPlainText(t);
        consoleOutput->ensureCursorVisible();
    }
    void handleInfo() {
        auto sel = pkgList->selectedItems(); if (sel.isEmpty()) return;
        QString pP = sel.first()->text().remove("[UPDATE]").remove("(REPO)").trimmed().split(' ').first().remove(QRegularExpression("-[0-9].*$"));
        pkgList->setVisible(false); consoleOutput->setVisible(true); consoleOutput->clear();
        currentAction = "INSPECTING"; updateLegend(isOverlayMode);
        QProcess eP; eP.start("eix", {"--pure-packages", "--exact", pP}); eP.waitForFinished();
        QString out = QString::fromLocal8Bit(eP.readAllStandardOutput());
        if (out.isEmpty()) { eP.start("eix", {"--pure-packages", pP}); eP.waitForFinished(); out = QString::fromLocal8Bit(eP.readAllStandardOutput()); }
        consoleOutput->append("<b style='color:#00ffcc;'>--- INSPECT: " + pP + " ---</b><br>");
        consoleOutput->append(out);
    }

    void showContextMenu(const QPoint &pos) {
        if (editorTools->isVisible() || isPurgeMode) return;
        auto sel = pkgList->selectedItems(); if (sel.isEmpty()) return;
        QMenu m(this);
        m.setStyleSheet("QMenu { background:#222; color:#fff; } QMenu::item:selected { background:#00ffcc; color:#000; }");
        if (isOverlayMode) m.addAction("🌐 Toggle Repository", [this]() { handleOverlayAction(); });
        else {
            int type = sel.first()->data(Qt::UserRole).toInt();
            QString pClean = sel.first()->text().remove("[UPDATE]").remove("(REPO)").trimmed().split(' ').first().remove(QRegularExpression("-[0-9].*$"));
            if (type == 999) m.addAction("➕ Install", [this]() { currentAction = "INSTALLING"; startEmerge("-av"); });
            else if (type == 888) {
                m.addAction("🚀 Upgrade", [this]() { currentAction = "UPGRADING"; startEmerge("-avuND"); });
                m.addAction("🔄 Rebuild", [this]() { currentAction = "REBUILDING"; startEmerge("-av --oneshot"); });
            } else m.addAction("🔄 Rebuild", [this]() { currentAction = "REBUILDING"; startEmerge("-av --oneshot"); });
            m.addSeparator();
            m.addAction("🔍 Inspect", [this]() { handleInfo(); });
            m.addAction("⚙️ UseFlags", [this]() { handleUseEdit(); });
            m.addSeparator();
            m.addAction("🔓 Unmask (~amd64)", [this, pClean]() {
                QProcess::execute("bash", {"-c", QString("echo '%1 ~amd64' | sudo tee -a /etc/portage/package.accept_keywords/zzz_portager_keywords").arg(pClean)});
                consoleOutput->append("<b style='color:#00ff00;'>[System] Unmasked " + pClean + ".</b>");
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
    GentooManager w; w.show();
    return a.exec();
}
