#ifndef SubmitCameraItem_H
#define SubmitCameraItem_H

#include "NotificationType.h"

class SubmitCameraItem
{
public:
	SubmitCameraItem(
		int64_t _id,
		int64_t _tgChatId,
		const std::string & _cameraName,
		const std::string & _url,
		const std::string & _whatToDetect,
		int _detectionThreshold,
		NotificationType _notificationType,
		bool _wasObjectPresentedLastTime,
		int _edgeLeft,
		int _edgeTop,
		int _edgeRight,
		int _edgeBottom
	) :
		m_id(_id),
		m_tgChatId(_tgChatId),
		m_detectionThreshold(_detectionThreshold),
		m_edgeLeft(_edgeLeft),
		m_edgeTop(_edgeTop),
		m_edgeRight(_edgeRight),
		m_edgeBottom(_edgeBottom),
		m_notificationType(_notificationType),
		m_wasObjectPresentedLastTime(_wasObjectPresentedLastTime),
		m_cameraName(_cameraName),
		m_url(_url),
		m_whatToDetect(_whatToDetect)
	{}

	int64_t getId() const {return m_id;}
	int64_t getTgChatId() const {return m_tgChatId;}
	int getDetectionThreshold() const {return m_detectionThreshold;}
	NotificationType getNotificationType() const {return m_notificationType;}
	bool wasObjectPresentedLastTime() const {return m_wasObjectPresentedLastTime;}

	int getEdgeLeft() const {return m_edgeLeft;}
	int getEdgeTop() const {return m_edgeTop;}
	int getEdgeRight() const {return m_edgeRight;}
	int getEdgeBottom() const {return m_edgeBottom;}

	const std::string & getCameraName() const {return m_cameraName;}
	const std::string & getWhatToDetect() const {return m_whatToDetect;}
	const std::string & getUrl() const {return m_url;}

private:
	const int64_t m_id;  // unique identifier of the item got from the server
	const int64_t m_tgChatId;
	const int m_detectionThreshold;

	// ROI of the image, in percents from the image size
	const int m_edgeLeft;
	const int m_edgeTop;
	const int m_edgeRight;
	const int m_edgeBottom;

	const NotificationType m_notificationType;
	const bool m_wasObjectPresentedLastTime;

	const std::string m_cameraName;
	const std::string m_url;
	const std::string m_whatToDetect;
};

inline bool operator== (const SubmitCameraItem & _first, const SubmitCameraItem & _second) {
	return _first.getId() == _second.getId();
}

#endif // SubmitCameraItem_H
