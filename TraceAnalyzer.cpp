/*
Lei Ye <leiy@cs.arizona.edu>
HadoopSim is a simulator for a Hadoop Runtime by replaying the collected traces.
*/
#include <iostream>
#include <fstream>
#include <algorithm>
#include <map>
#include <vector>
#include "TraceReader.h"
#include "TraceAnalyzer.h"
#include "JobTracker.h"
using namespace std;

void getTimeStatistics(map<string, long> &timeSet, long &minTime, long &maxTime, long &medianTime)
{
    vector<long> exeTime;
    map<string, long>::iterator timeIt;

    for(timeIt = timeSet.begin(); timeIt != timeSet.end(); timeIt++) {
        exeTime.push_back(timeIt->second);
    }
    sort(exeTime.begin(), exeTime.end());

    size_t size = exeTime.size();
    minTime = exeTime[0];
    maxTime = exeTime[size - 1];
    medianTime = (size % 2) ? exeTime[size>>1] : (exeTime[(size>>1) - 1] + exeTime[size>>1])/2 ;
}

void analyzeJobTaskExeTime(map<string, Job> &completedJobs)
{
    map<string, long> jobTime;
    map<string, long> mapTaskTime;
    map<string, long> reduceTaskTime;
    map<string, long>::iterator timeIt;
    map<string, Job>::iterator jobIt;
    map<string, Task>::iterator taskIt;
    long minTime, maxTime, medianTime;
    ofstream csvFile;

    for(jobIt = completedJobs.begin(); jobIt != completedJobs.end(); jobIt++) {
        Job job = jobIt->second;
        jobTime.insert(pair<string, long>(job.getJobID(), job.getEndTime() - job.getStarTime()));

        csvFile.open((job.getJobID() + ".csv").c_str());
        mapTaskTime.clear();
        map<string, Task> mapTasks = job.getCompletedMaps();
        for(taskIt = mapTasks.begin(); taskIt != mapTasks.end(); taskIt++) {
            TaskStatus taskStatus = taskIt->second.getTaskStatus();
            mapTaskTime.insert(pair<string, long>(taskStatus.taskAttemptID, taskStatus.finishTime - taskStatus.startTime));
        }
        getTimeStatistics(mapTaskTime, minTime, maxTime, medianTime);
        csvFile<<"MapTaskTime,,minTime,"<<minTime<<",,maxTime,"<<maxTime<<",,medianTime,"<<medianTime<<endl;
        for(timeIt = mapTaskTime.begin(); timeIt != mapTaskTime.end(); timeIt++) {
            csvFile<<timeIt->first<<","<<timeIt->second<<endl;
        }
        csvFile<<endl;

        reduceTaskTime.clear();
        map<string, Task> reduceTasks = job.getCompletedReduces();
        for(taskIt = reduceTasks.begin(); taskIt != reduceTasks.end(); taskIt++) {
            TaskStatus taskStatus = taskIt->second.getTaskStatus();
            reduceTaskTime.insert(pair<string, long>(taskStatus.taskAttemptID, taskStatus.finishTime - taskStatus.startTime));
        }
        getTimeStatistics(reduceTaskTime, minTime, maxTime, medianTime);
        csvFile<<"ReduceTaskTime,,minTime,"<<minTime<<",,maxTime,"<<maxTime<<",,medianTime,"<<medianTime<<endl;
        for(timeIt = reduceTaskTime.begin(); timeIt != reduceTaskTime.end(); timeIt++) {
            csvFile<<timeIt->first<<","<<timeIt->second<<endl;
        }
        csvFile.close();
    }

    csvFile.open("job.csv");
    getTimeStatistics(jobTime, minTime, maxTime, medianTime);
    csvFile<<"JobTime,,minTime,"<<minTime<<",,maxTime,"<<maxTime<<",,medianTime,"<<medianTime<<endl;
    for(timeIt = jobTime.begin(); timeIt != jobTime.end(); timeIt++) {
        csvFile<<timeIt->first<<","<<timeIt->second<<endl;
    }
    csvFile.close();
}

void analyzeJobTaskExeTime(deque<JobStory> &jobSet)
{
    map<string, long> jobTime;
    map<string, long> mapTaskTime;
    map<string, long> reduceTaskTime;
    map<string, long>::iterator timeIt;
    long minTime, maxTime, medianTime;
    ofstream csvFile;

    for(size_t i = 0; i < jobSet.size(); i++) {
        jobTime.insert(pair<string, long>(jobSet[i].jobID, jobSet[i].finishTime - jobSet[i].launchTime));

        csvFile.open((jobSet[i].jobID + "_rawtime.csv").c_str());
        mapTaskTime.clear();
        for(size_t j = 0; j < jobSet[i].mapTasks.size(); j++) {
            TaskStory task = jobSet[i].mapTasks[j];
            mapTaskTime.insert(pair<string, long>(task.taskID, task.finishTime - task.startTime));
        }
        getTimeStatistics(mapTaskTime, minTime, maxTime, medianTime);
        csvFile<<"MapTaskTime,,minTime,"<<minTime<<",,maxTime,"<<maxTime<<",,medianTime,"<<medianTime<<endl;
        for(timeIt = mapTaskTime.begin(); timeIt != mapTaskTime.end(); timeIt++) {
            csvFile<<timeIt->first<<","<<timeIt->second<<endl;
        }
        csvFile<<endl;

        reduceTaskTime.clear();
        for(size_t j = 0; j < jobSet[i].reduceTasks.size(); j++) {
            TaskStory task = jobSet[i].reduceTasks[j];
            reduceTaskTime.insert(pair<string, long>(task.taskID, task.finishTime - task.startTime));
        }
        getTimeStatistics(reduceTaskTime, minTime, maxTime, medianTime);
        csvFile<<"ReduceTaskTime,,minTime,"<<minTime<<",,maxTime,"<<maxTime<<",,medianTime,"<<medianTime<<endl;
        for(timeIt = reduceTaskTime.begin(); timeIt != reduceTaskTime.end(); timeIt++) {
            csvFile<<timeIt->first<<","<<timeIt->second<<endl;
        }
        csvFile.close();
    }

    csvFile.open("job_rawtime.csv");
    getTimeStatistics(jobTime, minTime, maxTime, medianTime);
    csvFile<<"JobTime,,minTime,"<<minTime<<",,maxTime,"<<maxTime<<",,medianTime,"<<medianTime<<endl;
    for(timeIt = jobTime.begin(); timeIt != jobTime.end(); timeIt++) {
        csvFile<<timeIt->first<<","<<timeIt->second<<endl;
    }
    csvFile.close();
}

void startAnalysis(bool isRawTrace)
{
    if (isRawTrace) {
        deque<JobStory> allJobs = getAllJobs();
        analyzeJobTaskExeTime(allJobs);
    } else {
        JobTracker *jobTracker = getJobTracker();
        if (!jobTracker->getCompletedJobs().empty())
            analyzeJobTaskExeTime(jobTracker->getCompletedJobs());
    }
}
