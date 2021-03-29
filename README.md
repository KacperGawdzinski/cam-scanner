# CamScanner
An OpenCV and Qt desktop app for making scans from images
![i](https://i.ibb.co/TWFD0Yr/front.png)

## Description
CamScanner is an app which will create image scan for you. After uploading it will (at it's best) try to determine paper edges automatically. However to handle wrong detection user has four sliders to use. To find about those values read:
- minCan, maxCan - https://docs.opencv.org/master/da/d22/tutorial_py_canny.html
- blur - used to remove image noise - better edge detection
- polyScale - epsilon for finding contour shape with less vertices. Larger -> less vertices

After finding paper corners image is being warped to find best output for user. Mostly useful in cases when photo was crooked.

Please note! Image render preview may not be exactly the same as actual render. That's because of working on much smaller image in program to increase it's speed. The same goes to image rotation - program will rotate image in order to fit most content for user.
## Installation
Follow [official setup](https://wiki.qt.io/How_to_setup_Qt_and_openCV_on_Windows)

## Future updates
Manual image corners (dots) moving
Black and white output when needed