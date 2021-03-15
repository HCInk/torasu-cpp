#include "../include/torasu/std/LIcore_logger.hpp"

#include <memory>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <cmath>


#define FANCY_MODE_DISABLED 0
#define FANCY_MODE_UNIX 0

#define FANCY_MODE FANCY_MODE_UNIX

#if FANCY_MODE == FANCY_MODE_UNIX
	#include <sys/ioctl.h> //ioctl() and TIOCGWINSZ
	#include <unistd.h> // for STDOUT_FILENO
#endif

namespace {

const char* getLvLName(torasu::LogLevel lvl) {
	switch (lvl) {
	case torasu::LogLevel::TRACE:
		return "TRACE";
	case torasu::LogLevel::DEBUG:
		return "DEBUG";
	case torasu::LogLevel::INFO:
		return "INFO ";
	case torasu::LogLevel::WARN:
		return "WARN ";
	case torasu::LogLevel::ERROR:
		return "ERROR";
	case torasu::LogLevel::SERVERE_ERROR:
		return "S-ERR";
	default:
		return "UNKWN";
	}
}

static const char* ANSI_RESET = "\33[0m";
static const char* ANSI_HIGHLIGTED_RED = "\33[97m\33[101m";
static const char* ANSI_RED = "\33[91m";
static const char* ANSI_YELLOW = "\33[93m";
static const char* ANSI_BLUE = "\33[94m";
static const char* ANSI_BRIGHT_GREEN = "\33[92m";
static const char* ANSI_DARK_GREEN = "\33[32m";
static const char* ANSI_GRAY = "\33[90m";
static const char* ANSI_CYAN = "\33[36m";
static const char* ANSI_MAGENTA = "\33[35m";

const char* getLvlAnsi(torasu::LogLevel lvl) {
	switch (lvl) {
	case torasu::LogLevel::TRACE:
		return ANSI_GRAY;
	case torasu::LogLevel::DEBUG:
		return ANSI_DARK_GREEN;
	case torasu::LogLevel::INFO:
		return ANSI_BLUE;
	case torasu::LogLevel::WARN:
		return ANSI_YELLOW;
	case torasu::LogLevel::ERROR:
		return ANSI_RED;
	case torasu::LogLevel::SERVERE_ERROR:
		return ANSI_HIGHLIGTED_RED;
	default:
		return "";
	}
}

static std::string groupStackToStr(const std::vector<torasu::LogId>& groupStack, bool tag=false) {
	std::string str;
	size_t pathSize = groupStack.size();
	const torasu::LogId* gsit = groupStack.data() + pathSize - 1;
	if (tag) pathSize--;
	for (size_t i = pathSize; i > 0; i--) {
		str += "/" + std::to_string(*gsit);
		gsit--;
	}

	if (tag) str += " [" + std::to_string(*gsit) + "]";

	return str;
}

inline std::string padStr(char c, size_t count) {
	std::string str;
	for (int i = count; i > 0; i--) str += c;
	return str;
}

#define REF_VIS_REP_GROUP true

static std::vector<std::string> visualizeRef(const std::vector<torasu::LogId>& currentGroupStack, torasu::tstd::LIcore_logger_logstore::StoreGroup* storeGroup, const torasu::LogInfoRef* ref) {
	try {
		std::vector<std::string> lines;

		std::string pref = groupStackToStr(currentGroupStack);

		torasu::tstd::LIcore_logger_logstore::StoreGroup* refGroup;
		if (!ref->groupRef->empty()) {
			std::unique_ptr<std::stack<torasu::tstd::LIcore_logger_logstore::StoreGroup*>> resolveStack(storeGroup->resolve(*ref->groupRef));
			refGroup = resolveStack->top();
			if (refGroup != nullptr) {
				pref += groupStackToStr(*ref->groupRef);
			} else {
				pref += "<GRES-ERR@" + std::to_string(resolveStack->size()) + ": " + groupStackToStr(*ref->groupRef) + "> ";
			}
		} else {
			refGroup = storeGroup;
		}

		std::string currLine = pref;

		if (!ref->causeRefs->empty()) {
			auto causeIt = ref->causeRefs->begin();
			for (;;) {
				const auto& cause = *causeIt;
				bool isTag;
				std::unique_ptr<std::stack<torasu::tstd::LIcore_logger_logstore::StoreGroup*>> resolveStack(refGroup->resolve(cause, &isTag));
				torasu::tstd::LIcore_logger_logstore::StoreGroup* causeGroup = resolveStack->empty() ? refGroup : resolveStack->top();
				if (causeGroup != nullptr) {
					currLine += groupStackToStr(cause, isTag);
					if (isTag) {
						auto foundEntry = causeGroup->tagged.find(cause[0]);
						torasu::LogMessage* msgEntry = foundEntry != causeGroup->tagged.end() ? dynamic_cast<torasu::LogMessage*>(foundEntry->second) : nullptr;
						if (msgEntry != nullptr) {
							currLine += " " + causeGroup->name + ": " + msgEntry->text;
						} else {
							currLine += " (!TAG" + std::to_string(cause[0]) + " in " + causeGroup->name + " not found!)";
						}
					} else {
						currLine += " (" + causeGroup->name + ")";
					}

				} else {
					currLine += "<CRES-ERR@" + std::to_string(resolveStack->size()) + ": " + groupStackToStr(cause) + ">";
				}

				lines.push_back(currLine);
				causeIt++;
				if (causeIt == ref->causeRefs->end()) break;
#if REF_VIS_REP_GROUP
				currLine = pref;
#else
				currLine = padStr(' ', pref.size());
#endif
			}
		} else {
			lines.push_back(currLine);
		}



		return lines;
	} catch (const std::exception& ex) {
		return {"<ERR! " + std::string(ex.what()) + ">"};
	}
}

std::string dispPercentage(double val, size_t decPlaces) {
	std::stringstream percDisp;
	if (!std::isnan(val)) {
		size_t decPlacesPrec = std::pow(10, decPlaces);
		percDisp << std::setw(2) << std::setfill('0') << std::floor(val*100);
		if (decPlaces > 0) percDisp << "." << std::setw(decPlaces) << std::setfill('0') << std::floor( static_cast<int32_t>(val*100*decPlacesPrec) % decPlacesPrec );
	} else {
		percDisp << "xx";
		if (decPlaces > 0) percDisp << "." << padStr('x', decPlaces);
	}
	percDisp << "%";
	return percDisp.str();
}

std::string makeProgressBar(std::string info, double done, double doing, size_t width) {
	std::string out;
	out += dispPercentage(done, 2);
	if (width > 20) {
		out += " ";
		int32_t areaLeft = width - out.length();
		size_t textSize;
		int32_t barSize = 0;
		if (!info.empty()) {
			if (areaLeft > 30) {
				textSize = areaLeft/2;
				if (textSize > 30) textSize = 30;
				barSize = areaLeft - textSize;
			} else {
				textSize = areaLeft;
			}
		} else {
			textSize = 0;
			barSize = areaLeft;
		}

		if (info.length() > textSize) {
			out += info.substr(0, textSize-3) + "...";
		} else {
			out += info + padStr(' ', textSize-info.length());
		}

		if (barSize > 5) {
			barSize -= 4;

			out += " [";
			if (!std::isnan(done)) {

				int32_t doneSize = !std::isnan(done) ? std::floor(done * barSize) : 0;
				int32_t doingSize = !std::isnan(doing) ? std::ceil(doing * barSize) : 0;

				int32_t restSize = barSize - doneSize - doingSize;

				out += padStr('#', doneSize);
				out += padStr('=', doingSize);
				out += padStr(' ', restSize);

			} else {
				out += padStr('-', barSize);
			}

			out += "] ";
		}
	}
	return out;
}

} // namespace


namespace torasu::tstd {

LIcore_logger::LIcore_logger(LogDisplayMode dispMode) : dispMode(dispMode) {}

LIcore_logger::~LIcore_logger() {
	if (currentStatus != nullptr) {
		std::cout << std::endl;
		delete currentStatus;
	}
}

void LIcore_logger::log(LogEntry* entry) {
	std::unique_lock lck(logMutex);
	std::unique_ptr<LogEntry> entryHolder(entry);

	bool useAnsi = dispMode >= BASIC_CLOLORED;
	std::string message;

	switch (entry->type) {
	case LT_GROUP_START: {
			auto* startEntry = static_cast<LogGroupStart*>(entry);
			if (useAnsi) message += ANSI_GRAY;
			message += "====  " + groupStackToStr(entry->groupStack) + " * " + startEntry->name;
			logstore.create(entry->groupStack, startEntry->name);
		}
		break;

	case LT_GROUP_END: {
			std::unique_ptr<std::stack<LIcore_logger_logstore::StoreGroup*>> resolveStack(logstore.resolve(entry->groupStack));
			auto found = resolveStack->top();
			if (found == nullptr) throw std::runtime_error("Failed to end log-group: Group not found!");
			found->progress.finished = true;
		}
		break;

	case LT_GROUP_UNREF: {
			// message += "(FREE)" + groupStackToStr(entry->groupStack) + " * " + entry->text;
			std::unique_ptr<std::stack<LIcore_logger_logstore::StoreGroup*>> resolveStack(logstore.resolve(entry->groupStack));
			auto found = resolveStack->top();
			if (found == nullptr) throw std::runtime_error("Failed to unregister log-group: Group not found!");
			logstore.remove(found);
		}
		break;

	default: {
			// Prefix

			std::string prefix;
			if (entry->type == LT_MESSAGE) {
				auto* msgEntry = static_cast<LogMessage*>(entry);
				if (useAnsi) message += getLvlAnsi(msgEntry->level);
				prefix += getLvLName(msgEntry->level);

			} else if (entry->type == LT_BENCHMARK) {
				if (useAnsi) message += ANSI_MAGENTA;
				prefix += "BENCH";
			} else if (entry->type == LT_PROGRESS) {
				if (useAnsi) message += ANSI_BRIGHT_GREEN;
				prefix += "PROG ";
			} else {
				if (useAnsi) message += ANSI_CYAN;
				prefix += "DATA ";
			}

			prefix += " ";

			// Group Display
			LIcore_logger_logstore::StoreGroup* foundGroup = nullptr;
			bool isTag = false;
			size_t prefSize;
			if (!entry->groupStack.empty()) {

				std::unique_ptr<std::stack<LIcore_logger_logstore::StoreGroup*>> resolveStack(logstore.resolve(entry->groupStack, &isTag));

				prefix += groupStackToStr(entry->groupStack, isTag);
				prefSize = prefix.size();
				prefix += " \t";

				foundGroup = resolveStack->top();
				if (foundGroup != nullptr) {
					if (isTag) foundGroup->tagged[entry->groupStack[0]] = entryHolder.get();
					foundGroup->logs.push_back(entryHolder.release());
					prefix += foundGroup->name;
				} else {
					prefix += "(UNKNOWN)";
				}
				prefix += ": ";
			} else {
				foundGroup = logstore.getRoot();
				foundGroup->logs.push_back(entryHolder.release());
				prefSize = prefix.size();
			}

			message += prefix;

			// Message

			if (entry->type == LT_MESSAGE) {
				LogMessage* msgEntry = static_cast<LogMessage*>(entry);
				message += msgEntry->text;

				if (msgEntry->info != nullptr) {
					auto refInfo = visualizeRef(isTag ? std::vector<LogId>(entry->groupStack.data() + 1, entry->groupStack.data()+entry->groupStack.size()) : entry->groupStack, foundGroup, msgEntry->info);
					std::string label = "Cause";
					bool firstLine = true;
					for (auto line : refInfo) {
						message += "\n" + padStr(' ', prefSize) + " \t " + (firstLine ? label : padStr(' ', label.size())) + " -> " + line;
						firstLine = false;
					}
				}
			} else if (entry->type == LT_BENCHMARK) {
				LogBenchmark* benchEntry = static_cast<LogBenchmark*>(entry);
				switch (benchEntry->benchType) {
				case LogBenchmark::BenchType_TAGGED: {
						auto* tag = benchEntry->benchInfo.opTag;
						message += (tag != nullptr ? *tag : "<unnamed>") + ": ";
						break;
					}
				case LogBenchmark::BenchType_GROUP:
					break;
				default: {
						message += "<unknown-bench-type>: ";
						break;
					}
				}
				const auto divisor = 1000;
				const char* unitLabel = "ms";

				double calcTime = benchEntry->calcTime != LogBenchmark::bench_t_MAX ? static_cast<double>(benchEntry->calcTime) / divisor : NAN;
				double elapsed = benchEntry->elapsed != LogBenchmark::bench_t_MAX ? static_cast<double>(benchEntry->elapsed) / divisor : NAN;
				// double position = benchEntry->position != LogBenchmark::bench_t_MAX ? static_cast<double>(benchEntry->position) / divisor : NAN;

				std::stringstream timeDisplayStr;

				// timeDisplayStr << std::setw(8);

				timeDisplayStr << std::floor(calcTime*divisor)/divisor << unitLabel;

				if (elapsed != NAN)
					timeDisplayStr << " ELAPSED: " << std::floor(elapsed*divisor)/divisor << unitLabel;

				// timeDisplayStr.precision(0);
				// if (position != NAN)
				// 	timeDisplayStr << " POS: " << std::floor(position) << unitLabel;


				if (benchEntry->benchType == LogBenchmark::BenchType_GROUP) {
					if (benchEntry->benchInfo.toContinue) {
						timeDisplayStr << " (to continue)";
						if (foundGroup != nullptr) foundGroup->groupBenchmarks.push_back(*benchEntry);
					} else {
						if (foundGroup != nullptr) {
							if (!foundGroup->groupBenchmarks.empty()) {
								foundGroup->groupBenchmarks.push_back(*benchEntry);

								double totalElapsed = NAN;
								if (benchEntry->elapsed != LogBenchmark::bench_t_MAX &&
										benchEntry->position != LogBenchmark::bench_t_MAX) {
									auto firstBench = foundGroup->groupBenchmarks.begin();
									if (firstBench->position != torasu::LogBenchmark::bench_t_MAX) {
										size_t totalElapsedRaw = benchEntry->elapsed+(benchEntry->position-firstBench->position);
										totalElapsed = static_cast<double>(totalElapsedRaw) / divisor;
									}
								}

								double totalCalcTime;
								{
									size_t totalCalcTimeRaw = 0;
									for (auto entry : foundGroup->groupBenchmarks) {
										if (entry.calcTime != LogBenchmark::bench_t_MAX) {
											totalCalcTimeRaw += entry.calcTime;
										}
									}
									totalCalcTime = static_cast<double>(totalCalcTimeRaw) / divisor;
								}

								timeDisplayStr << "\n" + padStr(' ', prefSize) << " \t -> TOTAL: "
											   << std::floor(totalCalcTime*divisor)/divisor << unitLabel;

								if (totalElapsed != NAN)
									timeDisplayStr << " ELAPSED: " << std::floor(totalElapsed*divisor)/divisor << unitLabel;

							} else {
								foundGroup->groupBenchmarks.push_back(*benchEntry);
							}
						}
					}

					message += timeDisplayStr.str();

					if (foundGroup != nullptr) foundGroup->groupBenchmarks.push_back(*benchEntry);
				}

			} else if (entry->type == LT_PROGRESS) {
				LogProgress* progressEntry = static_cast<LogProgress*>(entry);

				std::string progLabel;

				if (foundGroup != nullptr) {
					auto& progress = foundGroup->progress;
					progress.total = progressEntry->total;
					progress.doing = progressEntry->doing;
					progress.done = progressEntry->done;
					if (!progress.hasInfo) progress.hasInfo = true;

					int32_t currPos = progress.done + progress.doing;
					if ( progress.labelPos <= currPos
							&& (!progressEntry->label.empty() || progress.labelPos != currPos) ) {
						progress.label = progressEntry->label;
						progress.labelPos = currPos;
					}

					progLabel = progress.label;
				} else {
					progLabel = progressEntry->label;
				}

				double progressVal = progressEntry->total > 0 ? static_cast<double>(progressEntry->done) / progressEntry->total : NAN;

				if (dispMode <= FANCY_ALL) {

					message += dispPercentage(progressVal, 2);

					if (progressEntry->total > 0) {
						message += " (" + std::to_string(progressEntry->done);
						if (progressEntry->doing > 1) {
							message += "[+" + std::to_string(progressEntry->doing) + "]";
						}
						message += "/" + std::to_string(progressEntry->total) + ")";
					}

					if (!progressEntry->label.empty()) {
						message += " - " + progressEntry->label;
					}

				} else {
					message = "";
				}

				if (dispMode >= FANCY_ALL) {
					double doingVal = progressEntry->total > 0 ? static_cast<double>(progressEntry->doing) / progressEntry->total : NAN;
					auto termWidth = getTerminalWidth();
					std::string statusText = makeProgressBar(progLabel, progressVal, doingVal, termWidth >= 0 ? termWidth : 10);
					auto* statusForm = new std::string(ANSI_BRIGHT_GREEN + statusText + ANSI_RESET);
					setStatus(statusForm, statusText.length());
				}


			} else {
				message += "[DataPacket-" + std::to_string(entry->type) + "]";
			}


		}
		break;
	}

	if (!message.empty()) {
		// Suffix

		if (useAnsi) {
			message += ANSI_RESET;
		}

		// Print

		println(message);
	}
}

torasu::LogId LIcore_logger::fetchSubId() {

	std::unique_lock lock(subIdCounterLock);
	auto subId = subIdCounter;
	subIdCounter++;
	return subId;

}

std::vector<LogId>* LIcore_logger::pathFromParent	(LogInterface* parent) const {
	if (parent == this) return new std::vector<LogId>(); // Found: parent is this
	else return nullptr; // Not found
}

static const char* RESET = "\r";
static const char* VT100_CLEAR = "\33[2K";
static const char* NEWLINE = "\n";

void LIcore_logger::println(const std::string& str) {
	if (currentStatus != nullptr) {
		std::cout << RESET << padStr(' ', statusDispLength) << VT100_CLEAR << RESET // Clear current status-area
				  << str << NEWLINE // Print line
				  << *currentStatus << std::flush; // Re-write status
	} else {
		std::cout << str << NEWLINE;
	}
}


void LIcore_logger::setStatus(const std::string* newStatus, size_t newStatusDispLength) {
	if (currentStatus != nullptr) {
		int32_t dispWidthToClear = statusDispLength;
		int32_t currDispWidth = getTerminalWidth();
		if (currDispWidth > 0 && dispWidthToClear > currDispWidth) dispWidthToClear = currDispWidth;

		std::cout << RESET << padStr(' ', dispWidthToClear) << VT100_CLEAR << RESET; // Clear current status-area
		delete currentStatus;
	}
	if (newStatus != nullptr) {
		std::cout << *newStatus << std::flush; // Write new status
	}
	currentStatus = newStatus;
	statusDispLength = newStatusDispLength;
}

int32_t LIcore_logger::getTerminalWidth() {
#if FANCY_MODE == FANCY_MODE_UNIX
	struct winsize size;
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &size);
	return size.ws_col;
#else
	return -1;
#endif
}

//
// Logstore
//

torasu::tstd::LIcore_logger_logstore::LIcore_logger_logstore() {}
torasu::tstd::LIcore_logger_logstore::~LIcore_logger_logstore() {}

torasu::tstd::LIcore_logger_logstore::StoreGroup::StoreGroup() {}

torasu::tstd::LIcore_logger_logstore::StoreGroup::StoreGroup(std::string name, torasu::LogId logId, StoreGroup* parent)
	: owner(parent), name(name) {

	parent->sub[logId] = this;
	owner->cleanupList.insert(this);

}

torasu::tstd::LIcore_logger_logstore::StoreGroup::~StoreGroup() {
	// std::cout << " RM " << this << std::endl;
	for (StoreGroup* toClean : cleanupList) delete toClean;
	for (LogEntry* entry : logs) delete entry;
}

std::stack<torasu::tstd::LIcore_logger_logstore::StoreGroup*>* torasu::tstd::LIcore_logger_logstore::StoreGroup::resolve(const std::vector<LogId>& path, bool* tag, size_t pathDepth) {
	if (tag != nullptr) *tag = false;
	auto* resolveStack = new std::stack<torasu::tstd::LIcore_logger_logstore::StoreGroup*>();
	if (pathDepth == 0) pathDepth = path.size();

	StoreGroup* cGroup = this;
	auto it = path.rbegin();
	for (size_t i = pathDepth; i > 0; i--) {
		LogId logId = *it;
		it++;
		auto& sub = cGroup->sub;
		auto found = sub.find(logId);

		if (found == sub.end()) {
			if (tag != nullptr && i == 1) { // Lowest level to be evalulated and is not found may be a tag
				*tag = true;
				break;
			}
			for (auto group : sub) {
				std::cout << " NOT " << logId << " BUT " << group.first << " @ " << std::to_string(pathDepth-i) << std::endl;
			}
			resolveStack->push(nullptr);
			break;
		}

		cGroup = found->second;
		resolveStack->push(cGroup);
	}

	return resolveStack;
}

LIcore_logger_logstore::StoreGroup* torasu::tstd::LIcore_logger_logstore::create(const std::vector<LogId>& path, std::string name) {
	StoreGroup* foundGroup;
	if (path.size() > 1) {
		std::unique_ptr<std::stack<StoreGroup*>> resolveStack(resolve(path, nullptr, path.size()-1));

		foundGroup = resolveStack->top();

		if (foundGroup == nullptr) {
			throw std::runtime_error("Failed to create new log-group, since parent can't be resolved! "
									 "(Creation of " + groupStackToStr(path) + " as " + name + " - parent #" + std::to_string(resolveStack->size()) + ")");
		}
	} else {
		foundGroup = &tree;
	}

	// delete automatically called
	return new StoreGroup(name, path[0], foundGroup);
}


void torasu::tstd::LIcore_logger_logstore::remove(StoreGroup* group) {
	group->owner->cleanupList.erase(group);
	delete group;
}

void torasu::tstd::LIcore_logger_logstore::StoreGroup::reown(StoreGroup* newOwner) {

	owner->cleanupList.erase(this);
	owner = newOwner;
	owner->cleanupList.insert(this);

}

} // namespace torasu::tstd
