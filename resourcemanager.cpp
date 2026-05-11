#include "resourcemanager.h"
#include <QDateTime>
#include <QDebug>


ResourceManager::ResourceManager()
    : m_nextPid(0), m_nextRid(0)
{}


void ResourceManager::addLog(const QString &msg, const QString &type)
{
    LogEntry e;
    e.timestamp = QDateTime::currentDateTime().toString("hh:mm:ss.zzz");
    e.message   = msg;
    e.type      = type;
    m_log.append(e);
}

void ResourceManager::clearLog()
{
    m_log.clear();
}


bool ResourceManager::addProcess(const QString &name)
{

    for (auto &p : m_processes)
        if (p.name == name) {
            addLog("Process '" + name + "' already exists.", "WARN");
            return false;
        }

    int pid = m_nextPid++;
    m_processes[pid] = Process(pid, name);
    m_allocation[pid];
    m_requests[pid];
    addLog("Process " + name + " (PID " + QString::number(pid) + ") created.");
    return true;
}

bool ResourceManager::removeProcess(int pid)
{
    if (!m_processes.contains(pid)) return false;


    for (auto it = m_allocation[pid].begin(); it != m_allocation[pid].end(); ++it) {
        if (m_resources.contains(it.key()))
            m_resources[it.key()].availableInstances += it.value();
    }
    m_allocation.remove(pid);
    m_requests.remove(pid);



    QString name = m_processes[pid].name;
    m_processes.remove(pid);
    addLog("Process " + name + " (PID " + QString::number(pid) + ") removed.");
    return true;
}

bool ResourceManager::processExists(int pid) const
{
    return m_processes.contains(pid);
}


bool ResourceManager::addResource(const QString &name, int instances)
{
    for (auto &r : m_resources)
        if (r.name == name) {
            addLog("Resource '" + name + "' already exists.", "WARN");
            return false;
        }

    int rid = m_nextRid++;
    m_resources[rid] = Resource(rid, name, instances);
    addLog("Resource " + name + " (RID " + QString::number(rid) +
           ", instances=" + QString::number(instances) + ") created.");
    return true;
}

bool ResourceManager::removeResource(int rid)
{
    if (!m_resources.contains(rid)) return false;


    for (auto &p : m_processes) {
        if (m_allocation[p.id].value(rid, 0) > 0) {
            addLog("Cannot remove resource " + m_resources[rid].name +
                       ": held by " + p.name, "WARN");
            return false;
        }
    }

    QString name = m_resources[rid].name;
    m_resources.remove(rid);
    addLog("Resource " + name + " (RID " + QString::number(rid) + ") removed.");
    return true;
}

bool ResourceManager::resourceExists(int rid) const
{
    return m_resources.contains(rid);
}


bool ResourceManager::allocateResource(int pid, int rid)
{
    if (!m_processes.contains(pid) || !m_resources.contains(rid)) return false;

    Resource &res = m_resources[rid];
    if (res.availableInstances < 1) {
        addLog("Resource " + res.name + " has no available instances for forced allocation.", "WARN");
        return false;
    }
    res.availableInstances--;
    m_allocation[pid][rid]++;
    addLog(m_processes[pid].name + " allocated " + res.name);
    return true;
}

bool ResourceManager::requestResource(int pid, int rid)
{
    if (!m_processes.contains(pid) || !m_resources.contains(rid)) return false;

    Resource &res = m_resources[rid];
    Process  &proc = m_processes[pid];

    if (res.availableInstances > 0) {

        res.availableInstances--;
        m_allocation[pid][rid]++;
        addLog(proc.name + " → " + res.name + " : GRANTED");
        return true;
    } else {

        m_requests[pid][rid]++;
        proc.isBlocked = true;
        addLog(proc.name + " → " + res.name + " : WAITING (resource busy)", "WARN");
        return false;
    }
}


bool ResourceManager::releaseResource(int pid, int rid)
{
    if (!m_processes.contains(pid) || !m_resources.contains(rid)) return false;
    if (m_allocation[pid].value(rid, 0) == 0) {
        addLog(m_processes[pid].name + " does not hold " + m_resources[rid].name, "WARN");
        return false;
    }

    m_allocation[pid][rid]--;
    if (m_allocation[pid][rid] == 0)
        m_allocation[pid].remove(rid);

    m_resources[rid].availableInstances++;
    addLog(m_processes[pid].name + " released " + m_resources[rid].name);

    for (auto &waitPid : m_processes.keys()) {
        if (m_requests[waitPid].value(rid, 0) > 0 &&
            m_resources[rid].availableInstances > 0)
        {
            m_requests[waitPid][rid]--;
            if (m_requests[waitPid][rid] == 0)
                m_requests[waitPid].remove(rid);

            m_resources[rid].availableInstances--;
            m_allocation[waitPid][rid]++;
            addLog(m_processes[waitPid].name + " was unblocked and got " + m_resources[rid].name);

            bool stillBlocked = false;
            for (auto it = m_requests[waitPid].begin(); it != m_requests[waitPid].end(); ++it)
                if (it.value() > 0) { stillBlocked = true; break; }
            m_processes[waitPid].isBlocked = stillBlocked;
            break;
        }
    }

    return true;
}


QMap<int, QSet<int>> ResourceManager::buildWaitForGraph() const
{
    QMap<int, QSet<int>> wfg;


    for (auto &p : m_processes)
        wfg[p.id];


    for (auto waitIt = m_requests.begin(); waitIt != m_requests.end(); ++waitIt) {
        int waitPid = waitIt.key();
        for (auto resIt = waitIt.value().begin(); resIt != waitIt.value().end(); ++resIt) {
            int rid = resIt.key();
            if (resIt.value() <= 0) continue;


            for (auto allocIt = m_allocation.begin(); allocIt != m_allocation.end(); ++allocIt) {
                int holderPid = allocIt.key();
                if (holderPid == waitPid) continue;
                if (allocIt.value().value(rid, 0) > 0) {
                    wfg[waitPid].insert(holderPid);
                }
            }
        }
    }
    return wfg;
}


bool ResourceManager::dfsCycleDetect(int node,
                                     const QMap<int,QSet<int>> &graph,
                                     QSet<int> &visited,
                                     QSet<int> &recStack,
                                     QVector<int> &path,
                                     QVector<QVector<int>> &cycles)
{
    visited.insert(node);
    recStack.insert(node);
    path.append(node);

    bool foundCycle = false;

    const QSet<int> &neighbors = graph.value(node);
    for (int neighbor : neighbors) {
        if (!visited.contains(neighbor)) {
            if (dfsCycleDetect(neighbor, graph, visited, recStack, path, cycles))
                foundCycle = true;
        } else if (recStack.contains(neighbor)) {

            QVector<int> cycle;
            int idx = path.indexOf(neighbor);
            for (int i = idx; i < path.size(); i++)
                cycle.append(path[i]);
            cycle.append(neighbor);
            cycles.append(cycle);
            foundCycle = true;
        }
    }

    path.removeLast();
    recStack.remove(node);
    return foundCycle;
}

QVector<QVector<int>> ResourceManager::getAllCycles()
{
    QMap<int,QSet<int>> wfg = buildWaitForGraph();
    QSet<int>  visited, recStack;
    QVector<int> path;
    QVector<QVector<int>> cycles;

    for (auto it = wfg.begin(); it != wfg.end(); ++it) {
        int node = it.key();
        if (!visited.contains(node)) {
            dfsCycleDetect(node, wfg, visited, recStack, path, cycles);
        }
    }
    return cycles;
}

QVector<int> ResourceManager::detectDeadlock()
{
    QVector<QVector<int>> cycles = getAllCycles();
    QSet<int> deadlocked;
    for (auto &cycle : cycles)
        for (int pid : cycle)
            if (pid != cycle.last() || cycle.size() == 1)
                deadlocked.insert(pid);


    deadlocked.clear();
    for (auto &cycle : cycles)
        for (int i = 0; i < cycle.size() - 1; i++)
            deadlocked.insert(cycle[i]);


    for (auto &p : m_processes)
        m_processes[p.id].isDeadlocked = false;
    for (int pid : deadlocked)
        if (m_processes.contains(pid))
            m_processes[pid].isDeadlocked = true;

    if (!deadlocked.isEmpty()) {
        QString pids;
        for (int pid : deadlocked)
            pids += m_processes.value(pid).name + " ";
        addLog("⚠ DEADLOCK DETECTED involving: " + pids.trimmed(), "DEADLOCK");
    } else {
        addLog("✓ No deadlock detected.", "INFO");
    }

    return deadlocked.values().toVector();
}


int ResourceManager::chooseBestVictim(const QVector<int> &deadlocked)
{

    int victim = -1;
    int minHeld = INT_MAX;
    for (int pid : deadlocked) {
        int held = 0;
        for (auto it = m_allocation[pid].begin(); it != m_allocation[pid].end(); ++it)
            held += it.value();
        if (held < minHeld) {
            minHeld = held;
            victim = pid;
        }
    }
    return victim;
}

bool ResourceManager::terminateProcess(int pid)
{
    if (!m_processes.contains(pid)) return false;

    QString name = m_processes[pid].name;
    addLog("🔴 RECOVERY: Terminating process " + name +
               " to break deadlock.", "RECOVERY");


    QMap<int,int> held = m_allocation.value(pid);
    for (auto it = held.begin(); it != held.end(); ++it) {
        int rid = it.key();
        int cnt = it.value();
        m_resources[rid].availableInstances += cnt;
        addLog("  Released " + QString::number(cnt) + "x " +
                   m_resources[rid].name + " from " + name, "RECOVERY");
    }


    m_requests.remove(pid);
    m_allocation.remove(pid);
    m_processes.remove(pid);


    for (int rid : held.keys()) {
        for (int waitPid : m_processes.keys()) {
            if (m_requests[waitPid].value(rid,0) > 0 &&
                m_resources[rid].availableInstances > 0)
            {
                m_requests[waitPid][rid]--;
                m_resources[rid].availableInstances--;
                m_allocation[waitPid][rid]++;
                m_processes[waitPid].isBlocked = false;
                addLog("  " + m_processes[waitPid].name +
                           " unblocked and got " + m_resources[rid].name, "RECOVERY");
            }
        }
    }

    return true;
}


void ResourceManager::reset()
{
    m_processes.clear();
    m_resources.clear();
    m_allocation.clear();
    m_requests.clear();
    m_log.clear();
    m_nextPid = 0;
    m_nextRid = 0;
    addLog("System reset.");
}

QString ResourceManager::stateSnapshot() const
{
    QString s;
    s += "=== PROCESSES ===\n";
    for (auto &p : m_processes)
        s += QString("  [P%1] %2  blocked=%3  deadlocked=%4\n")
                 .arg(p.id).arg(p.name)
                 .arg(p.isBlocked ? "yes" : "no")
                 .arg(p.isDeadlocked ? "YES" : "no");

    s += "\n=== RESOURCES ===\n";
    for (auto &r : m_resources)
        s += QString("  [R%1] %2  total=%3  available=%4\n")
                 .arg(r.id).arg(r.name).arg(r.totalInstances).arg(r.availableInstances);

    s += "\n=== ALLOCATION ===\n";
    for (auto pit = m_allocation.begin(); pit != m_allocation.end(); ++pit)
        for (auto rit = pit.value().begin(); rit != pit.value().end(); ++rit)
            if (rit.value() > 0)
                s += QString("  %1 holds %2x %3\n")
                         .arg(m_processes.value(pit.key()).name)
                         .arg(rit.value())
                         .arg(m_resources.value(rit.key()).name);

    s += "\n=== WAITING (REQUESTS) ===\n";
    for (auto pit = m_requests.begin(); pit != m_requests.end(); ++pit)
        for (auto rit = pit.value().begin(); rit != pit.value().end(); ++rit)
            if (rit.value() > 0)
                s += QString("  %1 waiting for %2x %3\n")
                         .arg(m_processes.value(pit.key()).name)
                         .arg(rit.value())
                         .arg(m_resources.value(rit.key()).name);
    return s;
}
