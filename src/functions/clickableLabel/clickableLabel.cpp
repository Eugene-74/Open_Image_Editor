#include "clickableLabel.h"


ClickableLabel::ClickableLabel(const QString& imagePath, QWidget* parent)
    : QLabel(parent) {
    // TODO mettre dans clickableLabel
    QPixmap pixmap(imagePath);
    this->setPixmap(pixmap.scaled(this->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    this->setAlignment(Qt::AlignCenter);

    // Activer l'événement de survol
    setMouseTracking(true);

    // Créer un effet d'opacité
    opacityEffect = new QGraphicsOpacityEffect(this);
    this->setGraphicsEffect(opacityEffect);
    opacityEffect->setOpacity(1.0);  // Opacité par défaut
}
// Gérer l'entrée de la souris
void ClickableLabel::enterEvent(QEvent* event) {
    opacityEffect->setOpacity(0.5);  // Réduire l'opacité à 50% au survol
    QLabel::enterEvent(event);
}

// Gérer la sortie de la souris
void ClickableLabel::leaveEvent(QEvent* event) {
    opacityEffect->setOpacity(1.0);  // Remettre l'opacité à 100% après le survol
    QLabel::leaveEvent(event);
}