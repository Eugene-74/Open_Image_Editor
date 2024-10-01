#include <QLabel>
#include <QEvent>

class ClickableLabel : public QLabel {
    Q_OBJECT

public:
    explicit ClickableLabel(const QString& imagePath, QWidget* parent = nullptr);

protected:
    void enterEvent(QEvent* event) override;  // Utiliser QEvent
    void leaveEvent(QEvent* event) override;

private:
    QString defaultImagePath;  // Variable pour stocker l'image par défaut
    QString hoverImagePath;    // Variable pour stocker l'image de surbrillance
};
