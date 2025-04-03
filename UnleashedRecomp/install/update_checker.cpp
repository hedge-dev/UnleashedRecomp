#include "update_checker.h"

#include <curl/curl.h>
#include <nlohmann/json.hpp>

#include "version.h"

#ifdef WIN32
#include <shellapi.h>
#endif

#include <os/logger.h>

// UpdateChecker

using json = nlohmann::json;

static const char *CHECK_URL = "https://api.github.com/repos/hedge-dev/UnleashedRecomp/releases/latest";
static const char *VISIT_URL = "https://github.com/hedge-dev/UnleashedRecomp/releases/latest";
static const char *USER_AGENT = "UnleashedRecomp-Agent";

static std::atomic<bool> g_updateCheckerInProgress = false;
static std::atomic<bool> g_updateCheckerFinished = false;
static UpdateChecker::Result g_updateCheckerResult = UpdateChecker::Result::NotStarted;

size_t updateCheckerWriteCallback(void *contents, size_t size, size_t nmemb, std::string *output)
{
    size_t totalSize = size * nmemb;
    output->append((char *)contents, totalSize);
    return totalSize;
}

static bool parseVersion(const std::string &versionStr, int &major, int &minor, int &revision)
{
    size_t start = 0;
    if (versionStr[0] == 'v')
    {
        start = 1;
    }

    size_t firstDot = versionStr.find('.', start);
    size_t secondDot = versionStr.find('.', firstDot + 1);

    if (firstDot == std::string::npos || secondDot == std::string::npos)
    {
        return false;
    }

    try
    {
        major = std::stoi(versionStr.substr(start, firstDot - start));
        minor = std::stoi(versionStr.substr(firstDot + 1, secondDot - firstDot - 1));
        revision = std::stoi(versionStr.substr(secondDot + 1));
    }
    catch (const std::exception &e)
    {
        LOGF_ERROR("Error while parsing version: {}.", e.what());
        return false;
    }

    return true;
}

void updateCheckerThread()
{
    CURL *curl = curl_easy_init();
    CURLcode res;
    int major, minor, revision;
    std::string response;
    curl_easy_setopt(curl, CURLOPT_URL, CHECK_URL);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, updateCheckerWriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, USER_AGENT);

    res = curl_easy_perform(curl);
    if (res == CURLE_OK)
    {
        try
        {
            json root = json::parse(response);
            auto tag_name_element = root.find("tag_name");
            if (tag_name_element != root.end() && tag_name_element->is_string())
            {
                if (parseVersion(*tag_name_element, major, minor, revision))
                {
                    if ((g_versionMajor < major) || (g_versionMajor == major  && g_versionMinor < minor) || (g_versionMajor == major && g_versionMinor == minor && g_versionRevision < revision))
                    {
                        g_updateCheckerResult = UpdateChecker::Result::UpdateAvailable;
                    }
                    else
                    {
                        g_updateCheckerResult = UpdateChecker::Result::AlreadyUpToDate;
                    }
                }
                else
                {
                    LOG_ERROR("Error while parsing response: tag_name does not contain a valid version string.");
                    g_updateCheckerResult = UpdateChecker::Result::Failed;
                }
            }
            else
            {
                LOG_ERROR("Error while parsing response: tag_name not found or not the right type.");
                g_updateCheckerResult = UpdateChecker::Result::Failed;
            }
        }
        catch (const json::exception &e)
        {
            LOGF_ERROR("Error while parsing response: {}", e.what());
            g_updateCheckerResult = UpdateChecker::Result::Failed;
        }
    }
    else
    {
        LOGF_ERROR("Error while performing request: {}", curl_easy_strerror(res));
        g_updateCheckerResult = UpdateChecker::Result::Failed;
    }

    curl_easy_cleanup(curl);

    g_updateCheckerFinished = true;
    g_updateCheckerInProgress = false;
}

void UpdateChecker::initialize()
{
    curl_global_init(CURL_GLOBAL_DEFAULT);
}

bool UpdateChecker::start()
{
    if (g_updateCheckerInProgress)
    {
        return false;
    }

    g_updateCheckerInProgress = true;
    g_updateCheckerFinished = false;
    std::thread thread(&updateCheckerThread);
    thread.detach();

    return true;
}

UpdateChecker::Result UpdateChecker::check()
{
    if (g_updateCheckerFinished)
    {
        return g_updateCheckerResult;
    }
    else if (g_updateCheckerInProgress)
    {
        return UpdateChecker::Result::InProgress;
    }
    else
    {
        return UpdateChecker::Result::NotStarted;
    }
}

void UpdateChecker::visitWebsite()
{
#if defined(WIN32)
    ShellExecuteA(0, 0, VISIT_URL, 0, 0, SW_SHOW);
#elif defined(__linux__)
    std::string command = "xdg-open " + std::string(VISIT_URL) + " &";
    std::system(command.c_str());
#elif defined(__APPLE__)
    std::string command = "open " + std::string(VISIT_URL) + " &";
    std::system(command.c_str());
#else
    static_assert(false, "Visit website not implemented for this platform.");
#endif
}
