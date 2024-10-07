#include "imageEditor.h"


// #include "../../functions/clickableLabel/ClickableLabel.h"

ClickableLabel::ClickableLabel(const QString& imagePath, QWidget* parent)
    : QLabel(parent) {
    // Load image using OpenCV with alpha channel
    cv::Mat image = cv::imread(imagePath.toStdString(), cv::IMREAD_UNCHANGED);
    if (!image.empty()) {
        // Convert BGR to RGB (if needed) and keep alpha channel
        if (image.channels() == 4) {
            // Create QImage from OpenCV Mat with alpha channel
            QImage qImage(image.data, image.cols, image.rows, image.step[0], QImage::Format_ARGB32);
            // Set the pixmap with scaling
            this->setPixmap(QPixmap::fromImage(qImage).scaled(this->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
        }
        else {
            // Handle images without an alpha channel (optional)
            cv::cvtColor(image, image, cv::COLOR_BGR2RGB);
            QImage qImage(image.data, image.cols, image.rows, image.step[0], QImage::Format_RGB888);
            this->setPixmap(QPixmap::fromImage(qImage).scaled(this->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
        }
    }
    else {
        // Handle the case where the image is not valid (optional)
        this->setText("Erreur : Image non valide !");
    }

    this->setAlignment(Qt::AlignCenter);

    // Enable mouse tracking
    setMouseTracking(true);


    // opacityEffect->setOpacity(1.0);  // Default opacity
    setStyleSheet(R"(
        QLabel {
            border: 2px solid transparent;
            border-radius: 15px;
        }
        QLabel:hover {
            border: 2px solid transparent;
            border-radius: 15px;
            background-color: #b3b3b3; 
        }
    )");
}
// Gérer l'entrée de la souris
void ClickableLabel::enterEvent(QEvent* event) {
    QLabel::enterEvent(event);
}

// Gérer la sortie de la souris
void ClickableLabel::leaveEvent(QEvent* event) {
    QLabel::leaveEvent(event);
}

void ClickableLabel::mousePressEvent(QMouseEvent* event) {

    if (event->button() == Qt::LeftButton) {
        // emit clicked();
        // Change style to indicate click
        setStyleSheet(R"(
        QLabel {
            border: 2px solid transparent;
            border-radius: 15px;
        }
        QLabel:hover {
            border: 2px solid #969393;
            border-radius: 15px;
            background-color: #9c9c9c; 
        }
    )");
    }
    QLabel::mousePressEvent(event);  // Call the base class implementation

}

void ClickableLabel::mouseReleaseEvent(QMouseEvent* event) {
    emit clicked();  // Émettre le signal quand on clique

    if (event->button() == Qt::LeftButton) {
        // Reset style on mouse release
        setStyleSheet(R"(
        QLabel {
            border: 2px solid transparent;
            border-radius: 15px;
        }
        QLabel:hover {
            border: 2px solid transparent;
            border-radius: 15px;
            background-color: #b3b3b3; 
        }
    )");
    }
    QLabel::mouseReleaseEvent(event);  // Call the base class implementation
}



ImageEditor::ImageEditor(ImagesData& i, QWidget* parent) : QMainWindow(parent), // Initialize the base class
imagesData(i) {

    QScreen* screen = QGuiApplication::primaryScreen();
    QRect screenR = screen->availableGeometry();
    qreal pixelRatio = screen->devicePixelRatio();
    // screen->geometry()
        // QRect screenSize = screen.geometry();
    QSize screenGeometry = screenR.size();

    int actionButtonSize;
    if (screenGeometry.width() < screenGeometry.height()) {
        actionButtonSize = (screenGeometry.width() * 1 / 24) / pixelRatio;
    }
    else {
        actionButtonSize = (screenGeometry.height() * 1 / 24) / pixelRatio;

    }

    QSize previewSize = (screenGeometry * 1 / 12) / pixelRatio;

    // Créer un widget central
    QWidget* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    // Créer un layout vertical pour toute la fenêtre
    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setSpacing(10);  // Espacement entre les widgets
    mainLayout->setContentsMargins(20, 20, 20, 20); // Marges autour des bords (gauche, haut, droite, bas)

    QHBoxLayout* actionButtonLayout = new QHBoxLayout();
    // Créer les boutons avec des tailles spécifiques
    // TODO mieux deffir pour que il soit carrer
    ClickableLabel* imageRotateLeft = new ClickableLabel("ressources/rotateRight.png", this);
    imageRotateLeft->setFixedSize(actionButtonSize, actionButtonSize); // Définir la taille fixe du bouton (largeur, hauteur)

    ClickableLabel* imageRotateRight = new ClickableLabel("ressources/rotateLeft.png", this);
    imageRotateRight->setFixedSize(actionButtonSize, actionButtonSize); // Définir la taille fixe du bouton (largeur, hauteur)

    actionButtonLayout->addWidget(imageRotateLeft);
    actionButtonLayout->addWidget(imageRotateRight);

    actionButtonLayout->setAlignment(Qt::AlignCenter);

    mainLayout->addLayout(actionButtonLayout);




    // Créer un layout horizontal pour les boutons
    QHBoxLayout* buttonLayout = new QHBoxLayout();

    // Créer les boutons avec des tailles spécifiques
    ClickableLabel* buttonImageBefore = new ClickableLabel("ressources/before.png", this);
    buttonImageBefore->setFixedSize(actionButtonSize, actionButtonSize); // Définir la taille fixe du bouton (largeur, hauteur)

    ClickableLabel* buttonImageNext = new ClickableLabel("ressources/next.png", this);
    buttonImageNext->setFixedSize(actionButtonSize, actionButtonSize); // Définir la taille fixe du bouton (largeur, hauteur)


    // Créer un QLabel pour afficher l'image
    imageLabel = new QLabel(this);
    // screenGeometry.size() * 5 / 6
    imageLabel->setFixedSize((screenGeometry.width() * 4 / 6) / pixelRatio, (screenGeometry.height() * 4 / 6) / pixelRatio); // Ajuster la taille de l'image
    imageLabel->setAlignment(Qt::AlignCenter); // Centrer l'image dans le QLabel


    buttonLayout->addWidget(buttonImageBefore);
    buttonLayout->addWidget(imageLabel, 0, Qt::AlignCenter);
    buttonLayout->addWidget(buttonImageNext);
    buttonLayout->setAlignment(Qt::AlignCenter); // Centrer les boutons horizontalement

    // Ajouter le layout des boutons au layout principal
    mainLayout->addLayout(buttonLayout);

    std::vector<QString> imagePaths;

    for (int i = 0; i < 3; ++i) {
        if (imagesData.getImageNumber() - i > 0) {
            std::cerr << imagesData.getImageNumber() - i << std::endl;
            imagePaths.push_back(QString::fromStdString(imagesData.getImageData(imagesData.getImageNumber() - i)->getImagePath()));
        }
    }

    imagePaths.push_back(QString::fromStdString(imagesData.getCurrentImageData()->getImagePath()));

    for (int i = 1; i < 4; ++i) {
        if (imagesData.getImageNumber() + i < imagesData.get().size()) {
            std::cerr << imagesData.getImageNumber() + i << std::endl;

            imagePaths.push_back(QString::fromStdString(imagesData.getImageData(imagesData.getImageNumber() + i)->getImagePath()));
            std::cerr << "path : " << imagesData.getImageData(imagesData.getImageNumber() + i)->getImagePath() << "\n" << std::endl;
        }
    }

    QHBoxLayout* previewButtonLayout = new QHBoxLayout();

    // ClickableLabel* previewButton = new ClickableLabel("ressources/rotateLeft.png", this);
    // previewButton->setFixedSize(previewSize); // Définir la taille fixe du bouton (largeur, hauteur)
    // previewButtonLayout->addWidget(previewButton);
    for (int i = 0; i < imagePaths.size(); ++i) {

        std::cerr << imagePaths[i].toStdString() << "\n" << std::endl;

        ClickableLabel* previewButton = new ClickableLabel(imagePaths[i], this);
        previewButton->setFixedSize(previewSize); // Set the fixed size for the button
        previewButtonLayout->addWidget(previewButton); // Add the button to the layout
    }

    previewButtonLayout->setAlignment(Qt::AlignCenter);
    mainLayout->addLayout(previewButtonLayout);

    // Connecter les images cliquables à des actions
    connect(buttonImageBefore, &ClickableLabel::clicked, [this]() { this->previousImage(); });
    connect(buttonImageNext, &ClickableLabel::clicked, [this]() { this->nextImage(); });
    connect(imageRotateLeft, &ClickableLabel::clicked, [this]() { this->rotateLeft(); });
    connect(imageRotateRight, &ClickableLabel::clicked, [this]() { this->rotateRight(); });

    // Définir le titre de la fenêtre
    setWindowTitle("Changer l'image dans QMainWindow");
}

void ImageEditor::setImage(ImageData& imageData) {
    std::string imagePath = imageData.getImagePath();
    std::cerr << imagePath << std::endl;


    cv::Mat image = cv::imread(imagePath, cv::IMREAD_UNCHANGED);
    if (!image.empty()) {


        Exiv2::ExifData exifData = imageData.getMetaData()->getExifData();
        if (exifData.empty()) {
            std::cerr << "No EXIF data found in image!" << std::endl;
        }
        else {

            if (exifData["Exif.Image.Orientation"].count() != 0) {
                int orientation = exifData["Exif.Image.Orientation"].toInt64();

                // Rotate the image based on the EXIF orientation
                switch (orientation) {
                case 3:
                    cv::rotate(image, image, cv::ROTATE_180);
                    break;
                case 6:
                    cv::rotate(image, image, cv::ROTATE_90_CLOCKWISE);
                    break;
                case 8:
                    cv::rotate(image, image, cv::ROTATE_90_COUNTERCLOCKWISE);
                    break;
                }
            }
        }
        // Convert the OpenCV image (BGR format) to QImage (RGB format)
        if (image.channels() == 4) {
            // Create QImage from OpenCV Mat with alpha channel
            QImage qImage(image.data, image.cols, image.rows, image.step[0], QImage::Format_ARGB32);
            // Set the pixmap with scaling
            imageLabel->setPixmap(QPixmap::fromImage(qImage).scaled(imageLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
        }
        else {
            // Handle images without an alpha channel (optional)
            cv::cvtColor(image, image, cv::COLOR_BGR2RGB);
            QImage qImage(image.data, image.cols, image.rows, image.step[0], QImage::Format_RGB888);
            imageLabel->setPixmap(QPixmap::fromImage(qImage).scaled(imageLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
        }
        // cv::cvtColor(image, image, cv::COLOR_BGR2RGB); // Convert BGR to RGB
        // QImage qImage(image.data, image.cols, image.rows, image.step[0], QImage::Format_RGB888);

        // // Set the QPixmap to the label with scaling
        // imageLabel->setPixmap(QPixmap::fromImage(qImage).scaled(imageLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
    else {
        imageLabel->setText("Erreur : Image non valide !");
    }
    reloadMainImage();
}


void ImageEditor::nextImage(){


    imagesData.setImageNumber(imagesData.getImageNumber() + 1);

    setImage(*imagesData.getImageData(imagesData.getImageNumber()));
}

void ImageEditor::previousImage(){


    imagesData.setImageNumber(imagesData.getImageNumber() - 1);


    setImage(*imagesData.getImageData(imagesData.getImageNumber()));
}

void ImageEditor::rotateLeft(){
    ImageData* imageData = imagesData.getCurrentImageData();
    int orientation = imageData->getImageOrientation();

    if (orientation == 1){
        orientation = 8;
    }
    else if (orientation == 3){
        orientation = 6;
    }
    else if (orientation == 8){
        orientation = 3;
    }
    else{
        orientation = 1;
    }
    imageData->turnImage(orientation);

    imagesData.saveImagesData();
    imageData->saveMetaData();
    reload();


}
void ImageEditor::rotateRight(){
    ImageData* imageData = imagesData.getCurrentImageData();
    // imageData->turnImage(3);
    int orientation = imageData->getImageOrientation();

    if (orientation == 1){
        orientation = 6;
    }
    else if (orientation == 3){
        orientation = 8;
    }
    else if (orientation == 6){
        orientation = 3;
    }
    else{
        orientation = 1;
    }
    imageData->turnImage(orientation);

    imagesData.saveImagesData();
    imageData->saveMetaData();
    reload();



}

void ImageEditor::reload(){

    setImage(*imagesData.getCurrentImageData());

}

void ImageEditor::reloadMainImage(){

    imageLabel->update();

}



