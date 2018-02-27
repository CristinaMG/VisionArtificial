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


void MainWindow::transformation_pixel(){

}


void MainWindow::threshold_image(){

    //valor umbral
    int valor;
    threshold(grayImage, destGrayImage, valor, 255, THRESH_BINARY);
}

<<<<<<< HEAD

void MainWindow::equalize_hist(){

     equalizeHist(grayImage, destGrayImage);
=======
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
>>>>>>> 938d9ac6f25174ac8a9480921414a3e5b22ae7a8
}

void MainWindow::gaussian_blur(){

}

<<<<<<< HEAD
void MainWindow::median_blur(){
=======
        //Warp
        auxDial = dialValue*2*PI/360;
>>>>>>> 938d9ac6f25174ac8a9480921414a3e5b22ae7a8

}

void MainWindow::lineal_filter(){

}

void MainWindow::dilatation(){

}

void MainWindow::erosion(){

}




