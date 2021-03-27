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

void MainWindow::drawPoints(vector<vector<Point>> points ) {
    Mat output;
    img.copyTo(output);
    if(points.size() > 0) {
        for(const auto &p2 : points[0]) {
            circle(output, p2, 10, Scalar(255, 255, 255), FILLED);
        }
        this->ui->scanImage_2->setPixmap(QPixmap::fromImage(QImage(output.data, output.cols, output.rows, output.step, QImage::Format_RGB888)));
    }
}

void MainWindow::preprocess() {
    //https://stackoverflow.com/questions/7731742/square-detection-doesnt-find-squares/7732392#7732392
    //upgraded with convexHull for contour amount reduction
    Mat a;
    this->img.copyTo(a);
    Mat blurred(a);
    medianBlur(a, blurred, this->blur);

    Mat gray0(blurred.size(), CV_8U), gray;
    vector<vector<Point> > contours;

    for (int c = 0; c < 3; c++) {
        int ch[] = {c, 0};
        mixChannels(&blurred, 1, &gray0, 1, ch, 1);
        const int threshold_level = 2;
        for (int l = 0; l < threshold_level; l++) {
            if (l == 0) {
                Canny(gray0, gray, this->minCan, this->maxCan, 3);
                dilate(gray, gray, Mat(), Point(-1,-1));
            }
            else {
                gray = gray0 >= (l+1) * 255 / threshold_level;
            }

            findContours(gray, contours, RETR_LIST, CHAIN_APPROX_SIMPLE);

            vector<vector<Point>> hull(contours.size());
            for( size_t i = 0; i < contours.size(); i++) {
                convexHull( contours[i], hull[i] );
            }
            vector<vector<Point> > res;
            vector<Point> approx;
            vector<vector<Point>> conPoly(hull.size());
            for (size_t i = 0; i < hull.size(); i++)
            {
                approxPolyDP(hull[i], approx, arcLength(Mat(hull[i]), true) * this->poly, true);
                if (approx.size() == 4 &&
                    fabs(contourArea(Mat(approx))) > 1000 &&
                    isContourConvex(Mat(approx)))
                {
                    res.push_back(approx);
                }
            }
            drawPoints(res);
        }
    }
}

vector<Point> MainWindow::getContours() {
    vector<vector<Point>> contours;
    vector<Vec4i> hierarchy;
    vector<Point> res;
    double maxArea = 0;
    int max = 0;
    findContours(this->p_img, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
    vector<vector<Point>> conPoly(contours.size());
    for (uint32_t i = 0; i < contours.size(); i++) {
        double area = contourArea(contours[i]);
        double perimeter = arcLength(contours[i], true);
        approxPolyDP(contours[i], conPoly[i], 0.02 * perimeter, true);
        if (area > maxArea) {
            maxArea = area;
            max = i;
        }
    }
    Mat output;
    this->img.copyTo(output);
    drawContours(output, contours, max, Scalar(255, 0, 255), 5);
    cvtColor(output, output, COLOR_BGR2RGB);
    this->ui->scanImage->setPixmap(QPixmap::fromImage(QImage(output.data, output.cols, output.rows, output.step, QImage::Format_RGB888)));
    return contours[max];
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setCentralWidget(this->centralWidget());
    string path = "src4.jpg";
    Mat orgImg, image;

    orgImg = imread(path);
    this->orgImg = orgImg;

    int max = std::max(orgImg.cols, orgImg.rows);
    this->aspect_ratio = 500 / (double)max;
    cv::resize(orgImg, image, Size(), aspect_ratio, aspect_ratio);

    int min = std::min(image.cols, image.rows);
    if(min == image.cols) {
        this->ui->scanImage->resize(min, 500);
        this->ui->scanImage_2->resize(min, 500);
    }
    else {
        this->ui->scanImage->resize(500, min);
        this->ui->scanImage_2->resize(500, min);
    }

    int offset = 610 - (110 + this->ui->scanImage->geometry().topLeft().x());
    if(offset > 0) {
        this->ui->scanImage->move(offset, this->ui->scanImage->geometry().topLeft().y());
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
    if(value >> 2 == 1) {
        value++;
    }
    this->blur = value;
}

void MainWindow::on_polyScale_slider_valueChanged(int value)
{
    this->poly = (double)value / 100;
    preprocess();
}
