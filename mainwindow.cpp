#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <iostream>
#include <string>
#include <stdint.h>
#include <vector>
#include <QDebug>
#include <QtCore>
#include <QtGui>
#include <QPixmap>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/objdetect.hpp>

using namespace cv;
using namespace std;

void MainWindow::preprocess() {
    Mat blurred_image;
    medianBlur(this->img, blurred_image, this->blur);

    Mat gray_image;
    cvtColor(blurred_image, gray_image, COLOR_BGR2GRAY);

    Mat canny_image;
    Canny(gray_image, canny_image, this->minCan, this->maxCan);

    Mat print;
    cvtColor(canny_image, print, COLOR_BGR2RGB);

    int min = std::min(print.cols, print.rows);
    if(min == print.cols)
        this->ui->scanImage->resize(min, 680);
    else
        this->ui->scanImage->resize(680, min);

    this->ui->scanImage->setPixmap(QPixmap::fromImage(QImage(print.data, print.cols, print.rows, print.step, QImage::Format_RGB888)));

    this->p_img = canny_image;
}

vector<Point> MainWindow::getContours() {
    vector<vector<Point>> contours;
    vector<Vec4i> hierarchy;
    vector<Point> res;
    double maxArea = 0;
    int max = 0;
    findContours(this->p_img, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE);
    vector<vector<Point>> conPoly(contours.size());
    for (uint32_t i = 0; i < contours.size(); i++) {
        double area = contourArea(contours[i]);
        //if (area > 2000) {
            double perimeter = arcLength(contours[i], false);
            approxPolyDP(contours[i], conPoly[i], 0.02 * perimeter, false);
            if (area > maxArea) {
                maxArea = area;
                max = i;
                //res = conPoly[i];
            }
       // }
        //drawContours(orgImg, contours, i, Scalar(255, 0, 255), 10);
    }
    Mat output = this->img;
    drawContours(output, contours, max, Scalar(255, 0, 255), 5);
    cvtColor(output, output, COLOR_BGR2RGB);
    this->ui->scanImage->setPixmap(QPixmap::fromImage(QImage(output.data, output.cols, output.rows, output.step, QImage::Format_RGB888)).scaled(680, 680, Qt::KeepAspectRatio));
    return contours[max];
}

void drawPoints(Mat &img, vector<Point> points) {
    for(const auto &p : points) {
        circle(img, p, 10, Scalar(255, 0, 255), FILLED);
    }
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setCentralWidget(this->centralWidget());
    string path = "src2.jpg";
    Mat orgImg, image;

    orgImg = imread(path);
    this->orgImg = orgImg;
    int max = std::max(orgImg.cols, orgImg.rows);
    if(max > 680) {
        this->aspect_ratio = 680 / (double)max;
        cv::resize(orgImg, image, Size(), aspect_ratio, aspect_ratio);
    }

    this->img = image;
    this->preprocess();
    waitKey(0);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_maxCan_slider_valueChanged(int value)
{
    this->maxCan = value;
    preprocess();
}

void MainWindow::on_minCan_slider_valueChanged(int value)
{
    this->minCan = value;
    preprocess();
}

void MainWindow::on_blur_slider_valueChanged(int value)
{
    if(value % 2 == 0)
        value++;
    this->blur = value;
    preprocess();
}

void MainWindow::on_pushButton_clicked()
{
    this->ui->blur_slider->setEnabled(false);
    this->ui->minCan_slider->setEnabled(false);
    this->ui->maxCan_slider->setEnabled(false);
    auto points = getContours();
    //drawPoints(points);
}

void MainWindow::on_pushButton_2_clicked()
{
    this->ui->blur_slider->setEnabled(true);
    this->ui->minCan_slider->setEnabled(true);
    this->ui->maxCan_slider->setEnabled(true);
}
