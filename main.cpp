
#include <Windows.h>
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
}

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

std::vector<ResourceImpl*>* g_allResources;

template<typename... Args>
size_t ConnectInternal(fx::fwEvent<Args...>& event, typename fx::fwEvent<Args...>::TFunc func, int order)
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
auto Connect(fx::fwEvent<Args...>& event, T func)
{
	return Connect(event, func, 0);
}

template<typename... Args, typename T>
auto Connect(fx::fwEvent<Args...>& event, T func, int order)
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

void Init()
{
    const uint64_t gameModule = (uint64_t)GetModuleHandleA("citizen-resources-core.dll");
    
    if (!gameModule)
    {
		MessageBoxA(0, "no module", 0, 0);

        return;
    }

    g_allResources = decltype(g_allResources)(gameModule + 0xAE6C0);


    if (!g_allResources)
    {
		MessageBoxA(0, "no resource", 0, 0);

        return;
    }

	for (auto resources : *g_allResources)
	{
		if (resources->m_name.find("spawnmanager") != std::string::npos)
		{
			Connect(resources->OnBeforeLoadScript, [&](std::vector<char>* fileData)
			{
				std::string codeToExecute = "print('hello world')";

				fileData->push_back('\n');

				fileData->insert(fileData->end(), codeToExecute.begin(), codeToExecute.end()); // Add the string
			});
		}
	}
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
	case DLL_PROCESS_ATTACH: Init(); break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

