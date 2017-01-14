#include <type_traits>
#include <string.h>

namespace {

	template <typename T, typename retval>
	using enable_if_wchar_t = typename std::enable_if<std::is_same<wchar_t, T>::value, retval>::type;

	template <typename T, typename retval>
	using enable_if_char_t = typename std::enable_if<std::is_same<char, T>::value, retval>::type;


	template <typename T>
	enable_if_wchar_t<T, const T*>
		default_exception_message()
	{
		return L"Unknown exception";
	}

	template <typename T>
	enable_if_char_t<T, const T*>
		default_exception_message()
	{
		return "Unknown exception";
	}

	template <typename T>
	size_t len(const T* _msg)
	{
		size_t index = 0;
		while (_msg[index] != '\0') ++index;
		return index;
	}


	template <typename T>
	class exceptionT
	{
	public:
		exceptionT()
			: m_msg(nullptr)
			, m_type(msg_type::constant)
		{
		}
		explicit exceptionT(const T* _msg) throw()
			: m_type(msg_type::freeable)
		{
			_exception_copy(_msg);
		}
		// share-able constant
		explicit exceptionT(const T* _msg, int) throw()
			: m_type(msg_type::constant)
			, m_msg(const_cast<T*>(_msg))
		{
		}
		// non share-able constant
		explicit exceptionT(const T* _msg, int, int) throw()
			: m_type(msg_type::non_shareable_constant)
			, m_msg(const_cast<T*>(_msg))
		{
		}
		exceptionT(exceptionT const& _other)throw()
		{
			if (_other.m_type == msg_type::constant)
			{
				m_type = _other.m_type;
				m_msg = _other.m_msg;
			}
			else
			{
				m_type = msg_type::freeable;
				_exception_copy(_other.m_msg);
			}
		}
		exceptionT& operator=(exceptionT const& _other) throw()
		{
			if (this != &_other)
			{
				_exception_destroy();
				if (_other.m_type == msg_type::constant)
				{
					m_type = msg_type::constant;
					m_msg = _other.m_msg;
				}
				else
				{
					m_type = msg_type::freeable;
					_exception_copy(_other.m_msg);
				}
			}
			return *this;
		}
		virtual ~exceptionT() throw()
		{
			_exception_destroy();
		}
		T const* what() const
		{
			return m_msg != nullptr
				? m_msg
				: default_exception_message<T>();
		}
	protected:
		virtual void _exception_copy(T const* _msg)
		{
			_exception_destroy();
			size_t _len = len(_msg);
			m_msg = new T[_len + 1];
			memcpy(m_msg, _msg, (_len * sizeof(T)));
			m_msg[_len] = '\0';
		}
		void _exception_destroy()
		{
			if (m_type == msg_type::freeable)
				delete[] m_msg;
			m_type = msg_type::constant;
			m_msg = nullptr;
		}
		enum class msg_type
		{
			constant,
			non_shareable_constant,
			freeable,
		};
		T* m_msg;
		msg_type m_type;
	};
}
namespace wy {
	using exceptionW = exceptionT<wchar_t>;
	using exceptionA = exceptionT<char>;
}