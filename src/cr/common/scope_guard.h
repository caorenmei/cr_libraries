#ifndef CR_COMMON_SCOPE_GUARD_H_
#define CR_COMMON_SCOPE_GUARD_H_

#include <functional>
#include <utility>

#include <boost/preprocessor/cat.hpp>

namespace cr
{
	/**
	 * 超出作用域自动回滚.
	 */
	class ScopeGuard
	{
	public:
		/**
		 * Constructor.
		 *
		 * @param rollback 回滚函数.
		 */
		explicit ScopeGuard(std::function<void()> rollback)
			: rollback_(std::move(rollback)),
			dismissed_(false)
		{}

		/**
		 * Move constructor.
		 *
		 * @param [in,out] other The other.
		 */
		ScopeGuard(ScopeGuard&& other)
			: rollback_(std::move(other.rollback_)),
			dismissed_(other.dismissed_)
		{
			other.dismissed_ = true;
		}

		/** Destructor. */
		~ScopeGuard()
		{
			if (!dismissed_)
			{
				rollback_();
			}
		}

        ScopeGuard(const ScopeGuard&) = delete;
        ScopeGuard& operator=(const ScopeGuard&&) = delete;
        ScopeGuard& operator=(ScopeGuard&& other) = delete;

        /**
         * Swaps the given other.
         *
         * @param [in,out]  other   The other.
         */
        void swap(ScopeGuard& other)
        {
            std::swap(rollback_, other.rollback_);
            std::swap(dismissed_, other.dismissed_);
        }

		/** 解除自动回滚. */
		void dismiss()
		{
			dismissed_ = true;
		}

	private:

		// 回滚函数
        std::function<void()> rollback_;
		// 是否取消自动化回滚
		bool dismissed_;
	};
}

#define CR_ON_SCOPE_EXIT(rollback) cr::ScopeGuard BOOST_PP_CAT(onScopeExit_, __LINE__)(rollback)

#endif // !CR_CORE_SCOPE_GUARD_H_