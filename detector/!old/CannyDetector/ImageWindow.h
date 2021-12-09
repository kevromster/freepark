#ifndef _ImageWindow_H_
#define _ImageWindow_H_

#include <string>

namespace cv {
	class Mat;
}

class ImageWindow {
public:
	ImageWindow(const cv::Mat & _image, const std::string & _strWindowName, int _nFlags);

	void show() const;
	const std::string & getWindowName() const {return m_strWindowName;}

private:
	const cv::Mat & m_image;
	const std::string m_strWindowName;
};

class Trackbar {
public:
	Trackbar(
		const std::string & _strTrackbarName,
		const ImageWindow & _windowToAttach,
		int _nInitialValue,
		int _nMaxValue,
		void * _pUserData
	);
	virtual ~Trackbar() {}

	void onAction(int _nPos);

private:
	virtual void onTrackbar(int _nPos, void * _pUserData) = 0;

private:
	int m_nTrackbarValue;
	const std::string m_strTrackbarName;
	void * m_pUserData;

	const ImageWindow & m_attachedWindow;
};

#endif  // _ImageWindow_H_
