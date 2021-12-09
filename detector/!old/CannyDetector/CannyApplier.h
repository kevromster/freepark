#ifndef _CannyApplier_H_
#define _CannyApplier_H_

#include <opencv2/core/core.hpp>

class CannyApplier {
public:
	CannyApplier(const cv::Mat & _initialImage, double _dLowThreshold, double _dHighThreshold);

	void apply();

	void setLowThreshold(double _dLowThreshold) {m_dLowThreshold = _dLowThreshold;}
	void setHighThreshold(double _dHighThreshold) {m_dHighThreshold = _dHighThreshold;}

	double getLowThreshold() const {return m_dLowThreshold;}
	double getHighThreshold() const {return m_dHighThreshold;}

	const cv::Mat & getResultImage() const {return m_resultImage;}
	const cv::Mat & getPreprocessedImage() const {return m_preprocessedImage;}

private:
	 void _prepareForCanny(const cv::Mat & _initialImage, cv::Mat & _preparedImage) const;

private:
	double m_dLowThreshold;
	double m_dHighThreshold;

	const cv::Mat & m_initialImage;
	cv::Mat m_preprocessedImage;
	cv::Mat m_resultImage;
};

#endif  // _CannyApplier_H_
