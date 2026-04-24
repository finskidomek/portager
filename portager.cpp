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
        statsLabel->setStyleSheet("font-weight: bold; color: #00ffcc; background: #222; padding: 10px; border-radius: 5px;");

        syncBtn = new QPushButton("🔄 Sync");
        globalUpdateBtn = new QPushButton("🚀 Upgrade");
        globalUpdateBtn->setEnabled(false);
        topCleanBtn = new QPushButton("🧹 Purge");
        etcUpBtn = new QPushButton("🔧 EtcUp");
        ovlBtn = new QPushButton("🌐 Overlays");
        auto *aboutBtn = new QPushButton("ℹ️ About");

        etcUpBtn->setStyleSheet("background-color: #fb8c00; color: white; font-weight: bold;");
        ovlBtn->setStyleSheet("background-color: #5e35b1; color: white; font-weight: bold;");

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

        // --- LIST, CONSOLE & INFO PANEL ---
        auto *splitter = new QSplitter(Qt::Vertical);
        auto *listWithLegendWidget = new QWidget();
        auto *listWithLegendLayout = new QHBoxLayout(listWithLegendWidget);
        listWithLegendLayout->setContentsMargins(0,0,0,0);

        pkgList = new QListWidget();
        QString imgPath = "/usr/share/portager/background.png";
        pkgList->setStyleSheet(QString("QListWidget { background-image: url('%1'); background-repeat: no-repeat; background-position: center; background-attachment: fixed; background-color: #121212; color: #e0e0e0; border: 1px solid #333; }").arg(imgPath));
        pkgList->setSelectionMode(QAbstractItemView::ExtendedSelection);
        pkgList->setContextMenuPolicy(Qt::CustomContextMenu);

        legendContainer = new QWidget();
        legendContainer->setFixedWidth(150);
        legendContainer->setStyleSheet("background: #1a1a1a; border-left: 1px solid #333;");
        auto *legendLayout = new QVBoxLayout(legendContainer);
        legendLayout->setAlignment(Qt::AlignTop);

        legendHeader = new QLabel("PKG INFO");
        legendHeader->setStyleSheet("color: #00ffcc; font-weight: bold; font-size: 9px; letter-spacing: 2px; margin-bottom: 8px;");
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

        // --- CONSOLE AREA ---
        auto *consoleWrapper = new QWidget();
        auto *consoleVLayout = new QVBoxLayout(consoleWrapper);
        consoleVLayout->setContentsMargins(0,0,0,0);
        consoleVLayout->setSpacing(0);

        // Panel narzędziowy dla edytora (z przyciskiem SAVE)
        editorTools = new QWidget();
        auto *toolsLayout = new QHBoxLayout(editorTools);
        toolsLayout->setContentsMargins(5,5,5,5);
        saveFlagsBtn = new QPushButton("💾 SAVE & APPLY CHANGES");
        saveFlagsBtn->setStyleSheet("background-color: #2e7d32; color: white; font-weight: bold; padding: 5px;");
        toolsLayout->addWidget(new QLabel("<b style='color:#00ffcc;'>EDITOR MODE</b>"));
        toolsLayout->addStretch();
        toolsLayout->addWidget(saveFlagsBtn);
        editorTools->setVisible(false);
        consoleVLayout->addWidget(editorTools);

        consoleOutput = new QTextEdit();
        consoleOutput->setReadOnly(true);
        consoleOutput->setStyleSheet("background-color: #000; color: #ffffff; font-family: monospace; border: 1px solid #333;");
        consoleVLayout->addWidget(consoleOutput);

        // KONTENER DLA DOLNEGO WEJŚCIA I PASKA
        auto *bottomInputWrapper = new QWidget();
        auto *bottomInputLayout = new QHBoxLayout(bottomInputWrapper);
        bottomInputLayout->setContentsMargins(0,0,0,0);
        bottomInputLayout->setSpacing(0);

        consoleInput = new QLineEdit();
        consoleInput->setPlaceholderText("Type response (e.g. 1, y, n) and press Enter...");
        consoleInput->setStyleSheet("background-color: #050505; color: #00ffcc; border: 1px solid #333; font-family: monospace; padding: 5px; height: 30px;");
        bottomInputLayout->addWidget(consoleInput);

        // --- NATYWNY PASEK POSTĘPU ---
        progressBar = new QProgressBar();
        progressBar->setVisible(false);
        progressBar->setTextVisible(false);
        progressBar->setStyleSheet(
            "QProgressBar { background-color: #050505; border: 1px solid #333; height: 30px; }"
            "QProgressBar::chunk { background-color: #00ffcc; }"
        );
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

        // --- SIGNALS ---
        connect(aboutBtn, &QPushButton::clicked, [this]() {
            QMessageBox::about(this, "About Portager", "<h2>Portager v2.5</h2><p>You are Terminal-free ;) </p><hr><p style='color:#ff5555;'><b>🚩 ATTENTION:</b> Use with caution.</p>");
        });

        connect(consoleInput, &QLineEdit::returnPressed, [this]() {
            if (installProcess->state() == QProcess::Running) {
                QString cmd = consoleInput->text() + "\n";
                installProcess->write(cmd.toLocal8Bit());
                consoleOutput->append("<span style='color:#555;'> &gt; " + consoleInput->text() + "</span>");
            }
            consoleInput->clear();
        });

        connect(saveFlagsBtn, &QPushButton::clicked, this, &GentooManager::saveUseFlags);
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
            } else {
                bool isEn = (sel.size() == 1 && sel.first()->data(Qt::UserRole).toInt() == 1);
                installBtn->setText(isEn ? "Disable" : "Enable");
                installBtn->setEnabled(sel.size() == 1);
                reinstallBtn->setEnabled(false); useBtn->setEnabled(false); uninstallBtn->setEnabled(false); infoBtn->setEnabled(false);
            }
        });

        connect(searchBar, &QLineEdit::textChanged, this, &GentooManager::handleSearch);
        connect(syncBtn, &QPushButton::clicked, this, &GentooManager::handleSync);
        connect(globalUpdateBtn, &QPushButton::clicked, [this]() { startEmerge("-avuDU --with-bdeps=y @world"); });
        connect(topCleanBtn, &QPushButton::clicked, [this]() { startEmerge("--depclean -p"); });
        connect(installBtn, &QPushButton::clicked, [this]() { if (!isOverlayMode) startEmerge("-av"); else handleOverlayAction(); });
        connect(reinstallBtn, &QPushButton::clicked, [this]() { startEmerge("-av --oneshot"); });
        connect(uninstallBtn, &QPushButton::clicked, [this]() { handleSecureRemove(); });
        connect(useBtn, &QPushButton::clicked, this, &GentooManager::handleUseEdit);
        connect(pkgList, &QListWidget::customContextMenuRequested, this, &GentooManager::showContextMenu);
        connect(installProcess, &QProcess::readyReadStandardOutput, this, &GentooManager::readInstallOutput);
        connect(installProcess, &QProcess::readyReadStandardError, this, &GentooManager::readInstallOutput);

        connect(installProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), [this](int exitCode) {
            progressBar->setRange(0, 100);
            progressBar->setValue(0);
            progressBar->setVisible(false);
            consoleInput->setVisible(true);
            consoleInput->setFocus();

            if (exitCode == 0) {
                loadLocalPackages();
                checkUpdates();
            }
        });

        connect(confirmBtn, &QPushButton::clicked, [this]() { confirmWidget->setVisible(false); installProcess->write("y\n"); });
        connect(cancelBtn, &QPushButton::clicked, [this]() { confirmWidget->setVisible(false); if (installProcess->state() == QProcess::Running) installProcess->write("n\n"); });

        loadLocalPackages();
        checkUpdates();
    }

private:
    QString terminal; QListWidget *pkgList; QLineEdit *searchBar; QTextEdit *consoleOutput;
    QLineEdit *consoleInput; QLabel *statsLabel, *selectionLabel, *legendHeader;
    QProgressBar *progressBar;
    QPushButton *syncBtn, *globalUpdateBtn, *topCleanBtn, *etcUpBtn, *ovlBtn;
    QPushButton *installBtn, *reinstallBtn, *uninstallBtn, *useBtn, *infoBtn, *saveFlagsBtn;
    QPushButton *confirmBtn, *cancelBtn; QWidget *confirmWidget, *legendItemsWidget, *legendContainer, *editorTools;
    QVBoxLayout *legendItemsLayout;
    QStringList localPkgs, updatablePkgs;
    QProcess *installProcess;
    bool isOverlayMode;

    void updateLegend(bool overlayMode) {
        QLayoutItem *child;
        while ((child = legendItemsLayout->takeAt(0)) != nullptr) { if (child->widget()) delete child->widget(); delete child; }
        auto addLeg = [&](QString c, QString t, bool it = false) {
            auto *h = new QHBoxLayout(); auto *b = new QLabel(); b->setFixedSize(10,10);
            b->setStyleSheet(QString("background:%1; border-radius:2px;").arg(c));
            auto *l = new QLabel(t); l->setStyleSheet(QString("font-size:11px; color:#999; %1").arg(it ? "font-style:italic;" : ""));
            h->addWidget(b); h->addWidget(l); h->addStretch(); legendItemsLayout->addLayout(h);
        };
        if (!overlayMode) { legendHeader->setText("PKG INFO"); addLeg("#00ff00", "Update"); addLeg("#eeeeee", "Installed"); addLeg("#555555", "Available", true); }
        else { legendHeader->setText("REPO INFO"); addLeg("#00ffcc", "Enabled"); addLeg("#888888", "Disabled"); }
    }

    void handleUseEdit() {
        if (!pkgList->currentItem()) return;
        QString pkgAtom = pkgList->currentItem()->text().remove("[UPDATE]").remove("(REPO)").trimmed().split(' ').first().remove(QRegularExpression("-[0-9].*$"));
        QString priorityFile = "/etc/portage/package.use/zzz_portager_use";

        QProcess gF; gF.start("bash", {"-c", QString("eix --pure-packages --exact %1 | grep -o '{.*}' | tr -d '{}'").arg(pkgAtom)});
        gF.waitForFinished();
        QString hT = QString("\n# --- PACKAGE: %1 ---\n").arg(pkgAtom);
        for(const QString &f : QString::fromLocal8Bit(gF.readAllStandardOutput()).trimmed().split(' ', Qt::SkipEmptyParts)) hT += QString("# %1\n").arg(f);
        hT += QString("%1 ").arg(pkgAtom);
        QProcess c; c.start("grep", {"-q", pkgAtom, priorityFile}); c.waitForFinished();
        if (c.exitCode() != 0) { QProcess a; a.start("bash", {"-c", QString("echo -e \"%1\" | sudo tee -a %2").arg(hT, priorityFile)}); a.waitForFinished(); }

        QFile file(priorityFile);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            consoleOutput->setReadOnly(false);
            consoleOutput->setPlainText(QString::fromLocal8Bit(file.readAll()));
            consoleOutput->setStyleSheet("background-color: #1a1a2e; color: #ffffff; font-family: monospace; border: 2px solid #00ffcc;");
            editorTools->setVisible(true);
            consoleOutput->verticalScrollBar()->setValue(consoleOutput->verticalScrollBar()->maximum());
            file.close();
        }
    }

    void saveUseFlags() {
        QString content = consoleOutput->toPlainText();
        QString priorityFile = "/etc/portage/package.use/zzz_portager_use";
        QProcess p; p.start("bash", {"-c", QString("sudo tee %1").arg(priorityFile)});
        p.write(content.toLocal8Bit()); p.closeWriteChannel(); p.waitForFinished();

        consoleOutput->setReadOnly(true);
        consoleOutput->setStyleSheet("background-color: #000; color: #ffffff; font-family: monospace; border: 1px solid #333;");
        editorTools->setVisible(false);
        consoleOutput->clear();
        consoleOutput->append("<b style='color:#00ffcc;'>[System] Changes Saved. Analyzing...</b>");
        startEmerge("-pv --changed-use --oneshot @world");
    }

    void handleSecureRemove() {
        auto sel = pkgList->selectedItems(); if (sel.isEmpty()) return;
        QString pkgs = "";
        for (auto *it : sel) pkgs += it->text().split(' ').first() + "\n";
        QMessageBox warn; warn.setWindowTitle("CRITICAL WARNING"); warn.setIcon(QMessageBox::Critical);
        warn.setText("<b style='color:red;'>ARE YOU ABSOLUTELY SURE?</b>");
        warn.setInformativeText("You are about to UNMERGE (force remove):\n\n" + pkgs + "\nThis can BROKE YOUR SYSTEM!");
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
        consoleOutput->clear(); consoleOutput->append("<b style='color:#5e35b1;'>[System] " + act + " " + name + "...</b>");
        installProcess->start("script", {"-q", "-c", cmd, "/dev/null"});
        toggleOverlayMode();
    }

    void updateStats() { statsLabel->setText(QString("📦 Installed: %1").arg(localPkgs.size())); }
    void loadLocalPackages() {
        QProcess p; p.start("qlist", {"-I", "-v"}); p.waitForFinished();
        localPkgs = QString::fromLocal8Bit(p.readAllStandardOutput()).split('\n', Qt::SkipEmptyParts); updateStats();
    }
    void handleSearch(const QString &t) {
        if (isOverlayMode) return;
        pkgList->setUpdatesEnabled(false); pkgList->clear();
        if (t.isEmpty()) { refreshList(localPkgs); pkgList->setUpdatesEnabled(true); return; }
        QStringList mt; for (const QString &p : localPkgs) if (p.contains(t, Qt::CaseInsensitive)) mt << p;
        refreshList(mt);
        if (t.length() >= 3) {
            QProcess eix; eix.start("eix", {"--pure-packages", "--format", "<category>/<name>\n", "--substring", t});
            eix.waitForFinished();
            for (QString res : QString::fromLocal8Bit(eix.readAllStandardOutput()).split('\n', Qt::SkipEmptyParts)) {
                res = res.trimmed(); bool found = false;
                for(int i=0; i<pkgList->count(); ++i) if(pkgList->item(i)->text().startsWith(res)) found = true;
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
            bool u = false;
            for (const QString &up : updatablePkgs) {
                QString pClean = p;
                pClean = pClean.remove(QRegularExpression("-[0-9].*$"));
                if (pClean == up.trimmed()) { u = true; break; }
            }
            if (u) {
                auto *it = new QListWidgetItem(p + " [UPDATE]");
                it->setForeground(QColor("#00ff00"));
                it->setData(Qt::UserRole, 888);
                pkgList->addItem(it);
            }
        }

        for (const QString &p : l) {
            bool u = false;
            for (const QString &up : updatablePkgs) {
                QString pClean = p;
                pClean = pClean.remove(QRegularExpression("-[0-9].*$"));
                if (pClean == up.trimmed()) { u = true; break; }
            }
            if (!u) {
                auto *it = new QListWidgetItem(p);
                it->setForeground(QColor("#eeeeee"));
                it->setData(Qt::UserRole, 0);
                pkgList->addItem(it);
            }
        }
    }

    void checkUpdates() {
        globalUpdateBtn->setEnabled(false);
        topCleanBtn->setEnabled(false);
        updatablePkgs.clear();

        QString repoPath = "/var/db/repos/gentoo/metadata/timestamp.chk";
        if (!QFile::exists(repoPath)) repoPath = "/var/db/repos/gentoo/.git";

        QFileInfo repoInfo(repoPath);
        if (repoInfo.exists()) {
            qint64 diffSecs = repoInfo.lastModified().secsTo(QDateTime::currentDateTime());
            qint64 diffHours = diffSecs / 3600;
            consoleOutput->append(QString("<b style='color:#00ffcc;'>[System] Last sync was %1 hours ago.</b>").arg(diffHours));
            if (diffHours < 3) {
                consoleOutput->append("<b style='color:#a8a8a8;'>[System] Skipping auto-sync (less than 3 hours since last sync). Use manual 'Sync' to override.</b>");
            }
        }

        consoleOutput->append("<b style='color:#ffff00;'>[System] Fast-checking updates via EIX...</b>");

        QProcess eixProc;
        eixProc.start("eix", {"--upgrade", "--pure-packages", "--format", "<category>/<name>\n"});
        eixProc.waitForFinished();
        updatablePkgs = QString::fromLocal8Bit(eixProc.readAllStandardOutput()).split('\n', Qt::SkipEmptyParts);

        if (!updatablePkgs.isEmpty()) {
            globalUpdateBtn->setToolTip(QString("EIX Updates: %1").arg(updatablePkgs.size()));
        }
        handleSearch(searchBar->text());

        consoleOutput->append("<b style='color:#fb8c00;'>[System] Deep dependency scanning via Emerge in background. Please wait...</b>");

        QProcess *p = new QProcess(this);
        connect(p, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), [this, p]() {
            QString output = QString::fromLocal8Bit(p->readAllStandardOutput());
            QStringList lines = output.split('\n', Qt::SkipEmptyParts);

            updatablePkgs.clear();

            QRegularExpression updateRegex("^\\[ebuild\\s+U.*?\\].*?([^\\s]+)");
            for (const QString &line : lines) {
                QRegularExpressionMatch match = updateRegex.match(line.trimmed());
                if (match.hasMatch()) {
                    QString pkgFull = match.captured(1);
                    QString pkgClean = pkgFull.remove(QRegularExpression("-[0-9].*$"));
                    updatablePkgs.append(pkgClean);
                }
            }

            if (!updatablePkgs.isEmpty()) {
                globalUpdateBtn->setEnabled(true);
                globalUpdateBtn->setToolTip(QString("Real Updates: %1").arg(updatablePkgs.size()));
            } else {
                globalUpdateBtn->setEnabled(false);
            }

            topCleanBtn->setEnabled(true);

            consoleOutput->append("<b style='color:#00ff00;'>[System] Deep scan complete. App is ready for use.</b>");

            handleSearch(searchBar->text());
            p->deleteLater();
        });

        p->start("emerge", {"-p", "--color=n", "--deep", "--with-bdeps=y", "@world"});
    }

    void handleSync() {
        syncBtn->setEnabled(false);
        consoleOutput->clear();

        consoleInput->setVisible(false);
        progressBar->setVisible(true);

        progressBar->setRange(0, 0);

        consoleOutput->append("<b style='color:#00ffcc;'>[System] Forcing manual sync...</b>");

        installProcess->start("script", {"-q", "-c", "sudo emaint sync -a && sudo eix-update", "/dev/null"});
    }

    void startEmerge(QString f) {
        QString p = "";
        if (!f.contains("@world") && !f.contains("--depclean")) {
            auto sel = pkgList->selectedItems(); if (sel.isEmpty()) return;
            QStringList ns; for (auto *it : sel) ns << it->text().remove("[UPDATE]").remove("(REPO)").trimmed().split(' ').first().remove(QRegularExpression("-[0-9].*$"));
            p = ns.join(" ");
        }

        consoleOutput->clear(); confirmWidget->setVisible(false);

        consoleInput->setVisible(false);
        progressBar->setVisible(true);

        progressBar->setRange(0, 0);

        QString autoUnmaskFlags = "--autounmask=y --autounmask-write=y --autounmask-continue=y";

        installProcess->start("script", {"-q", "-c", "sudo emerge --color=y " + autoUnmaskFlags + " " + f + " " + p, "/dev/null"});
    }

    void readInstallOutput() {
        QByteArray d = installProcess->readAllStandardOutput() + installProcess->readAllStandardError();
        QString t = QString::fromLocal8Bit(d);

        if (t.contains("Autounmask changes successfully written") || t.contains("needs updating")) {
            consoleOutput->append("<br><b style='color:#00ffcc;'>[Automation] Config files need updating. Auto-merging now...</b>");

            QProcess unmaskProc;
            unmaskProc.start("bash", {"-c", "sudo etc-update --automode -5"});
            unmaskProc.waitForFinished();

            consoleOutput->append("<b style='color:#00ff00;'>[Automation] Configs merged! Process will resume automatically.</b>");
        }

        t.replace(QRegularExpression("\x1B\\][0-9];.*?\x07|\x1B\\[[0-9;]*[a-zA-Z]"), "");
        t.remove('\r').replace("&", "&amp;").replace("<", "&lt;").replace(">", "&gt;");
        t.replace(QRegularExpression("(\\s-\\w+)"), "<span style='color:#ff5555;'>\\1</span>");
        t.replace(QRegularExpression("(\\+\\w+)"), "<span style='color:#00ff00;'>\\1</span>");
        consoleOutput->moveCursor(QTextCursor::End); consoleOutput->insertHtml(t.replace("\n", "<br>"));

        if (t.contains("[Yes/No]")) {
            confirmWidget->setVisible(true);
            progressBar->setRange(0, 100);
            progressBar->setValue(100);
        }
        consoleOutput->ensureCursorVisible();
    }

    void handleInfo() {
        auto sel = pkgList->selectedItems(); if (sel.isEmpty()) return;
        QString pP = sel.first()->text().remove("[UPDATE]").remove("(REPO)").trimmed().split(' ').first().remove(QRegularExpression("-[0-9].*$"));
        QString pO = pP.contains("/") ? pP.split('/').last() : pP;
        consoleOutput->clear(); consoleOutput->append("<b style='color:#00ffcc; font-size: 14px;'>--- INSPECT: " + pP + " ---</b>");
        QProcess eP; eP.start("eix", {"--pure-packages", "--exact", pP}); eP.waitForFinished();
        QString eR = QString::fromLocal8Bit(eP.readAllStandardOutput()).trimmed();
        if (eR.isEmpty()) { eP.start("eix", {"--pure-packages", pP}); eP.waitForFinished(); eR = QString::fromLocal8Bit(eP.readAllStandardOutput()).trimmed(); }
        consoleOutput->append(eR);
        consoleOutput->append("<br><b style='color:#ffff00;'>📜 History (qlop):</b>");
        QProcess qP; qP.start("bash", {"-c", QString("qlop -l %1 | tail -n 5").arg(pO)}); qP.waitForFinished();
        consoleOutput->append(QString::fromLocal8Bit(qP.readAllStandardOutput()).trimmed());
        consoleOutput->append("<br><b style='color:#fb8c00;'>🚑 Logs (24h):</b>");
        QProcess jP; jP.start("bash", {"-c", QString("journalctl -xe --since '24h ago' --no-pager | grep -i '%1' | tail -n 5").arg(pO)}); jP.waitForFinished();
        consoleOutput->append(QString::fromLocal8Bit(jP.readAllStandardOutput()).trimmed());
    }

       void showContextMenu(const QPoint &pos) {
        auto sel = pkgList->selectedItems(); if (sel.isEmpty()) return;
        QMenu m(this); m.setStyleSheet("QMenu { background:#222; color:#fff; } QMenu::item:selected { background:#00ffcc; color:#000; }");
        if (isOverlayMode) {
            bool isEn = sel.first()->data(Qt::UserRole).toInt() == 1;
            if (isEn) m.addAction("❌ Disable Repository", [this]() { handleOverlayAction(); });
            else m.addAction("✅ Enable & Sync", [this]() { handleOverlayAction(); });
        } else {
            int t = sel.first()->data(Qt::UserRole).toInt();
            QString pClean = sel.first()->text().remove("[UPDATE]").remove("(REPO)").trimmed().split(' ').first().remove(QRegularExpression("-[0-9].*$"));

            if (t == 999) m.addAction("➕ Install", [this]() { startEmerge("-av"); });
            else {
                if (t == 888) m.addAction("🚀 Upgrade", [this]() { startEmerge("-avuND"); });
                m.addAction("🔄 Rebuild", [this]() { startEmerge("-av --oneshot"); });
                m.addSeparator();
                m.addAction("🗑️ Remove (Dangerous)", [this]() { handleSecureRemove(); });
            }
            m.addSeparator();
            m.addAction("🔍 Inspect", [this]() { handleInfo(); });
            m.addAction("⚙️ UseFlags", [this]() { handleUseEdit(); });

            m.addAction("🔓 Force Unmask (~amd64)", [this, pClean]() {
                QString priorityKeywordsFile = "/etc/portage/package.accept_keywords/zzz_portager_keywords";
                QProcess::execute("sudo mkdir -p /etc/portage/package.accept_keywords");
                QString command = QString("echo '%1 ~amd64' | sudo tee -a %2").arg(pClean, priorityKeywordsFile);
                QProcess::execute("bash", {"-c", command});
                consoleOutput->append("<b style='color:#00ff00;'>[Automation] Manually unmasked " + pClean + "!</b>");
            });
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
