#ifndef _CannyWindow_H_
#define _CannyWindow_H_

#include <memory>

#include "ImageWindow.h"

class CannyApplier;

class ThresholdCannyTrackbar : public Trackbar {
public:
	ThresholdCannyTrackbar(
		const std::string & _strTrackbarName,
		const ImageWindow & _windowToAttach,
		int _nInitialValue,
		CannyApplier & _cannyApplier
	);

private:
	virtual void onTrackbar(int _nPos, void*);
	virtual void setCannyThreshold(CannyApplier & _applier, int _nThreshold) = 0;

private:
	static const int ms_nMaxValue = 200;
	CannyApplier & m_cannyApplier;
};

class LowThresholdTrackbar : public ThresholdCannyTrackbar {
public:
	LowThresholdTrackbar(const ImageWindow & _windowToAttach, CannyApplier & _cannyApplier);

private:
	void setCannyThreshold(CannyApplier & _applier, int _nThreshold);
};

class HighThresholdTrackbar : public ThresholdCannyTrackbar {
public:
	HighThresholdTrackbar(const ImageWindow & _windowToAttach, CannyApplier & _cannyApplier);

private:
	void setCannyThreshold(CannyApplier & _applier, int _nThreshold);
};

class CannyWindow : public ImageWindow {
public:
	CannyWindow(CannyApplier & _applier, const std::string & _strWindowName, int _nFlags);

private:
	CannyApplier & m_applier;
	std::unique_ptr<LowThresholdTrackbar> m_pLowThresholdTrackbar;
	std::unique_ptr<HighThresholdTrackbar> m_pHighThresholdTrackbar;
};

#endif  // _CannyWindow_H_
