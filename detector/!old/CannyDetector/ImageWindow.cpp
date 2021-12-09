#include "ImageWindow.h"

#include <opencv2/highgui/highgui.hpp>

ImageWindow::ImageWindow(const cv::Mat & _image, const std::string & _strWindowName, int _nFlags) :
	m_image(_image), m_strWindowName(_strWindowName)
{
	assert(!m_strWindowName.empty());
	cv::namedWindow(m_strWindowName, _nFlags);
}

void ImageWindow::show() const {
	cv::imshow(m_strWindowName, m_image);
}

static
void trackbarCallback(int _nPos, void * _pUserData);

Trackbar::Trackbar(
	const std::string & _strTrackbarName,
	const ImageWindow & _windowToAttach,
	int _nInitialValue,
	int _nMaxValue,
	void * _pUserData
) :
	m_nTrackbarValue(_nInitialValue),
	m_strTrackbarName(_strTrackbarName),
	m_pUserData(_pUserData),
	m_attachedWindow(_windowToAttach)
{
	assert(!m_strTrackbarName.empty());
	assert(_nInitialValue <= _nMaxValue);
	cv::createTrackbar(m_strTrackbarName, m_attachedWindow.getWindowName(), &m_nTrackbarValue, _nMaxValue, &trackbarCallback, this);
}

void Trackbar::onAction(int _nPos) {
	onTrackbar(_nPos, m_pUserData);
	m_attachedWindow.show();
}

static
void trackbarCallback(int _nPos, void * _pUserData) {
	assert(_pUserData != nullptr);
	static_cast<Trackbar*>(_pUserData)->onAction(_nPos);
}
