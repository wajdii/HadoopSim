/*
Lei Ye <leiy@cs.arizona.edu>
HadoopSim is a simulator for a Hadoop Runtime by replaying the collected traces.
*/
#include <assert.h>
#include <stdio.h>
#include "Cluster.h"
#include "NetMonitor.h"
using namespace HadoopNetSim;
using namespace std;

static FILE *netFile = NULL;
static unordered_map<LinkId,ns3::Ptr<Link>> netSimLinks;

void checkLinkStat(void)
{
    for(LinkId id = 1; id <= netSimLinks.size(); ++id) {
        ns3::Ptr<LinkStat> stat = getNetSim()->GetLinkStat(id);
        fprintf(netFile, "LinkStat(%d) bandwidth=%02.2f%% queue=%02.2f%%\n", stat->id(), stat->bandwidth_utilization()*100, stat->queue_utilization()*100);
        LinkId rid = -id;
        stat = getNetSim()->GetLinkStat(rid);
        fprintf(netFile, "LinkStat(%d) bandwidth=%02.2f%% queue=%02.2f%%\n", stat->id(), stat->bandwidth_utilization()*100, stat->queue_utilization()*100);
    }
    fprintf(netFile, "\n");
    ns3::Simulator::Schedule(ns3::Seconds(3.0), &checkLinkStat);
}

void enableNetMonitor(unordered_map<LinkId,ns3::Ptr<Link>> links, string debugDir)
{
    netSimLinks = links;
    netFile = fopen((debugDir + "linkstat.txt").c_str(), "w");
    ns3::Simulator::Schedule(ns3::Seconds(3.0), &checkLinkStat);
}

void disableNetMonitor()
{
    if(netFile)
        fclose(netFile);
}
