#pragma once

#include <vector>
#include <string>
#include <functional>
#include <memory>
#include <atomic>
#include <type_traits>

namespace fx
{
	template<typename... Args>
	class fwEvent
	{
	public:
		using TFunc = std::function<bool(Args...)>;

	public:
		struct callback
		{
			TFunc function;
			std::unique_ptr<callback> next = nullptr;
			int order = 0;
			size_t cookie = -1;

			callback(TFunc func)
				: function(func)
			{
			}
		};

		std::unique_ptr<callback> m_callbacks;
		std::atomic<size_t> m_connectCookie = 0;
	};


	class Resource
	{
	public:
		char pad_040[0x40];
		fx::fwEvent<std::vector<char>*> OnBeforeLoadScript;
		fx::fwEvent<> OnStart;
		fx::fwEvent<> OnStop;
		fx::fwEvent<> OnEnter;
		fx::fwEvent<> OnLeave;
		fx::fwEvent<> OnCreate;
		fx::fwEvent<> OnActivate;
		fx::fwEvent<> OnDeactivate;
		fx::fwEvent<> OnRemove;
	};


	class ResourceImpl : public Resource
	{
	public:
		std::string m_name;

	};

	class NetLibrary
	{
	public:
		char pad[0x0f8];
		std::string m_currentServerUrl;

	};


	template<typename... Args>
	inline size_t ConnectInternal(fx::fwEvent<Args...>& event, typename fx::fwEvent<Args...>::TFunc func, int order)
	{
		if (!func)
			return -1;

		auto cookie = event.m_connectCookie++;
		auto cb = std::unique_ptr<typename fx::fwEvent<Args...>::callback>(new typename fx::fwEvent<Args...>::callback(func));
		cb->order = order;
		cb->cookie = cookie;

		if (!event.m_callbacks)
		{
			event.m_callbacks = std::move(cb);
		}
		else
		{
			auto cur = &event.m_callbacks;
			typename fx::fwEvent<Args...>::callback* last = nullptr;

			while (*cur && order >= (*cur)->order)
			{
				last = cur->get();
				cur = &(*cur)->next;
			}

			cb->next = std::move(*cur);
			(!last ? event.m_callbacks : last->next) = std::move(cb);
		}

		return cookie;
	}


	template<typename... Args, typename T>
	inline auto Connect(fx::fwEvent<Args...>& event, T func)
	{
		return Connect(event, func, 0);
	}

	template<typename... Args, typename T>
	inline auto Connect(fx::fwEvent<Args...>& event, T func, int order)
	{
		if constexpr (std::is_same_v<std::invoke_result_t<T, Args...>, bool>)
		{
			return ConnectInternal(event, func, order);
		}
		else
		{
			return ConnectInternal(event, [func](Args&&... args)
				{
					std::invoke(func, args...);
			return true;
				},
				order);
		}
	}

}
