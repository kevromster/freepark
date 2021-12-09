#ifndef MathUtils_H
#define MathUtils_H

#include <cmath>
#include <opencv2/core/types.hpp>

namespace MathUtils {

const double EPSILON = 1.e-6;

inline double degreesToRadians(double _dDegrees) {
	return (_dDegrees * M_PI) / 180.;
}

inline double radiansToDegrees(double _dRadians) {
	return (_dRadians * 180.) / M_PI;
}

/**
 * Projects point to the given line using the speciifed projection angle.
 * Here on the picture we want to project point P to the line OD with the projection angle a.
 *   P   the point to project;
 *   a   the projection angle betweeen vertical line and PD line;
 *   O   the beginning of the line to proect to;
 *   b   the angle between the projection line and OX axis, O and b fully specify the line;
 *   D   the projected point.
 *
 *      * P
 *      |\
 *      |a\
 *      |  \
 *      |   \
 *      |    * D
 *      |  /
 *      | /
 *      *
 *     /
 *    / b
 *   *----- X
 *   O
 *
 *
 * @param _pointToProject the point to project
 * @param _dProjectionAngle the projection angle to use, radians
 * @param _lineStart the beginning of the line to project to
 * @param _dLineAngle the angle between the line and OX axis, radians
 * @return the projected point
 */
cv::Point2d projectPointToLine(const cv::Point2d & _pointToProject, double _dProjectionAngle, const cv::Point2d & _lineStart, double _dLineAngle);

} // namespace MathUtils

#endif // MathUtils_H
