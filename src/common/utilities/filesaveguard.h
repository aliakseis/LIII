#pragma once

#include <QString>
#include <QFile>
#include "filesystem_utils.h"

namespace utilities
{

struct FileSaveGuard
{
    explicit FileSaveGuard(const QString& filename)
    : tmpfile(filename), ok_(false), tempFileNotNeeded(false)
    {
        if (QFile::exists(filename))
        {
            orig_file = filename;
            QString tmpFilename = filename + '-';

            for (int counter = 0; counter < 50 && !tmpfile.rename(tmpFilename); ++counter)
            {
                tmpFilename = tmpFilename + '-';
            }
        }
        else
        {
            tmpfile.setFileName("");
            tempFileNotNeeded = true;
        }
    }

    ~FileSaveGuard()
    {
        if (tmpfile.exists())
        {
            if (ok_)
            {
                tmpfile.remove();
            }
            else
            {
                if (!(utilities::DeleteFileWithWaiting(orig_file) && tmpfile.rename(orig_file)))
                {
                    // TODO: only miracle can save us now
                }
            }
        }
    }
    bool isTempFileNoError() { return tmpfile.exists() || tempFileNotNeeded; }
    void ok() { ok_ = true; }
private:
    QString orig_file;
    QFile tmpfile;
    bool ok_;
    bool tempFileNotNeeded;
};

} // namespace utilities

