#ifndef CR_COMMON_SCOPE_GUARD_H_
#define CR_COMMON_SCOPE_GUARD_H_

#include <type_traits>
#include <utility>

namespace cr
{
	/**
	 * �����������Զ��ع�.
	 *
	 * @tparam TRollback �ع�����.
	 */
	template <typename TRollback>
	class ScopeGuard
	{
	public:
		/**
		 * Constructor.
		 *
		 * @param rollback �ع�����.
		 */
		explicit ScopeGuard(TRollback&& rollback)
			: rollback_(std::forward<TRollback>(rollback)),
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


		/** ����Զ��ع�. */
		void dismiss()
		{
			dismissed_ = true;
		}

	private:

		// �ع�����
		TRollback rollback_;
		// �Ƿ�ȡ���Զ����ع�
		bool dismissed_;
	};
}

#define CR_ON_SCOPE_GUARD(varName, rollback)\
	auto CR_MACRO_CAT(lamdbaRollback, __LINE__) = std::move(rollback);\
	 cr::ScopeGuard<std::decay_t<decltype(CR_MACRO_CAT(lamdbaRollback, __LINE__))>> varName(std::move(CR_MACRO_CAT(lamdbaRollback, __LINE__)))

#define CR_ON_SCOPE_EXIT(rollback) CR_ON_SCOPE_GUARD(CR_MACRO_CAT(onScopeExit, __LINE__), rollback)

#endif // !CR_CORE_SCOPE_GUARD_H_