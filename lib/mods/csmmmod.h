#ifndef CSMMMOD_H
#define CSMMMOD_H

#include <QtConcurrent>
#include "lib/gameinstance.h"
#include "lib/uimessage.h"

#define NORMAL_PRIORITY 0
#define MAP_DESCRIPTOR_COUNT_PRIORITY 500000
#define FREE_SPACE_PRIORITY 1000000

class CSMMMod;

typedef QVector<QSharedPointer<CSMMMod>> ModListType;

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

    virtual ~CSMMMod() {}
};

class ArcFileInterface {
public:
    /**
     * @brief Typedef for functions that modify the contents of .arc files; takes the extracted directory path as the last argument.
     */
    typedef std::function<void(const QString &, GameInstance &, const ModListType &, const QString &)> ModifyArcFunction;

    /**
     * @brief Used to modify .arc files on save.
     * @return a mapping from the .arc file path relative to the game root to the function used to modify the .arc
     */
    virtual QMap<QString, ModifyArcFunction> modifyArcFile() = 0;

    virtual ~ArcFileInterface() {}
};

class GeneralInterface {
public:
    /**
     * @brief Reads game files, possibly modifying the map descriptors.
     * @param root the game root directory
     * @param gameInstance the game instance
     * @param modList the mod list
     */
    virtual void loadFiles(const QString &root, GameInstance &gameInstance, const ModListType &modList) = 0;

    /**
     * @brief Modifies game files
     * @param root the game root directory
     * @param gameInstance the game instance
     * @param modList the mod list
     */
    virtual void saveFiles(const QString &root, GameInstance &gameInstance, const ModListType &modList) = 0;

    virtual ~GeneralInterface() {}
};

class UiMessageInterface {
public:
    /**
     * @brief These functions take the UiMessage to be loaded as the last argument.
     */
    typedef std::function<void(const QString &, GameInstance &, const ModListType &, const UiMessage &)> LoadMessagesFunction;
    /**
     * @brief These functions take the UiMessage to be manipulated as the last argument.
     */
    typedef std::function<void(const QString &, GameInstance &, const ModListType &, UiMessage &)> SaveMessagesFunction;

    /**
     * @return a mapping from the language file path relative to root to a function for loading that language file
     */
    virtual QMap<QString, LoadMessagesFunction> loadUiMessages() = 0;
    /**
     * @return a mapping from the language file path relative to root to a function for freeing old l10n ids
     */
    virtual QMap<QString, SaveMessagesFunction> freeUiMessages() = 0;
    /**
     * @brief Allocates new localization message ids.
     * @param root the root directory of the game
     * @param gameInstance the game instance object
     * @param modList the mod list
     */
    virtual void allocateUiMessages(const QString &root, GameInstance &gameInstance, const ModListType &modList) = 0;
    /**
     * @return a mapping from the language file path relative to root to a function for saving to the language file
     */
    virtual QMap<QString, SaveMessagesFunction> saveUiMessages() = 0;
    virtual ~UiMessageInterface() {}
};

class ModException : public QException {
public:
    ModException(const QString &msgVal) : message(msgVal) {}
    const QString &getMessage() const { return message; }
    void raise() const override { throw *this; }
    ModException *clone() const override { return new ModException(*this); }
private:
    QString message;
};

#endif // CSMMMOD_H
