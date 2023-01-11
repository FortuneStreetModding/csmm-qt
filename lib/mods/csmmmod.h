#ifndef CSMMMOD_H
#define CSMMMOD_H

#include <QtConcurrent>
#include "lib/gameinstance.h"
#include "lib/uimessage.h"
#include "lib/python/pythonbindings.h"
#include "csmmmod_decl.h"

class ArcFileInterface {
public:
    /**
     * @brief Typedef for functions that modify the contents of .arc files; takes the extracted directory path as the last argument.
     */
    typedef std::function<void(const QString &, GameInstance *, const ModListType &, const QString &)> ModifyArcFunction;

    /**
     * @brief Used to modify .arc files on save.
     * @return a mapping from the .arc file path relative to the game root to the function used to modify the .arc
     */
    virtual QMap<QString, ModifyArcFunction> modifyArcFile() { return {}; };

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
    virtual void loadFiles(const QString &root, GameInstance *gameInstance, const ModListType &modList) {};

    /**
     * @brief Modifies game files
     * @param root the game root directory
     * @param gameInstance the game instance
     * @param modList the mod list
     */
    virtual void saveFiles(const QString &root, GameInstance *gameInstance, const ModListType &modList) {};

    virtual ~GeneralInterface() {}
};

class UiMessageInterface {
public:
    /**
     * @brief These functions take the UiMessage to be loaded as the last argument.
     */
    typedef std::function<void(const QString &, GameInstance *, const ModListType &, const UiMessage *)> LoadMessagesFunction;
    /**
     * @brief These functions take the UiMessage to be manipulated as the last argument.
     */
    typedef std::function<void(const QString &, GameInstance *, const ModListType &, UiMessage *)> SaveMessagesFunction;

    /**
     * @return a mapping from the language file path relative to root to a function for loading that language file
     */
    virtual QMap<QString, LoadMessagesFunction> loadUiMessages() { return {}; };
    /**
     * @brief Allocates new localization message ids.
     * @param root the root directory of the game
     * @param gameInstance the game instance object
     * @param modList the mod list
     */
    virtual void allocateUiMessages(const QString &root, GameInstance *gameInstance, const ModListType &modList) {};
    /**
     * @return a mapping from the language file path relative to root to a function for saving to the language file
     */
    virtual QMap<QString, SaveMessagesFunction> saveUiMessages() { return {}; };
    virtual ~UiMessageInterface() {}
};

class ModException : public QException, public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
    const char *what() const noexcept override { return std::runtime_error::what(); }
    ModException(const QString &str) : std::runtime_error(str.toStdString()) {}
    void raise() const override { throw *this; }
    ModException *clone() const override { return new ModException(*this); }
};

#endif // CSMMMOD_H
