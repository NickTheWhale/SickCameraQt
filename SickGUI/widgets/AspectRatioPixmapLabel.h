#pragma once

#include <qlabel>
#include <qpixmap>
#include <qresizeevent>

class AspectRatioPixmapLabel : public QLabel
{
    Q_OBJECT
public:
    explicit AspectRatioPixmapLabel(QWidget* parent = 0);
    QPixmap scaledPixmap() const;
public slots:
    void setPixmap(const QPixmap&);
    void resizeEvent(QResizeEvent*);
private:
    QPixmap pix;
};