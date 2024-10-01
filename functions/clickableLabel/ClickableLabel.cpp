#include "ClickableLabel.h"

ClickableLabel::ClickableLabel(const QString& imagePath, QWidget* parent)
    : QLabel(parent), defaultImagePath(imagePath), hoverImagePath(imagePath) {
    // Définir l'image par défaut
    setPixmap(QPixmap(defaultImagePath).scaled(50, 50, Qt::KeepAspectRatio)); // Ajuster la taille de l'image
}

void ClickableLabel::enterEvent(QEvent* event) {
    // Changer l'image ou le style quand la souris entre
    hoverImagePath = ":/ressources/hover_image.png"; // Spécifier une image différente pour l'effet de survol
    setPixmap(QPixmap(hoverImagePath).scaled(50, 50, Qt::KeepAspectRatio)); // Redimensionner si nécessaire
    QLabel::enterEvent(event);
}

void ClickableLabel::leaveEvent(QEvent* event) {
    // Revenir à l'image par défaut quand la souris quitte
    setPixmap(QPixmap(defaultImagePath).scaled(50, 50, Qt::KeepAspectRatio)); // Remettre l'image par défaut
    QLabel::leaveEvent(event);
}
