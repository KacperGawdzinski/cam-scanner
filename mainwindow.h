#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <opencv2/imgcodecs.hpp>

QT_BEGIN_NAMESPACE
namespace Ui { class CamScanner; }
QT_END_NAMESPACE

class CamScanner : public QMainWindow
{
    Q_OBJECT

public:
    CamScanner(QWidget *parent = nullptr);
    ~CamScanner();

private slots:
    void on_maxCan_slider_valueChanged(int value);

    void on_minCan_slider_valueChanged(int value);

    void on_blur_slider_valueChanged(int value);

    void on_polyScale_slider_valueChanged(int value);

    void on_pushButton_clicked();

private:
    Ui::CamScanner *ui;
    std::vector<std::vector<cv::Point>> contours;
    cv::Mat orgImg;
    cv::Mat preprocessed;
    cv::Mat result;

    double aspect_ratio = 1.0;
    double poly = 0.02;
    int minCan = 30;
    int maxCan = 50;
    int blur = 9;
    std::string filename;

    void preprocess();
    std::vector<cv::Point> getContours();
    void drawPoints(std::vector<cv::Point>);
    void on_trackbar(int, void*);
};
#endif // MAINWINDOW_H
