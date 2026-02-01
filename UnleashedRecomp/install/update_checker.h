#pragma once

// 6 hour long timeout between checks.
static constexpr double g_timeBetweenUpdateChecksInSeconds = 6 * 60 * 60;

struct UpdateChecker
{
    enum class Result
    {
        NotStarted,
        InProgress,
        AlreadyUpToDate,
        UpdateAvailable,
        Failed
    };

    static void initialize();
    static bool start();
    static Result check();
    static void visitWebsite();
};
