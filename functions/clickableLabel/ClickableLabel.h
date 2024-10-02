#ifndef CLICKABLELABEL_H
#define CLICKABLELABEL_H

#include <QLabel>
#include <QEvent>
#include <QGraphicsOpacityEffect>


class ClickableLabel : public QLabel {
    Q_OBJECT

public:
    explicit ClickableLabel(const QString& imagePath, QWidget* parent = nullptr);

signals:
    void clicked();  // Signal émis lors du clic

protected:
    void enterEvent(QEvent* event) override;
    void leaveEvent(QEvent* event) override;
    // Gestion de l'événement de clic de souris
    void mousePressEvent(QMouseEvent* event) override {
        emit clicked();  // Émettre le signal quand on clique
    }

private:
    QGraphicsOpacityEffect* opacityEffect;  // Déclaration de l'effet d'opacité
    QPixmap defaultPixmap;                  // Image par défaut pour le QLabel
};

#endif
