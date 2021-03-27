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
#include <QMessageBox>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/objdetect.hpp>

using namespace cv;
using namespace std;

vector<Point> reorder(vector<Point> points) {
    vector<Point> res;
    vector<int> sum, diff;
    for(uint i = 0; i < 4; i++) {
        sum.push_back(points[i].x + points[i].y);
        diff.push_back(points[i].x - points[i].y);
    }
    res.push_back(points[min_element(sum.begin(), sum.end()) - sum.begin()]);
    res.push_back(points[max_element(diff.begin(), diff.end()) - diff.begin()]);
    res.push_back(points[min_element(diff.begin(), diff.end()) - diff.begin()]);
    res.push_back(points[max_element(sum.begin(), sum.end()) - sum.begin()]);
    return res;
}

void CamScanner::drawPoints(vector<Point> points) {
    if(points.size() == 4) {
        Mat output;
        this->preprocessed.copyTo(output);
        cvtColor(output, output, COLOR_BGR2RGB);
        int i = 1;
        for(const auto &p : points) {
            circle(output, p, 10, Scalar(255, 255, 255), FILLED);
            i++;
        }
        this->ui->scanImage->setPixmap(QPixmap::fromImage(QImage(output.data, output.cols, output.rows, output.step, QImage::Format_RGB888)));
    }
}

Mat getWarp(Mat img, vector<Point> p, float width, float height, double aspect_ratio) {
    width = width / aspect_ratio;
    height = height / aspect_ratio;
    Mat warp;
    Point2f src[4] = { p[0] / aspect_ratio, p[1] / aspect_ratio, p[2] / aspect_ratio, p[3] / aspect_ratio };
    Point2f dst[4] = { {0.0, 0.0}, {width, 0.0}, {0.0, height}, {width, height} };
    Mat matrix = getPerspectiveTransform(src, dst);
    warpPerspective(img, warp, matrix, Point(width, height));
    return warp;
}

void CamScanner::preprocess() {
    //https://stackoverflow.com/questions/7731742/square-detection-doesnt-find-squares/7732392#7732392
    //upgraded with convexHull for contour amount reduction
    Mat a;
    this->preprocessed.copyTo(a);
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
            if(res.size() > 0) {
                auto ordered = reorder(res[0]);
                drawPoints(ordered);
                float w = max(ordered[1].x - ordered[0].x, ordered[3].x - ordered[2].x);
                float h = max(ordered[2].y - ordered[0].y, ordered[3].y - ordered[1].y);
                Mat imgWarp = getWarp(this->orgImg, ordered, w, h, aspect_ratio);
                cvtColor(imgWarp, imgWarp, COLOR_BGR2RGB);
                imgWarp.copyTo(this->result);

                int max = std::max(imgWarp.cols, imgWarp.rows);
                if(max > imgWarp.rows) {
                    rotate(imgWarp, imgWarp, ROTATE_90_CLOCKWISE);
                }

                double render_aspect_ratio = 680 / (double)max;
                cv::resize(imgWarp, imgWarp, Size(), render_aspect_ratio, render_aspect_ratio);

                int min = std::min(imgWarp.cols, imgWarp.rows);
                this->ui->render->resize(min, 680);

                this->ui->render->setPixmap(QPixmap::fromImage(QImage(imgWarp.data, imgWarp.cols, imgWarp.rows, imgWarp.step, QImage::Format_RGB888)));
            }
        }
    }
}

CamScanner::CamScanner(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::CamScanner)
{
    ui->setupUi(this);
    this->setCentralWidget(this->centralWidget());
    this->filename = "src.jpg";
    Mat orgImg, image;

    orgImg = imread(this->filename);
    this->orgImg = orgImg;
    orgImg.copyTo(image);

    int max = std::max(orgImg.cols, orgImg.rows);
    if(max > orgImg.rows) {
        rotate(orgImg, image, ROTATE_90_CLOCKWISE);
    }

    this->aspect_ratio = 680 / (double)max;
    cv::resize(image, image, Size(), aspect_ratio, aspect_ratio);

    int min = std::min(image.cols, image.rows);
    this->ui->scanImage->resize(min, 680);

    int offset = 500 - this->ui->scanImage->width();
    if(offset > 0) {
        this->ui->scanImage->move(this->ui->scanImage->geometry().topLeft().x() + offset, this->ui->scanImage->geometry().topLeft().y());
    }
    this->preprocessed = image;
    this->preprocess();
    waitKey(0);
}

CamScanner::~CamScanner()
{
    delete ui;
}

void CamScanner::on_maxCan_slider_valueChanged(int value)
{
    this->maxCan = value;
    preprocess();
}

void CamScanner::on_minCan_slider_valueChanged(int value)
{
    this->minCan = value;
    preprocess();
}

void CamScanner::on_blur_slider_valueChanged(int value)
{
    if(value % 2 == 0) {
        value++;
    }
    this->blur = value;
}

void CamScanner::on_polyScale_slider_valueChanged(int value)
{
    this->poly = (double)value / 100;
    preprocess();
}

void CamScanner::on_pushButton_clicked()
{
    cvtColor(this->result, this->result, COLOR_BGR2RGB);
    imwrite("scan.jpg", this->result);
    QMessageBox::information(this, tr("Confirmation"), tr("Scan successfully created."));
}
