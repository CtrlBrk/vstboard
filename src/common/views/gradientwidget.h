#ifndef GRADIENTWIDGET_H
#define GRADIENTWIDGET_H

#include <QWidget>
#include <QPen>

namespace View {

    class GradientWidget : public QWidget
    {
        Q_OBJECT

    public:
        explicit GradientWidget(QWidget *parent = 0);

        QColor getSelectedColor() {return m_selected_color;}
        int getRed() {return m_selected_color.red();}
        int getGreen() {return m_selected_color.green();}
        int getBlue() {return m_selected_color.blue();}
        qreal getAlpha() {return alpha;}
        qreal getHue() {return hue;}
        qreal getSaturation() {return saturation;}
        qreal getValue() {return value;}

    signals:
        void colorSelected(const QColor &color);

    public slots:
        void setMainColor(const QColor &col);
        void setRed(int r);
        void setGreen(int g);
        void setBlue(int b);
        void setAlpha(int a);
        void setHue(int h);
        void setSaturation(int s);
        void setValue(int v);



    protected:
        void mouseMoveEvent(QMouseEvent *event);
        void mousePressEvent(QMouseEvent *event);
        void mouseReleaseEvent(QMouseEvent *event);
        void enterEvent(QEvent *event);
        void leaveEvent(QEvent *event);
        void paintEvent(QPaintEvent *event);
        void resizeEvent(QResizeEvent *event);

    private:
        float hue,saturation,value,alpha;
        QColor m_selected_color;    // Couleur sélectionné dans le dégradé
        QImage m_gradient_image;    // Image contenant le degradé à afficher
        QPoint m_cursor_position;   // Position du point de sélection
        bool m_left_button_pressed; // Bouton gauche de la souris pressé ou non

        void updateGradientImage();
        void cursorMoved();
        void updateCursorPosition();
    };
}
#endif // GRADIENTWIDGET_H
