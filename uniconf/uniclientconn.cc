/*
 * Worldvisions Tunnel Vision Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * Manages a connection between the UniConf client and daemon.
 */
#include "uniclientconn.h"
#include "wvaddr.h"
#include "wvtclstring.h"

/***** UniClientConn *****/

const UniClientConn::CommandInfo UniClientConn::cmdinfos[
    UniClientConn::NUM_COMMANDS] = {
    // requests
    { "noop", "noop: verify that the connection is active" },
    { "get", "get <key>: get the value of a key" },
    { "set", "set <key> <value>: sets the value of a key" },
    { "del", "del <key>: deletes the key" },
    { "zap", "zap <key>: deletes the children of a key" },
    { "subt", "subt <key>: enumerates the children of a key" },
    { "reg", "reg <key> <depth>: registers for change notification" },
    { "ureg", "ureg <key> <depth>: unregisters for change notification" },
    { "quit", "quit: kills the session nicely" },
    { "help", "help: returns this help text" },
    
    // command completion replies
    { "OK", "OK <payload>: reply on command success" },
    { "FAIL", "FAIL <payload>: reply on command failure" },

    // partial replies
    { "VAL", "VAL <key> <value>: intermediate reply value of a key" },
    { "TEXT", "TEXT <text>: intermediate reply of a text message" },

    // events
    { "HELLO", "HELLO <message>: sent by server on connection" },
    { "FGET", "FGET <key>: forget key and its children" }
};


UniClientConn::UniClientConn(IWvStream *_s) :
    WvStreamClone(_s),
    log(WvString("UniConf to %s", *_s->src()), WvLog::Debug5),
    closed(false), payloadbuf("")
{
    log("Opened\n");
}


UniClientConn::~UniClientConn()
{
    close();
}

bool UniClientConn::isok() const
{
    return msgbuf.used() != 0 || WvStreamClone::isok();
}


void UniClientConn::close()
{
    if (! closed)
    {
        closed = true;
        WvStreamClone::close();
        log("Closed\n");
    }
}


WvString UniClientConn::readmsg()
{
    WvString word;
    while ((word = wvtcl_getword(msgbuf, "\n", false)).isnull())
    {
        char *line = getline(0);
        if (line)
        {
            msgbuf.putstr(line);
            msgbuf.put('\n');
        }
        else
        {
            if (! isok())
            {
                // possibly left some incomplete command behind
                msgbuf.zap();
            }
            return WvString::null;
        }
    }
    log("Read: %s\n", word);
    return word;
}


void UniClientConn::writemsg(WvStringParm msg)
{
    write(msg);
    write("\n");
    log("Wrote: %s\n", msg);
}


UniClientConn::Command UniClientConn::readcmd()
{
    for (;;)
    {
        WvString msg(readmsg());
        if (msg.isnull())
            return NONE;

        // extract command, leaving the remainder in payloadbuf
        payloadbuf.reset(msg);
        WvString cmd(readarg());

        for (int i = 0; i < NUM_COMMANDS; ++i)
            if (strcasecmp(cmdinfos[i].name, cmd.cstr()) == 0)
                return Command(i);
        return INVALID;
    }
}


WvString UniClientConn::readarg()
{
    return wvtcl_getword(payloadbuf, " ");
}


void UniClientConn::writecmd(UniClientConn::Command cmd, WvStringParm msg)
{
    if (msg)
        writemsg(WvString("%s %s", cmdinfos[cmd].name, msg));
    else
        writemsg(cmdinfos[cmd].name);
}


void UniClientConn::writeok(WvStringParm payload)
{
    writecmd(REPLY_OK, payload);
}


void UniClientConn::writefail(WvStringParm payload)
{
    writecmd(REPLY_FAIL, payload);
}


void UniClientConn::writevalue(const UniConfKey &key, WvStringParm value)
{
    writecmd(PART_VALUE, WvString("%s %s", wvtcl_escape(key),
        wvtcl_escape(value)));
}


void UniClientConn::writetext(WvStringParm text)
{
    writecmd(PART_TEXT, wvtcl_escape(text));
}
