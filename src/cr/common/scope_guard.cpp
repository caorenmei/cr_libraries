#include "scope_guard.h"

namespace cr
{
    ScopeGuard::ScopeGuard(std::function<void()> rollback)
        : rollback_(std::move(rollback)),
        dismissed_(false)
    {}

    ScopeGuard::ScopeGuard(ScopeGuard&& other)
        : rollback_(std::move(other.rollback_)),
        dismissed_(other.dismissed_)
    {
        other.dismissed_ = true;
    }

    ScopeGuard::~ScopeGuard()
    {
        if (!dismissed_)
        {
            rollback_();
        }
    }

    void ScopeGuard::swap(ScopeGuard& other)
    {
        std::swap(rollback_, other.rollback_);
        std::swap(dismissed_, other.dismissed_);
    }

    void ScopeGuard::dismiss()
    {
        dismissed_ = true;
    }
}