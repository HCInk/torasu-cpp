#include "task-distribution-test.hpp"

#include <chrono>
#include <iostream>
#include <unistd.h>
#include <math.h>

using namespace std;

namespace torasu::texample {

ConcurrentLogger::ConcurrentLogger() {
	log("LOG", "READY!");
}

ConcurrentLogger::~ConcurrentLogger() {
	log("LOG", "SHUTDOWN.");
}

void ConcurrentLogger::log(string tag, string message) {
	
	writeLock.lock();

	cout << "[" << tag << "] " << message << endl;

	writeLock.unlock();

}

void TaskDistTask::run(TaskDistInstance* instance) {
	uint64_t sleepTime;
	auto found = timeMap.find(instance->name);
	if (found != timeMap.end()) {
		sleepTime = found->second;
	} else {
		sleepTime = defaultTime;
	}
	sleepTime /= instance->speed;
	usleep(sleepTime);
}


string TaskDistTask::getType() {
	return type;
}

int TaskDistInstance::run(TaskDistInstance* instance) {
	cout << "TEST" << endl;

	while (instance->doRun) {
		instance->queueLock.lock();
		if (!instance->queue.empty()) {
			TaskDistTask next = instance->queue.front();
			instance->queue.pop();
			instance->queueLock.unlock();

			// instance->log(string("RUN SELF: ") + next.type);

			auto start = chrono::high_resolution_clock::now(); 

			next.run(instance);
			
			auto end = chrono::high_resolution_clock::now();

			auto duration =	chrono::duration_cast<chrono::milliseconds>(end - start); 

			instance->tsiMap[next.type] = duration.count();

			// instance->log( next.type + " TIME: " + std::to_string(duration.count()) + "ms" );


		} else {
			instance->queueLock.unlock();
			// instance->log("NO TASK, sleeping");
			usleep(100L*1000L);
		}
		instance->readjustFQI();

	}
	instance->runLock.unlock();

	return 0;
}

void TaskDistInstance::launch() {
	runLock.lock();
	doRun = true;
	runner = new thread(run, this);
	runner->detach();
}

void TaskDistInstance::stop() {
	doRun = false;
	runLock.lock();
	runLock.unlock();
}

void TaskDistInstance::addToQueue(TaskDistTask task) {
	// log(string("Add to queue: ") + task.getType());
	queueLock.lock();

	queue.push(task);

	queueLock.unlock();

	readjustFQI(true);
}

bool TaskDistInstance::isEmpty() {
	queueLock.lock();

	bool empty = queue.empty();

	queueLock.unlock();
	
	return empty;
}

void TaskDistInstance::printStats(std::vector<std::string> query) {
	string statMsg = "STATS::";
	statMsg += "	QUEUE: " + to_string(queue.size());
	statMsg += "	FQI: " + to_string(fqi);

	statMsg += "		TIMES: ";
	int tsiCount = 0;
	double tsiTotal = 0;
	for (string task : query) {
		statMsg += "	" + task + ": ";
		auto found = tsiMap.find(task);
		if (found != tsiMap.end()) {
			double tsi = found->second;
			statMsg += to_string(tsi);
			tsiCount++;
			tsiTotal += tsi;
		} else {
			statMsg += "--";
		}
	}

	statMsg += "AVG: " + to_string(tsiTotal/tsiCount);

	log(statMsg);
	
}

void TaskDistInstance::readjustFQI(bool adding) {
	queueLock.lock();

	int currentQueueSize = queue.size();

	double directionFactor = static_cast<double>(currentQueueSize+1)/(targetQueueSize+1);

	if (directionFactor > 1) {
		fqi = fqi+fqi*adjustmentSpeed*directionFactor;
	} else {
		if (!adding) {
			fqi = fqi-fqi*adjustmentSpeed*(1/directionFactor);
		}
	}

	if (fqi < 0.0001) {
		fqi = 0.0001;
	}

	// fqi = (pow((static_cast<double>(currentQueueSize+1)/(targetQueueSize+1)), 4) * fqi*adjustmentSpeed) + (fqi*(1-adjustmentSpeed));
	
	queueLock.unlock();
}


inline void getProcessingExperience(TaskDistInstance* instance, string type, double& avgTsi, double& taskTsi) {

		int entriesThere = 0;
		avgTsi = 0;
		taskTsi = -1;
		for (auto experiencedObject : instance->tsiMap) {
			if (experiencedObject.first == type) {
				taskTsi = experiencedObject.second;
			}
			avgTsi += experiencedObject.second;
			entriesThere++;
		}
		
		if (entriesThere > 0) {
			avgTsi /= entriesThere;
		} else {
			avgTsi = -1;
		}

}

bool TaskDistInstance::findBestExport(string type, TaskDistInstance*& outTargetInstance, double& outPci, bool& outHasOwnExpierence) {
	double taskTsiHere;
	double avgTsiHere;
	getProcessingExperience(this, type, avgTsiHere, taskTsiHere);
	
	if (taskTsiHere > 0) {
		outHasOwnExpierence = true;
	} else {
		outHasOwnExpierence = false;
		taskTsiHere = 1;
		if (avgTsiHere < 0) {
			avgTsiHere = 1;
		}
	}

	double fqiOnNew = INFINITY;
	double bestPci = 0;

	double avgTsiThere;
	double taskTsiThere;
	for (TaskDistInstance* connection : connections) {
		if (connection->doRun) {

			getProcessingExperience(connection, type, avgTsiThere, taskTsiThere);

			if (taskTsiThere > 0) {
				
				double pai = avgTsiHere/avgTsiThere;
				double pci = taskTsiThere*pai;

				if (bestPci < pci) {
					bestPci = pci;
					outTargetInstance = connection;
				}

			} else {
				if (bestPci <= 0 && fqiOnNew > connection->fqi) {
					fqiOnNew = connection->fqi;
					outTargetInstance = connection;
				}
			}

		}
	}

	

	return true;
}

void taskDistTest() {
	cout << "HELLOWO!" << endl;
	ConcurrentLogger logger;

	int instanceCount = 4;
	TaskDistInstance instances[] = {
									TaskDistInstance("I0", &logger), 
									TaskDistInstance("I1", &logger), 
									TaskDistInstance("I2", &logger), 
									TaskDistInstance("I3", &logger, 0.5)
									};
	
	auto t1t = map<string, uint64_t>();
	t1t["I1"] = 200*1000;
	t1t["I2"] = 300*1000;

	auto t3t = map<string, uint64_t>();
	t1t["I2"] = 90*1000;
	t1t["I3"] = 250*1000;

	int taskCount = 4;
	TaskDistTask tasks[] = {
							TaskDistTask("T0", 100*1000, map<string, uint64_t>()),
							TaskDistTask("T1", 400*1000, t1t),
							TaskDistTask("T2", 300*1000, map<string, uint64_t>()),
							TaskDistTask("T3", 200*1000, t3t)
							};

	for (int ii = 0; ii < instanceCount; ii++) {
		instances[ii].launch();
	}
	

	for (int i = 0; i < 100; ) {
		
		if (instances[0].fqi <= 1) {
			
			for (int ti = 0; ti < taskCount; ti++) {
				// for (int ii = 0; ii < instanceCount; ii++) {
				// 	instances[ii].addToQueue( tasks[ti] );
				// }
				instances[0].addToQueue( tasks[ti] );
			}
			i++;
		} else {
			usleep(100*1000);
		}
		
		for (int ii = 0; ii < instanceCount; ii++) {
			instances[ii].printStats({"T0", "T1", "T2", "T3"});
		}
		cout << "===== STATS =====" << endl;
	}
	
	bool hasRunningThread = false;
	int ii = 0;
	while ( true ) {
		
		if (instances[ii].doRun) {

			if (!instances[ii].isEmpty()) {
				hasRunningThread = true;
				// cout << "WAITING FOR INSTANCE " << instances[ii].name << endl;
			}
			instances[ii].printStats({"T0", "T1", "T2", "T3"});
		} 

		ii++;
		if (ii >= instanceCount) {
			if (hasRunningThread) {
				cout << "===== STATS =====" << endl;
				ii = 0;
				hasRunningThread = false;
				usleep(100*1000);
			} else {
				break;
			}
		}
	}
	

	

}

} // namespace torasu::texample