#pragma once

namespace download
{

struct resume_supported_tag {};
struct resume_unsupported_tag {};

struct speed_readable_tag {};
struct speed_limitable_tag {};

template <class DownloaderType>
struct DownloaderTraits
{
    typedef typename DownloaderType::resume_support_category resume_support_category;
    typedef typename DownloaderType::speed_access_category speed_access_category;
};

} // namespace download
