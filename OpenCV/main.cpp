#include <opencv2/opencv.hpp>

int main() {
    cv::Mat originalImage = cv::imread("bananas.jpg");
    cv::imshow("Original", originalImage);

    int x = 20;
    int y = 20;
    int width = 20;
    int height = 20;

    cv::Rect roiRect(x, y, width, height);
    cv::Mat croppedImage = originalImage(roiRect);

    cv::imshow("Cropped", croppedImage);


    cv::waitKey(0);

    return 0;
}