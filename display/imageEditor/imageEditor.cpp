#include "imageEditor.h"


// #include "../../functions/clickableLabel/ClickableLabel.h"

ClickableLabel::ClickableLabel(const QString& imagePath, QWidget* parent, QSize size)
    : QLabel(parent) {
    // Load image using OpenCV with alpha channel
    cv::Mat image = cv::imread(imagePath.toStdString(), cv::IMREAD_UNCHANGED);

    size = (size - QSize(10, 10));

    if (!image.empty()) {

        // Convert BGR to RGB (if needed) and keep alpha channel
        if (image.channels() == 4) {
            // Create QImage from OpenCV Mat with alpha channel
            QImage qImage(image.data, image.cols, image.rows, image.step[0], QImage::Format_ARGB32);
            // Set the pixmap with scaling
            this->setPixmap(QPixmap::fromImage(qImage).scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        }
        else {
            // Handle images without an alpha channel (optional)
            cv::cvtColor(image, image, cv::COLOR_BGR2RGB);

            cv::Mat resizedImage;

            // cv::resize(image, resizedImage, cv::Size(size.width(), size.height()), 0, 0, cv::INTER_CUBIC); // Utilisez INTER_LINEAR pour un redimensionnement rapide
            // QImage qImage(resizedImage.data, resizedImage.cols, resizedImage.rows, resizedImage.step[0], QImage::Format_RGB888);
            QImage qImage(image.data, image.cols, image.rows, image.step[0], QImage::Format_RGB888);
            this->setPixmap(QPixmap::fromImage(qImage).scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation));
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



ImageEditor::ImageEditor(Data& i, QWidget* parent) : QMainWindow(parent), // Initialize the base class
data(i) {

    QScreen* screen = QGuiApplication::primaryScreen();
    QRect screenR = screen->availableGeometry();
    // TO delete after 
    screenR.setSize(QSize(1920, 1080));
    qreal pixelRatio = screen->devicePixelRatio();
    QSize screenGeometry = screenR.size();

    int actionButtonSize;
    if (screenGeometry.width() < screenGeometry.height()) {
        actionButtonSize = (screenGeometry.width() * 1 / 12) / pixelRatio;
    }
    else {
        actionButtonSize = (screenGeometry.height() * 1 / 12) / pixelRatio;

    }

    previewSize = (screenGeometry * 1 / 12) / pixelRatio;



    // Créer un widget central
    QWidget* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    // Créer un layout vertical pour toute la fenêtre
    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setSpacing(5);  // Espacement entre les widgets
    mainLayout->setContentsMargins(5, 5, 5, 5); // Marges autour des bords (gauche, haut, droite, bas)


    QHBoxLayout* actionButtonLayout = new QHBoxLayout();
    // Créer les boutons avec des tailles spécifiques
    // TODO mieux deffir pour que il soit carrer

    QSize actionSize(actionButtonSize, actionButtonSize);

    ClickableLabel* imageRotateRight = new ClickableLabel("ressources/rotateRight.png", this, actionSize);
    imageRotateRight->setFixedSize(actionSize); // Définir la taille fixe du bouton (largeur, hauteur)

    ClickableLabel* imageRotateLeft = new ClickableLabel("ressources/rotateLeft.png", this, actionSize);
    imageRotateLeft->setFixedSize(actionSize); // Définir la taille fixe du bouton (largeur, hauteur)

    ClickableLabel* imageDelete = new ClickableLabel("ressources/rotateRight.png", this, actionSize);
    imageDelete->setFixedSize(actionSize); // Définir la taille fixe du bouton (largeur, hauteur)

    actionButtonLayout->addWidget(imageRotateRight);
    actionButtonLayout->addWidget(imageRotateLeft);
    actionButtonLayout->addWidget(imageDelete);


    actionButtonLayout->setAlignment(Qt::AlignCenter);

    mainLayout->addLayout(actionButtonLayout);





    // Créer un layout horizontal pour les boutons
    QHBoxLayout* buttonLayout = new QHBoxLayout();

    // Créer les boutons avec des tailles spécifiques
    ClickableLabel* buttonImageBefore = new ClickableLabel("ressources/before.png", this, actionSize);
    buttonImageBefore->setFixedSize(actionSize); // Définir la taille fixe du bouton (largeur, hauteur)

    ClickableLabel* buttonImageNext = new ClickableLabel("ressources/next.png", this, actionSize);
    buttonImageNext->setFixedSize(actionSize);// Définir la taille fixe du bouton (largeur, hauteur)


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


    previewButtonLayout = new QHBoxLayout();

    mainLayout->addLayout(previewButtonLayout);


    updatePreview();

    // Connecter les images cliquables à des actions
    connect(buttonImageBefore, &ClickableLabel::clicked, [this]() { this->previousImage(); });
    connect(buttonImageNext, &ClickableLabel::clicked, [this]() { this->nextImage(); });

    connect(imageRotateLeft, &ClickableLabel::clicked, [this]() { this->rotateLeft(); });
    connect(imageRotateRight, &ClickableLabel::clicked, [this]() { this->rotateRight(); });
    connect(imageDelete, &ClickableLabel::clicked, [this]() { this->data.preDeleteImage(data.imagesData.getImageNumber()); });


    // Définir le titre de la fenêtre
    setWindowTitle("Changer l'image");
}

void ImageEditor::setImage(ImageData& imageData) {
    std::string imagePath = imageData.getImagePath();


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
    }
    else {
        imageLabel->setText("Erreur : Image non valide !");
    }
    reloadMainImage();
}


void ImageEditor::nextImage(){

    ImagesData& imagesData = data.imagesData;

    imagesData.setImageNumber(imagesData.getImageNumber() + 1);

    setImage(*imagesData.getImageData(imagesData.getImageNumber()));
    reload();
}

void ImageEditor::previousImage(){

    ImagesData& imagesData = data.imagesData;

    imagesData.setImageNumber(imagesData.getImageNumber() - 1);


    setImage(*imagesData.getImageData(imagesData.getImageNumber()));
    reload();
}

void ImageEditor::rotateLeft(){

    ImagesData& imagesData = data.imagesData;

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

    ImagesData& imagesData = data.imagesData;

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

    ImagesData& imagesData = data.imagesData;


    setImage(*imagesData.getCurrentImageData());
    updatePreview();



}

void ImageEditor::reloadMainImage(){

    imageLabel->update();

}



void ImageEditor::updatePreview() {

    ImagesData& imagesData = data.imagesData;

    std::vector<QString> imagePaths;

    int currentImageNumber = imagesData.getImageNumber();
    int totalImages = imagesData.get().size();

    int under = 0;
    for (int i = 4; i >= 1; --i) {

        if (currentImageNumber - i >= 0) {
            imagePaths.push_back(QString::fromStdString(imagesData.getImageData(currentImageNumber - i)->getImagePath()));
            under += 1;
        }
    }

    imagePaths.push_back(QString::fromStdString(imagesData.getCurrentImageData()->getImagePath()));

    for (int i = 1; i <= 4; ++i) {

        if (currentImageNumber + i <= totalImages - 1) {
            imagePaths.push_back(QString::fromStdString(imagesData.getImageData(currentImageNumber + i)->getImagePath()));
        }
    }

    // Effacer les boutons existants
    QLayoutItem* item;
    while ((item = previewButtonLayout->takeAt(0)) != nullptr) {
        delete item->widget(); // Supprimer le widget associé
        delete item; // Supprimer l'élément de layout
    }

    // Créer et ajouter les nouveaux boutons
    for (int i = 0; i < imagePaths.size(); ++i) {

        ClickableLabel* previewButton = new ClickableLabel(imagePaths[i], this, previewSize);

        previewButton->setFixedSize(previewSize); // Définir la taille fixe du bouton
        int imageNbr = imagesData.getImageNumber() + i - under;
        connect(previewButton, &ClickableLabel::clicked, [this, imageNbr]() {
            data.imagesData.setImageNumber(imageNbr);
            setImage(*data.imagesData.getImageData(data.imagesData.getImageNumber()));
            // TODO reload not working ::: Segmentation fault (core dumped) ::: so preview doesn't update
            // reload();
            });

        previewButtonLayout->addWidget(previewButton); // Ajouter le bouton au layout
    }

    previewButtonLayout->setAlignment(Qt::AlignCenter);
}
