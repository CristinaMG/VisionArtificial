#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/video/video.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/calib3d/calib3d.hpp>

#include <rcdraw.h>
#include <ui_pixelTForm.h>
#include <ui_operOrderForm.h>
#include <ui_lFilterForm.h>



using namespace cv;

namespace Ui {
    class MainWindow;
}

class PixelTDialog : public QDialog, public Ui::PixelTForm
{
     Q_OBJECT
public:
     PixelTDialog(QDialog *parent=0) : QDialog(parent){
       setupUi(this);
    }
};

class LFilterDialog : public QDialog, public Ui::LFilterForm
{
     Q_OBJECT
public:
     LFilterDialog(QDialog *parent=0) : QDialog(parent){
       setupUi(this);
    }
};


class OperOrderDialog : public QDialog, public Ui::OperOrderForm
{
     Q_OBJECT
public:
     OperOrderDialog(QDialog *parent=0) : QDialog(parent){
       setupUi(this);
    }
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    QTimer timer;

    VideoCapture *cap;
    RCDraw *visorS, *visorD;
    QImage *imgS, *imgD;
    Mat colorImage, grayImage, destColorImage, destGrayImage;
    Mat gray2ColorImage, destGray2ColorImage;
    bool capture, showColorImage, winSelected;
    Rect imageWindow;
    PixelTDialog pixelTDialog;
    LFilterDialog lFilterDialog;
    OperOrderDialog operOrderDialog;

    Mat kernel;

public slots:
    void compute();
    void start_stop_capture(bool start);
    void change_color_gray(bool color);
    void selectWindow(QPointF p, int w, int h);
    void deselectWindow();
    void load_image();
    void save_image();

    void comboBox_image(int index);

    void pixel_image();
    void closePixel();
    void kernel_image();
    void closeKernel();
    void operOrder_image();
    void closeOperOrder();

    void transformation_pixel();
    void threshold_image();
    void equalize_hist();
    void gaussian_blur();
    void median_blur();
    void lineal_filter();
    void dilatation();
    void erosion();

    void apply_several();

    void read_kernel();

};


#endif // MAINWINDOW_H
