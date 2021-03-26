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

struct trackbar_info {
    trackbar_info(Mat i, MainWindow *mw) : img(i), main_window(mw) {}
    cv::Mat img;
    int minCan = 30;
    int maxCan = 50;
    int blur = 9;
    int old_blur = 9;
    MainWindow *main_window;
};

void myButtonName_callback(int state, void* userData) {
    // do something
    // qDebug() <<"test";
}

void MainWindow::on_trackbar(int a , void *b) {
    auto tb = (trackbar_info*)b;
    if(tb->old_blur != tb->blur) {
        if(tb->blur % 2 == 0)
            tb->blur++;
        tb->old_blur = tb->blur;
    }

    Mat blurred_image;
    medianBlur(tb->img, blurred_image, tb->blur);

    Mat gray_image;
    cvtColor(blurred_image, gray_image, COLOR_BGR2GRAY);

    Mat canny_image;
    Canny(gray_image, canny_image, tb->minCan, tb->maxCan);

    Mat print;
    cvtColor(canny_image, print, COLOR_BGR2RGB);
    tb->main_window->ui->scanImage->setPixmap(QPixmap::fromImage(QImage(print.data, print.cols, print.rows, print.step, QImage::Format_RGB888)));
    tb->main_window->show();
}

Mat MainWindow::preprocess(Mat img) {
    namedWindow("Trackbars", WINDOW_NORMAL);
    createButton("Accept", myButtonName_callback, NULL, QT_PUSH_BUTTON|QT_NEW_BUTTONBAR);// create a push button in a new row

    double height = 800;
    Mat image;
    Size imageSize(int(height / img.rows * img.cols), height);
    cv::resize(img, image, imageSize);

    auto *tb = new trackbar_info(image, this);
    createTrackbar("minCan", "", &tb->minCan, 500, on_trackbar, tb);
    createTrackbar("maxCan", "", &tb->maxCan, 500, on_trackbar, tb);
    createTrackbar("blur", "", &tb->blur, 99, on_trackbar, tb);

    Mat blurred_image;
    medianBlur(image, blurred_image, tb->blur);

    Mat gray_image;
    cvtColor(blurred_image, gray_image, COLOR_BGR2GRAY);

    Mat canny_image;
    Canny(gray_image, canny_image, tb->minCan, tb->maxCan);

    Mat print;
    cvtColor(canny_image, print, COLOR_BGR2RGB);
    this->ui->scanImage->setPixmap(QPixmap::fromImage(QImage(print.data, print.cols, print.rows, print.step, QImage::Format_RGB888)));
    this->show();

    return canny_image;
}

vector<Point> getContours(Mat &preprocessed, Mat &orgImg) {
    vector<vector<Point>> contours;
    vector<Vec4i> hierarchy;
    vector<Point> res;
    double maxArea = 0;
    int max = 0;
    findContours(preprocessed, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE);
    vector<vector<Point>> conPoly(contours.size());
    for (uint32_t i = 0; i < contours.size(); i++) {
        double area = contourArea(contours[i]);
        if (area > 2000) {
            double perimeter = arcLength(contours[i], true);
            approxPolyDP(contours[i], conPoly[i], 0.02 * perimeter, true);
            if (area > maxArea && conPoly[i].size() == 4) {
                maxArea = area;
                max = i;
                //res = conPoly[i];
            }
        }
        //drawContours(orgImg, contours, i, Scalar(255, 0, 255), 10);
    }
    drawContours(orgImg, contours, max, Scalar(255, 0, 255), 10);
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

    this->setFixedSize(1280,768);
    string path = "src.jpg";
    Mat orgImg, resizedImg;
    orgImg = imread(path);
    Mat preprocessed = this->preprocess(orgImg);
    auto points = getContours(preprocessed, orgImg);
    drawPoints(orgImg, points);
    cv::resize(orgImg, resizedImg, Size(), 0.3, 0.3);
    waitKey(0);
}

MainWindow::~MainWindow()
{
    delete ui;
}
