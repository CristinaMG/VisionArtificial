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

    kernel.create(3,3,CV_32FC1);

    connect(&timer,SIGNAL(timeout()),this,SLOT(compute()));
    connect(ui->captureButton,SIGNAL(clicked(bool)),this,SLOT(start_stop_capture(bool)));
    connect(ui->colorButton,SIGNAL(clicked(bool)),this,SLOT(change_color_gray(bool)));
    connect(visorS,SIGNAL(windowSelected(QPointF, int, int)),this,SLOT(selectWindow(QPointF, int, int)));
    connect(visorS,SIGNAL(pressEvent()),this,SLOT(deselectWindow()));

    connect(ui->loadButton,SIGNAL(clicked(bool)),this,SLOT(load_image()));
    connect(ui->saveButton,SIGNAL(clicked(bool)),this,SLOT(save_image()));

    connect(ui->pixelTButton, SIGNAL(clicked(bool)),this,SLOT(pixel_image()));
    connect(ui->kernelButton, SIGNAL(clicked(bool)),this,SLOT(kernel_image()));
    connect(ui->operOrderButton, SIGNAL(clicked(bool)),this,SLOT(operOrder_image()));

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

    comboBox_image(ui->operationComboBox->currentIndex());

    if(winSelected)
    {
        visorS->drawSquare(QPointF(imageWindow.x+imageWindow.width/2, imageWindow.y+imageWindow.height/2), imageWindow.width,imageWindow.height, Qt::green );
    }
    visorS->update();
    visorD->update();

    calculateHistogram();
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
    if(showColorImage){
        cvtColor(destColorImage, saveImage, CV_RGB2BGR);

    }else{
        cvtColor(destGrayImage, saveImage, CV_GRAY2BGR);
    }

    QString fileName = QFileDialog::getSaveFileName(this,
           tr("Save File"), "",
           tr("All Files (*)"));
    try{
        imwrite(fileName.toStdString(), saveImage);
    }catch(...){qDebug()<<"Error al guardar imagen";}

}

void MainWindow::comboBox_image(int index){
    switch (index) {
    case 0:
        transformation_pixel();
        break;
    case 1:
        threshold_image();
        break;
    case 2:
        equalize_hist();
        break;
    case 3:
        gaussian_blur();
        break;
    case 4:
        median_blur();
        break;
    case 5:
        lineal_filter();
        break;
    case 6:
        dilatation();
        break;
    case 7:
        erosion();
        break;
    case 8:
        ellipticalErosion();
        break;
    case 9:
        apply_several();
        break;
    default:
        break;
    }
}


void MainWindow::pixel_image(){
    pixelTDialog.show();
    connect(pixelTDialog.okButton,SIGNAL(clicked(bool)),this,SLOT(closePixel()));
    connect(pixelTDialog.negativeButton, SIGNAL(clicked(bool)), this, SLOT(apply_negative()));
}

void MainWindow::closePixel(){
    pixelTDialog.close();
}

void MainWindow::kernel_image(){
    lFilterDialog.show();
    connect(lFilterDialog.okButton,SIGNAL(clicked(bool)),this,SLOT(closeKernel()));
}

void MainWindow::closeKernel(){
    lFilterDialog.close();
}

void MainWindow::operOrder_image(){
    operOrderDialog.show();
    connect(operOrderDialog.okButton,SIGNAL(clicked(bool)),this,SLOT(closeOperOrder()));
}

void MainWindow::closeOperOrder(){
    operOrderDialog.close();
}

void MainWindow::apply_negative(){
    Mat lookUpTable(1, 256, CV_8U);
    uchar* p = lookUpTable.ptr();

    int i = 255;
    for(;i>=0; i--){
        p[i] = (uchar)(255-i);
    }

    pixelTDialog.newPixelBox1->setValue(255);
    pixelTDialog.newPixelBox2->setValue(170);
    pixelTDialog.newPixelBox3->setValue(85);
    pixelTDialog.newPixelBox4->setValue(0);

    LUT(grayImage, lookUpTable, destGrayImage);
}

// Histograma: El ultimo parametro es un array constante
void MainWindow::transformation_pixel(){

    Mat lookUpTable(1, 256, CV_8U);
    uchar* p = lookUpTable.ptr();
    int i;
    float incF = 0.;
    float origpoint2 = pixelTDialog.origPixelBox2->value();
    float origpoint3 = pixelTDialog.origPixelBox3->value();

    float newpoint1 = pixelTDialog.newPixelBox1->value();
    float newpoint2 = pixelTDialog.newPixelBox2->value();
    float newpoint3 = pixelTDialog.newPixelBox3->value();
    float newpoint4 = pixelTDialog.newPixelBox4->value();

    i = 0;
    for(; i < origpoint2; i++){
        incF = ((newpoint2-newpoint1)/origpoint2)*i;
        p[i] = (uchar)(incF + newpoint1);
    }

    for(; i < origpoint3; i++){
        incF = ((newpoint3-newpoint2)/(origpoint3-origpoint2))*(i-origpoint2);
        p[i] = (uchar)(incF + newpoint2);
    }

    for(; i < 256; i++){
        incF = ((newpoint4-newpoint3)/(255-origpoint3))*(i-origpoint3);
        p[i] = (uchar)(incF + newpoint3);
    }

    LUT(grayImage, lookUpTable, destGrayImage);
}

void MainWindow::threshold_image(){
    int valor = ui->thresholdSpinBox->value();
    threshold(grayImage, destGrayImage, valor, 255, THRESH_BINARY);
}

void MainWindow::equalize_hist(){
    equalizeHist(grayImage, destGrayImage);
}

void MainWindow::gaussian_blur(){
    GaussianBlur(grayImage, destGrayImage,Size(ui->gaussWidthBox->value(), ui->gaussWidthBox->value()), ui->gaussWidthBox->value()/5., ui->gaussWidthBox->value()/5.);
}

void MainWindow::median_blur(){
    medianBlur(grayImage, destGrayImage, 3);
}

void MainWindow::read_kernel(){
    kernel.at<float>(0,0) = lFilterDialog.kernelBox11->value();
    kernel.at<float>(0,1) = lFilterDialog.kernelBox12->value();
    kernel.at<float>(0,2) = lFilterDialog.kernelBox13->value();
    kernel.at<float>(1,0) = lFilterDialog.kernelBox21->value();
    kernel.at<float>(1,1) = lFilterDialog.kernelBox22->value();
    kernel.at<float>(1,2) = lFilterDialog.kernelBox23->value();
    kernel.at<float>(2,0) = lFilterDialog.kernelBox31->value();
    kernel.at<float>(2,1) = lFilterDialog.kernelBox32->value();
    kernel.at<float>(2,2) = lFilterDialog.kernelBox33->value();
}

void MainWindow::lineal_filter(){
    read_kernel();
    filter2D(grayImage, destGrayImage, -1, kernel, Point(-1,-1), lFilterDialog.addedVBox->value());
}

void MainWindow::dilatation(){
    threshold_image();
    Mat copyDest;
    destGrayImage.copyTo(copyDest);
    dilate(copyDest, destGrayImage, Mat());
}

void MainWindow::erosion(){
    threshold_image();
    Mat copyDest;
    destGrayImage.copyTo(copyDest);
    erode(copyDest, destGrayImage, Mat());
}

void MainWindow::ellipticalErosion(){
    threshold_image();
    Mat copyDest;
    destGrayImage.copyTo(copyDest);
    Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE,cv::Size(5,5));
    erode(copyDest, destGrayImage, kernel);
}

void MainWindow::apply_several(){
    if(operOrderDialog.firstOperCheckBox->isChecked())
        comboBox_image(operOrderDialog.operationComboBox1->currentIndex());

    if(operOrderDialog.secondOperCheckBox->isChecked())
        comboBox_image(operOrderDialog.operationComboBox2->currentIndex());

    if(operOrderDialog.thirdOperCheckBox->isChecked())
        comboBox_image(operOrderDialog.operationComboBox3->currentIndex());

    if(operOrderDialog.fourthOperCheckBox->isChecked())
        comboBox_image(operOrderDialog.operationComboBox4->currentIndex());
}


void MainWindow::calculateHistogram(){
    Mat origHist, destHist;
    float ranges[] = {0 ,256};
    const float * arr_ranges[] = {ranges};
    int channel = 0;
    int size = 256;

    cv::calcHist(&grayImage, 1, &channel, Mat(), origHist, 1, &size, arr_ranges);

    Mat histS(100, 320, CV_8UC1, cv::Scalar(255));
    cv::normalize(origHist, origHist, 0, 100, cv::NORM_MINMAX);

    int hpt = static_cast<int>(0.9*256);
    for(int i= 1; i<256; i++){
        float binVal = origHist.at<float>(i);
        int intensity = static_cast<int>(binVal*hpt/100);
        cv::line(histS, cv::Point(i, 256), cv::Point(i,256-intensity),cv::Scalar::all(0));
    }

    cv::imshow("histogram Source", histS);


    cv::calcHist(&destGrayImage, 1, &channel, Mat(), destHist, 1, &size, arr_ranges);

    Mat histD(100, 320, CV_8UC1, cv::Scalar(255));
    cv::normalize(destHist, destHist, 0, 100, cv::NORM_MINMAX);

    int hptD = static_cast<int>(0.9*256);
    for(int i= 1; i<256; i++){
        float binValD = origHist.at<float>(i);
        int intensityD = static_cast<int>(binValD*hptD/100);
        cv::line(histD, cv::Point(i, 256), cv::Point(i,256-intensityD),cv::Scalar::all(0));
    }

    cv::imshow("histogram Dest", histD);

}
