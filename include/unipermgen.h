/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 */

#ifndef __UNIPERMGEN_H
#define __UNIPERMGEN_H

#include "unifiltergen.h"
#include "wvstringtable.h"

/**
 * UniPermGen wraps a tree encoding Unix-style permissions, and provides an
 * API for setting and checking them.  read permission allows you to read the
 * value of a key, write allows you to set it (duh).  exec allows you to get
 * subkeys.  You cannot iterate on a key unless you have exec permission.
 * (This is badly named, but it's inherited from Unix.)
 *
 * UniPermGen cannot be created with a moniker due to all the extra methods.
 * Instead, just create one with new and mount it with UniConf::mountgen.  (Of
 * course, in most cases it will be passed to a UniSecureGen's constructor
 * anyway.)
 */
class UniPermGen : public UniFilterGen
{

public:
    UniPermGen(UniConfGen *_gen);
    UniPermGen(WvStringParm moniker);

    enum Level { USER = 0, GROUP, WORLD };
    enum Type { READ = 0, WRITE, EXEC };

    struct Credentials
    {
        WvString user;
        mutable WvStringTable groups;
        // mutable because stupid WvHashTable has no const lookup methods
        
        Credentials() : groups(7) { }
    };

    /** get and set the owner for a path */
    void setowner(const UniConfKey &path, WvStringParm owner);
    WvString getowner(const UniConfKey &path);

    /** get and set the group for a path */
    void setgroup(const UniConfKey &path, WvStringParm group);
    WvString getgroup(const UniConfKey &path);

    /**
     * Return true if a user with the given credentials is allowed to
     * read/write/exec the given path.
     */
    bool getread(const UniConfKey &path, const Credentials &cred)
        { return getperm(path, cred, READ); }
    bool getwrite(const UniConfKey &path, const Credentials &cred)
        { return getperm(path, cred, WRITE); }
    bool getexec(const UniConfKey &path, const Credentials &cred)
        { return getperm(path, cred, EXEC); }

    bool getperm(const UniConfKey &path, const Credentials &cred, Type type);

    void setread(const UniConfKey &path, Level level, bool read)
        { setperm(path, level, READ, read); }
    void setwrite(const UniConfKey &path, Level level, bool write)
        { setperm(path, level, WRITE, write); }
    void setexec(const UniConfKey &path, Level level, bool exec)
        { setperm(path, level, EXEC, exec); }

    void setperm(const UniConfKey &path, Level level, Type type, bool val);

    /**
     * Set permissions for path using Unix style chmod (with the second form,
     * be sure to use octal)
     */
    void chmod(const UniConfKey &path, int owner, int group, int world);
    void chmod(const UniConfKey &path, int mode);

    /** Return the default permission for a given type */ 
    bool defaultperm(Type type);

private:

    struct Perms
    {
        WvString owner;
        WvString group;
        bool mode[3][3];
    };

    void parse(WvStringParm str, Perms &perms);
    WvString format(const Perms &perms);
};


#endif // __UNIPERMGEN_H