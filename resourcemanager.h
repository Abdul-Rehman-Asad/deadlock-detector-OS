#ifndef RESOURCEMANAGER_H
#define RESOURCEMANAGER_H

#include <QString>
#include <QVector>
#include <QMap>
#include <QSet>
#include <QStringList>



struct Process {
    int     id;
    QString name;
    bool    isBlocked;
    bool    isDeadlocked;

    Process() : id(-1), isBlocked(false), isDeadlocked(false) {}
    Process(int id, const QString &name)
        : id(id), name(name), isBlocked(false), isDeadlocked(false) {}
};

struct Resource {
    int     id;
    QString name;
    int     totalInstances;
    int     availableInstances;

    Resource() : id(-1), totalInstances(1), availableInstances(1) {}
    Resource(int id, const QString &name, int instances = 1)
        : id(id), name(name),
        totalInstances(instances), availableInstances(instances) {}
};

struct LogEntry {
    QString timestamp;
    QString message;
    QString type;
};



class ResourceManager
{
public:
    ResourceManager();


    bool    addProcess(const QString &name);
    bool    removeProcess(int pid);
    bool    processExists(int pid) const;


    bool    addResource(const QString &name, int instances = 1);
    bool    removeResource(int rid);
    bool    resourceExists(int rid) const;


    bool    requestResource(int pid, int rid);
    bool    releaseResource(int pid, int rid);
    bool    allocateResource(int pid, int rid);

    QMap<int, QSet<int>> buildWaitForGraph() const;

    QVector<int>    detectDeadlock();
    QVector<QVector<int>> getAllCycles();


    bool    terminateProcess(int pid);
    int     chooseBestVictim(const QVector<int> &deadlocked);

    const QMap<int, Process>   &processes()  const { return m_processes; }
    const QMap<int, Resource>  &resources()  const { return m_resources; }

    const QMap<int, QMap<int,int>> &allocation() const { return m_allocation; }

    const QMap<int, QMap<int,int>> &requests()   const { return m_requests; }


    const QVector<LogEntry> &log() const { return m_log; }
    void clearLog();

    void reset();
    QString stateSnapshot() const;

private:
    QMap<int, Process>          m_processes;
    QMap<int, Resource>         m_resources;
    QMap<int, QMap<int,int>>    m_allocation;
    QMap<int, QMap<int,int>>    m_requests;

    int m_nextPid;
    int m_nextRid;

    void addLog(const QString &msg, const QString &type = "INFO");

    bool dfsCycleDetect(int node,
                        const QMap<int,QSet<int>> &graph,
                        QSet<int> &visited,
                        QSet<int> &recStack,
                        QVector<int> &path,
                        QVector<QVector<int>> &cycles);

    QVector<LogEntry> m_log;
};

#endif
