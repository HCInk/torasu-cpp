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

			TaskDistInstance* foundTargetInstance = NULL;
			double foundPci;
			bool hasOwnExpierence;
			instance->findBestExport(next.type, &foundTargetInstance, &foundPci, &hasOwnExpierence);
			if (hasOwnExpierence && foundTargetInstance != NULL) {
				//instance->log(string("OTHER: ") + next.type + " => " + foundTargetInstance->name);
				foundTargetInstance->addToQueue(next);
				continue;
			}

			//instance->log(string("RUN SELF: ") + next.type);

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
	string statMsg = "[" + name + "] STATS::";
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

	statMsg += "	AVG: " + to_string(tsiTotal/tsiCount);

	cout << statMsg << endl;
	
}

void TaskDistInstance::readjustFQI(bool adding) {
	queueLock.lock();

	int currentQueueSize = queue.size();

	double directionFactor = static_cast<double>(currentQueueSize+1)/(targetQueueSize+1);

	if (directionFactor > 1) {
		if (adding) {
			fqi = fqi+fqi*adjustmentSpeed*directionFactor;
		}
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


double TaskDistInstance::getATSI(TaskDistInstance* instance, string type, double* outTaskTsi) {

	int entriesMeasured = 0;
	double avgTsiThere = 0;
	double avgTsiHere = 0;
	double taskTsi = -1;

	// Inner join average / find tsi for current type on instance
	for (auto experiencedObject : instance->tsiMap) {

		if (experiencedObject.first == type) {
			taskTsi = experiencedObject.second;
		}

		auto foundHere = tsiMap.find(experiencedObject.first);
		if (foundHere != tsiMap.end()) {
			avgTsiThere += experiencedObject.second;
			avgTsiHere += foundHere->second;
			entriesMeasured++;
		}

	}
	
	if (taskTsi < 0) {
		*outTaskTsi = -1;
		return -1;
	}

	*outTaskTsi = taskTsi;

	if (entriesMeasured > 0) {
		avgTsiThere /= entriesMeasured;
		avgTsiHere /= entriesMeasured;
	} else {
		return -1;
	}


	double pai = avgTsiHere/avgTsiThere;

	return taskTsi*pai;

}

bool TaskDistInstance::findBestExport(string type, TaskDistInstance** outTargetInstance, double* outPci, bool* outHasOwnExpierence) {
	double taskTsiHere;

	auto foundExperienceHere = tsiMap.find(type);

	if (foundExperienceHere != tsiMap.end()) {
		*outHasOwnExpierence = true;
		taskTsiHere = foundExperienceHere->second;
	} else {
		*outHasOwnExpierence = false;
		taskTsiHere = 1;
	}

	double fqiOnNew = INFINITY;
	double bestPci = -1;

	double taskTsiThere;
	for (TaskDistInstance* connection : connections) {
		if (connection->doRun) {
			if (connection == NULL) {
				cout << "CON NULL" << endl;
			}
			//cout << "CON PTR " << connection << endl;;

			double atsiThere = getATSI(connection, type, &taskTsiThere);

			if (atsiThere > 0) {
				
				double pci = taskTsiHere/atsiThere;

				if (bestPci < pci) {
					if (connection->fqi < pci) {
						bestPci = pci;
						*outTargetInstance = connection;
					}
				}

			} else {
				if (bestPci <= 0 && fqiOnNew > connection->fqi) {
					if (connection->fqi < 1) {
						fqiOnNew = connection->fqi;
						*outTargetInstance = connection;
					}
				}
			}

		}
	}

	*outPci = bestPci;

	return true;
}

void taskDistTest() {
	cout << "HELLOWO!" << endl;
	ConcurrentLogger logger;

	int instanceCount = 6;
	TaskDistInstance instances[] = {
									TaskDistInstance("I0", &logger, 10), 
									TaskDistInstance("I1", &logger, 10), 
									TaskDistInstance("I2", &logger, 10), 
									TaskDistInstance("I3", &logger, 5),
									TaskDistInstance("I4", &logger, 20), 
									TaskDistInstance("I5", &logger, 5)
									};

	instances[0].addConnection(instances+1);
	instances[0].addConnection(instances+2);
	instances[0].addConnection(instances+3);
	instances[0].addConnection(instances+4);
	instances[0].addConnection(instances+5);
	
	auto t1t = map<string, uint64_t>();
	t1t["I1"] = 200*1000;
	t1t["I2"] = 300*1000;

	auto t3t = map<string, uint64_t>();
	t3t["I2"] = 90*1000;
	t3t["I3"] = 2500*1000;

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
	

	for (int i = 0; i < 5000; ) {
		
		if (instances[0].fqi <= 1) {
			
			for (int ti = 0; ti < taskCount; ti++) {
				switch (ti) {
				
				default:
					instances[0].addToQueue( tasks[ti] );
					break;

				// case 0:
				// case 1:
				// 	if (rand()%10 > 2) {
				// 		instances[0].addToQueue( tasks[ti] );
				// 	}
				// 	break;
				// case 2:
				// case 3:
				// 	if (rand()%10 > 8) {
				// 		instances[0].addToQueue( tasks[ti] );
				// 	}
				// 	break;
				// default:
				// 	break;

				}
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