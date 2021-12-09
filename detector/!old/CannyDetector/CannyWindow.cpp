#include "CannyWindow.h"
#include "CannyApplier.h"

ThresholdCannyTrackbar::ThresholdCannyTrackbar(
	const std::string & _strTrackbarName,
	const ImageWindow & _windowToAttach,
	int _nInitialValue,
	CannyApplier & _cannyApplier
) :
	Trackbar(_strTrackbarName, _windowToAttach, _nInitialValue, ms_nMaxValue, NULL), m_cannyApplier(_cannyApplier)
{}

void ThresholdCannyTrackbar::onTrackbar(int _nPos, void*) {
	setCannyThreshold(m_cannyApplier, _nPos);
	m_cannyApplier.apply();
}

LowThresholdTrackbar::LowThresholdTrackbar(const ImageWindow & _windowToAttach, CannyApplier & _cannyApplier) :
	ThresholdCannyTrackbar("LowThreshold", _windowToAttach, _cannyApplier.getLowThreshold(), _cannyApplier)
{}

void LowThresholdTrackbar::setCannyThreshold(CannyApplier & _applier, int _nThreshold) {
	_applier.setLowThreshold(_nThreshold);
}

HighThresholdTrackbar::HighThresholdTrackbar(const ImageWindow & _windowToAttach, CannyApplier & _cannyApplier) :
	ThresholdCannyTrackbar("HighThreshold", _windowToAttach, _cannyApplier.getHighThreshold(), _cannyApplier)
{}

void HighThresholdTrackbar::setCannyThreshold(CannyApplier & _applier, int _nThreshold) {
	_applier.setHighThreshold(_nThreshold);
}

CannyWindow::CannyWindow(CannyApplier & _applier, const std::string & _strWindowName, int _nFlags) :
	ImageWindow(_applier.getResultImage(), _strWindowName, _nFlags),
	m_applier(_applier)
{
	m_pLowThresholdTrackbar = std::make_unique<LowThresholdTrackbar>(*this, m_applier);
	m_pHighThresholdTrackbar = std::make_unique<HighThresholdTrackbar>(*this, m_applier);
}
