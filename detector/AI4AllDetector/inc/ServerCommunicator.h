#ifndef ServerCommunicator_H
#define ServerCommunicator_H

#include "SubmitCameraItem.h"

#include <vector>

namespace ServerCommunicator
{

// returns array of camera items to run detection for
std::vector<SubmitCameraItem> requestSubmitCameraItems();

// updates 'object_presented_last_time' field of the camera item on the server
void setSubmitCameraItemObjectFound(int64_t _id, bool _objectFound);

} // namespace ServerCommunicator

#endif // ServerCommunicator_H
