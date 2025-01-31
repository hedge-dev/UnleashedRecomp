#pragma once

struct UpdateChecker
{
    enum Result
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
