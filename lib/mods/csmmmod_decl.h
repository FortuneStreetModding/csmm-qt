#ifndef CSMMMOD_DECL_H
#define CSMMMOD_DECL_H

#include <QString>
#include <QSet>

constexpr int NORMAL_PRIORITY = 0;
constexpr int MAP_DESCRIPTOR_COUNT_PRIORITY = 500000;
constexpr int FREE_SPACE_PRIORITY = 1000000;

class CSMMMod {
public:
    /**
     * @return the id of this mod
     */
    virtual QString modId() const = 0;
    /**
     * @return the priority of this mod (higher priority mods will be loaded before lower priority ones)
     */
    virtual int priority() const { return NORMAL_PRIORITY; }
    /**
     * @return a set of modids that this mod depends on, doesn't enforce any running order otherwise on its own
     */
    virtual QSet<QString> depends() const { return {}; }
    /**
     * @return a set of modids of mods that this mod should run after but are not necessarily required
     */
    virtual QSet<QString> after() const { return {}; }
    /**
     * @return a set of modids of mods that this mod should run before but are not necessarily required
     */
    virtual QSet<QString> before() const { return {}; }

    /**
     * @return the modpack directory, useful for loading configuration file(s)
     */
    QString modpackDir() const { return mpDir; };

    void setModpackDir(const QString &dir) { mpDir = dir; }

    virtual ~CSMMMod() {}
private:
    QString mpDir;
};

#endif // CSMMMOD_DECL_H
