#ifndef Task_H
#define Task_H

#include <string>

// enumeration for reporting current phase of a task.
typedef enum Phase {
    STARTING,
    MAP,
    SHUFFLE,
    SORT,
    REDUCE,
    CLEANUP
}Phase;

// what state is the task in?
typedef enum State {
    RUNNING,
    SUCCEEDED,
    FAILED,
    UNASSIGNED,
    KILLED,
    COMMIT_PENDING,
    FAILED_UNCLEAN,
    KILLED_UNCLEAN
}State;

typedef enum Type {
    MAPTASK,
    REDUCETASK
}Type;

typedef enum ActionType {
    NO_ACTION,
    LAUNCH_TASK,
    KILL_TASK,
    START_REDUCEPHASE,
    FETCH_MAPDATA
}ActionType;

typedef struct TaskStatus {
    std::string taskTracker;
    std::string jobID;
    std::string taskAttemptID;
    Type type;
    bool isRemote;
    bool isSucceeded;
    std::string dataSource;
    long dataSize;      // bytes, only used for Map Tasks right now
    long mapDataCouter; // only used for Reduce Tasks
    double progress;
    Phase runPhase;
    State runState;
    long startTime;
    long finishTime;
    long duration;      // only used for CPU time (ms)
    long outputSize;
}TaskStatus;

typedef struct TaskAction {
    ActionType type;
    TaskStatus status;
}TaskAction;

// It is only used for Reduce Tasks to fectch data generated by Map Tasks
typedef struct MapDataAction {
    std::string reduceTaskID;
    std::string dataSource; // which node running map tasks produces the data
    long dataSize;          // bytes
}MapDataAction;

class Task {
public:
    Task() { }
    Task(std::string jobID, std::string taskID, Type type, bool isRemote, long startTime, long duration, long inputSize, long outputSize);
    void setTaskTrackerName(std::string taskTracker);
    std::string getTaskTrackerName();
    TaskStatus getTaskStatus();
    bool isSucceeded();
    void updateTaskStatus(bool isRemote, std::string trackerName, std::string dataSource);
    void updateTaskStatus(TaskStatus status);
    void setTaskStatus(double progress, long now);
private:
    TaskStatus status;
};

void dumpTaskStatus(TaskStatus &status);
void dumpTaskAction(TaskAction &action);

#endif // Task_H
