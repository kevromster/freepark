#include "AI4AllDetectorApp.h"
#include "AI4AllDetectorLog.h"
#include "AI4AllDetectorException.h"
#include "ServerCommunicator.h"

#include <thread>

#include <tgbot/tgbot.h>
#include <tgbot/types/InlineKeyboardMarkup.h>

namespace {

const std::string TF_CARS_GRAPH_FILE = "cars_my_trained_tensorflow_graph.pb";
const std::string TF_GENERAL_PURPOSE_GRAPH_FILE = "general_purpose_tensorflow_graph.pb";
const std::string TF_LABEL_FILE = "mscoco_label_map.pbtxt";

const long SLEEP_INTERVAL_SECS = 10;

// Returns duration in seconds from the moment specified up to the current time.
long getDurationFromMoment(const std::chrono::time_point<std::chrono::high_resolution_clock> & _moment) {
	return std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now() - _moment).count();
}

} // anonymous namespace

AI4AllDetectorApp::AI4AllDetectorApp() :
	m_detector(TF_CARS_GRAPH_FILE, TF_GENERAL_PURPOSE_GRAPH_FILE, TF_LABEL_FILE)
{}

int AI4AllDetectorApp::run(const std::vector<std::string>&) {
	for (;;) {
		const std::chrono::time_point<std::chrono::high_resolution_clock> startTime = std::chrono::high_resolution_clock::now();

		try {
			HDLOG << "Requesting items for detection...";
			const std::vector<SubmitCameraItem> items = ServerCommunicator::requestSubmitCameraItems();
			HDLOG << "Got " << items.size() << " items for detection";

			for (const auto & item : items) {
				m_detector.run(item);
			}
		} catch (const TFException & _ex) {
			LOG(ERROR) << "!!! TFException caught while running the detection: " << _ex.message();
		} catch (const std::exception & _ex) {
			LOG(ERROR) << "!!! std::exception caught while running the detection: " << _ex.what();
		}

		if (getDurationFromMoment(startTime) < SLEEP_INTERVAL_SECS) {
			HDLOG << "sleeping before next check...";
			std::this_thread::sleep_for(std::chrono::seconds(SLEEP_INTERVAL_SECS));
			HDLOG << "woke up";
		}
	}

	return 0;
}
