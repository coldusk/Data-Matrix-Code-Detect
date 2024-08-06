#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>

using namespace std;
using namespace cv;

// 计算两点之间的距离
double distance(Point2f p1, Point2f p2) {
	return sqrt((p1.x - p2.x) * (p1.x - p2.x) + (p1.y - p2.y) * (p1.y - p2.y));
}

int main(int argc, char** argv) {
	Mat image, enhance;
	image = imread("image_path");//输入待检测图片的文件地址
	if (!image.data) {
		cout << "could not load image..4." << endl;
		return -1;
	}
	char input_win[] = "input image";
	

	//对比度跟亮度的调整
	int height = image.rows;
	int width = image.cols;
	enhance = Mat::zeros(image.size(), image.type());
	float alpha = 3;//对比度
	float beta = 40;//亮度
	for (int row = 0; row < height; row++)
	{
		for (int col = 0; col < width; col++)
		{
			if (image.channels() == 3)
			{
				int b = image.at<Vec3b>(row, col)[0];
				int g = image.at<Vec3b>(row, col)[1];
				int r = image.at<Vec3b>(row, col)[2];

				enhance.at<Vec3b>(row, col)[0] = saturate_cast<uchar>(b * alpha + beta);
				enhance.at<Vec3b>(row, col)[1] = saturate_cast<uchar>(g * alpha + beta);
				enhance.at<Vec3b>(row, col)[2] = saturate_cast<uchar>(r * alpha + beta);

			}
			else if (image.channels() == 1)
			{
				float v = image.at<uchar>(row, col);
				enhance.at<uchar>(row, col) = saturate_cast<uchar>(v * alpha + beta);
			}
		}
	}

	/*imshow("强化后图片", enhance);*/
    // 将图像转换为灰度图
    Mat gray;
    cvtColor(enhance, gray, COLOR_BGR2GRAY);

    // 检测角点
    vector<Point2f> corners;
    goodFeaturesToTrack(gray, corners, 100, 0.08, 8);//函数参数如下：输入图像，检测到的所有角点的vector，检测到角点数最大值，角点质量水平（0.01--0.1），角点间最小距离

    // 使用K-means聚类对角点进行聚类
    int clusterCount = 4; // 聚类数目，可根据需要调整
    Mat labels, centers;
    kmeans(corners, clusterCount, labels, TermCriteria(TermCriteria::EPS + TermCriteria::COUNT, 10, 1.0), 3, KMEANS_PP_CENTERS, centers);

    // 统计每个聚类的角点数目
    vector<int> clusterSizes(clusterCount, 0);
    for (int i = 0; i < labels.rows; i++) {
        clusterSizes[labels.at<int>(i)]++;
    }

    // 初始化sortedIndices并按角点数目排序
    vector<int> sortedIndices(clusterCount);
    for (int i = 0; i < clusterCount; i++) {
        sortedIndices[i] = i;
    }
    sort(sortedIndices.begin(), sortedIndices.end(), [&](int a, int b) {
        return clusterSizes[a] > clusterSizes[b];
        });

    // 为角点数最多(或前二，可调)的聚类绘制最小包围圆，并标记角点(可选)
    Scalar colors[] = { Scalar(0, 255, 0), Scalar(0,0,255)};
    for (int i = 0; i < 1; i++) {
        int clusterIdx = sortedIndices[i];
        vector<Point2f> clusterPoints;
        vector<Point2f> validPoints;
        Point2f clusterCenter(centers.at<float>(clusterIdx, 0), centers.at<float>(clusterIdx, 1));

        // 计算聚类中所有点到聚类中心的距离的平均值
        double averageDistance = 0;
        for (int j = 0; j < corners.size(); j++) {
            if (labels.at<int>(j) == clusterIdx) {
                double dist = distance(corners[j], clusterCenter);
                averageDistance += dist;
                clusterPoints.push_back(corners[j]);
            }
        }
        averageDistance /= clusterPoints.size();

        // 过滤出有效的点,只有到聚类中心的距离小于各点到聚类中心点平均距离的1.1倍的点才能作为有效点
        for (const auto& point : clusterPoints) {
            if (distance(point, clusterCenter) <= 1.1 * averageDistance) {
                validPoints.push_back(point);
                // 标记有效角点
                /*circle(image, point, 3, colors[i], -1);*/
            }
        }

        // 计算最小包围圆
        Point2f center;
        float radius;
        if (!validPoints.empty()) {
            minEnclosingCircle(validPoints, center, radius);
            // 绘制最小圆，将最小圆半径扩大10%再画
            circle(image, center, radius*1.1, colors[i], 2);
        }
    }

    // 显示结果图像
    imshow("定位结果", image);

	waitKey(0);
	return 0;
}