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

    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

private:
    Ui::MainWindow *ui;
    cv::Mat orgImg;
    cv::Mat img;
    cv::Mat p_img;
    int minCan = 30;
    double aspect_ratio;
    int maxCan = 50;
    int blur = 9;
    void preprocess();
    std::vector<cv::Point> getContours();
    void on_trackbar(int, void*);
};
#endif // MAINWINDOW_H
