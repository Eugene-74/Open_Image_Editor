#include <QApplication>
#include <QWidget>
#include <QPushButton>
#include <QVBoxLayout>

#include <QMainWindow>
#include <QAction>
#include <QMenuBar>
#include <QStatusBar>
#include <QFile>
#include <QTextStream>
// #include <QIcon>

#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>

class MainWindow : public QWidget {
public:
    MainWindow() {
        QVBoxLayout* layout = new QVBoxLayout(this);

        QPushButton* button1 = new QPushButton("Bouton 1", this);
        QPushButton* button2 = new QPushButton("Bouton 2", this);

        layout->addWidget(button1);
        layout->addWidget(button2);

        connect(button1, &QPushButton::clicked, this, &MainWindow::onButton1Clicked);
        connect(button2, &QPushButton::clicked, this, &MainWindow::onButton2Clicked);
    }

private slots:
    void onButton1Clicked() {
        // Code à exécuter lorsque le bouton 1 est cliqué
        cv::Mat image = cv::imread("image.JPG");
        if (image.empty()) {
            std::cerr << "Erreur: Impossible de charger l'image" << std::endl;
            return;
        }
        cv::imshow("Image", image);
        cv::waitKey(0);
    }

    void onButton2Clicked() {
        // Code à exécuter lorsque le bouton 2 est cliqué
        std::cout << "Bouton 2 cliqué!" << std::endl;
    }
};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    QMainWindow mainWindow;
    mainWindow.setWindowTitle("Easy Image Editor");
    // window.resize(300, 200);
    mainWindow.showMaximized();
    mainWindow.show();
    mainWindow.setWindowIcon(QIcon("resssource/icon.ico"));   

    
   // Charger le fichier QSS
    QFile file("style.qss");
    file.open(QFile::ReadOnly);
    QTextStream stream(&file);
    QString styleSheet = stream.readAll();
    app.setStyleSheet(styleSheet);

    // Crée une fenêtre principale
    // QMainWindow mainWindow;

    // Crée un widget central
    QWidget *centralWidget = new QWidget();
    mainWindow.setCentralWidget(centralWidget);

    // Crée un layout vertical pour le widget central
    QVBoxLayout *layout = new QVBoxLayout();
    centralWidget->setLayout(layout);

    // Ajouter des boutons
    QPushButton *button1 = new QPushButton("Bouton 1");
    QPushButton *button2 = new QPushButton("Bouton 2");
    layout->addWidget(button1);
    layout->addWidget(button2);

    return app.exec();
}
