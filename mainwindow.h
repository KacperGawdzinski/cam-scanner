#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <opencv2/imgcodecs.hpp>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_maxCan_slider_valueChanged(int value);

    void on_minCan_slider_valueChanged(int value);

    void on_blur_slider_valueChanged(int value);

    void on_polyScale_slider_valueChanged(int value);

private:
    Ui::MainWindow *ui;
    std::vector<std::vector<cv::Point>> contours;
    cv::Mat orgImg;
    cv::Mat img;
    cv::Mat p_img;
    int minCan = 30;
    double aspect_ratio;
    double poly = 0.02;
    int maxCan = 50;
    int blur = 9;
    void preprocess();
    std::vector<cv::Point> getContours();
    void drawPoints(std::vector<std::vector<cv::Point>>);
    void on_trackbar(int, void*);
};
#endif // MAINWINDOW_H
