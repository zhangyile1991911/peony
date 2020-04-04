//
// Created by 张亦乐 on 2019-01-08.
//
#pragma once
#include <algorithm>
#include <sys/time.h>
#include "IAppender.h"
#include "3party/plog/Converters/UTF8Converter.h"
#include "3party/plog/Util.h"
namespace plog
{
    template<class Formatter, class Converter = UTF8Converter>
    class HourFileAppender : public IAppender
    {
    public:
        HourFileAppender(const util::nchar* nameprefix):
        m_fileNamePrefix(nameprefix)
        {

        }
        virtual void write(const Record& record)
        {
            util::MutexLock lock(m_mutex);
            struct tm* info = localtime(&record.getTime().time);
            if(m_firstWrite)
            {
                openLogFile(info);
                m_startHour = info->tm_hour;
                m_firstWrite = false;
            }
            else if(info->tm_hour != m_startHour)
            {
                rollLogFiles(info);
            }
            m_file.write(Converter::convert(Formatter::format(record)));
        }

        void rollLogFiles(struct tm* t)
        {
            m_file.close();
            util::nstring lastFileName = buildFileName(t);
            openLogFile(t);
            m_startHour = t->tm_hour;
            m_firstWrite = false;
        }
    private:
        void openLogFile(struct tm* t)
        {
            util::nstring fileName = buildFileName(t);
            int fileSize = m_file.open(fileName.c_str());
            if(0 == fileSize)
            {
                m_file.write(Converter::header(Formatter::header()));
            }
        }

        util::nstring buildFileName(struct tm* t)
        {
            util::nostringstream ss;
            ss<<m_fileNamePrefix<<"_"<<(t->tm_mon+1)<<"_"<<t->tm_mday<<"_"<<t->tm_hour<<".txt";
            return ss.str();
        }
    private:
        util::Mutex     m_mutex;
        util::File      m_file;
        util::nstring   m_fileNamePrefix;
        bool            m_firstWrite;
        int             m_startHour;
    };
}
