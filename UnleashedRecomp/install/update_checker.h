#pragma once

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
