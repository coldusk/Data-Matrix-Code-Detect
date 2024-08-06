#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>

using namespace std;
using namespace cv;

// ��������֮��ľ���
double distance(Point2f p1, Point2f p2) {
	return sqrt((p1.x - p2.x) * (p1.x - p2.x) + (p1.y - p2.y) * (p1.y - p2.y));
}

int main(int argc, char** argv) {
	Mat image, enhance;
	image = imread("image_path");//��������ͼƬ���ļ���ַ
	if (!image.data) {
		cout << "could not load image..4." << endl;
		return -1;
	}
	char input_win[] = "input image";
	

	//�Աȶȸ����ȵĵ���
	int height = image.rows;
	int width = image.cols;
	enhance = Mat::zeros(image.size(), image.type());
	float alpha = 3;//�Աȶ�
	float beta = 40;//����
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

	/*imshow("ǿ����ͼƬ", enhance);*/
    // ��ͼ��ת��Ϊ�Ҷ�ͼ
    Mat gray;
    cvtColor(enhance, gray, COLOR_BGR2GRAY);

    // ���ǵ�
    vector<Point2f> corners;
    goodFeaturesToTrack(gray, corners, 100, 0.08, 8);//�����������£�����ͼ�񣬼�⵽�����нǵ��vector����⵽�ǵ������ֵ���ǵ�����ˮƽ��0.01--0.1�����ǵ����С����

    // ʹ��K-means����Խǵ���о���
    int clusterCount = 4; // ������Ŀ���ɸ�����Ҫ����
    Mat labels, centers;
    kmeans(corners, clusterCount, labels, TermCriteria(TermCriteria::EPS + TermCriteria::COUNT, 10, 1.0), 3, KMEANS_PP_CENTERS, centers);

    // ͳ��ÿ������Ľǵ���Ŀ
    vector<int> clusterSizes(clusterCount, 0);
    for (int i = 0; i < labels.rows; i++) {
        clusterSizes[labels.at<int>(i)]++;
    }

    // ��ʼ��sortedIndices�����ǵ���Ŀ����
    vector<int> sortedIndices(clusterCount);
    for (int i = 0; i < clusterCount; i++) {
        sortedIndices[i] = i;
    }
    sort(sortedIndices.begin(), sortedIndices.end(), [&](int a, int b) {
        return clusterSizes[a] > clusterSizes[b];
        });

    // Ϊ�ǵ������(��ǰ�����ɵ�)�ľ��������С��ΧԲ������ǽǵ�(��ѡ)
    Scalar colors[] = { Scalar(0, 255, 0), Scalar(0,0,255)};
    for (int i = 0; i < 1; i++) {
        int clusterIdx = sortedIndices[i];
        vector<Point2f> clusterPoints;
        vector<Point2f> validPoints;
        Point2f clusterCenter(centers.at<float>(clusterIdx, 0), centers.at<float>(clusterIdx, 1));

        // ������������е㵽�������ĵľ����ƽ��ֵ
        double averageDistance = 0;
        for (int j = 0; j < corners.size(); j++) {
            if (labels.at<int>(j) == clusterIdx) {
                double dist = distance(corners[j], clusterCenter);
                averageDistance += dist;
                clusterPoints.push_back(corners[j]);
            }
        }
        averageDistance /= clusterPoints.size();

        // ���˳���Ч�ĵ�,ֻ�е��������ĵľ���С�ڸ��㵽�������ĵ�ƽ�������1.1���ĵ������Ϊ��Ч��
        for (const auto& point : clusterPoints) {
            if (distance(point, clusterCenter) <= 1.1 * averageDistance) {
                validPoints.push_back(point);
                // �����Ч�ǵ�
                /*circle(image, point, 3, colors[i], -1);*/
            }
        }

        // ������С��ΧԲ
        Point2f center;
        float radius;
        if (!validPoints.empty()) {
            minEnclosingCircle(validPoints, center, radius);
            // ������СԲ������СԲ�뾶����10%�ٻ�
            circle(image, center, radius*1.1, colors[i], 2);
        }
    }

    // ��ʾ���ͼ��
    imshow("��λ���", image);

	waitKey(0);
	return 0;
}