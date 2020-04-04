//
// Created by 张亦乐 on 2018/6/27.
//

#ifndef EVENTMANAGER_HPP
#define EVENTMANAGER_HPP

#include "TVarList.hpp"
class EventManager_Base
{
public:
    typedef std::function<void(const char*, IVarList&)> event_handler;
    typedef std::vector<event_handler> event_handler_list;
    typedef std::map<std::string, event_handler_list> EventsMap;

    EventManager_Base() {}
    virtual ~EventManager_Base() {}

    virtual void connect(const char* eventname, event_handler callback)
    {
        _EventHandlers[eventname].push_back(callback);
    }

    virtual void disconnect(const char* eventname)
    {
        EventsMap::iterator it = _EventHandlers.find(std::string(eventname));
        if (it != _EventHandlers.end())
        {
            _EventHandlers.erase(it);
        }
    }

    virtual void disconnect(const char* eventname, event_handler callback)
    {
        EventsMap::iterator it = _EventHandlers.find(std::string(eventname));
        if (it != _EventHandlers.end())
        {
            event_handler_list& EventList = it->second;
            event_handler_list::iterator EventIter = EventList.begin();
            for (; EventIter != EventList.end(); EventIter++)
            {
                event_handler handler = *EventIter;
                if (handler.target_type().operator==(callback.target_type()))
                {
                    EventList.erase(EventIter);
                    break;
                }

            }//end for
        }//end if
    }

    virtual bool invoke(const char* eventname, IVarList& arg)
    {
        if (_EventHandlers.empty())
        {
            return false;
        }

        EventsMap::iterator it = _EventHandlers.find(eventname);
        if (it == _EventHandlers.end())
        {
            return false;
        }

        event_handler_list& event_lists = it->second;
        for (event_handler& handler : event_lists)
        {
            handler(eventname, arg);
        }
        return true;
    }

private:
    EventsMap			_EventHandlers;
};

#endif //EVENTMANAGER_HPP
