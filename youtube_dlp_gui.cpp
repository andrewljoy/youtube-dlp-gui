#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QFileDialog>
#include <QMessageBox>
#include <QProcess>
#include <QTextEdit>
#include <QString>
#include <QRegularExpression>
#include <QUrl>
#include <QDir>
#include <QStandardPaths>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QCheckBox>

// YouTubeDLPWindow: A Qt-based GUI for downloading videos/audio using yt-dlp.
// Provides input fields for URL and save path, quality selection, and progress display.
class YouTubeDLPWindow : public QWidget {
    Q_OBJECT
public:
    // Constructor: Initializes the GUI with widgets and layout.
    YouTubeDLPWindow(QWidget *parent = nullptr);

private slots:
    // chooseFolder: Opens a dialog to select the save directory.
    void chooseFolder();
    // startDownload: Initiates the download by running yt-dlp with user inputs.
    void startDownload();
    // processError: Handles errors when the yt-dlp process fails to start.
    void processError(QProcess::ProcessError error);
    // readProcessOutput: Parses yt-dlp output and displays progress.
    void readProcessOutput();
    // processFinished: Handles yt-dlp completion or failure.
    void processFinished(int exitCode, QProcess::ExitStatus exitStatus);

private:
    QLineEdit *urlEdit; // URL input field
    QComboBox *videoQualityCombo; // Video quality selector
    QComboBox *audioQualityCombo; // Audio quality selector
    QComboBox *subtitleLangCombo; // Subtitle language selector
    QLineEdit *savePathEdit; // Save path display
    QPushButton *chooseFolderButton; // Folder selection button
    QPushButton *downloadButton; // Download button
    QCheckBox *sponsorBlockCheck; // SponsorBlock option
    QTextEdit *progressOutput; // Download progress display
    QProcess *process = nullptr; // yt-dlp process
    bool hasProgressLine = false; // Track progress line state
};

// Constructor implementation
YouTubeDLPWindow::YouTubeDLPWindow(QWidget *parent) : QWidget(parent) {
    setWindowTitle("YouTube-DLP GUI");
    resize(600, 300);

    // Initialize input widgets
    urlEdit = new QLineEdit(this);
    urlEdit->setPlaceholderText("Paste YouTube link here");
    urlEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed); // Dynamic width

    videoQualityCombo = new QComboBox(this);
    videoQualityCombo->addItems({"4K (2160p)", "1080p", "720p", "480p", "None"}); // Video quality options, None for audio-only
    videoQualityCombo->setCurrentIndex(0); // Default to highest (4K)

    audioQualityCombo = new QComboBox(this);
    audioQualityCombo->addItems({"320kbps", "256kbps", "128kbps"}); // Audio bitrate options
    audioQualityCombo->setCurrentIndex(0); // Default to highest (320kbps)

    subtitleLangCombo = new QComboBox(this);
    subtitleLangCombo->addItems({"None", "English (en)", "French (fr)", "Spanish (es)", "German (de)", "Italian (it)", "Portuguese (pt)", "Russian (ru)", "Japanese (ja)", "Chinese (zh)", "Arabic (ar)"}); // Subtitle language options
    subtitleLangCombo->setCurrentIndex(0); // Default to None

    savePathEdit = new QLineEdit(this);
    // Set default save path: Videos, Downloads, or home directory
    QDir dir;
    QString defaultPath = QStandardPaths::writableLocation(QStandardPaths::MoviesLocation);
    if (!dir.exists(defaultPath)) {
        defaultPath = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
        if (!dir.exists(defaultPath)) {
            defaultPath = QDir::homePath();
        }
    }
    savePathEdit->setText(defaultPath);
    savePathEdit->setPlaceholderText("Change via Choose Folder");
    savePathEdit->setReadOnly(true); // Prevent manual editing of save path

    chooseFolderButton = new QPushButton("Choose Folder", this);
    downloadButton = new QPushButton("Download", this);
    sponsorBlockCheck = new QCheckBox("Remove sponsor segments", this);

    // Initialize output display
    progressOutput = new QTextEdit(this);
    progressOutput->setReadOnly(true); // Always visible, initially blank

    // Set up layout with labeled rows
    auto *mainLayout = new QVBoxLayout(this);
    // Lambda to create a labeled row with optional extra widget
    auto addLabeledWidget = [&](const QString &label, QWidget *widget, QWidget *extra = nullptr) {
        auto *row = new QHBoxLayout;
        row->addWidget(new QLabel(label));
        row->addWidget(widget);
        if (extra) row->addWidget(extra);
        mainLayout->addLayout(row);
    };

    // Add URL row (full width)
    auto *urlRow = new QHBoxLayout;
    urlRow->addWidget(new QLabel("YouTube URL:"));
    urlRow->addWidget(urlEdit);
    mainLayout->addLayout(urlRow);

    // Add quality row (video, audio, subtitles side by side)
    auto *qualityRow = new QHBoxLayout;
    qualityRow->addWidget(new QLabel("Video Quality:"));
    qualityRow->addWidget(videoQualityCombo);
    qualityRow->addWidget(new QLabel("Audio Quality:"));
    qualityRow->addWidget(audioQualityCombo);
    qualityRow->addWidget(new QLabel("Subtitles:"));
    qualityRow->addWidget(subtitleLangCombo);
    qualityRow->addStretch(); // Fill remaining space
    mainLayout->addLayout(qualityRow);

    // Add SponsorBlock checkbox
    mainLayout->addWidget(sponsorBlockCheck);

    // Add save path row
    addLabeledWidget("Save Folder:", savePathEdit, chooseFolderButton);
    mainLayout->addWidget(downloadButton);
    mainLayout->addWidget(progressOutput);

    // Connect button signals to slots
    connect(chooseFolderButton, &QPushButton::clicked, this, &YouTubeDLPWindow::chooseFolder);
    connect(downloadButton, &QPushButton::clicked, this, &YouTubeDLPWindow::startDownload);
}

// chooseFolder: Opens a dialog to select the save directory.
void YouTubeDLPWindow::chooseFolder() {
    QString folder = QFileDialog::getExistingDirectory(this, "Select Save Folder");
    if (!folder.isEmpty()) savePathEdit->setText(folder); // Update save path if selected
}

// startDownload: Initiates the download by running yt-dlp with user inputs.
void YouTubeDLPWindow::startDownload() {
    QString url = urlEdit->text();
    QString savePath = savePathEdit->text();
    // Check for missing inputs
    if (url.isEmpty() || savePath.isEmpty()) {
        QMessageBox::critical(this, "Error", "Please provide a URL and save folder.");
        return;
    }

    // Warn if URL scheme is not http or https
    QUrl urlObj(url);
    if (urlObj.isValid() && !urlObj.scheme().startsWith("http")) {
        QMessageBox::StandardButton reply = QMessageBox::warning(
            this, "Warning",
            "The URL does not use http or https. This may be unsupported by yt-dlp. Proceed?",
            QMessageBox::Yes | QMessageBox::No);
        if (reply == QMessageBox::No) return; // Abort if user cancels
    }

    // Get metadata with --dump-json to check for playlists or channels
    QProcess dumpJsonProcess;
    dumpJsonProcess.start("yt-dlp", QStringList() << "--dump-json" << url);
    dumpJsonProcess.waitForFinished();
    if (dumpJsonProcess.exitCode() != 0) {
        progressOutput->append("---------------------");
        progressOutput->append("Failed to get metadata: " + dumpJsonProcess.readAllStandardError());
        progressOutput->append("---------------------");
        downloadButton->setText("Download");
        downloadButton->setEnabled(true);
        return;
    }
    QString jsonOutput = dumpJsonProcess.readAllStandardOutput();
    QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonOutput.toUtf8());
    if (jsonDoc.isNull()) {
        progressOutput->append("---------------------");
        progressOutput->append("Invalid metadata from yt-dlp");
        progressOutput->append("---------------------");
        downloadButton->setText("Download");
        downloadButton->setEnabled(true);
        return;
    }
    QJsonObject json = jsonDoc.object();
    if (json.contains("entries") && json["entries"].isArray()) {
        QJsonArray entries = json["entries"].toArray();
        if (entries.size() > 1) {
            QString title = json["title"].toString();
            QMessageBox::StandardButton reply = QMessageBox::warning(
                this, "Multiple Videos Detected",
                QString("You are attempting to download '%1' with %2 videos. Are you sure?").arg(title).arg(entries.size()),
                QMessageBox::Yes | QMessageBox::No);
            if (reply == QMessageBox::No) return;
        }
    }

    // Reset progress state and clear output
    hasProgressLine = false;
    progressOutput->clear();
    progressOutput->append(QString("Downloading URL: %1").arg(url));

    // Indicate download in progress
    downloadButton->setText("Downloading...");
    downloadButton->setEnabled(false);

    // Build yt-dlp command arguments
    QStringList args;
    args << "-o" << QString("%1/%(title)s.%(ext)s").arg(savePath); // Output path template

    // Determine format based on quality selections
    int videoIndex = videoQualityCombo->currentIndex();
    int audioIndex = audioQualityCombo->currentIndex();
    QString videoFormat;
    if (videoIndex < 4) { // Video quality selected (not None)
        switch (videoIndex) {
            case 0: videoFormat = "bestvideo[height<=2160]+bestaudio/best"; break; // 4K
            case 1: videoFormat = "bestvideo[height<=1080]+bestaudio/best"; break; // 1080p
            case 2: videoFormat = "bestvideo[height<=720]+bestaudio/best"; break; // 720p
            case 3: videoFormat = "bestvideo[height<=480]+bestaudio/best"; break; // 480p
        }
        args << "-f" << videoFormat << "--merge-output-format" << "mp4";
    } else { // None selected, download audio only
        QString audioQuality = audioQualityCombo->itemText(audioIndex).split("kbps").first();
        args << "-x" << "--audio-format" << "mp3" << "--audio-quality" << audioQuality;
    }

    // Add subtitle options
    int subIndex = subtitleLangCombo->currentIndex();
    if (subIndex > 0) {
        QString lang = subtitleLangCombo->itemText(subIndex).section("(", 1, 1).section(")", 0, 0);
        args << "--write-subs" << "--sub-langs" << lang;
    }

    // Add SponsorBlock option
    if (sponsorBlockCheck->isChecked()) {
        args << "--sponsorblock-remove" << "all";
    }

    args << url;

    // Start yt-dlp process
    process = new QProcess(this);
    connect(process, &QProcess::errorOccurred, this, &YouTubeDLPWindow::processError);
    connect(process, &QProcess::readyReadStandardOutput, this, &YouTubeDLPWindow::readProcessOutput);
    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &YouTubeDLPWindow::processFinished);
    process->start("yt-dlp", args); // Run yt-dlpWITH arguments
}

// processError: Handles errors when the yt-dlp process fails to start.
void YouTubeDLPWindow::processError(QProcess::ProcessError error) {
    Q_UNUSED(error); // Avoid unused parameter warning
    progressOutput->append("---------------------");
    progressOutput->append("Failed to start download: " + process->errorString());
    progressOutput->append("---------------------");
    downloadButton->setText("Download");
    downloadButton->setEnabled(true);
    process->deleteLater();
    process = nullptr;
}

// readProcessOutput: Parses yt-dlp output and displays progress.
void YouTubeDLPWindow::readProcessOutput() {
    QString output = process->readAllStandardOutput();
    for (const QString &line : output.split('\n', Qt::SkipEmptyParts)) {
        QString trimmed = line.trimmed();
        // Extract percentage from [download] lines (e.g., "45.6%")
        QRegularExpression re("(\\d+\\.\\d+)%");
        QRegularExpressionMatch match = re.match(trimmed);
        if (match.hasMatch()) {
            QString progressText = QString("Progress: %1").arg(match.captured(1));
            if (hasProgressLine) {
                // Overwrite existing progress line
                QTextCursor cursor = progressOutput->textCursor();
                cursor.movePosition(QTextCursor::End);
                cursor.select(QTextCursor::LineUnderCursor);
                cursor.removeSelectedText();
                cursor.insertText(progressText);
            } else {
                // Create new progress line
                progressOutput->append(progressText);
                hasProgressLine = true;
            }
        } else {
            // Append non-progress lines (e.g., errors)
            progressOutput->append(trimmed);
        }
    }
    // Scroll to show latest output
    progressOutput->moveCursor(QTextCursor::End);
    progressOutput->ensureCursorVisible();
}

// processFinished: Handles yt-dlp completion or failure.
void YouTubeDLPWindow::processFinished(int exitCode, QProcess::ExitStatus exitStatus) {
    Q_UNUSED(exitStatus); // Avoid unused parameter warning
    // Append completion message with ASCII separators
    progressOutput->append("---------------------");
    progressOutput->append(exitCode == 0 ? "Download Complete" : "Download Failed");
    progressOutput->append("---------------------");
    // Reset button to allow new downloads
    downloadButton->setText("Download");
    downloadButton->setEnabled(true);
    process->deleteLater(); // Schedule process cleanup
    process = nullptr;
}

// main: Entry point, creates and runs the Qt application.
int main(int argc, char *argv[]) {
    QApplication app(argc, argv); // Initialize Qt application
    YouTubeDLPWindow window; // Create main window
    window.show(); // Display - Show window
    return app.exec(); // Run event loop
}

#include "youtube_dlp_gui.moc"
