#include "MathUtils.h"
#include "CarDetectorLog.h"

#include <opencv2/core/types.hpp>

namespace MathUtils {

cv::Point2d projectPointToLine(const cv::Point2d & _pointToProject, double _dProjectionAngle, const cv::Point2d & _lineStart, double _dLineAngle) {
	if (std::abs(std::abs(_dLineAngle) - M_PI_2) < EPSILON)
	{
		// project to vertical line as it is OX axis
		// TODO: debug it
		LOG(WARNING) << "TODO: debug this case";
		const cv::Point2d fakeLineStart(_lineStart.y, _lineStart.x);
		const cv::Point2d projection =
			projectPointToLine(_pointToProject, _dProjectionAngle > 0. ? _dProjectionAngle - M_PI_2 : _dProjectionAngle + M_PI_2, fakeLineStart, 0.);
		return cv::Point2d(_lineStart.x, 2.*_lineStart.y - projection.x);
	}

	const cv::Point2d ptX(_pointToProject.x, _lineStart.y);
	const double dDistanceToLineByY = ptX.y - _pointToProject.y - (ptX.x - _lineStart.x) * std::tan(_dLineAngle);
	const double dDistanceToLineByAngle = dDistanceToLineByY * std::cos(_dLineAngle) / std::cos(_dProjectionAngle - _dLineAngle);

	const double deltaX = dDistanceToLineByAngle * std::sin(_dProjectionAngle);
	const double deltaY = dDistanceToLineByAngle * std::cos(_dProjectionAngle);

	const cv::Point2d projectedPoint(_pointToProject.x + deltaX, _pointToProject.y + deltaY);
	return projectedPoint;
}

}  // namespace MathUtils
