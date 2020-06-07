#ifndef EXAMPLES_SRC_TASK_DISTROBUTION_TEST_HPP_
#define EXAMPLES_SRC_TASK_DISTROBUTION_TEST_HPP_

#include <string>
#include <queue>
#include <queue>
#include <mutex>
#include <map>
#include <thread>
#include <vector>

// #include <iostream>

namespace torasu::texample {

class ConcurrentLogger {
private:
	std::mutex writeLock;
public:
	ConcurrentLogger();
	~ConcurrentLogger();

	void log(std::string tag, std::string message);
};

class TaskDistInstance;

class TaskDistTask {
public:
	std::string type;
	uint64_t defaultTime;
	std::map<std::string, uint64_t> timeMap;

	TaskDistTask(std::string type, uint64_t defaultTime, std::map<std::string, uint64_t> timeMap) {
		this->type = type;
		this->defaultTime = defaultTime;
		this->timeMap = timeMap;
	}
	~TaskDistTask() {}

	void run(TaskDistInstance* instance);
	std::string getType();
};


class TaskDistInstance {
public:
	std::string name;
	double speed;
	ConcurrentLogger* logger;
	std::thread* runner = NULL;
	std::mutex runLock;
	
	std::queue<TaskDistTask> queue;
	std::mutex queueLock;

	std::vector<TaskDistInstance*> connections;

	// Map of TSIs per task-type
	std::map<std::string, double> tsiMap;

	volatile double fqi = 1;

	int targetQueueSize = 100;
	double adjustmentSpeed = 0.03;

	volatile bool doRun;

	TaskDistInstance(std::string name, ConcurrentLogger* logger, double speed = 1) {
		this->name = name;
		this->logger = logger;
		this->speed = speed;
		// std::cout << "MAKE INSTANCE " << this << std::endl;
	}

	~TaskDistInstance() {
		// std::cout << "FREE INSTANCE " << this << std::endl;
		if (runner != NULL) {
			stop();
			delete runner;

		}
	}
	static int run(TaskDistInstance* instance);
	void launch();
	void stop();
	void addToQueue(TaskDistTask task);
 	bool isEmpty();
	void printStats(std::vector<std::string> query);
	
	inline void addConnection(TaskDistInstance* con) {
		connections.push_back(con);
	}

	void readjustFQI(bool adding = false);

	bool findBestExport(std::string type, TaskDistInstance*& outTargetInstance, double& outPci, bool& outHasOwnExpierence);

	inline void log(std::string msg) {
		logger->log(std::string("INSTANCE:") + name, msg);
	}

};

void taskDistTest();
	
} // namespace torasu::texample

#endif // EXAMPLES_SRC_TASK_DISTROBUTION_TEST_HPP_
