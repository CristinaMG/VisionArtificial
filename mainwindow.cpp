#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    cap = new VideoCapture(0);
    if(!cap->isOpened())
        cap = new VideoCapture(1);
    capture = true;
    showColorImage = false;
    winSelected = false;
    cap->set(CV_CAP_PROP_FRAME_WIDTH, 320);
    cap->set(CV_CAP_PROP_FRAME_HEIGHT, 240);
    imgS = new QImage(320,240, QImage::Format_RGB888);
    visorS = new RCDraw(320,240, imgS, ui->imageFrameS);
    imgD = new QImage(320,240, QImage::Format_RGB888);
    visorD = new RCDraw(320,240, imgD, ui->imageFrameD);

    colorImage.create(240,320,CV_8UC3);
    grayImage.create(240,320,CV_8UC1);
    destColorImage.create(240,320,CV_8UC3);
    destGrayImage.create(240,320,CV_8UC1);
    gray2ColorImage.create(240,320,CV_8UC3);
    destGray2ColorImage.create(240,320,CV_8UC3);

    connect(&timer,SIGNAL(timeout()),this,SLOT(compute()));
    connect(ui->captureButton,SIGNAL(clicked(bool)),this,SLOT(start_stop_capture(bool)));
    connect(ui->colorButton,SIGNAL(clicked(bool)),this,SLOT(change_color_gray(bool)));
    connect(visorS,SIGNAL(windowSelected(QPointF, int, int)),this,SLOT(selectWindow(QPointF, int, int)));
    connect(visorS,SIGNAL(pressEvent()),this,SLOT(deselectWindow()));
    connect(ui->loadButton,SIGNAL(clicked(bool)),this,SLOT(load_image()));
    connect(ui->saveButton,SIGNAL(clicked(bool)),this,SLOT(save_image()));
    connect(ui->copyButton,SIGNAL(clicked(bool)),this,SLOT(copy_image()));
    connect(ui->resizeButton,SIGNAL(clicked(bool)),this,SLOT(resize_image()));
    connect(ui->enlargeButton,SIGNAL(clicked(bool)),this,SLOT(enlarge_image()));

    timer.start(60);
}

MainWindow::~MainWindow()
{
    delete ui;
    delete cap;
    delete visorS;
    delete visorD;
    delete imgS;
    delete imgD;

}


void MainWindow::compute()
{

    if(capture && cap->isOpened())
    {
        *cap >> colorImage;

        cvtColor(colorImage, grayImage, CV_BGR2GRAY);
        cvtColor(colorImage, colorImage, CV_BGR2RGB);

    }

    if(ui->warpButton->isChecked())
        warp_image();

    //Warping con warpAffine transforma de x,y a otras coord transformando

    if(showColorImage)
    {
        memcpy(imgS->bits(), colorImage.data , 320*240*3*sizeof(uchar));
        memcpy(imgD->bits(), destColorImage.data , 320*240*3*sizeof(uchar));
    }
    else
    {
        cvtColor(grayImage,gray2ColorImage, CV_GRAY2RGB);
        cvtColor(destGrayImage,destGray2ColorImage, CV_GRAY2RGB);
        memcpy(imgS->bits(), gray2ColorImage.data , 320*240*3*sizeof(uchar));
        memcpy(imgD->bits(), destGray2ColorImage.data , 320*240*3*sizeof(uchar));

    }

    if(winSelected)
    {
        visorS->drawSquare(QPointF(imageWindow.x+imageWindow.width/2, imageWindow.y+imageWindow.height/2), imageWindow.width,imageWindow.height, Qt::green );
    }
    visorS->update();
    visorD->update();

}

void MainWindow::start_stop_capture(bool start)
{
    if(start)
    {
        ui->captureButton->setText("Stop capture");
        capture = true;
    }
    else
    {
        ui->captureButton->setText("Start capture");
        capture = false;
    }
}

void MainWindow::change_color_gray(bool color)
{
    if(color)
    {
        ui->colorButton->setText("Gray image");
        showColorImage = true;
    }
    else
    {
        ui->colorButton->setText("Color image");
        showColorImage = false;
    }
}

void MainWindow::selectWindow(QPointF p, int w, int h)
{
    QPointF pEnd;
    if(w>0 && h>0)
    {
        imageWindow.x = p.x()-w/2;
        if(imageWindow.x<0)
            imageWindow.x = 0;
        imageWindow.y = p.y()-h/2;
        if(imageWindow.y<0)
            imageWindow.y = 0;
        pEnd.setX(p.x()+w/2);
        if(pEnd.x()>=320)
            pEnd.setX(319);
        pEnd.setY(p.y()+h/2);
        if(pEnd.y()>=240)
            pEnd.setY(239);
        imageWindow.width = pEnd.x()-imageWindow.x;
        imageWindow.height = pEnd.y()-imageWindow.y;

        winSelected = true;
    }
}

void MainWindow::deselectWindow()
{
    winSelected = false;
}

void MainWindow::load_image()
{
    QString fileName = QFileDialog::getOpenFileName(this,
           tr("Open File"), "",
           tr("All Files (*)"));

    if (fileName.isEmpty())
           return;
    else {

           colorImage = imread(fileName.toStdString(), CV_LOAD_IMAGE_COLOR);
           cv::resize(colorImage, colorImage, Size(320,240));
           cvtColor(colorImage, colorImage, CV_BGR2RGB);
           cvtColor(colorImage, grayImage, CV_BGR2GRAY);

           ui->captureButton->setText("Start capture");
           capture = false;
           ui->captureButton->setChecked(false);
    }
}

void MainWindow::save_image()
{
    Mat saveImage;
    if(showColorImage)
        cvtColor(destColorImage, saveImage, CV_RGB2BGR);
    else
        cvtColor(destGrayImage, saveImage, CV_GRAY2BGR);

    QString fileName = QFileDialog::getSaveFileName(this,
           tr("Save File"), "",
           tr("All Files (*)"));

    try{
        imwrite(fileName.toStdString(), saveImage);
    }catch(...){}

}

void MainWindow::copy_image()
{
    if(winSelected){
        int x = (320-imageWindow.width)/2;
        int y = (240-imageWindow.height)/2;

        destColorImage.setTo(0); //to put in black
        Mat winD = destColorImage(cv::Rect(x, y, imageWindow.width, imageWindow.height));
        Mat(colorImage, imageWindow).copyTo(winD);

        destGrayImage.setTo(0);
        winD = destGrayImage(cv::Rect(x, y, imageWindow.width, imageWindow.height));
        Mat(grayImage, imageWindow).copyTo(winD);

    }else {
        colorImage.copyTo(destColorImage);
        grayImage.copyTo(destGrayImage);
    }
}

void MainWindow::resize_image()
{
    if(winSelected){
        cv::resize(Mat(colorImage, imageWindow), destColorImage, Size(320,240));
        cv::resize(Mat(grayImage, imageWindow), destGrayImage, Size(320,240));
    }
}

void MainWindow::enlarge_image()
{
    if(winSelected){
        destColorImage.setTo(0); //to put in black
        destGrayImage.setTo(0);

        float fx = 320./imageWindow.width;
        float fy = 240./imageWindow.height;

        //Utilizar una imagen auxiliar y copiar el contenido para no perderlo
        Mat auxDC, auxDG;
        if(fx <= fy){
            int x = 0;
            int y = (240-imageWindow.height*fx)/2;

            auxDC.create(imageWindow.height*fx,320,CV_8UC3);
            auxDG.create(imageWindow.height*fx,320, CV_8UC1);

            cv::resize(Mat(colorImage, imageWindow), auxDC, Size(), fx, fx);
            auxDC.copyTo(destColorImage(cv::Rect(x, y, auxDC.cols, auxDC.rows)));
            cv::resize(Mat(grayImage, imageWindow), auxDG, Size(), fx, fx);
            auxDG.copyTo(destGrayImage(cv::Rect(x, y, auxDG.cols, auxDG.rows)));
        }else{
            int x = (320-imageWindow.width*fy)/2;
            int y = 0;

            auxDC.create(240,imageWindow.width*fy,CV_8UC3);
            auxDG.create(240,imageWindow.width*fy, CV_8UC1);

            cv::resize(Mat(colorImage, imageWindow), auxDC, Size(), fy, fy);
            auxDC.copyTo(destColorImage(cv::Rect(x, y, auxDC.cols, auxDC.rows)));
            cv::resize(Mat(grayImage, imageWindow), auxDG, Size(), fy, fy);
            auxDG.copyTo(destGrayImage(cv::Rect(x, y, auxDG.cols, auxDG.rows)));
        }
    }    
}

void MainWindow::warp_image(){

        int dialValue, horizontalValue, verticalValue;
        float auxDial,zoomValue;
        dialValue = ui->angleDial->value();
        horizontalValue = ui->horizontalScrollBar->value();
        verticalValue = ui->verticalScrollBar->value();
        zoomValue = ui->zoomScrollBar->value()/10.;

        //Warp
        auxDial = dialValue*2*PI/360;

        cv::Matx<float, 2, 3> mt(cos(auxDial),
                                 sin(auxDial),
                                 horizontalValue + 160 -160*cos(auxDial) -120*sin(auxDial),
                                 -sin(auxDial),
                                 cos(auxDial),
                                 verticalValue + 120 +160*sin(auxDial) -120*cos(auxDial));



        Size sDest(320, 240);
        warpAffine(grayImage, destGrayImage, mt, sDest);
        warpAffine(colorImage, destColorImage, mt, sDest);

        //Zoom
        float cols = 320./zoomValue;
        float rows = 240./zoomValue;
        int x = 160-cols/2;
        int y = 120-rows/2;

        //Gray
        Mat winDG = destGrayImage(cv::Rect(x, y, cols, rows));
        Mat auxDG;
        winDG.copyTo(auxDG);
        cv::resize(auxDG, destGrayImage, Size(320,240));

        //Color
        Mat winDC = destColorImage(cv::Rect(x, y, cols, rows));
        Mat auxDC;
        winDC.copyTo(auxDC);
        cv::resize(auxDC, destColorImage, Size(320,240));
}


