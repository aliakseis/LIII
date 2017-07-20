#include "global_functions.h"

#include <iterator>
#include <algorithm>
#include <functional>
#include <set>

#include <QStringList>
#include <QRegExp>
#include <QVariant>
#include <QNetworkRequest>
#include <QUrl>
#include <QList>
#include <QLocale>
#include <QDomElement>
#include <QDomNode>
#include <QDomNodeList>
#include <QDesktopServices>
#include <QMessageBox>
#include <QFileInfo>
#include <QDir>

#include "utilities/utils.h"
#include "utilities/filesystem_utils.h"
#include "downloadtype.h"


namespace {

bool IsVideoFile(const QString& filename)
{
    const char* VIDEO_FILE_EXTS[] =
    {
        "264", "3g2", "3gp", "3gp2", "3gpp", "3gpp2", "3mm", "3p2", "60d", "787",
        "aaf", "aep", "aepx", "aet", "aetx", "ajp", "ale", "amv", "amx", "anim", "arf", "asf", "asx", "avb", "avd", "avi", "avp", "avs",
        "avs", "axm", "bdm", "bdmv", "bik", "bin", "bix", "bmk", "bnp", "box", "bs4", "bsf", "byu", "camproj", "camrec", "clpi", "cmmp",
        "cmmtpl", "cmproj", "cmrec", "cpi", "cst", "cvc", "d2v", "d3v", "dat", "dav", "dce", "dck", "ddat", "dif", "dir", "divx", "dlx",
        "dmb", "dmsd", "dmsd3d", "dmsm", "dmsm3d", "dmss", "dnc", "dpa", "dpg", "dream", "dsy", "dv", "dv-avi", "dv4", "dvdmedia",
        "dvr", "dvr-ms", "dvx", "dxr", "dzm", "dzp", "edl", "evo", "eye", "f4p", "f4v", "fbr", "fbr", "fbz", "fcp", "fcproject",
        "flc", "flh", "fli", "flv", "flx", "gfp", "gl", "grasp", "gts", "gvi", "gvp", "h264", "hdmov", "hkm", "ifo", "imovieproj",
        "imovieproject", "irf", "ism", "ismc", "ismv", "iva", "ivf", "ivr", "ivs", "izz", "izzy", "jts", "jtv", "k3g", "lrec", "lsf",
        "lsx", "m15", "m1pg", "m1v", "m21", "m21", "m2a", "m2p", "m2t", "m2ts", "m2v", "m4e", "m4u", "m4v", "m75", "meta", "mgv", "mj2",
        "mjp", "mjpg", "mkv", "mmv", "mnv", "mob", "mod", "modd", "moff", "moi", "moov", "mov", "movie", "mp21", "mp21", "mp2v", "mp3", "flac", "wav", "mp4",
        "mp4v", "mpe", "mpeg", "mpeg4", "mpf", "mpg", "mpg2", "mpgindex", "mpl", "mpls", "mpv", "mpv2", "mqv", "msdvd", "mse",
        "msh", "mswmm", "mts", "mtv", "mvb", "mvc", "mvd", "mve", "mvp", "mvp", "mvy", "mxf", "mys", "ncor", "nsv", "nuv", "nvc", "ogm", "ogv",
        "ogx", "osp", "par", "pds", "pgi", "photoshow", "piv", "playlist", "pmf", "pmv", "pns", "ppj", "prel", "pro", "prproj",
        "psh", "pssd", "pva", "pvr", "pxv", "qt", "qtch", "qtl", "qtm", "qtz", "r3d", "rcproject", "rdb", "rec", "rm", "rmd", "rmd", "rmp",
        "rms", "rmvb", "roq", "rp", "rsx", "rts", "rts", "rv", "sbk", "scc", "scm", "scm", "screenflow", "sec", "seq",
        "sfd", "sfvidcap", "smi", "smil", "smk", "sml", "smv", "spl", "sqz", "ssm", "str", "stx", "svi", "swi", "swt",
        "tda3mt", "tdx", "tivo", "tix", "tod", "tp", "tp0", "tpd", "tpr", "trp", "ts", "tsp", "tvs", "vc1", "vcpf", "vcr", "vcv", "vdo", "vdr",
        "veg", "vem", "vep", "vf", "vft", "vfw", "vfz", "vgz", "vid", "video", "viewlet", "viv", "vivo", "vlab", "vob", "vp3", "vp6", "vp7",
        "vpj", "vro", "vs4", "vse", "vsp", "wcp", "webm", "wlmp", "wm", "wmd", "wmmp", "wmv", "wmx", "wot", "wp3", "wpl", "wtv", "wvx",
        "xej", "xel", "xesc", "xfl", "xlmv", "xvid", "yuv", "zm1", "zm2", "zm3", "zmv",
        "mp3", "flac", "wav"
    };
    QString s = QFileInfo(filename).suffix().toLower();
    const auto end = std::end(VIDEO_FILE_EXTS);
    return !s.isEmpty() && std::find(std::begin(VIDEO_FILE_EXTS), end, s) != end;
}

} // namespace

namespace global_functions
{

using namespace app_settings;


QString GetNormalizedDomain(const QUrl& url)
{
    QString host = url.host();
    if (host.startsWith("www."))
    {
        host = host.right(host.length() - 4);
    }
    return host;
}


bool IsContentFile(const QString& filename)
{
    return IsVideoFile(filename)
           || DownloadType::determineType(filename) == DownloadType::TorrentFile;
}

bool IsExecutable(const QString& filename)
{
    return QFileInfo(filename).isExecutable();
}


void openFile(const QString& filename, QWidget* parent)
{
    if (!global_functions::IsExecutable(filename)
            || QMessageBox::question(
                parent, QObject::tr("Possible Threat"),
                QObject::tr("Are you sure you want to start an executable file\n\"%1\"?").arg(filename),
                QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes)
    {
        QDesktopServices::openUrl(QUrl::fromLocalFile(filename));
    }
}

QString GetVideoFolder()
{
    QString fixedFolder = QSettings().value(VideoFolder).toString().trimmed();
    QStringList pathItems = fixedFolder.split(QRegExp("[/\\\\]+"), QString::SkipEmptyParts);

    fixedFolder = fixedFolder.isEmpty() ?
        utilities::getPathForDownloadFolder() :
        (fixedFolder.startsWith(QDir::separator()) ? QDir::separator() : QString()) + pathItems.join(QDir::separator());
    return fixedFolder.endsWith(QDir::separator()) ? fixedFolder : fixedFolder + QDir::separator();
}


int GetMaximumNumberLoadsActual()
{
    QSettings settings;
    return settings.value(UnlimitedLabel, UnlimitedLabel_Default).toBool()
        ? 10 : settings.value(MaximumNumberLoads, MaximumNumberLoads_Default).toInt();
}

int GetTrafficLimitActual()
{
    QSettings settings;
    return settings.value(IsTrafficLimited, IsTrafficLimited_Default).toBool()
        ? settings.value(TrafficLimitKbs, TrafficLimitKbs_Default).toInt() : 0;
}

} // namespace global_functions
